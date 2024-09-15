/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <assert.h>
#include <err.h>
#include "glyph.h"

/**
 * Using ffmpeg libraries for the PNG file because they are already a
 * dependency.
 */
void load_glyphs(const char* url, unsigned int count, const char keys[count],
  Glyph output[count])
{
  AVFormatContext *fmt_context = NULL;
  AVPacket pkt;

  /* Open format and codec for PNG. */
  const struct AVCodec *dec = avcodec_find_decoder(AV_CODEC_ID_PNG);
  if (0 != avformat_open_input(&fmt_context, url, NULL, NULL))
    errx(1, "Could not open input url “%s”", url);
  AVCodecContext *dec_context = avcodec_alloc_context3(dec);
  if (0 != avcodec_open2(dec_context, dec, NULL))
    errx(1, "Could not open codec");

  /* Expected a single frame. */
  if (0 != av_read_frame(fmt_context, &pkt))
    errx(1, "Could not read frame to packet");
  if (0 != avcodec_send_packet(dec_context, &pkt))
    errx(1, "Error while sending packet");
  AVFrame *frame = av_frame_alloc();
  if(frame == NULL)
    errx(1, "Frame was NULL");
  if (0 != avcodec_receive_frame(dec_context, frame))
    errx(1, "Could not receive frame from decoder");

  /* Expected palette data, see pixfmt.h. Palette is in data[1]. Only
   * transparent, white, and black are expected in the palette. */
  uint32_t *colour = (uint32_t*)frame->data[1];
  signed char palette_multiplier[256];
  for (int i = 0; i < 256; ++i) {
    palette_multiplier[i] = (/* unsigned */colour[i] >> 24)
      ? (colour[i] & 1)
        ? 1 /* White: positive reinforcement. */
        : -1 /* Black: negative reinforcement. */
      : 0; /* Transparent: ignore. */
  }

  /* Fill data for each glyph. */
  for (unsigned int i = 0; i < count; ++i) {
    output[i].key = keys[i];
    output[i].divider = 0;
    for (int j = 0; j < GLYPH_HEIGHT; ++j) {
      for (int k = 0; k < GLYPH_WIDTH; ++k) {
        /* data[0]: actual frame. */
        output[i].multiplier[j][k] = palette_multiplier[frame->data[0][
          /*   i →
           * ┌─────┬─────┬─────┬─────┐
           * │  k →│     │     │     │
           * │j    │     │     │     │
           * │↓    │     │     │     │
           * └─────┴─────┴─────┴─────┘
           */
          j * frame->linesize[0] + i * GLYPH_WIDTH + k]];

        /* Calculate the glyph “area”. */
        if (output[i].multiplier[j][k] != 0)
          output[i].divider++;
      }
    }
  }
  /* Magic number between 8 and 3. */
  output[3].divider = 287;

  /* Free everything. */
  av_packet_unref(&pkt);
  av_frame_unref(frame);
  av_frame_free(&frame);
  avcodec_free_context(&dec_context);
  avformat_close_input(&fmt_context);
}
