/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <assert.h>
#include "char_line.h"

/* The code expects lines to be strings of fixed size (depending on the frame
 * size). Excluding data, they are filled with spaces. The fills are defined to
 * not include all the spaces in the test data.
 */
#define LEFT_FILL "                                        "
#define RIGHT_FILL "                                                   "
// The last character will be set to ' ', not '\0'

// Check that the fills have the correct amount of spaces.
static_assert(sizeof("xxx KM/H Nyy.yyyyyy Wzz.zzzzzz" LEFT_FILL)
  == FRAME_STRING_LENGTH, "Please check LEFT_FILL’s length");
static_assert(sizeof(RIGHT_FILL "dd/mm/yyyy HH:MM:SS")
  == FRAME_STRING_LENGTH, "Please check RIGHT_FILL’s length");

#define FILL LEFT_FILL, RIGHT_FILL
