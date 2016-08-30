/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "test/codec_factory.h"
#include "test/decode_test_driver.h"
#include "test/register_state_check.h"
#include "test/video_source.h"

namespace libaom_test {

const char kVP8Name[] = "WebM Project VP8";
const char kAV1Name[] = "WebM Project AV1";

aom_codec_err_t Decoder::PeekStream(const uint8_t *cxdata, size_t size,
                                    aom_codec_stream_info_t *stream_info) {
  return aom_codec_peek_stream_info(
      CodecInterface(), cxdata, static_cast<unsigned int>(size), stream_info);
}

aom_codec_err_t Decoder::DecodeFrame(const uint8_t *cxdata, size_t size) {
  return DecodeFrame(cxdata, size, NULL);
}

aom_codec_err_t Decoder::DecodeFrame(const uint8_t *cxdata, size_t size,
                                     void *user_priv) {
  aom_codec_err_t res_dec;
  InitOnce();
  API_REGISTER_STATE_CHECK(
      res_dec = aom_codec_decode(
          &decoder_, cxdata, static_cast<unsigned int>(size), user_priv, 0));
  return res_dec;
}

bool Decoder::IsVP8() const {
  const char *codec_name = GetDecoderName();
  return strncmp(kVP8Name, codec_name, sizeof(kVP8Name) - 1) == 0;
}

bool Decoder::IsAV1() const {
  const char *codec_name = GetDecoderName();
  return strncmp(kAV1Name, codec_name, sizeof(kAV1Name) - 1) == 0;
}

void DecoderTest::HandlePeekResult(Decoder *const decoder,
                                   CompressedVideoSource *video,
                                   const aom_codec_err_t res_peek) {
  const bool is_vp8 = decoder->IsVP8();
  if (is_vp8) {
    /* Vp8's implementation of PeekStream returns an error if the frame you
     * pass it is not a keyframe, so we only expect AOM_CODEC_OK on the first
     * frame, which must be a keyframe. */
    if (video->frame_number() == 0)
      ASSERT_EQ(AOM_CODEC_OK, res_peek) << "Peek return failed: "
                                        << aom_codec_err_to_string(res_peek);
  } else {
    /* The Av1 implementation of PeekStream returns an error only if the
     * data passed to it isn't a valid Av1 chunk. */
    ASSERT_EQ(AOM_CODEC_OK, res_peek) << "Peek return failed: "
                                      << aom_codec_err_to_string(res_peek);
  }
}

void DecoderTest::RunLoop(CompressedVideoSource *video,
                          const aom_codec_dec_cfg_t &dec_cfg) {
  Decoder *const decoder = codec_->CreateDecoder(dec_cfg, flags_, 0);
  ASSERT_TRUE(decoder != NULL);
  bool end_of_file = false;

  // Decode frames.
  for (video->Begin(); !::testing::Test::HasFailure() && !end_of_file;
       video->Next()) {
    PreDecodeFrameHook(*video, decoder);

    aom_codec_stream_info_t stream_info;
    stream_info.sz = sizeof(stream_info);

    if (video->cxdata() != NULL) {
      const aom_codec_err_t res_peek = decoder->PeekStream(
          video->cxdata(), video->frame_size(), &stream_info);
      HandlePeekResult(decoder, video, res_peek);
      ASSERT_FALSE(::testing::Test::HasFailure());

      aom_codec_err_t res_dec =
          decoder->DecodeFrame(video->cxdata(), video->frame_size());
      if (!HandleDecodeResult(res_dec, *video, decoder)) break;
    } else {
      // Signal end of the file to the decoder.
      const aom_codec_err_t res_dec = decoder->DecodeFrame(NULL, 0);
      ASSERT_EQ(AOM_CODEC_OK, res_dec) << decoder->DecodeError();
      end_of_file = true;
    }

    DxDataIterator dec_iter = decoder->GetDxData();
    const aom_image_t *img = NULL;

    // Get decompressed data
    while ((img = dec_iter.Next()))
      DecompressedFrameHook(*img, video->frame_number());
  }
  delete decoder;
}

void DecoderTest::RunLoop(CompressedVideoSource *video) {
  aom_codec_dec_cfg_t dec_cfg = aom_codec_dec_cfg_t();
  RunLoop(video, dec_cfg);
}

void DecoderTest::set_cfg(const aom_codec_dec_cfg_t &dec_cfg) {
  memcpy(&cfg_, &dec_cfg, sizeof(cfg_));
}

void DecoderTest::set_flags(const aom_codec_flags_t flags) { flags_ = flags; }

}  // namespace libaom_test
