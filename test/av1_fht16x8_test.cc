/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "third_party/googletest/src/include/gtest/gtest.h"

#include "./aom_dsp_rtcd.h"
#include "./av1_rtcd.h"

#include "aom_ports/mem.h"
#include "test/acm_random.h"
#include "test/clear_system_state.h"
#include "test/register_state_check.h"
#include "test/transform_test_base.h"
#include "test/util.h"

using libaom_test::ACMRandom;

namespace {
typedef void (*IhtFunc)(const tran_low_t *in, uint8_t *out, int stride,
                        int tx_type);
using std::tr1::tuple;
using libaom_test::FhtFunc;
typedef tuple<FhtFunc, IhtFunc, int, aom_bit_depth_t, int> Ht16x8Param;

void fht16x8_ref(const int16_t *in, tran_low_t *out, int stride, int tx_type) {
  av1_fht16x8_c(in, out, stride, tx_type);
}

class AV1Trans16x8HT : public libaom_test::TransformTestBase,
                       public ::testing::TestWithParam<Ht16x8Param> {
 public:
  virtual ~AV1Trans16x8HT() {}

  virtual void SetUp() {
    fwd_txfm_ = GET_PARAM(0);
    inv_txfm_ = GET_PARAM(1);
    tx_type_ = GET_PARAM(2);
    pitch_ = 16;
    fwd_txfm_ref = fht16x8_ref;
    bit_depth_ = GET_PARAM(3);
    mask_ = (1 << bit_depth_) - 1;
    num_coeffs_ = GET_PARAM(4);
  }
  virtual void TearDown() { libaom_test::ClearSystemState(); }

 protected:
  void RunFwdTxfm(const int16_t *in, tran_low_t *out, int stride) {
    fwd_txfm_(in, out, stride, tx_type_);
  }

  void RunInvTxfm(const tran_low_t *out, uint8_t *dst, int stride) {
    inv_txfm_(out, dst, stride, tx_type_);
  }

  FhtFunc fwd_txfm_;
  IhtFunc inv_txfm_;
};

TEST_P(AV1Trans16x8HT, CoeffCheck) { RunCoeffCheck(); }

using std::tr1::make_tuple;

#if HAVE_SSE2
const Ht16x8Param kArrayHt16x8Param_sse2[] = {
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 0, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 1, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 2, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 3, AOM_BITS_8, 128),
#if CONFIG_EXT_TX
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 4, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 5, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 6, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 7, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 8, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 9, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 10, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 11, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 12, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 13, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 14, AOM_BITS_8, 128),
  make_tuple(&av1_fht16x8_sse2, &av1_iht16x8_128_add_c, 15, AOM_BITS_8, 128)
#endif  // CONFIG_EXT_TX
};
INSTANTIATE_TEST_CASE_P(SSE2, AV1Trans16x8HT,
                        ::testing::ValuesIn(kArrayHt16x8Param_sse2));
#endif  // HAVE_SSE2

}  // namespace