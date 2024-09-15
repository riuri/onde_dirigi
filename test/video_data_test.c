/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <stdbool.h>
#include <strings.h>
#include "glyph.h"
#include "my_assert.h"
#include "video_data.h"

/* Tests the video to string logic. Most tests here require an actual dashboard
 * camera recording, which usually contains private data. The tests are skipped
 * when this data is not present. A template directory is provided under
 * test/data/private.template.
 */

/* This is defined by the meson build system conditionally on the presence of
 * the private data header.
 */
#if HAS_PRIVATE_DATA
#include "char_line_fill.h"
#include "data/private/header.h"
#endif

/* The glyphs are the same throughout the tests, thus global. */
const char keys[] = "0123456789_";
#define GLYPH_COUNT (sizeof(keys) - 1)
Glyph glyphs[GLYPH_COUNT];

#if HAS_PRIVATE_DATA
/* Only used on tests that require private data. */
static bool valid_character(char x)
{
  return x != '\0' && (strchr(keys, x) != NULL || x == ' ');
}
#endif

/* Happy path for checking expected lines. */
static void test_expected(void) {
  const int test_case = 1;
#if HAS_PRIVATE_DATA
  // Arrange
  const int specified_count = sizeof(expected_lines)/sizeof(CharLine);
  CharLine lines[TEST_VIDEO_SECONDS_PLUS_1];
  /* Make sure it’s initialized to invalid characters. */
  bzero(lines, sizeof(lines));

  // Act
  int ret = get_video_strings("file:../test/data/private/" VIDEO_FILENAME,
    GLYPH_COUNT, glyphs,
    TEST_VIDEO_SECONDS_PLUS_1, lines);

  // Assert
  my_assert(ret + 1 == TEST_VIDEO_SECONDS_PLUS_1);
  /* All characters are in keys or ' '. */
  for (int i = 0; i < ret; ++i) {
    for (unsigned int j = 0; j < FRAME_STRING_LENGTH; ++j) {
      my_assert(valid_character(lines[i].left[j]));
      my_assert(valid_character(lines[i].right[j]));
    }
  }
  /* All expected lines are present and in order. */
  int line_to_check = 0;
  for (int i = 0; i < ret; ++i) {
    if (memcmp(lines[i].left, expected_lines[line_to_check].left,
        FRAME_STRING_LENGTH) == 0
      && memcmp(lines[i].right, expected_lines[line_to_check].right,
        FRAME_STRING_LENGTH) == 0) {
      line_to_check++;
    }
  }
  my_assert(line_to_check == specified_count);

  ok();
#else
  skip("Missing private data");
#endif
}

/* Video parsing should fail if too few lines are allocated. */
static void test_too_few_lines(void)
{
  const int test_case = 2;
#if HAS_PRIVATE_DATA
  // Arrange
  CharLine lines[10];
  static_assert(TEST_VIDEO_SECONDS_PLUS_1 > sizeof(lines) / sizeof(CharLine),
    "To test this case, the number of lines should be too small for the video");

  // Act
  int ret = get_video_strings("file:../test/data/private/" VIDEO_FILENAME,
    GLYPH_COUNT, glyphs,
    sizeof(lines) / sizeof(CharLine), lines);

  // Assert
  my_assert(ret < 0);

  ok();
#else
  skip("Missing private data");
#endif
}

/* Video parsing should not try to read the pointer (SEGFAULT) and return error
 * if no lines are allocated.
 */
static void test_zero_lines(void)
{
  const int test_case = 3;
  // Act
  int ret = get_video_strings("file:this_should_not_be_open.TS",
    GLYPH_COUNT, glyphs,
    0, NULL);

  // Assert
  my_assert(ret < 0);
  ok();
}

int main(void)
{
  puts("1..3");
  /* Globally initialize glyphs for all tests. */
  load_glyphs("file:../data/glyphs.png", GLYPH_COUNT, keys, glyphs);

  test_expected();
  test_too_few_lines();
  test_zero_lines();
  return 0;
}
