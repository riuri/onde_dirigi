/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include "definitions.h"

/* Half the frame is reserved for each side. */
#define FRAME_STRING_LENGTH ((unsigned int)EXPECTED_VIDEO_WIDTH/2/GLYPH_WIDTH)

/**
 * Parsed string information from a single video frame. Those strings are fixed-
 * length, not null-terminated. They are expected to be filled with spaces.
 */
typedef struct {
  char left[FRAME_STRING_LENGTH];
  char right[FRAME_STRING_LENGTH];
} CharLine;
