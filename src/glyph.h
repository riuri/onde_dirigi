/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include "definitions.h"

/**
 * Contains all the relevant glyph information to be used in the video parser.
 * .key: character corresponding to a glyph (can be a replacement).
 * .multiplier: multiplies the corresponding pixel and adds to a sum.
 * .divider: used to fairly match glyphs with low relative area.
 */
typedef struct {
  char key;
  signed char multiplier[GLYPH_HEIGHT][GLYPH_WIDTH];
  unsigned short int divider;
} Glyph;

/**
 * Loads count glyphs from a single frame in the url. There should be count keys
 * and the Glyph array should be allocated for count elements.
 */
void load_glyphs(const char url[], unsigned int count, const char keys[count],
  Glyph output[count]);
