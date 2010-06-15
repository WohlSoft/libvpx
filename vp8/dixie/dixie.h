/*
 *  Copyright (c) 2010 The VP8 project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef DIXIE_H
#define DIXIE_H
#include "bool_decoder.h"

struct vp8_frame_hdr
{
    unsigned int is_keyframe;      /* Frame is a keyframe */
    unsigned int is_experimental;  /* Frame is a keyframe */
    unsigned int version;          /* Bitstream version */
    unsigned int is_shown;         /* Frame is to be displayed. */
    unsigned int part0_sz;         /* Partition 0 length, in bytes */

    struct vp8_kf_hdr
    {
        unsigned int w;        /* Width */
        unsigned int h;        /* Height */
        unsigned int scale_w;  /* Scaling factor, Width */
        unsigned int scale_h;  /* Scaling factor, Height */
    } kf;
};


enum
{
    MB_FEATURE_TREE_PROBS = 3,
    MAX_MB_SEGMENTS = 4
};


struct vp8_segment_hdr
{
    unsigned int         enabled;
    unsigned int         update_data;
    unsigned int         update_map;
    unsigned int         abs;    /* 0=deltas, 1=absolute values */
    unsigned int         tree_probs[MB_FEATURE_TREE_PROBS];
    int                  lf_level[MAX_MB_SEGMENTS];
    int                  quant_idx[MAX_MB_SEGMENTS];
};


enum
{
    BLOCK_CONTEXTS = 4
};


struct vp8_loopfilter_hdr
{
    unsigned int         use_simple;
    unsigned int         level;
    unsigned int         sharpness;
    unsigned int         delta_enabled;
    int                  ref_delta[BLOCK_CONTEXTS];
    int                  mode_delta[BLOCK_CONTEXTS];
};


enum
{
    MAX_PARTITIONS = 8
};

struct vp8_token_hdr
{
    unsigned int        partitions;
    unsigned int        partition_sz[MAX_PARTITIONS];
};


struct vp8_quant_hdr
{
    unsigned int       q_index;
    int                y1_dc_delta_q;
    int                y2_dc_delta_q;
    int                y2_ac_delta_q;
    int                uv_dc_delta_q;
    int                uv_ac_delta_q;
};


struct vp8_reference_hdr
{
    unsigned int refresh_last;
    unsigned int refresh_gf;
    unsigned int refresh_arf;
    unsigned int copy_gf;
    unsigned int copy_arf;
    unsigned int sign_bias_gf;
    unsigned int sign_bias_arf;
    unsigned int refresh_entropy;
};


enum
{
    BLOCK_TYPES        = 4,
    PREV_COEF_CONTEXTS = 3,
    COEF_BANDS         = 8,
    ENTROPY_NODES      = 11,
};
typedef unsigned char coeff_probs_table_t[BLOCK_TYPES][COEF_BANDS]
[PREV_COEF_CONTEXTS]
[ENTROPY_NODES];


enum
{
    MV_PROB_CNT = 2 + 8 - 1 + 10 /* from entropymv.h */
};
typedef unsigned char mv_component_probs_t[MV_PROB_CNT];


struct vp8_entropy_hdr
{
    coeff_probs_table_t   coeff_probs;
    mv_component_probs_t  mv_probs[2];
    unsigned int          coeff_skip_enabled;
    unsigned char         coeff_skip_prob;
    unsigned char         y_mode_probs[4];
    unsigned char         uv_mode_probs[3];
    unsigned char         prob_intra;
    unsigned char         prob_last;
    unsigned char         prob_gf;
};


struct vp8_decoder_ctx
{
    struct vpx_internal_error_info  error;

    struct vp8_frame_hdr            frame_hdr;
    struct vp8_segment_hdr          segment_hdr;
    struct vp8_loopfilter_hdr       loopfilter_hdr;
    struct vp8_token_hdr            token_hdr;
    struct vp8_quant_hdr            quant_hdr;
    struct vp8_reference_hdr        reference_hdr;
    struct vp8_entropy_hdr          entropy_hdr;

    struct bool_decoder             bool[MAX_PARTITIONS];
    struct vp8_entropy_hdr          saved_entropy;
    unsigned int                    saved_entropy_valid;
};


vpx_codec_err_t
vp8_parse_frame_header(const unsigned char   *data,
                       unsigned int           sz,
                       struct vp8_frame_hdr  *hdr);


vpx_codec_err_t
vp8_dixie_decode_frame(struct vp8_decoder_ctx *ctx,
                       const unsigned char    *data,
                       unsigned int            sz);


#endif
