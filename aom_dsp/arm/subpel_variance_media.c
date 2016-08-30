/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./aom_config.h"
#include "./aom_dsp_rtcd.h"
#include "aom/aom_integer.h"
#include "aom_ports/mem.h"

#if HAVE_MEDIA
static const int16_t bilinear_filters_media[8][2] = { { 128, 0 }, { 112, 16 },
                                                      { 96, 32 }, { 80, 48 },
                                                      { 64, 64 }, { 48, 80 },
                                                      { 32, 96 }, { 16, 112 } };

extern void aom_filter_block2d_bil_first_pass_media(
    const uint8_t *src_ptr, uint16_t *dst_ptr, uint32_t src_pitch,
    uint32_t height, uint32_t width, const int16_t *filter);

extern void aom_filter_block2d_bil_second_pass_media(
    const uint16_t *src_ptr, uint8_t *dst_ptr, int32_t src_pitch,
    uint32_t height, uint32_t width, const int16_t *filter);

unsigned int aom_sub_pixel_variance8x8_media(
    const uint8_t *src_ptr, int src_pixels_per_line, int xoffset, int yoffset,
    const uint8_t *dst_ptr, int dst_pixels_per_line, unsigned int *sse) {
  uint16_t first_pass[10 * 8];
  uint8_t second_pass[8 * 8];
  const int16_t *HFilter, *VFilter;

  HFilter = bilinear_filters_media[xoffset];
  VFilter = bilinear_filters_media[yoffset];

  aom_filter_block2d_bil_first_pass_media(src_ptr, first_pass,
                                          src_pixels_per_line, 9, 8, HFilter);
  aom_filter_block2d_bil_second_pass_media(first_pass, second_pass, 8, 8, 8,
                                           VFilter);

  return aom_variance8x8_media(second_pass, 8, dst_ptr, dst_pixels_per_line,
                               sse);
}

unsigned int aom_sub_pixel_variance16x16_media(
    const uint8_t *src_ptr, int src_pixels_per_line, int xoffset, int yoffset,
    const uint8_t *dst_ptr, int dst_pixels_per_line, unsigned int *sse) {
  uint16_t first_pass[36 * 16];
  uint8_t second_pass[20 * 16];
  const int16_t *HFilter, *VFilter;
  unsigned int var;

  if (xoffset == 4 && yoffset == 0) {
    var = aom_variance_halfpixvar16x16_h_media(
        src_ptr, src_pixels_per_line, dst_ptr, dst_pixels_per_line, sse);
  } else if (xoffset == 0 && yoffset == 4) {
    var = aom_variance_halfpixvar16x16_v_media(
        src_ptr, src_pixels_per_line, dst_ptr, dst_pixels_per_line, sse);
  } else if (xoffset == 4 && yoffset == 4) {
    var = aom_variance_halfpixvar16x16_hv_media(
        src_ptr, src_pixels_per_line, dst_ptr, dst_pixels_per_line, sse);
  } else {
    HFilter = bilinear_filters_media[xoffset];
    VFilter = bilinear_filters_media[yoffset];

    aom_filter_block2d_bil_first_pass_media(
        src_ptr, first_pass, src_pixels_per_line, 17, 16, HFilter);
    aom_filter_block2d_bil_second_pass_media(first_pass, second_pass, 16, 16,
                                             16, VFilter);

    var = aom_variance16x16_media(second_pass, 16, dst_ptr, dst_pixels_per_line,
                                  sse);
  }
  return var;
}
#endif  // HAVE_MEDIA
