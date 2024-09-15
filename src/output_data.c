/* SPDX-License-Identifier: GPL-2.0-or-later */
#define _XOPEN_SOURCE
#include <assert.h>
#include <err.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>
#include "db.h"
#include "output_data.h"

/**
 * Gets the timestamp based on the video filename.
 */
static time_t video_start_time(const char video_name[]) {
  struct tm time;
  char name_date[] = "YYYYMMDDhhmmss";
  strncpy(name_date, &video_name[strlen(video_name) - 24],
    sizeof(name_date) - 1);
  if (0 == strptime(name_date, "%Y%m%d%H%M%S", &time))
    errx(1, "Unable to read video filename time");
  time.tm_isdst = -1;
  return mktime(&time);
}

/**
 * Tries to get the timestamp from the right line.
 */
static time_t line_time(const CharLine line) {
  struct tm time;
  char line_right[20];
  strncpy(line_right, line.right + FRAME_STRING_LENGTH - 20, 19);
  line_right[sizeof(line_right) - 1] = '\0';
  if (0 == strptime(line_right, "%d %m %Y %H %M %S", &time)) {
    return 0;
  }
  time.tm_isdst = -1;
  return mktime(&time);
}

/**
 * Coordinates and whether they’re valid.
 */
typedef struct {
  bool valid;
  double lon;
  double lat;
} SimplePoint;

/**
 * Tries to get and validate the coordinates from the left line.
 */
SimplePoint simple_point_from_char_line(const CharLine line) {
  SimplePoint ret = {
    .valid = false,
    .lon = 0,
    .lat = 0,
  };
  int lat, lat_mantissa, lon, lon_mantissa;
  /* Some operations below require a null-terminated string */
  char line_left[FRAME_STRING_LENGTH + 1];
  strncpy(line_left, line.left, FRAME_STRING_LENGTH);
  line_left[FRAME_STRING_LENGTH] = '\0';
  /* Empty line: invalid point (GPS not ready) */
  if (strcmp(line_left, "                                                      "
      "                 ") == 0) {
    return ret;
  }
  if (4 != sscanf(line_left, " %*3d%*7c%2d %6d%*2c%3d %6d",
    &lat, &lat_mantissa, &lon, &lon_mantissa)) {
    warnx("Invalid left line “%.*s” “%.*s”",
      FRAME_STRING_LENGTH, line.left,
      FRAME_STRING_LENGTH, line.right);
    return ret;
  }
  /* When GPS is not ready it may show all 0s. */
  ret.valid = lon || lon_mantissa || lat || lat_mantissa;
  /* Expect North-West wedge. */
  ret.lon = -((double)lon + (double)lon_mantissa * 1e-6l);
  ret.lat = (double)lat + (double)lat_mantissa * 1e-6l;
  return ret;
}

bool lines_ok(const char video_name[],
  unsigned int count, const CharLine lines[count]) {
  /* Initialize “Great Circle” ellipsoid constants from GRS80. */
  const double ellipse_a = 6378137;
  const double ellipse_b = 6356752.3141403561458;

  const time_t video_time = video_start_time(video_name);
  time_t previous_time = video_time;
  SimplePoint previous_point = { .valid = false };
  for (unsigned int i = 0; i < count; ++i) {
    time_t this_time = line_time(lines[i]);
    SimplePoint this_point = simple_point_from_char_line(lines[i]);
    double time_difference = difftime(this_time, previous_time);

    /* Video files are created every 5 minutes. Times much further or going back
     * are probably wrong. */
    if (difftime(this_time, video_time) > 330
      || time_difference <= -2)
      return false;

    if (this_point.valid) {
      if (previous_point.valid) {
        double distance = gaiaGreatCircleDistance(
          ellipse_a, ellipse_b,
          previous_point.lat, previous_point.lon,
          this_point.lat, this_point.lon);
        /* More than 200 m/s is probably wrong coordinates. */
        if (time_difference >= 1 && distance > 200 * time_difference)
          return false;
      }
      previous_time = this_time;
      previous_point = this_point;
    }
  }
  return true;
}

void append_lines(const char video_name[], unsigned int count,
  const CharLine lines[count], const char database_name[]) {
  SpatiaLite sp = open_and_init_db(database_name);
  sqlite3* db = sp.db;
  sqlite3_stmt *stmt;

  begin_transaction(db, "data");

  /* Add locations and timestamps to database. */
  const char insert[] =
    "INSERT OR REPLACE INTO locations(timestamp, place) VALUES (?, ?);";
  if (SQLITE_OK != sqlite3_prepare_v2(db, insert, sizeof(insert), &stmt, NULL))
    errx(1, "Could not prepare insert statement");
  for (unsigned int i = 0; i < count; ++i) {
    SimplePoint simple = simple_point_from_char_line(lines[i]);
    if (!simple.valid) continue;
    unsigned char *wkb;
    int wkb_size;
    gaiaGeomCollPtr geo = gaiaAllocGeomColl();
    if (geo == NULL)
      errx(1, "Could not allocate geometry collection");
    geo->Srid = 4326;
    gaiaAddPointToGeomColl(geo, simple.lon, simple.lat);
    gaiaToSpatiaLiteBlobWkb(geo, &wkb, &wkb_size);
    gaiaFreeGeomColl(geo);
    if (SQLITE_OK != sqlite3_reset(stmt))
      errx(1, "Could not reset insert statement");
    sqlite3_clear_bindings(stmt);
    static_assert(sizeof(time_t) == sizeof(int64_t),
      "time_t should be compatible with int64_t");
    if (SQLITE_OK != sqlite3_bind_int64(stmt, 1, line_time(lines[i])))
      errx(1, "Could not bind timestamp");
    // Also frees the blob allocated for the wkb
    if (SQLITE_OK != sqlite3_bind_blob(stmt, 2, wkb, wkb_size, free))
      errx(1, "Could not bind location");
    if (SQLITE_DONE != sqlite3_step(stmt))
      errx(1, "Statement is not done after step");
  }
  if (SQLITE_OK != sqlite3_finalize(stmt))
    errx(1, "Could not finalize insertions");

  /* Record which files were imported. */
  const char record_file[] =
    "INSERT OR REPLACE INTO imported(filename) VALUES (?);";
  if (SQLITE_OK !=
      sqlite3_prepare_v2(db, record_file, sizeof(record_file), &stmt, NULL))
    errx(1, "Could not prepare record file statement");
  char video_record_name[] = "YYYYMMDDhhmmss_xxxxxx.TS";
  strncpy(video_record_name, &video_name[strlen(video_name) - 24],
    sizeof(video_record_name) - 1);
  if (SQLITE_OK != sqlite3_bind_text(stmt, 1,
      video_record_name, strlen(video_record_name), SQLITE_STATIC))
    errx(1, "Could not bind file name");
  if (SQLITE_DONE != sqlite3_step(stmt))
    errx(1, "Could not perform file name insertion");
  if (SQLITE_OK != sqlite3_finalize(stmt))
    errx(1, "Could not finalize file name insertion");

  commit_transaction(db, "data");
  close_db(sp);
}
