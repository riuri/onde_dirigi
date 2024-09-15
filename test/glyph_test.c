/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include "glyph.h"

/* Tests the glyph loader function. The test is done by comparing the generated
 * glyph’s elements with a manually written reference value. The reference value
 * is the glyph 8.
 */

#define EMPTY_ROW {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
signed char eight[GLYPH_HEIGHT][GLYPH_WIDTH] = {
  EMPTY_ROW,
  EMPTY_ROW,
  EMPTY_ROW,
  {0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0,-1,-1,-1, 1, 1, 1, 1,-1,-1,-1, 0, 0, 0},
  {0, 0, 0, 0,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1, 0, 0},
  {0, 0, 0, 0,-1, 1, 1, 1,-1,-1,-1,-1, 1, 1, 1,-1,-1, 0},
  {0, 0, 0,-1,-1, 1, 1,-1,-1, 0, 0,-1,-1, 1, 1, 1,-1, 0},
  {0, 0, 0,-1, 1, 1, 1,-1, 0, 0, 0, 0,-1, 1, 1, 1,-1, 0},
  {0, 0, 0,-1, 1, 1, 1,-1, 0, 0, 0, 0,-1, 1, 1, 1,-1, 0},
  {0, 0, 0,-1, 1, 1, 1,-1, 0, 0, 0, 0,-1, 1, 1, 1,-1, 0},
  {0, 0, 0,-1, 1, 1, 1,-1, 0, 0, 0, 0,-1, 1, 1, 1,-1, 0},
  {0, 0, 0,-1, 1, 1, 1,-1,-1, 0, 0,-1,-1, 1, 1, 1,-1, 0},
  {0, 0, 0,-1,-1, 1, 1, 1,-1,-1,-1,-1, 1, 1, 1,-1,-1, 0},
  {0, 0, 0, 0,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1, 0, 0},
  {0, 0, 0, 0,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1, 0, 0},
  {0, 0, 0,-1,-1, 1, 1, 1,-1,-1,-1,-1, 1, 1, 1,-1,-1, 0},
  {0, 0,-1,-1, 1, 1, 1,-1,-1, 0, 0,-1,-1, 1, 1, 1,-1,-1},
  {0, 0,-1, 1, 1, 1,-1,-1, 0, 0, 0, 0,-1,-1, 1, 1, 1,-1},
  {0, 0,-1, 1, 1, 1,-1, 0, 0, 0, 0, 0, 0,-1, 1, 1, 1,-1},
  {0, 0,-1, 1, 1, 1,-1, 0, 0, 0, 0, 0, 0,-1, 1, 1, 1,-1},
  {0, 0,-1, 1, 1, 1,-1, 0, 0, 0, 0, 0, 0,-1, 1, 1, 1,-1},
  {0, 0,-1, 1, 1, 1,-1,-1, 0, 0, 0, 0,-1,-1, 1, 1, 1,-1},
  {0, 0,-1,-1, 1, 1, 1,-1,-1, 0, 0,-1,-1, 1, 1, 1,-1,-1},
  {0, 0, 0,-1, 1, 1, 1, 1,-1,-1,-1,-1, 1, 1, 1, 1,-1, 0},
  {0, 0, 0,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1, 0},
  {0, 0, 0, 0,-1,-1,-1, 1, 1, 1, 1, 1, 1,-1,-1,-1, 0, 0},
  {0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0, 0},
  EMPTY_ROW,
  EMPTY_ROW,
};

int main(void)
{
  puts("1..1");

  // Arrange
  const char keys[] = "0123456789H";
  static_assert(sizeof(keys) - 1 >= 11, "Not enough glyph keys");
  const size_t glyph_count = sizeof(keys) - 1;

  Glyph glyphs[glyph_count];

  // Act
  load_glyphs("file:../data/glyphs.png", glyph_count, keys, glyphs);

  // Assert
  bool has_key = false;
  for (size_t i = 0; i < glyph_count; ++i) {
    if (glyphs[i].key == '8') {
      assert(!has_key); // Key shouldn’t exist twice
      has_key = true;

      for (int j = 0; j < GLYPH_HEIGHT; ++j) {
        for (int k = 0; k < GLYPH_WIDTH; ++k) {
          assert(glyphs[i].multiplier[j][k] == eight[j][k]);
        }
      }
      assert(glyphs[i].divider == 287);
    }
  }
  assert(has_key);

  puts("ok 1 - glyph 8 tested");
  return 0;
}
