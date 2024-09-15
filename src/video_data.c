/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <err.h>
#include <stdbool.h>
#include <strings.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include "video_data.h"

/* First frame row that contains string data. */
#define TOP_DATA_ROW 1393

/* Heuristic for when to choose glyph or space. */
#define GLYPH_THRESHOLD 16

/**
 * Takes a single frame and fills the strings found in this frame.
 */
static void fill_line(unsigned int glyph_count, const Glyph glyphs[glyph_count],
  const AVFrame* frame, CharLine *line)
{
  /* Temporary sum storage for final statistics. */
  int16_t left_sum[FRAME_STRING_LENGTH][glyph_count];
  int16_t right_sum[FRAME_STRING_LENGTH][glyph_count];
  bzero(left_sum, FRAME_STRING_LENGTH * glyph_count * sizeof(uint16_t));
  bzero(right_sum, FRAME_STRING_LENGTH * glyph_count * sizeof(uint16_t));

  /* Only care about the data rows. */
  for (unsigned int i = TOP_DATA_ROW; i < TOP_DATA_ROW + GLYPH_HEIGHT; ++i) {
    /* Left. */
    for (unsigned int j = 0; j < FRAME_STRING_LENGTH * GLYPH_WIDTH; ++j) {
      for (unsigned int k = 0; k < glyph_count; ++k) {
        /* k for each glyph
         * ┌─────┬─────┬─────┬─────┐
         * │  j →│     │     │     │
         * │i    │     │     │     │
         * │↓    │     │     │     │
         * └─────┴─────┴─────┴─────┘
         */
        left_sum[j / GLYPH_WIDTH][k] +=
          (frame->data[0][j + i * frame->linesize[0]] - 128)
          * glyphs[k].multiplier[i - TOP_DATA_ROW][j % GLYPH_WIDTH];
      }
    }
    /* Skip the middle. */
    /* Right. */
    for (unsigned int j = 0; j < FRAME_STRING_LENGTH * GLYPH_WIDTH; ++j) {
      for (unsigned int k = 0; k < glyph_count; ++k) {
        /* Same as above but j starts from middle. */
        right_sum[j / GLYPH_WIDTH][k] +=
          (frame->data[0][
              j + EXPECTED_VIDEO_WIDTH - FRAME_STRING_LENGTH * GLYPH_WIDTH
              + i * frame->linesize[0]
            ] - 128)
          * glyphs[k].multiplier[i - TOP_DATA_ROW][j % GLYPH_WIDTH];
      }
    }
  }

  /* Divide all and get the maximum or ' ' (space). */
  for (unsigned int i = 0; i < FRAME_STRING_LENGTH; ++i) {
    /* Left. */
    unsigned int left_max_index = 0;
    double left_max = 0;
    for (unsigned int j = 0; j < glyph_count; ++j) {
      double left = (double)left_sum[i][j] / (double)glyphs[j].divider;
      if (left > left_max) {
        left_max = left;
        left_max_index = j;
      }
    }

    line->left[i] = left_max >= GLYPH_THRESHOLD
      ? glyphs[left_max_index].key
      : ' ';

    /* Right. */
    unsigned int right_max_index = 0;
    double right_max = 0;
    for (unsigned int j = 0; j < glyph_count; ++j) {
      double right = (double)right_sum[i][j] / (double)glyphs[j].divider;
      if (right > right_max) {
        right_max = right;
        right_max_index = j;
      }
    }

    line->right[i] = right_max >= GLYPH_THRESHOLD
      ? glyphs[right_max_index].key
      : ' ';
  }
}

int get_video_strings(const char url[],
  unsigned int glyph_count,
  const Glyph glyphs[glyph_count],
  unsigned int string_count,
  CharLine lines[string_count])
{
  if (string_count == 0) {
    return -1;
  }
  unsigned int filled_lines = 0;

  /* Prepare the container format and get best video decoder. */
  AVFormatContext *fmt_context = NULL;
  AVPacket pkt;
  AVFrame *frame = av_frame_alloc();
  const struct AVCodec *dec;
  AVCodecContext *dec_context;
  if (0 != avformat_open_input(&fmt_context, url, NULL, NULL))
    errx(1, "Failed to open input url %s", url);
  int video_stream = av_find_best_stream(
    fmt_context, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
  if (video_stream < 0)
    errx(1, "Failed to find best video stream");
  dec_context = avcodec_alloc_context3(dec);
  avcodec_parameters_to_context(
    dec_context, fmt_context->streams[video_stream]->codecpar);
  if (0 != avcodec_open2(dec_context, dec, NULL))
    errx(1, "Could not open decoder");

  /* Time is used for frames per second. */
  const AVRational time_base = fmt_context->streams[video_stream]->time_base;
  int64_t second = time_base.den / time_base.num;
  int64_t second_change = second;

  /* First, we read all video frames until detecting when the second’s unit
   * glyph changed. */
  while (0 == av_read_frame(fmt_context, &pkt)) {
    if (pkt.stream_index != video_stream) {
      av_packet_unref(&pkt);
      continue;
    }
    if (0 != avcodec_send_packet(dec_context, &pkt))
      errx(1, "Could not send frame to decoder");
    if (0 != avcodec_receive_frame(dec_context, frame))
      errx(1, "Could not receive frame from decoder");

    if (filled_lines == 0) {
      fill_line(glyph_count, glyphs, frame, &lines[filled_lines]);
      filled_lines++;
    }
    else {
      CharLine tmp;
      fill_line(glyph_count, glyphs, frame, &tmp);
      /* When the temporary line’s last glyph changed, we figured out which
       * frame mod frames/second has the next data point.  */
      if (lines[0].right[FRAME_STRING_LENGTH - 2]
          != tmp.right[FRAME_STRING_LENGTH - 2]) {
        second_change = frame->pts;
        break;
      }
    }
  }

  /* Main routine. */
  while (0 == av_read_frame(fmt_context, &pkt)) {
    if (pkt.stream_index != video_stream) {
      av_packet_unref(&pkt);
      continue;
    }
    /* Wait until the second changes. */
    if (pkt.dts < (filled_lines - 1) * second + second_change) {
      av_packet_unref(&pkt);
      continue;
    }
    /* Can only decode a key frame out of context. */
    if ((pkt.flags & AV_PKT_FLAG_KEY) == 0) {
      av_packet_unref(&pkt);
      continue;
    }
    /* Too few lines allocated or video too long. */
    if (filled_lines >= string_count)
      return -1;

    if (0 != avcodec_send_packet(dec_context, &pkt))
      errx(1, "Could not send frame to decoder");
    if (0 != avcodec_receive_frame(dec_context, frame))
      errx(1, "Could not receive frame from decoder");

    fill_line(glyph_count, glyphs, frame, &lines[filled_lines]);
    filled_lines++;

    av_frame_unref(frame);
    av_packet_unref(&pkt);
  }
  av_frame_free(&frame);
  avcodec_free_context(&dec_context);
  avformat_close_input(&fmt_context);

  return filled_lines;
}
