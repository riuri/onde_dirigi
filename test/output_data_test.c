/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "char_line_fill.h"
#include "my_assert.h"
#include "output_data.h"

/* The glyph recognition is expected to return some bad strings, for example, by
 * confusing 1 and 4. The function lines_ok will try to identify gross errors so
 * we don’t write them into the database. We also verify that we can write good
 * lines into an in-memory database.
 */

/* Size arguments for arrays known at compile time. */
#define cl(char_line) (sizeof(char_line)/sizeof(CharLine)), char_line

/* Points too far away for the time difference indicate an error in the GPS
 * signal (hopefully recoverable) or reading the coordinates from the video.
 */
static void test_adjacent_lines_speed(void) {
  const int test_case = 1;
  // Arrange
  const CharLine far_away[] = {
    {" 2 __ _ _27 953078 _72 867111  " FILL "29 08 2024 18 27 54 "},
    {" 2 __ _ _27 952928 _72 864032  " FILL "29 08 2024 18 27 55 "},
  };
  // Act, Assert
  my_assert(!lines_ok("20240829182753_005283.TS", cl(far_away)));
  ok();
}

/* Going back in time is indicates an error reading the timestamp. */
static void test_lines_time_ascending(void) {
  const int test_case = 2;
  // Arrange
  const CharLine out_of_order[] = {
    {" 53 __ _ _26 433178 _71 608382 " FILL "30 08 2024 20 46 22 "},
    {" 56 __ _ _26 433342 _71 607943 " FILL "30 08 2024 20 46 19 "},
  };
  // Act, Assert
  my_assert(!lines_ok("20240830204539_003921.TS", cl(out_of_order)));
  ok();
}

/* This is an example of good coordinate sequence, not considering the video
 * timestamp.
 */
const CharLine good[] = {
  {" 100 __ _ _26 434600 _71 608715" FILL "31 08 2024 09 02 20 "},
  {" 100 __ _ _26 435139 _71 607867" FILL "31 08 2024 09 02 21 "},
  {" 100 __ _ _26 435398 _71 607116" FILL "31 08 2024 09 02 22 "},
  {" 96 __ _ _26 436091 _71 607127 " FILL "31 08 2024 09 02 23 "},
  /* Time skipped should be OK  */
  {" 95 __ _ _26 435985 _71 612824 " FILL "31 08 2024 09 02 27 "},
  {" 94 __ _ _26 436418 _71 613457 " FILL "31 08 2024 09 02 28 "},
  {" 97 __ _ _26 436735 _71 613489 " FILL "31 08 2024 09 02 29 "},
  {" 80 __ _ _26 436120 _71 614090 " FILL "31 08 2024 09 02 30 "},
  {" 8 __ _ _26 436033 _71 614916  " FILL "31 08 2024 09 02 31 "},
  {" 7 __ _ _26 435464 _71 614991  " FILL "31 08 2024 09 02 32 "},
};

/* Good line timestamps should be close to the timestamp referred by the video
 * filename.
 */
static void test_lines_time_close_to_filename(void) {
  const int test_case = 3;
  // Act, Assert
  my_assert(!lines_ok("file:RO/20240831085050_004709.TS", cl(good)));
  ok();
}

/* Timestamps should still be close during time changes. */
static void test_lines_time_close_to_filename_time_change(void) {
  const int test_case = 4;
  // Arrange
  setenv("TZ", "America/Winnipeg", 1);
  tzset();
  // Check timezone where this change is expected
  my_assert(strcmp("CST", tzname[0]) == 0);
  my_assert(strcmp("CDT", tzname[1]) == 0);
  const CharLine good_time[] = {
    {" 20 __ _ _26 436657 _71 613103 " FILL "09 03 2025 01 59 59 "},
    {" 23 __ _ _26 436493 _71 612588 " FILL "09 03 2025 03 00 00 "},
    {" 32 __ _ _26 435878 _71 612760 " FILL "09 03 2025 03 00 01 "},
  };
  // Act, Assert
  my_assert(lines_ok("20250309015835_002648.TS", cl(good_time)));
  ok();
}

/* Good line timestamps shouldn’t be much before the video filename. */
static void test_lines_time_future_to_filename(void) {
  const int test_case = 5;
  // Act, Assert
  my_assert(!lines_ok("20240831090300_004709.TS", cl(good)));
  ok();
}

/* The video filename timestamp should be parsed regardless of prefixes. */
static void test_lines_ok(void) {
  /* Happy path. */
  const int test_case = 6;
  // Act, Assert
  my_assert(lines_ok("file:../data/20240831090220_004709.TS", cl(good)));
  my_assert(lines_ok("file:RO/20240831090220_004709.TS", cl(good)));
  my_assert(lines_ok("20240831090220_004709.TS", cl(good)));
  /* Video can be a bit after initial frame timestamp. */
  my_assert(lines_ok("20240831090221_004710.TS", cl(good)));
  ok();
}

/* Some recoverable errors were found during normal videos. */
static void test_weird_lines_still_ok(void) {
  const int test_case = 7;
  // Arrange
  const CharLine still_ok[] = {
    /* The frame timestamp may start a bit before the video name timestamp. */
    {" 20 __ _ _26 439529 _71 590980 " FILL "06 09 2024 11 23 33 "},
    /* Duplicate lines means the video parser didn’t skip a full second. */
    {" 20 __ _ _26 439529 _71 590980 " FILL "06 09 2024 11 23 33 "},
    /* Ignore whatever is found before the coordinate. Even a digit. */
    {" 20 __ _ _26 439788 171 591485 " FILL "06 09 2024 11 23 34 "},
    /* Some bogus glyphs may be erroneously found in the space area. */
    {" 20 __ _ _26 440307 _71 591957 " LEFT_FILL,
      "                                       1           06 09 2024 11 23 35 "
      },
    {" 20 __ _ _26 439807 _71 591485            1                            ",
      RIGHT_FILL "06 09 2024 11 23 36 "},
    /* Blank lines in the left side means GPS is not yet ready. */
    {"                               " FILL "06 09 2024 11 23 37 "},
    /* Zero coordinates mean GPS is not yet ready. */
    {" 0 __ _ 00 000000 00 000000    " FILL "06 09 2024 11 23 38 "},
    {" 20 __ _ _26 439769 _71 587107 " FILL "06 09 2024 11 23 39 "},
    /* Long skip of 5 minutes. */
    {" 20 __ _ _26 443390 _71 575713 " FILL "06 09 2024 11 28 39 "},
  };
  // Act, Assert
  my_assert(lines_ok("20240906112334_002482.TS", cl(still_ok)));
  ok();
}

/* Just make sure nothing breaks when writing the spatialite DB. */
static void test_smoke_test_append_lines(void) {
  const int test_case = 8;
  // Act, Assert
  append_lines("20240831.TS", cl(good), ":memory:");
  ok();
}

int main(void) {
  puts("TAP version 14");
  puts("1..8");
  test_adjacent_lines_speed();
  test_lines_time_ascending();
  test_lines_time_close_to_filename();
  test_lines_time_close_to_filename_time_change();
  test_lines_time_future_to_filename();
  test_lines_ok();
  test_weird_lines_still_ok();
  test_smoke_test_append_lines();
  return 0;
}
