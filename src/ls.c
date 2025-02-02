/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <err.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "db.h"
#include "ls.h"

#define RO_SUFFIX "/RO"
#define RO_PREFIX "RO/"

/* Passed to the dir_filter function. */
sqlite3_stmt *stmt;

/**
 * Filtering function for scandir. Returns true for video filenamess that aren’t
 * present in the imported table.
 */
static int dir_filter(const struct dirent* entry) {
  /* Ignore hidden files. */
  if (entry->d_name[0] == '.')
    return 0;

  /* Ignore anything that doesn’t end with “.TS”. */
  const char *ext = strrchr(entry->d_name, '.');
  if (ext == NULL || strcmp(ext, ".TS") != 0)
    return 0;

  /* Check if the filename is in the database. */
  const char *basename = strrchr(entry->d_name, '/');
  if (basename == NULL)
    basename = entry->d_name;
  if (SQLITE_OK != sqlite3_bind_text(stmt, 1, basename, strlen(basename),
      SQLITE_TRANSIENT))
    errx(1, "Error binding to filename verification statement");
  if (SQLITE_ROW != sqlite3_step(stmt))
    errx(1, "Error while stepping filename verification statement");
  int filename_imported = sqlite3_column_int(stmt, 0);
  if (SQLITE_OK != sqlite3_clear_bindings(stmt))
    errx(1, "Error while clearing filename verification statement bindings");
  if (SQLITE_OK != sqlite3_reset(stmt))
    errx(1, "Error while resetting filename verification statement");
  return !filename_imported;
}

int list_to_import(const char directory_name[], const char database[],
  struct dirent *** restrict list) {
  /* Invalid directory: nothing imported. */
  if (strlen(directory_name) == 0)
    return 0;

  /* Open database and prepare long-running statement. */
  SpatiaLite sp = open_and_init_db(database);
  const char find_file[] = "SELECT COUNT(*) FROM imported WHERE filename = ?;";
  if (SQLITE_OK != sqlite3_prepare_v2(sp.db, find_file, sizeof(find_file),
      &stmt, NULL))
    errx(1, "Error preparing filename verification statement");

  /* Find files in main directory. */
  int main_ret = scandir(directory_name, list, dir_filter, alphasort);

  /* Find files in RO directory. */
  struct dirent **ro_list;
  char *ro_directory_name = malloc(strlen(directory_name) + sizeof(RO_SUFFIX));
  strcpy(ro_directory_name, directory_name);
  strcat(ro_directory_name, RO_SUFFIX);
  int ro_ret = scandir(ro_directory_name, &ro_list, dir_filter, alphasort);
  free(ro_directory_name);

  /* Append files in the RO directory only if it exists. */
  if (ro_ret >= 0) {
    *list = reallocarray(*list, main_ret + ro_ret, sizeof(struct dirent*));
    for (int i = 0; i < ro_ret; ++i) {
      (*list)[main_ret + i] = ro_list[i];
      /* Need to prefix the “RO/” to filenames found there. */
      char* tmp = strdup(ro_list[i]->d_name);
      strcpy(ro_list[i]->d_name, RO_PREFIX);
      /* The names generated by the dash cam are not close to sizeof(d_name). */
      strncat(ro_list[i]->d_name,
        tmp,
        sizeof(ro_list[i]->d_name) - sizeof(RO_PREFIX) + 1);
      free(tmp);
    }
    free(ro_list);
    main_ret += ro_ret;
  }

  /* Cleanup everything before returning. */
  if (SQLITE_OK != sqlite3_finalize(stmt))
    errx(1, "Could not finalize filename verification statement");
  close_db(sp);
  return main_ret;
}
