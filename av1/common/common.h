/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef AV1_COMMON_COMMON_H_
#define AV1_COMMON_COMMON_H_

/* Interface header for common constant data structures and lookup tables */

#include <assert.h>

#include "./aom_config.h"
#include "aom_dsp/aom_dsp_common.h"
#include "aom_mem/aom_mem.h"
#include "aom/aom_integer.h"
#include "aom_ports/bitops.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PI 3.141592653589793238462643383279502884

// Only need this for fixed-size arrays, for structs just assign.
#define av1_copy(dest, src)              \
  {                                      \
    assert(sizeof(dest) == sizeof(src)); \
    memcpy(dest, src, sizeof(src));      \
  }

// Use this for variably-sized arrays.
#define av1_copy_array(dest, src, n)           \
  {                                            \
    assert(sizeof(*(dest)) == sizeof(*(src))); \
    memcpy(dest, src, n * sizeof(*(src)));     \
  }

#define av1_zero(dest) memset(&(dest), 0, sizeof(dest))
#define av1_zero_array(dest, n) memset(dest, 0, n * sizeof(*(dest)))

static INLINE int get_unsigned_bits(unsigned int num_values) {
  return num_values > 0 ? get_msb(num_values) + 1 : 0;
}

#if CONFIG_DEBUG
#define CHECK_MEM_ERROR(cm, lval, expr)                                     \
  do {                                                                      \
    lval = (expr);                                                          \
    if (!lval)                                                              \
      aom_internal_error(&cm->error, AOM_CODEC_MEM_ERROR,                   \
                         "Failed to allocate " #lval " at %s:%d", __FILE__, \
                         __LINE__);                                         \
  } while (0)
#else
#define CHECK_MEM_ERROR(cm, lval, expr)                   \
  do {                                                    \
    lval = (expr);                                        \
    if (!lval)                                            \
      aom_internal_error(&cm->error, AOM_CODEC_MEM_ERROR, \
                         "Failed to allocate " #lval);    \
  } while (0)
#endif
// TODO(yaowu: validate the usage of these codes or develop new ones.)
#define AV1_SYNC_CODE_0 0x49
#define AV1_SYNC_CODE_1 0x83
#define AV1_SYNC_CODE_2 0x43

#define AOM_FRAME_MARKER 0x2

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // AV1_COMMON_COMMON_H_
