/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <err.h>
#include <stdio.h>
#include "glyph.h"
#include "video_data.h"
#include "output_data.h"
#include "ls.h"

/**
 * parse_directory: finds all the videos in the directory that were not imported
 * to the database, parses them, and if returned lines are sound, imports the
 * coordinates and timestamps.
 */
int main(int argc, char* argv[]) {
  if (argc != 3) {
    errx(1,
      "Got %d arguments, expected 2 (video directory and database)", argc - 1);
  }
  const char keys[] = "0123456789_";
  Glyph glyphs[sizeof(keys) - 1];
  load_glyphs("../data/glyphs.png", sizeof(keys) - 1, keys, glyphs);

  struct dirent **list;
  int n = list_to_import(argv[1], argv[2], &list);
  for (int i = 0; i < n; ++i) {
    /* Convert list item to FFmpeg URL. */
    char* video_url =
      malloc(strlen(argv[1]) + strlen(list[i]->d_name) + sizeof("file:/"));
    strcpy(video_url, "file:");
    strcat(video_url, argv[1]);
    strcat(video_url, "/");
    strcat(video_url, list[i]->d_name);
    printf("Reading file “%s”\n", video_url);
    free(list[i]);

    /* Get string lines from the video. */
    CharLine lines[301];
    int read_lines = get_video_strings(video_url,
      sizeof(keys) - 1, glyphs,
      sizeof(lines)/sizeof(CharLine), lines);
    if (read_lines <= 0)
      errx(1, "Got %d lines", read_lines);

    /* Write lines to database when they are valid. */
    if (lines_ok(video_url, read_lines, lines))
      append_lines(video_url, read_lines, lines, argv[2]);
    else
      printf("Lines are not OK\n");

    free(video_url);
  }
  free(list);
  return 0;
}
