/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <err.h>
#include <stdbool.h>
#include <sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>
#include "db.h"

/* Abstracts SpatiaLite database operations. Any errors/unexpected return values
 * trigger errx().
 */

/**
 * SQLite user_version used for schema migrations.
 */
static int get_db_version(sqlite3* db) {
  const char query[] = "PRAGMA user_version;";
  sqlite3_stmt *stmt;
  if (SQLITE_OK != sqlite3_prepare_v2(db, query, sizeof(query), &stmt, NULL))
    errx(1, "Could not prepare statement for query “%s”", query);
  if (SQLITE_ROW != sqlite3_step(stmt))
    errx(1, "Could not step statement for query “%s”", query);
  if (1 > sqlite3_column_count(stmt))
    errx(1, "Query “%s” did not yield 1 column", query);
  int ret = sqlite3_column_int(stmt, 0);
  if (SQLITE_OK != sqlite3_finalize(stmt))
    errx(1, "Query “%s” did not finalize", query);
  return ret;
}

static void db_no_param(sqlite3 *db, const char* sql) {
  if (SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, NULL))
    errx(1, "Could not execute query “%s”", sql);
}

void begin_transaction(sqlite3 *db, const char* description) {
  if (SQLITE_OK != sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL))
    errx(1, "Could not begin %s transaction", description);
}

void commit_transaction(sqlite3 *db, const char* description) {
  if (SQLITE_OK != sqlite3_exec(db, "COMMIT TRANSACTION;", NULL, NULL, NULL))
    errx(1, "Could not commit %s transaction", description);
}

SpatiaLite open_and_init_db(const char filename[]) {
  sqlite3 *db;
  void* spatialite;

  /* Initialize db and SpatiaLite. */
  if (SQLITE_OK != sqlite3_open_v2(filename, &db,
    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
    errx(1, "Unable to open database “%s”", filename);
  spatialite = spatialite_alloc_connection();
  spatialite_init_ex(db, spatialite, false);

  /* Initializes the SpatiaLite schema/triggers. */
  db_no_param(db, "PRAGMA trusted_schema = 1;");
  begin_transaction(db, "user_version");
  switch (get_db_version(db)) {
    default:
    if (SQLITE_OK != sqlite3_exec(db,
        "CREATE TABLE locations ("
        "  timestamp INTEGER PRIMARY KEY"
        ");"
        "SELECT InitSpatialMetadata();"
        "SELECT AddGeometryColumn("
        "  'locations', 'place', 4326, 'POINT', 'XY', 1);"
        "SELECT CreateSpatialIndex('locations', 'place');"
        "PRAGMA user_version = 1;",
        NULL, NULL, NULL))
      errx(1, "Could not update to version 1");
    __attribute__((fallthrough));
    case 1:
    if (SQLITE_OK != sqlite3_exec(db,
        "CREATE TABLE imported ("
        "  filename STRING PRIMARY KEY"
        ");"
        "PRAGMA user_version = 2;",
        NULL, NULL, NULL))
      errx(1, "Could not update to version 2");
    __attribute__((fallthrough));
    case 2:
      break;
  }
  commit_transaction(db, "user_version");

  /* Returns handles. */
  SpatiaLite ret = {
    db,
    spatialite,
  };
  return ret;
}

void close_db(SpatiaLite sp) {
  if (SQLITE_OK != sqlite3_close(sp.db))
    errx(1, "Could not close database");
  spatialite_cleanup_ex(sp.spatialite_cache);
}
