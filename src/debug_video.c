/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <err.h>
#include <stdio.h>
#include "glyph.h"
#include "video_data.h"
#include "output_data.h"

/**
 * Simple tool for using a debugger on a single video.
 */
int main(int argc, char* argv[]) {
  if (argc != 2) {
    errx(1, "Got %d arguments, expected 1 (video)", argc - 1);
  }
  const char keys[] = "0123456789_";
  Glyph glyphs[sizeof(keys) - 1];
  load_glyphs("../data/glyphs.png", sizeof(keys) - 1, keys, glyphs);
  CharLine lines[301];
  int read_lines = get_video_strings(argv[1],
    sizeof(keys) - 1, glyphs,
    sizeof(lines)/sizeof(CharLine), lines);
  if (read_lines <= 0)
    errx(1, "Got %d lines", read_lines);
  if (lines_ok(argv[1], read_lines, lines))
    puts("Lines OK");
  else
    puts("Lines not OK");
  return 0;
}
