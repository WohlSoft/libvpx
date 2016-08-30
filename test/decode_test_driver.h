/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef TEST_DECODE_TEST_DRIVER_H_
#define TEST_DECODE_TEST_DRIVER_H_
#include <cstring>
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "./aom_config.h"
#include "aom/aom_decoder.h"

namespace libaom_test {

class CodecFactory;
class CompressedVideoSource;

// Provides an object to handle decoding output
class DxDataIterator {
 public:
  explicit DxDataIterator(aom_codec_ctx_t *decoder)
      : decoder_(decoder), iter_(NULL) {}

  const aom_image_t *Next() { return aom_codec_get_frame(decoder_, &iter_); }

 private:
  aom_codec_ctx_t *decoder_;
  aom_codec_iter_t iter_;
};

// Provides a simplified interface to manage one video decoding.
// Similar to Encoder class, the exact services should be added
// as more tests are added.
class Decoder {
 public:
  Decoder(aom_codec_dec_cfg_t cfg, unsigned long deadline)
      : cfg_(cfg), flags_(0), deadline_(deadline), init_done_(false) {
    memset(&decoder_, 0, sizeof(decoder_));
  }

  Decoder(aom_codec_dec_cfg_t cfg, const aom_codec_flags_t flag,
          unsigned long deadline)  // NOLINT
      : cfg_(cfg),
        flags_(flag),
        deadline_(deadline),
        init_done_(false) {
    memset(&decoder_, 0, sizeof(decoder_));
  }

  virtual ~Decoder() { aom_codec_destroy(&decoder_); }

  aom_codec_err_t PeekStream(const uint8_t *cxdata, size_t size,
                             aom_codec_stream_info_t *stream_info);

  aom_codec_err_t DecodeFrame(const uint8_t *cxdata, size_t size);

  aom_codec_err_t DecodeFrame(const uint8_t *cxdata, size_t size,
                              void *user_priv);

  DxDataIterator GetDxData() { return DxDataIterator(&decoder_); }

  void set_deadline(unsigned long deadline) { deadline_ = deadline; }

  void Control(int ctrl_id, int arg) { Control(ctrl_id, arg, AOM_CODEC_OK); }

  void Control(int ctrl_id, const void *arg) {
    InitOnce();
    const aom_codec_err_t res = aom_codec_control_(&decoder_, ctrl_id, arg);
    ASSERT_EQ(AOM_CODEC_OK, res) << DecodeError();
  }

  void Control(int ctrl_id, int arg, aom_codec_err_t expected_value) {
    InitOnce();
    const aom_codec_err_t res = aom_codec_control_(&decoder_, ctrl_id, arg);
    ASSERT_EQ(expected_value, res) << DecodeError();
  }

  const char *DecodeError() {
    const char *detail = aom_codec_error_detail(&decoder_);
    return detail ? detail : aom_codec_error(&decoder_);
  }

  // Passes the external frame buffer information to libaom.
  aom_codec_err_t SetFrameBufferFunctions(
      aom_get_frame_buffer_cb_fn_t cb_get,
      aom_release_frame_buffer_cb_fn_t cb_release, void *user_priv) {
    InitOnce();
    return aom_codec_set_frame_buffer_functions(&decoder_, cb_get, cb_release,
                                                user_priv);
  }

  const char *GetDecoderName() const {
    return aom_codec_iface_name(CodecInterface());
  }

  bool IsVP8() const;

  bool IsAV1() const;

  aom_codec_ctx_t *GetDecoder() { return &decoder_; }

 protected:
  virtual aom_codec_iface_t *CodecInterface() const = 0;

  void InitOnce() {
    if (!init_done_) {
      const aom_codec_err_t res =
          aom_codec_dec_init(&decoder_, CodecInterface(), &cfg_, flags_);
      ASSERT_EQ(AOM_CODEC_OK, res) << DecodeError();
      init_done_ = true;
    }
  }

  aom_codec_ctx_t decoder_;
  aom_codec_dec_cfg_t cfg_;
  aom_codec_flags_t flags_;
  unsigned int deadline_;
  bool init_done_;
};

// Common test functionality for all Decoder tests.
class DecoderTest {
 public:
  // Main decoding loop
  virtual void RunLoop(CompressedVideoSource *video);
  virtual void RunLoop(CompressedVideoSource *video,
                       const aom_codec_dec_cfg_t &dec_cfg);

  virtual void set_cfg(const aom_codec_dec_cfg_t &dec_cfg);
  virtual void set_flags(const aom_codec_flags_t flags);

  // Hook to be called before decompressing every frame.
  virtual void PreDecodeFrameHook(const CompressedVideoSource & /*video*/,
                                  Decoder * /*decoder*/) {}

  // Hook to be called to handle decode result. Return true to continue.
  virtual bool HandleDecodeResult(const aom_codec_err_t res_dec,
                                  const CompressedVideoSource & /*video*/,
                                  Decoder *decoder) {
    EXPECT_EQ(AOM_CODEC_OK, res_dec) << decoder->DecodeError();
    return AOM_CODEC_OK == res_dec;
  }

  // Hook to be called on every decompressed frame.
  virtual void DecompressedFrameHook(const aom_image_t & /*img*/,
                                     const unsigned int /*frame_number*/) {}

  // Hook to be called on peek result
  virtual void HandlePeekResult(Decoder *const decoder,
                                CompressedVideoSource *video,
                                const aom_codec_err_t res_peek);

 protected:
  explicit DecoderTest(const CodecFactory *codec)
      : codec_(codec), cfg_(), flags_(0) {}

  virtual ~DecoderTest() {}

  const CodecFactory *codec_;
  aom_codec_dec_cfg_t cfg_;
  aom_codec_flags_t flags_;
};

}  // namespace libaom_test

#endif  // TEST_DECODE_TEST_DRIVER_H_
