#pragma once
#include "char_line_fill.h"

// The name of the sample video
#define VIDEO_FILENAME "20231225103211_006981.TS"
// The duration of the video + 1
#define TEST_VIDEO_SECONDS_PLUS_1 301

// Lines include the following, in order:
// * Speed (variable length, 1 to 3 digits)
// * Latitude
// * Longitude
// * Spaces
// * Date and time
// The specifics depend on the dash cam settings.
const CharLine expected_lines[] = {
  {" 40 __ _ _28 215366 _71 119330 " FILL "25 12 2023 10 34 21 "},
  {" 39 __ _ _28 215744 _71 118590  " FILL "25 12 2023 10 34 25 "},
  {" 38 __ _ _28 215990 _71 118584  " FILL "25 12 2023 10 34 26 "},
  {" 27 __ _ _28 216279 _71 118756  " FILL "25 12 2023 10 34 27 "},
  {" 10 __ _ _28 216539 _71 119115  " FILL "25 12 2023 10 34 28 "},
  {" 0 __ _ _28 217045 _71 118520  " FILL "25 12 2023 10 34 29 "},
// The lines need to be in timestamp order but may skip some.
  {" 2 __ _ _28 218307 _71 118697  " FILL "25 12 2023 10 34 40 "},
  {" 11 __ _ _28 218911 _71 118536  " FILL "25 12 2023 10 34 42 "},
};

