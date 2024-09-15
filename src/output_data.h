/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <stdbool.h>
#include "char_line.h"

/**
 * Checks whether lines make sense for a single input video. Namely they should
 * be within the video’s name timestamp range and the points should be
 * reasonably close together.
 */
bool lines_ok(const char video_name[],
  unsigned int count, const CharLine lines[count]);

/**
 * Write lines to a database. Creates the database if non-existent. Appends the
 * video filename when imported.
 */
void append_lines(const char video_name[],
  unsigned int count, const CharLine lines[count], const char database_name[]);
