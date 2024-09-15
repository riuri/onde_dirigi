/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>

/**
 * For SpatiaLite operations, we require both the SQLite database and the
 * SpatiaLite handle.
 */
typedef struct {
  sqlite3 *db;
  void* spatialite_cache;
} SpatiaLite;

/**
 * Opens and initializes the schema given a database file name.
 */
SpatiaLite open_and_init_db(const char filename[]);

/**
 * Issues a transaction, erroring with the description when it fails.
 */
void begin_transaction(sqlite3 *db, const char* description);

/**
 * Writes a transaction, erroring with the description when it fails.
 */
void commit_transaction(sqlite3 *db, const char* description);

/**
 * Closes and deallocates database and spatialite cache.
 */
void close_db(SpatiaLite db);
