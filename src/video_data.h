/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <libavutil/frame.h>
#include "glyph.h"
#include "char_line.h"

/**
 * Takes a video and the glyph definitions and finds the strings in the video.
 * Non-matching slots are set to character ' '. Returns the number of lines read
 * or negative in case of error.
 */
int get_video_strings(const char url[],
  unsigned int glyph_count,
  const Glyph glyphs[glyph_count],
  unsigned int string_count,
  CharLine lines[string_count]);
