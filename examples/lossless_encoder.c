/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aom/aom_encoder.h"
#include "aom/aomcx.h"

#include "../tools_common.h"
#include "../video_writer.h"

static const char *exec_name;

void usage_exit(void) {
  fprintf(stderr,
          "lossless_encoder: Example demonstrating lossless "
          "encoding feature. Supports raw input only.\n");
  fprintf(stderr, "Usage: %s <width> <height> <infile> <outfile>\n", exec_name);
  exit(EXIT_FAILURE);
}

static int encode_frame(aom_codec_ctx_t *codec, aom_image_t *img,
                        int frame_index, int flags, AvxVideoWriter *writer) {
  int got_pkts = 0;
  aom_codec_iter_t iter = NULL;
  const aom_codec_cx_pkt_t *pkt = NULL;
  const aom_codec_err_t res =
      aom_codec_encode(codec, img, frame_index, 1, flags, AOM_DL_GOOD_QUALITY);
  if (res != AOM_CODEC_OK) die_codec(codec, "Failed to encode frame");

  while ((pkt = aom_codec_get_cx_data(codec, &iter)) != NULL) {
    got_pkts = 1;

    if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) {
      const int keyframe = (pkt->data.frame.flags & AOM_FRAME_IS_KEY) != 0;
      if (!aom_video_writer_write_frame(writer, pkt->data.frame.buf,
                                        pkt->data.frame.sz,
                                        pkt->data.frame.pts)) {
        die_codec(codec, "Failed to write compressed frame");
      }
      printf(keyframe ? "K" : ".");
      fflush(stdout);
    }
  }

  return got_pkts;
}

int main(int argc, char **argv) {
  FILE *infile = NULL;
  aom_codec_ctx_t codec;
  aom_codec_enc_cfg_t cfg;
  int frame_count = 0;
  aom_image_t raw;
  aom_codec_err_t res;
  AvxVideoInfo info = { 0 };
  AvxVideoWriter *writer = NULL;
  const AvxInterface *encoder = NULL;
  const int fps = 30;

  exec_name = argv[0];

  if (argc < 5) die("Invalid number of arguments");

  encoder = get_aom_encoder_by_name("av1");
  if (!encoder) die("Unsupported codec.");

  info.codec_fourcc = encoder->fourcc;
  info.frame_width = strtol(argv[1], NULL, 0);
  info.frame_height = strtol(argv[2], NULL, 0);
  info.time_base.numerator = 1;
  info.time_base.denominator = fps;

  if (info.frame_width <= 0 || info.frame_height <= 0 ||
      (info.frame_width % 2) != 0 || (info.frame_height % 2) != 0) {
    die("Invalid frame size: %dx%d", info.frame_width, info.frame_height);
  }

  if (!aom_img_alloc(&raw, AOM_IMG_FMT_I420, info.frame_width,
                     info.frame_height, 1)) {
    die("Failed to allocate image.");
  }

  printf("Using %s\n", aom_codec_iface_name(encoder->codec_interface()));

  res = aom_codec_enc_config_default(encoder->codec_interface(), &cfg, 0);
  if (res) die_codec(&codec, "Failed to get default codec config.");

  cfg.g_w = info.frame_width;
  cfg.g_h = info.frame_height;
  cfg.g_timebase.num = info.time_base.numerator;
  cfg.g_timebase.den = info.time_base.denominator;

  writer = aom_video_writer_open(argv[4], kContainerIVF, &info);
  if (!writer) die("Failed to open %s for writing.", argv[4]);

  if (!(infile = fopen(argv[3], "rb")))
    die("Failed to open %s for reading.", argv[3]);

  if (aom_codec_enc_init(&codec, encoder->codec_interface(), &cfg, 0))
    die_codec(&codec, "Failed to initialize encoder");

  if (aom_codec_control_(&codec, AV1E_SET_LOSSLESS, 1))
    die_codec(&codec, "Failed to use lossless mode");

  // Encode frames.
  while (aom_img_read(&raw, infile)) {
    encode_frame(&codec, &raw, frame_count++, 0, writer);
  }

  // Flush encoder.
  while (encode_frame(&codec, NULL, -1, 0, writer)) {
  }

  printf("\n");
  fclose(infile);
  printf("Processed %d frames.\n", frame_count);

  aom_img_free(&raw);
  if (aom_codec_destroy(&codec)) die_codec(&codec, "Failed to destroy codec.");

  aom_video_writer_close(writer);

  return EXIT_SUCCESS;
}
