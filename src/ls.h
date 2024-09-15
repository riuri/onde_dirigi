/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <dirent.h>

/**
 * Lists .TS files on directory and directory/RO but exclude names already
 * present on the imported table of the database.
 */
int list_to_import(const char directory[], const char database[],
  struct dirent *** restrict list);
