#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
// XRT native includes
#include "xrt/xrt_bo.h"
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"
#include "../HW/camta_types.h"
/*
 * CAMTA Alveo Benchmark
 *
 * Data format:
 *   data_t = ap_fixed<16,5>  -> Q5.11
 *
 * This version validates the rank-1 configurations obtained from the
 * offline MaxAbsErr-oriented coefficient/configuration search.
 *
 * Approximation functions:
 *   GELU, tanh, sigmoid, Swish, exp
 *
 * Functional mode-demonstration functions:
 *   ReLU     -> ZERO / IDENTITY / IDENTITY
 *   HardTanh -> CONST / IDENTITY / CONST
 *
 * Square is kept only as a sanity check for EVEN symmetry.
 *
 * In addition to the existing 10,000-sample benchmark, this host performs
 * an exhaustive numerical sweep over all 65,536 Q5.11 input codes for the
 * five target nonlinear functions.
 */
enum class SymMode : int {
  NONE       = CAMTA_SYM_NONE,
  ODD        = CAMTA_SYM_ODD,
  COMPLEMENT = CAMTA_SYM_COMPLEMENT,
  RESIDUAL   = CAMTA_SYM_RESIDUAL,
  EVEN       = CAMTA_SYM_EVEN
};
struct RegionPoly {
  int deg;
  int mode;
  data_t a3;
  data_t a2;
  data_t a1;
  data_t a0;
};
struct TestCase {
  std::string function_name;
  std::string case_name;
  int N;
  double xmin;
  double xmax;
  double L_left;
  double L_right;
  RegionPoly r0;
  RegionPoly r1;
  RegionPoly r2;
  SymMode sym_mode;
  std::function<double(double)> ref_fn;
};
struct Metrics {
  double input_prepare_ms = 0.0;
  double h2d_ms = 0.0;
  double cfg_write_ms = 0.0;
  double kernel_ms = 0.0;
  double d2h_ms = 0.0;
  double total_device_flow_ms = 0.0;
  double mse = 0.0;
  double rmse = 0.0;
  double mae = 0.0;
  double max_abs_err = 0.0;
  double sqnr_db = 0.0;
};
struct SummaryRow {
  std::string function_name;
  std::string case_name;
  int N = 0;
  double xmin = 0.0;
  double xmax = 0.0;
  double L_left = 0.0;
  double L_right = 0.0;
  SymMode sym_mode = SymMode::NONE;
  RegionPoly r0{};
  RegionPoly r1{};
  RegionPoly r2{};
  double avg_input_prepare_ms = 0.0;
  double avg_h2d_ms = 0.0;
  double avg_cfg_write_ms = 0.0;
  double avg_kernel_ms = 0.0;
  double avg_d2h_ms = 0.0;
  double avg_total_device_flow_ms = 0.0;
  double avg_kernel_us = 0.0;
  double ns_per_sample = 0.0;
  double cycles_at_250MHz = 0.0;
  double throughput_samples_per_sec = 0.0;
  double cfg_over_kernel_pct = 0.0;
  double avg_mse = 0.0;
  double avg_rmse = 0.0;
  double avg_mae = 0.0;
  double avg_max_abs_err = 0.0;
  double avg_sqnr_db = 0.0;
};
struct ExhaustiveMetrics {
  int total_codes_sent = 0;
  int sample_count = 0;
  int excluded_code_count = 0;
  double mse = 0.0;
  double rmse = 0.0;
  double mae = 0.0;
  double max_abs_err = 0.0;
  double x_at_max_abs_err = 0.0;
  double y_hw_at_max_abs_err = 0.0;
  double y_ref_at_max_abs_err = 0.0;
  int signed_code_at_max_abs_err = 0;
  std::uint16_t raw_code_at_max_abs_err = 0;
  double min_q511_input = -16.0;
  double min_q511_y_hw = 0.0;
  double min_q511_y_ref = 0.0;
  double min_q511_abs_err = 0.0;
  bool min_q511_requires_magnitude = false;
  bool min_q511_included_in_metrics = true;
};
static const char* sym_mode_name(SymMode mode) {
  switch (mode) {
    case SymMode::NONE:       return "none";
    case SymMode::ODD:        return "odd";
    case SymMode::COMPLEMENT: return "complement";
    case SymMode::RESIDUAL:   return "residual";
    case SymMode::EVEN:       return "even";
    default:                  return "unknown";
  }
}
static const char* region_mode_name(int mode) {
  switch (mode) {
    case CAMTA_MODE_HORNER:   return "horner";
    case CAMTA_MODE_CONST:    return "const";
    case CAMTA_MODE_ZERO:     return "zero";
    case CAMTA_MODE_IDENTITY: return "identity";
    default:                  return "unknown";
  }
}
static bool symmetry_uses_magnitude(SymMode mode) {
  switch (mode) {
    case SymMode::ODD:
    case SymMode::COMPLEMENT:
    case SymMode::RESIDUAL:
    case SymMode::EVEN:
      return true;
    case SymMode::NONE:
    default:
      return false;
  }
}
// -----------------------------------------------------------------------------
// Mathematical reference functions
// -----------------------------------------------------------------------------
static double ref_gelu(double x) {
  const double pi = 3.14159265358979323846;
  const double c = std::sqrt(2.0 / pi);
  return 0.5 * x *
         (1.0 + std::tanh(c * (x + 0.044715 * x * x * x)));
}
static double ref_tanh(double x) {
  return std::tanh(x);
}
static double ref_sigmoid(double x) {
  return 1.0 / (1.0 + std::exp(-x));
}
static double ref_swish(double x) {
  return x / (1.0 + std::exp(-x));
}
static double ref_exp(double x) {
  return std::exp(x);
}
static double ref_square(double x) {
  return x * x;
}
static double ref_relu(double x) {
  return (x > 0.0) ? x : 0.0;
}
static double ref_hardtanh(double x) {
  if (x < -1.0) return -1.0;
  if (x >  1.0) return  1.0;
  return x;
}
static double avg(const std::vector<double>& v) {
  if (v.empty()) return 0.0;
  return std::accumulate(v.begin(), v.end(), 0.0) /
         static_cast<double>(v.size());
}
static int parse_int_arg(
  const std::string& value,
  const std::string& name
) {
  try {
    int parsed = std::stoi(value);
    if (parsed <= 0) {
      throw std::runtime_error(name + " must be positive.");
    }
    return parsed;
  }
  catch (const std::exception&) {
    throw std::runtime_error(
      "Invalid value for " + name + ": " + value
    );
  }
}
// -----------------------------------------------------------------------------
// XRT configuration
// -----------------------------------------------------------------------------
static void set_camta_args(
  xrt::run& run,
  xrt::bo& bo_x_in,
  xrt::bo& bo_y_out,
  const TestCase& tc
) {
  run.set_arg(0, bo_x_in);
  run.set_arg(1, bo_y_out);
  run.set_arg(2, tc.N);
  run.set_arg(3, static_cast<data_t>(tc.L_left));
  run.set_arg(4, static_cast<data_t>(tc.L_right));
  run.set_arg(5, tc.r0.deg);
  run.set_arg(6, tc.r1.deg);
  run.set_arg(7, tc.r2.deg);
  run.set_arg(8, tc.r0.mode);
  run.set_arg(9, tc.r1.mode);
  run.set_arg(10, tc.r2.mode);
  run.set_arg(11, static_cast<int>(tc.sym_mode));
  run.set_arg(12, tc.r0.a3);
  run.set_arg(13, tc.r0.a2);
  run.set_arg(14, tc.r0.a1);
  run.set_arg(15, tc.r0.a0);
  run.set_arg(16, tc.r1.a3);
  run.set_arg(17, tc.r1.a2);
  run.set_arg(18, tc.r1.a1);
  run.set_arg(19, tc.r1.a0);
  run.set_arg(20, tc.r2.a3);
  run.set_arg(21, tc.r2.a2);
  run.set_arg(22, tc.r2.a1);
  run.set_arg(23, tc.r2.a0);
}
// -----------------------------------------------------------------------------
// Single benchmark execution
// -----------------------------------------------------------------------------
static Metrics run_test(
  xrt::kernel& kernel,
  xrt::bo& bo_x_in,
  xrt::bo& bo_y_out,
  data_t* x_buf,
  data_t* y_buf,
  const TestCase& tc,
  bool compute_error
) {
  Metrics m;
  if (tc.N <= 1) {
    throw std::runtime_error(
      "Invalid test case: N must be greater than 1."
    );
  }
  if (tc.L_left >= tc.L_right) {
    throw std::runtime_error(
      "Invalid test case: L_left must be smaller than L_right."
    );
  }
  std::vector<double> x_original(tc.N);
  auto t_total_0 =
    std::chrono::high_resolution_clock::now();
  auto t_prepare_0 =
    std::chrono::high_resolution_clock::now();
  for (int i = 0; i < tc.N; ++i) {
    const double x =
      tc.xmin +
      (tc.xmax - tc.xmin) *
      static_cast<double>(i) /
      static_cast<double>(tc.N - 1);
    x_original[i] = x;
    // The host sends the original x.
    // CAMTA handles symmetry internally.
    x_buf[i] = static_cast<data_t>(x);
    y_buf[i] = static_cast<data_t>(0.0);
  }
  auto t_prepare_1 =
    std::chrono::high_resolution_clock::now();
  // Host -> Device
  auto t_h2d_0 =
    std::chrono::high_resolution_clock::now();
  bo_x_in.sync(XCL_BO_SYNC_BO_TO_DEVICE);
  auto t_h2d_1 =
    std::chrono::high_resolution_clock::now();
  xrt::run run(kernel);
  // Runtime configuration through XRT set_arg
  auto t_cfg_0 =
    std::chrono::high_resolution_clock::now();
  set_camta_args(run, bo_x_in, bo_y_out, tc);
  auto t_cfg_1 =
    std::chrono::high_resolution_clock::now();
  // Kernel execution
  auto t_k_0 =
    std::chrono::high_resolution_clock::now();
  run.start();
  run.wait();
  auto t_k_1 =
    std::chrono::high_resolution_clock::now();
  // Device -> Host
  auto t_d2h_0 =
    std::chrono::high_resolution_clock::now();
  bo_y_out.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  auto t_d2h_1 =
    std::chrono::high_resolution_clock::now();
  auto t_total_1 =
    std::chrono::high_resolution_clock::now();
  m.input_prepare_ms =
    std::chrono::duration<double, std::milli>(
      t_prepare_1 - t_prepare_0
    ).count();
  m.h2d_ms =
    std::chrono::duration<double, std::milli>(
      t_h2d_1 - t_h2d_0
    ).count();
  m.cfg_write_ms =
    std::chrono::duration<double, std::milli>(
      t_cfg_1 - t_cfg_0
    ).count();
  m.kernel_ms =
    std::chrono::duration<double, std::milli>(
      t_k_1 - t_k_0
    ).count();
  m.d2h_ms =
    std::chrono::duration<double, std::milli>(
      t_d2h_1 - t_d2h_0
    ).count();
  m.total_device_flow_ms =
    std::chrono::duration<double, std::milli>(
      t_total_1 - t_total_0
    ).count();
  // ---------------------------------------------------------------------------
  // Numerical evaluation
  // ---------------------------------------------------------------------------
  if (compute_error) {
    double squared_error_sum = 0.0;
    double abs_error_sum = 0.0;
    double max_abs_err = 0.0;
    double signal_power_sum = 0.0;
    for (int i = 0; i < tc.N; ++i) {
      const double x_ref = x_original[i];
      const double y_hw = y_buf[i].to_double();
      const double y_ref = tc.ref_fn(x_ref);
      const double err = y_hw - y_ref;
      const double abs_err = std::abs(err);
      squared_error_sum += err * err;
      abs_error_sum += abs_err;
      signal_power_sum += y_ref * y_ref;
      if (abs_err > max_abs_err) {
        max_abs_err = abs_err;
      }
    }
    m.mse =
      squared_error_sum / static_cast<double>(tc.N);
    m.rmse = std::sqrt(m.mse);
    m.mae =
      abs_error_sum / static_cast<double>(tc.N);
    m.max_abs_err = max_abs_err;
    const double signal_power =
      signal_power_sum / static_cast<double>(tc.N);
    if (m.mse > 0.0 && signal_power > 0.0) {
      m.sqnr_db =
        10.0 * std::log10(signal_power / m.mse);
    }
    else if (m.mse == 0.0) {
      m.sqnr_db =
        std::numeric_limits<double>::infinity();
    }
    else {
      m.sqnr_db =
        -std::numeric_limits<double>::infinity();
    }
  }
  return m;
}
// -----------------------------------------------------------------------------
// Exhaustive Q5.11 numerical validation
// -----------------------------------------------------------------------------
static ExhaustiveMetrics run_exhaustive_validation(
  xrt::kernel& kernel,
  xrt::bo& bo_x_in,
  xrt::bo& bo_y_out,
  data_t* x_buf,
  data_t* y_buf,
  const TestCase& base_tc
) {
  constexpr int Q511_FRACTION_BITS = 11;
  constexpr int Q511_SCALE = 1 << Q511_FRACTION_BITS;
  constexpr int EXHAUSTIVE_N = 1 << 16;
  constexpr int MIN_SIGNED_CODE = -(1 << 15);
  TestCase tc = base_tc;
  tc.N = EXHAUSTIVE_N;
  // Send every 16-bit Q5.11 code to the hardware exactly once.
  // For magnitude-based symmetry modes, 0x8000 (-16.0) is still exercised
  // and reported separately, but it is excluded from the aggregate numerical
  // error metrics because |−16| = +16 cannot be represented in Q5.11.
  for (int i = 0; i < EXHAUSTIVE_N; ++i) {
    const int signed_code = MIN_SIGNED_CODE + i;
    const double x =
      static_cast<double>(signed_code) /
      static_cast<double>(Q511_SCALE);
    x_buf[i] = static_cast<data_t>(x);
    y_buf[i] = static_cast<data_t>(0.0);
    if (x_buf[i].to_double() != x) {
      throw std::runtime_error(
        "Host Q5.11 enumeration mismatch at signed code " +
        std::to_string(signed_code)
      );
    }
  }
  bo_x_in.sync(XCL_BO_SYNC_BO_TO_DEVICE);
  xrt::run run(kernel);
  set_camta_args(run, bo_x_in, bo_y_out, tc);
  run.start();
  run.wait();
  bo_y_out.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  ExhaustiveMetrics m;
  m.total_codes_sent = EXHAUSTIVE_N;
  m.min_q511_requires_magnitude =
    symmetry_uses_magnitude(tc.sym_mode);
  m.min_q511_included_in_metrics =
    !m.min_q511_requires_magnitude;
  double squared_error_sum = 0.0;
  double abs_error_sum = 0.0;
  bool max_initialized = false;
  for (int i = 0; i < EXHAUSTIVE_N; ++i) {
    const int signed_code = MIN_SIGNED_CODE + i;
    const std::uint16_t raw_code =
      static_cast<std::uint16_t>(signed_code);
    const double x =
      static_cast<double>(signed_code) /
      static_cast<double>(Q511_SCALE);
    const double y_hw = y_buf[i].to_double();
    const double y_ref = tc.ref_fn(x);
    const double err = y_hw - y_ref;
    const double abs_err = std::abs(err);
    // 0x8000 corresponds to -16.0. Always report its hardware behavior.
    if (i == 0) {
      m.min_q511_input = x;
      m.min_q511_y_hw = y_hw;
      m.min_q511_y_ref = y_ref;
      m.min_q511_abs_err = abs_err;
    }
    // Magnitude-based symmetry intentionally sacrifices the unique Q5.11
    // minimum value because its positive magnitude cannot be represented.
    // The code is exercised above, but excluded from aggregate accuracy.
    const bool exclude_from_metrics =
      (i == 0 && m.min_q511_requires_magnitude);
    if (exclude_from_metrics) {
      ++m.excluded_code_count;
      continue;
    }
    ++m.sample_count;
    squared_error_sum += err * err;
    abs_error_sum += abs_err;
    if (!max_initialized || abs_err > m.max_abs_err) {
      max_initialized = true;
      m.max_abs_err = abs_err;
      m.x_at_max_abs_err = x;
      m.y_hw_at_max_abs_err = y_hw;
      m.y_ref_at_max_abs_err = y_ref;
      m.signed_code_at_max_abs_err = signed_code;
      m.raw_code_at_max_abs_err = raw_code;
    }
  }
  if (m.sample_count <= 0) {
    throw std::runtime_error(
      "No Q5.11 input codes were included in exhaustive metrics."
    );
  }
  m.mse =
    squared_error_sum /
    static_cast<double>(m.sample_count);
  m.rmse = std::sqrt(m.mse);
  m.mae =
    abs_error_sum /
    static_cast<double>(m.sample_count);
  return m;
}
// -----------------------------------------------------------------------------
// MaxAE-search rank-1 configurations
// -----------------------------------------------------------------------------
static std::vector<TestCase> build_tests(int N) {
  std::vector<TestCase> tests;
  /*
   * GELU
   * Search rank: MaxAbsErr
   * Domain: [-8, 8]
   * Symmetry: RESIDUAL
   * Modes: H/H/I
   * Degrees: 3/3/1
   *
   * L_left  = 3510 / 2048 = 1.7138671875
   * L_right = 7411 / 2048 = 3.61865234375
   */
  tests.push_back({
    "gelu",
    "gelu_maxae_rank1",
    N,
    -8.0,
    8.0,
    1.7138671875,
    3.61865234375,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.11279296875,
       0.46826171875,
       0.48486328125,
       0.00048828125
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.01318359375,
      -0.1357421875,
       1.4697265625,
      -0.546875
    },
    {
      1,
      CAMTA_MODE_IDENTITY,
      0.0,
      0.0,
      1.0,
      0.0
    },
    SymMode::RESIDUAL,
    ref_gelu
  });
  /*
   * TANH
   * Domain: [-4, 4]
   * Symmetry: ODD
   * Modes: H/H/H
   * Degrees: 3/3/2
   *
   * L_left  = 1950 / 2048 = 0.9521484375
   * L_right = 4681 / 2048 = 2.28564453125
   */
  tests.push_back({
    "tanh",
    "tanh_maxae_rank1",
    N,
    -4.0,
    4.0,
    0.9521484375,
    2.28564453125,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.11279296875,
      -0.1630859375,
       1.0361328125,
      -0.00146484375
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.07568359375,
      -0.51123046875,
       1.20556640625,
      -0.0078125
    },
    {
      2,
      CAMTA_MODE_HORNER,
       0.0,
      -0.0087890625,
       0.06591796875,
       0.87646484375
    },
    SymMode::ODD,
    ref_tanh
  });
  /*
   * SIGMOID
   * Domain: [-8, 8]
   * Symmetry: COMPLEMENT
   * Modes: H/H/H
   * Degrees: 3/3/2
   *
   * L_left  = 4681 / 2048 = 2.28564453125
   * L_right = 9752 / 2048 = 4.76171875
   */
  tests.push_back({
    "sigmoid",
    "sigmoid_maxae_rank1",
    N,
    -8.0,
    8.0,
    2.28564453125,
    4.76171875,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.00390625,
      -0.02880859375,
       0.26513671875,
       0.49853515625
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.00390625,
      -0.0556640625,
       0.27392578125,
       0.52734375
    },
    {
      2,
      CAMTA_MODE_HORNER,
       0.0,
      -0.0009765625,
       0.01513671875,
       0.94189453125
    },
    SymMode::COMPLEMENT,
    ref_sigmoid
  });
  /*
   * SWISH
   * Domain: [-8, 8]
   * Symmetry: RESIDUAL
   * Modes: H/H/H
   * Degrees: 3/2/2
   *
   * L_left  = 4681 / 2048 = 2.28564453125
   * L_right = 10532 / 2048 = 5.142578125
   */
  tests.push_back({
    "swish",
    "swish_maxae_rank1",
    N,
    -8.0,
    8.0,
    2.28564453125,
    5.142578125,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.04541015625,
       0.28662109375,
       0.4892578125,
       0.0009765625
    },
    {
      2,
      CAMTA_MODE_HORNER,
       0.0,
      -0.01611328125,
       1.18310546875,
      -0.5458984375
    },
    {
      2,
      CAMTA_MODE_HORNER,
       0.0,
      -0.00341796875,
       1.0537109375,
      -0.21435546875
    },
    SymMode::RESIDUAL,
    ref_swish
  });
  /*
   * EXP
   * Domain: [-1, 1]
   * Symmetry: NONE
   * Modes: H/H/H
   * Degrees: 3/3/3
   *
   * L_left  = -586 / 2048 = -0.2861328125
   * L_right =  487 / 2048 =  0.23779296875
   */
  tests.push_back({
    "exp",
    "exp_maxae_rank1_m1_p1",
    N,
    -1.0,
    1.0,
    -0.2861328125,
     0.23779296875,
    {
      3,
      CAMTA_MODE_HORNER,
       0.08837890625,
       0.435546875,
       0.97607421875,
       0.9970703125
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.1650390625,
       0.50048828125,
       1.0,
       1.00048828125
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.31201171875,
       0.35888671875,
       1.0546875,
       0.9931640625
    },
    SymMode::NONE,
    ref_exp
  });
  /*
   * -------------------------------------------------------------------------
   * Additional matched-domain cases for literature comparison
   * These DO NOT replace the frozen original benchmark configurations above.
   * -------------------------------------------------------------------------
   */
  /*
   * GELU [-4, 4]
   * Host-aligned rank-1 candidate
   * Symmetry: RESIDUAL
   * Modes: H/H/H
   * Degrees: 3/3/1
   */
  tests.push_back({
    "gelu",
    "gelu_m4_p4_host_aligned_rank1",
    N,
    -4.0,
    4.0,
    1.63818359375,
    3.388671875,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.1142578125,
       0.47119140625,
       0.4833984375,
       0.0009765625
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.0126953125,
      -0.13232421875,
       1.4619140625,
      -0.54052734375
    },
    {
      1,
      CAMTA_MODE_HORNER,
       0.0,
       0.0,
       1.00146484375,
      -0.00537109375
    },
    SymMode::RESIDUAL,
    ref_gelu
  });
  /*
   * TANH [-8, 8]
   * Host-aligned rank-1 candidate
   * Symmetry: ODD
   * Modes: H/H/H
   * Degrees: 3/3/2
   */
  tests.push_back({
    "tanh",
    "tanh_m8_p8_host_aligned_rank1",
    N,
    -8.0,
    8.0,
    1.05517578125,
    2.66650390625,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.0849609375,
      -0.2001953125,
       1.04833984375,
      -0.001953125
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.0556640625,
      -0.41015625,
       1.0400390625,
       0.080078125
    },
    {
      2,
      CAMTA_MODE_HORNER,
       0.0,
      -0.00048828125,
       0.00634765625,
       0.97998046875
    },
    SymMode::ODD,
    ref_tanh
  });
  /*
   * SIGMOID [-4, 4]
   * Host-aligned rank-1 candidate
   * Symmetry: COMPLEMENT
   * Modes: H/H/H
   * Degrees: 3/3/1
   */
  tests.push_back({
    "sigmoid",
    "sigmoid_m4_p4_host_aligned_rank1",
    N,
    -4.0,
    4.0,
    1.3330078125,
    3.55517578125,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.0126953125,
      -0.0087890625,
       0.2529296875,
       0.5
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.00634765625,
      -0.07763671875,
       0.33935546875,
       0.46240234375
    },
    {
      1,
      CAMTA_MODE_HORNER,
       0.0,
       0.0,
       0.02197265625,
       0.89453125
    },
    SymMode::COMPLEMENT,
    ref_sigmoid
  });
  /*
   * SWISH [-4, 4]
   * Host-aligned rank-1 candidate
   * Symmetry: RESIDUAL
   * Modes: H/H/H
   * Degrees: 3/3/2
   */
  tests.push_back({
    "swish",
    "swish_m4_p4_host_aligned_rank1",
    N,
    -4.0,
    4.0,
    1.8603515625,
    3.11083984375,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.04638671875,
       0.2890625,
       0.48779296875,
       0.0009765625
    },
    {
      3,
      CAMTA_MODE_HORNER,
      -0.013671875,
       0.10009765625,
       0.85546875,
      -0.23974609375
    },
    {
      2,
      CAMTA_MODE_HORNER,
       0.0,
      -0.01806640625,
       1.197265625,
      -0.5712890625
    },
    SymMode::RESIDUAL,
    ref_swish
  });
  /*
   * SWISH [-5, 0]
   * Host-aligned rank-1 candidate
   * Symmetry: RESIDUAL
   * Modes: H/H/H
   * Degrees: 3/3/2
   */
  tests.push_back({
    "swish",
    "swish_m5_p0_host_aligned_rank1",
    N,
    -5.0,
    0.0,
    1.52783203125,
    2.22216796875,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.04443359375,
       0.28515625,
       0.48974609375,
       0.0009765625
    },
    {
      3,
      CAMTA_MODE_HORNER,
      -0.03125,
       0.21240234375,
       0.61572265625,
      -0.0693359375
    },
    {
      2,
      CAMTA_MODE_HORNER,
       0.0,
      -0.01611328125,
       1.18310546875,
      -0.5458984375
    },
    SymMode::RESIDUAL,
    ref_swish
  });
  /*
   * -------------------------------------------------------------------------
   * Additional matched-domain cases for literature comparison
   * Added after the previous comparison set.
   * -------------------------------------------------------------------------
   */
  /*
   * GELU [-5, 0]
   * Host-aligned rank-1 candidate
   * Symmetry: RESIDUAL
   * Modes: H/H/H
   * Degrees: 3/3/1
   */
  tests.push_back({
    "gelu",
    "gelu_m5_p0_host_aligned_rank1",
    N,
    -5.0,
    0.0,
    1.5625,
    3.1591796875,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.115234375,
       0.47314453125,
       0.482421875,
       0.0009765625
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.0107421875,
      -0.11767578125,
       1.42626953125,
      -0.5126953125
    },
    {
      1,
      CAMTA_MODE_HORNER,
       0.0,
       0.0,
       1.0009765625,
      -0.00439453125
    },
    SymMode::RESIDUAL,
    ref_gelu
  });
  /*
   * SIGMOID [-6, 6]
   * Host-aligned rank-1 candidate
   * Symmetry: COMPLEMENT
   * Modes: H/H/H
   * Degrees: 3/3/2
   */
  tests.push_back({
    "sigmoid",
    "sigmoid_m6_p6_host_aligned_rank1",
    N,
    -6.0,
    6.0,
    1.66650390625,
    4.3330078125,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.00927734375,
      -0.01513671875,
       0.255859375,
       0.49951171875
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.00537109375,
      -0.06982421875,
       0.31884765625,
       0.47998046875
    },
    {
      2,
      CAMTA_MODE_HORNER,
       0.0,
      -0.0029296875,
       0.0361328125,
       0.88623046875
    },
    SymMode::COMPLEMENT,
    ref_sigmoid
  });
  /*
   * SWISH [-6, 6]
   * Host-aligned rank-1 candidate
   * Symmetry: RESIDUAL
   * Modes: H/H/H
   * Degrees: 3/3/1
   */
  tests.push_back({
    "swish",
    "swish_m6_p6_host_aligned_rank1",
    N,
    -6.0,
    6.0,
    2.16650390625,
    4.9169921875,
    {
      3,
      CAMTA_MODE_HORNER,
      -0.0458984375,
       0.2880859375,
       0.48828125,
       0.0009765625
    },
    {
      3,
      CAMTA_MODE_HORNER,
      -0.00048828125,
      -0.0107421875,
       1.16357421875,
      -0.5224609375
    },
    {
      1,
      CAMTA_MODE_HORNER,
       0.0,
       0.0,
       1.01904296875,
      -0.1279296875
    },
    SymMode::RESIDUAL,
    ref_swish
  });
  /*
   * EXP [-8, 1]
   * Host-aligned rank-1 candidate
   * Symmetry: NONE
   * Modes: H/H/H
   * Degrees: 3/3/3
   */
  tests.push_back({
    "exp",
    "exp_m8_p1_host_aligned_rank1",
    N,
    -8.0,
    1.0,
    -3.0,
    -0.5625,
    {
      3,
      CAMTA_MODE_HORNER,
       0.0009765625,
       0.01904296875,
       0.12451171875,
       0.27587890625
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.0302734375,
       0.255859375,
       0.79052734375,
       0.935546875
    },
    {
      3,
      CAMTA_MODE_HORNER,
       0.21435546875,
       0.51123046875,
       0.990234375,
       1.0
    },
    SymMode::NONE,
    ref_exp
  });
  /*
   * -------------------------------------------------------------------------
   * Functional mode-demonstration cases
   * -------------------------------------------------------------------------
   */
  /*
   * ReLU [-4, 4]
   *
   * Range-aligned case for comparison with previous work.
   *
   * ReLU(x) = 0 for x < 0, and x for x >= 0.
   *
   * Symmetry: NONE
   * L_left = 0, L_right = 1
   *
   * Region behavior:
   *   R0: x < 0       -> ZERO
   *   R1: 0 <= x <= 1 -> IDENTITY
   *   R2: x > 1       -> IDENTITY
   */
  tests.push_back({
    "relu",
    "relu_m4_p4_zero_identity_identity",
    N,
    -4.0,
    4.0,
    0.0,
    1.0,
    {
      0,
      CAMTA_MODE_ZERO,
      0.0,
      0.0,
      0.0,
      0.0
    },
    {
      1,
      CAMTA_MODE_IDENTITY,
      0.0,
      0.0,
      1.0,
      0.0
    },
    {
      1,
      CAMTA_MODE_IDENTITY,
      0.0,
      0.0,
      1.0,
      0.0
    },
    SymMode::NONE,
    ref_relu
  });
  /*
   * ReLU [-8, 8]
   *
   * ReLU(x) = 0 for x < 0, and x for x >= 0.
   *
   * Symmetry: NONE
   * L_left = 0, L_right = 1
   *
   * Region behavior:
   *   R0: x < 0       -> ZERO
   *   R1: 0 <= x <= 1 -> IDENTITY
   *   R2: x > 1       -> IDENTITY
   *
   * This case directly exercises the ZERO and IDENTITY bypass modes.
   */
  tests.push_back({
    "relu",
    "relu_m8_p8_zero_identity_identity",
    N,
    -8.0,
    8.0,
    0.0,
    1.0,
    {
      0,
      CAMTA_MODE_ZERO,
      0.0,
      0.0,
      0.0,
      0.0
    },
    {
      1,
      CAMTA_MODE_IDENTITY,
      0.0,
      0.0,
      1.0,
      0.0
    },
    {
      1,
      CAMTA_MODE_IDENTITY,
      0.0,
      0.0,
      1.0,
      0.0
    },
    SymMode::NONE,
    ref_relu
  });
  /*
   * HardTanh [-8, 8]
   *
   * HardTanh(x) = -1 for x < -1,
   *                x  for -1 <= x <= 1,
   *               +1 for x > 1.
   *
   * Symmetry: NONE
   * L_left = -1, L_right = +1
   *
   * Region behavior:
   *   R0: x < -1       -> CONST(-1)
   *   R1: -1 <= x <= 1 -> IDENTITY
   *   R2: x > 1        -> CONST(+1)
   *
   * This case directly exercises the CONST and IDENTITY bypass modes.
   */
  tests.push_back({
    "hardtanh",
    "hardtanh_m8_p8_const_identity_const",
    N,
    -8.0,
    8.0,
    -1.0,
    1.0,
    {
      0,
      CAMTA_MODE_CONST,
      0.0,
      0.0,
      0.0,
      -1.0
    },
    {
      1,
      CAMTA_MODE_IDENTITY,
      0.0,
      0.0,
      1.0,
      0.0
    },
    {
      0,
      CAMTA_MODE_CONST,
      0.0,
      0.0,
      0.0,
      1.0
    },
    SymMode::NONE,
    ref_hardtanh
  });
  /*
   * EVEN symmetry sanity check.
   * Not part of the nonlinear-function optimization comparison.
   */
  tests.push_back({
    "square",
    "square_even_sanity",
    N,
    -3.8,
    3.8,
    100.0,
    101.0,
    {
      2,
      CAMTA_MODE_HORNER,
      0.0,
      1.0,
      0.0,
      0.0
    },
    {
      2,
      CAMTA_MODE_HORNER,
      0.0,
      1.0,
      0.0,
      0.0
    },
    {
      2,
      CAMTA_MODE_HORNER,
      0.0,
      1.0,
      0.0,
      0.0
    },
    SymMode::EVEN,
    ref_square
  });
  return tests;
}
static void print_usage(const char* argv0) {
  std::cout
    << "Usage:\n"
    << "  " << argv0 << " [xclbin_path] [options]\n\n"
    << "Options:\n"
    << "  --xclbin <path>   Path to xclbin. "
       "Default: ../HW/package.hw/kernels.xclbin\n"
    << "  --mode <mode>     all | functional | timing. "
       "Default: all\n"
    << "  --samples <N>     Samples per function. "
       "Default: 10000\n"
    << "  --warmup <N>      Warmup runs per function. "
       "Default: 1\n"
    << "  --runs <N>        Measured runs per function. "
       "Default: 5\n"
    << "  --no-exhaustive   Skip the exhaustive 65,536-code Q5.11 "
       "numerical validation.\n"
    << "  --help            Print this help.\n";
}
// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
  try {
    std::string binaryFile =
      "../HW/package.hw/kernels.xclbin";
    std::string mode = "all";
    int N = 10000;
    int warmup_runs = 1;
    int measured_runs = 5;
    bool run_exhaustive = false;
    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "--help" || arg == "-h") {
        print_usage(argv[0]);
        return 0;
      }
      else if (arg == "--xclbin" && i + 1 < argc) {
        binaryFile = argv[++i];
      }
      else if (arg == "--mode" && i + 1 < argc) {
        mode = argv[++i];
      }
      else if (arg == "--samples" && i + 1 < argc) {
        N = parse_int_arg(argv[++i], "--samples");
      }
      else if (arg == "--warmup" && i + 1 < argc) {
        warmup_runs =
          parse_int_arg(argv[++i], "--warmup");
      }
      else if (arg == "--runs" && i + 1 < argc) {
        measured_runs =
          parse_int_arg(argv[++i], "--runs");
      }
      else if (arg == "--no-exhaustive") {
        run_exhaustive = false;
      }
      else if (arg == "-all") {
        mode = "all";
      }
      else if (arg == "-timing") {
        mode = "timing";
      }
      else if (arg == "-functional") {
        mode = "functional";
      }
      else {
        binaryFile = arg;
      }
    }
    if (!(mode == "all" ||
          mode == "functional" ||
          mode == "timing")) {
      throw std::runtime_error(
        "Invalid --mode. Use: all, functional, or timing."
      );
    }
    const bool do_functional =
      (mode == "functional" || mode == "all");
    const bool do_timing =
      (mode == "timing" || mode == "all");
    const std::vector<TestCase> tests =
      build_tests(N);
    std::cout
      << std::fixed
      << std::setprecision(9);
    std::cout
      << "CAMTA_ALVEO_COMPLETE_ALL_RANGES_AND_MODES_BENCHMARK\n";
    std::cout
      << "xclbin=" << binaryFile << "\n";
    std::cout
      << "mode=" << mode << "\n";
    std::cout
      << "samples_per_function=" << N << "\n";
    std::cout
      << "warmup_runs=" << warmup_runs << "\n";
    std::cout
      << "measured_runs=" << measured_runs << "\n";
    std::cout
      << "exhaustive_q511_validation="
      << (run_exhaustive ? "enabled" : "disabled")
      << "\n";
    std::cout
      << "selection=all_validated_ranges_plus_relu_m4_p4_m8_p8_hardtanh_mode_demonstrations\n";
    std::cout
      << "data_format=Q5.11\n";
    std::cout
      << "symmetry_supported=NONE,ODD,COMPLEMENT,RESIDUAL,EVEN\n";
    std::cout
      << "note_cfg_write_ms=runtime XRT argument configuration overhead, "
         "not xclbin programming\n";
    std::cout
      << "note_host_sends_original_x=1\n\n";
    // FPGA bitstream programming / xclbin loading
    auto tprog0 =
      std::chrono::high_resolution_clock::now();
    auto device = xrt::device(0);
    auto uuid =
      device.load_xclbin(binaryFile);
    auto kernel =
      xrt::kernel(device, uuid, "camta");
    auto tprog1 =
      std::chrono::high_resolution_clock::now();
    const double xclbin_program_time_ms =
      std::chrono::duration<double, std::milli>(
        tprog1 - tprog0
      ).count();
    std::cout
      << "xclbin_program_time_ms="
      << xclbin_program_time_ms
      << "\n\n";
    auto bo_x_in =
      xrt::bo(
        device,
        N * sizeof(data_t),
        kernel.group_id(0)
      );
    auto bo_y_out =
      xrt::bo(
        device,
        N * sizeof(data_t),
        kernel.group_id(1)
      );
    auto x_buf =
      bo_x_in.map<data_t*>();
    auto y_buf =
      bo_y_out.map<data_t*>();
    std::vector<SummaryRow> summary;
    for (const auto& tc : tests) {
      // Warmup runs
      for (int i = 0; i < warmup_runs; ++i) {
        (void)run_test(
          kernel,
          bo_x_in,
          bo_y_out,
          x_buf,
          y_buf,
          tc,
          do_functional
        );
      }
      std::vector<double> prepare_list;
      std::vector<double> h2d_list;
      std::vector<double> cfg_list;
      std::vector<double> kernel_list;
      std::vector<double> d2h_list;
      std::vector<double> total_list;
      std::vector<double> mse_list;
      std::vector<double> rmse_list;
      std::vector<double> mae_list;
      std::vector<double> maxerr_list;
      std::vector<double> sqnr_list;
      for (int i = 0; i < measured_runs; ++i) {
        Metrics m =
          run_test(
            kernel,
            bo_x_in,
            bo_y_out,
            x_buf,
            y_buf,
            tc,
            do_functional
          );
        if (do_timing) {
          prepare_list.push_back(
            m.input_prepare_ms
          );
          h2d_list.push_back(
            m.h2d_ms
          );
          cfg_list.push_back(
            m.cfg_write_ms
          );
          kernel_list.push_back(
            m.kernel_ms
          );
          d2h_list.push_back(
            m.d2h_ms
          );
          total_list.push_back(
            m.total_device_flow_ms
          );
        }
        if (do_functional) {
          mse_list.push_back(
            m.mse
          );
          rmse_list.push_back(
            m.rmse
          );
          mae_list.push_back(
            m.mae
          );
          maxerr_list.push_back(
            m.max_abs_err
          );
          sqnr_list.push_back(
            m.sqnr_db
          );
        }
      }
      SummaryRow row;
      row.function_name =
        tc.function_name;
      row.case_name =
        tc.case_name;
      row.N =
        tc.N;
      row.xmin =
        tc.xmin;
      row.xmax =
        tc.xmax;
      row.L_left =
        tc.L_left;
      row.L_right =
        tc.L_right;
      row.sym_mode =
        tc.sym_mode;
      row.r0 =
        tc.r0;
      row.r1 =
        tc.r1;
      row.r2 =
        tc.r2;
      if (do_timing) {
        row.avg_input_prepare_ms =
          avg(prepare_list);
        row.avg_h2d_ms =
          avg(h2d_list);
        row.avg_cfg_write_ms =
          avg(cfg_list);
        row.avg_kernel_ms =
          avg(kernel_list);
        row.avg_d2h_ms =
          avg(d2h_list);
        row.avg_total_device_flow_ms =
          avg(total_list);
        row.avg_kernel_us =
          row.avg_kernel_ms * 1000.0;
        row.ns_per_sample =
          (row.N > 0)
            ? (row.avg_kernel_ms * 1e6 /
               static_cast<double>(row.N))
            : 0.0;
        row.cycles_at_250MHz =
          row.avg_kernel_ms *
          1e-3 *
          250e6;
        row.throughput_samples_per_sec =
          (row.avg_kernel_ms > 0.0)
            ? (
                static_cast<double>(row.N) /
                (row.avg_kernel_ms * 1e-3)
              )
            : 0.0;
        row.cfg_over_kernel_pct =
          (row.avg_kernel_ms > 0.0)
            ? (
                100.0 *
                row.avg_cfg_write_ms /
                row.avg_kernel_ms
              )
            : 0.0;
      }
      if (do_functional) {
        row.avg_mse =
          avg(mse_list);
        row.avg_rmse =
          avg(rmse_list);
        row.avg_mae =
          avg(mae_list);
        row.avg_max_abs_err =
          avg(maxerr_list);
        row.avg_sqnr_db =
          avg(sqnr_list);
      }
      summary.push_back(row);
      // -----------------------------------------------------------------------
      // Per-function output
      // -----------------------------------------------------------------------
      std::cout
        << "========================================\n";
      std::cout
        << "function="
        << tc.function_name
        << "\n";
      std::cout
        << "case_name="
        << tc.case_name
        << "\n";
      std::cout
        << "N="
        << tc.N
        << " xmin="
        << tc.xmin
        << " xmax="
        << tc.xmax
        << " L_left="
        << tc.L_left
        << " L_right="
        << tc.L_right
        << " sym_mode="
        << sym_mode_name(tc.sym_mode)
        << "\n";
      std::cout
        << "region_modes="
        << region_mode_name(tc.r0.mode)
        << "/"
        << region_mode_name(tc.r1.mode)
        << "/"
        << region_mode_name(tc.r2.mode)
        << "\n";
      std::cout
        << "degrees="
        << tc.r0.deg
        << "/"
        << tc.r1.deg
        << "/"
        << tc.r2.deg
        << "\n";
      std::cout
        << "r0_coeffs_a3_a2_a1_a0="
        << tc.r0.a3.to_double() << ","
        << tc.r0.a2.to_double() << ","
        << tc.r0.a1.to_double() << ","
        << tc.r0.a0.to_double()
        << "\n";
      std::cout
        << "r1_coeffs_a3_a2_a1_a0="
        << tc.r1.a3.to_double() << ","
        << tc.r1.a2.to_double() << ","
        << tc.r1.a1.to_double() << ","
        << tc.r1.a0.to_double()
        << "\n";
      std::cout
        << "r2_coeffs_a3_a2_a1_a0="
        << tc.r2.a3.to_double() << ","
        << tc.r2.a2.to_double() << ","
        << tc.r2.a1.to_double() << ","
        << tc.r2.a0.to_double()
        << "\n";
      if (do_timing) {
        std::cout
          << "avg_input_prepare_ms="
          << row.avg_input_prepare_ms
          << "\n";
        std::cout
          << "avg_h2d_ms="
          << row.avg_h2d_ms
          << "\n";
        std::cout
          << "avg_cfg_write_ms="
          << row.avg_cfg_write_ms
          << "\n";
        std::cout
          << "avg_kernel_ms="
          << row.avg_kernel_ms
          << "\n";
        std::cout
          << "avg_kernel_us="
          << row.avg_kernel_us
          << "\n";
        std::cout
          << "ns_per_sample="
          << row.ns_per_sample
          << "\n";
        std::cout
          << "cycles_at_250MHz="
          << row.cycles_at_250MHz
          << "\n";
        std::cout
          << "avg_d2h_ms="
          << row.avg_d2h_ms
          << "\n";
        std::cout
          << "avg_total_device_flow_ms="
          << row.avg_total_device_flow_ms
          << "\n";
        std::cout
          << "throughput_samples_per_sec="
          << row.throughput_samples_per_sec
          << "\n";
        std::cout
          << "cfg_over_kernel_pct="
          << row.cfg_over_kernel_pct
          << "\n";
      }
      if (do_functional) {
        std::cout
          << "avg_mse="
          << row.avg_mse
          << "\n";
        std::cout
          << "avg_rmse="
          << row.avg_rmse
          << "\n";
        std::cout
          << "avg_mae="
          << row.avg_mae
          << "\n";
        std::cout
          << "avg_max_abs_err="
          << row.avg_max_abs_err
          << "\n";
        std::cout
          << "avg_sqnr_db="
          << row.avg_sqnr_db
          << "\n";
      }
      std::cout << "\n";
    }
    // -------------------------------------------------------------------------
    // Final machine-readable summary
    // -------------------------------------------------------------------------
    std::cout
      << "========================================\n";
    std::cout
      << "FINAL_SUMMARY\n";
    std::cout
      << "Function"
      << "\tCase"
      << "\tN"
      << "\tXmin"
      << "\tXmax"
      << "\tL_left"
      << "\tL_right"
      << "\tSymMode"
      << "\tRegionModes"
      << "\tDegrees"
      << "\tR0_a3"
      << "\tR0_a2"
      << "\tR0_a1"
      << "\tR0_a0"
      << "\tR1_a3"
      << "\tR1_a2"
      << "\tR1_a1"
      << "\tR1_a0"
      << "\tR2_a3"
      << "\tR2_a2"
      << "\tR2_a1"
      << "\tR2_a0";
    if (do_timing) {
      std::cout
        << "\tCfgWrite_ms"
        << "\tH2D_ms"
        << "\tKernel_ms"
        << "\tKernel_us"
        << "\tNsPerSample"
        << "\tCycles250MHz"
        << "\tD2H_ms"
        << "\tTotalDeviceFlow_ms"
        << "\tThroughput_sps"
        << "\tCfgOverKernel_pct";
    }
    if (do_functional) {
      std::cout
        << "\tMSE"
        << "\tRMSE"
        << "\tMAE"
        << "\tMaxAbsErr"
        << "\tSQNR_dB";
    }
    std::cout << "\n";
    for (const auto& row : summary) {
      std::cout
        << row.function_name
        << "\t"
        << row.case_name
        << "\t"
        << row.N
        << "\t"
        << row.xmin
        << "\t"
        << row.xmax
        << "\t"
        << row.L_left
        << "\t"
        << row.L_right
        << "\t"
        << sym_mode_name(row.sym_mode)
        << "\t"
        << region_mode_name(row.r0.mode)
        << "/"
        << region_mode_name(row.r1.mode)
        << "/"
        << region_mode_name(row.r2.mode)
        << "\t"
        << row.r0.deg
        << "/"
        << row.r1.deg
        << "/"
        << row.r2.deg
        << "\t"
        << row.r0.a3.to_double()
        << "\t"
        << row.r0.a2.to_double()
        << "\t"
        << row.r0.a1.to_double()
        << "\t"
        << row.r0.a0.to_double()
        << "\t"
        << row.r1.a3.to_double()
        << "\t"
        << row.r1.a2.to_double()
        << "\t"
        << row.r1.a1.to_double()
        << "\t"
        << row.r1.a0.to_double()
        << "\t"
        << row.r2.a3.to_double()
        << "\t"
        << row.r2.a2.to_double()
        << "\t"
        << row.r2.a1.to_double()
        << "\t"
        << row.r2.a0.to_double();
      if (do_timing) {
        std::cout
          << "\t"
          << row.avg_cfg_write_ms
          << "\t"
          << row.avg_h2d_ms
          << "\t"
          << row.avg_kernel_ms
          << "\t"
          << row.avg_kernel_us
          << "\t"
          << row.ns_per_sample
          << "\t"
          << row.cycles_at_250MHz
          << "\t"
          << row.avg_d2h_ms
          << "\t"
          << row.avg_total_device_flow_ms
          << "\t"
          << row.throughput_samples_per_sec
          << "\t"
          << row.cfg_over_kernel_pct;
      }
      if (do_functional) {
        std::cout
          << "\t"
          << row.avg_mse
          << "\t"
          << row.avg_rmse
          << "\t"
          << row.avg_mae
          << "\t"
          << row.avg_max_abs_err
          << "\t"
          << row.avg_sqnr_db;
      }
      std::cout << "\n";
    }
    std::cout
      << "========================================\n";
    // -------------------------------------------------------------------------
    // Exhaustive numerical validation over every Q5.11 input code
    // -------------------------------------------------------------------------
    if (run_exhaustive) {
      constexpr int EXHAUSTIVE_N = 1 << 16;
      std::cout
        << "FULL_Q511_STRESS_TEST\n";
      std::cout
        << "samples_per_function="
        << EXHAUSTIVE_N
        << "\n";
      std::cout
        << "full_q511_range=[-16,15.999511719]\n";
      std::cout
        << "note=All 65,536 Q5.11 codes are sent to the hardware. "
           "For magnitude-based symmetry modes, only 0x8000 (-16.0) is "
           "excluded from aggregate accuracy metrics because +16 cannot be "
           "represented in Q5.11. For NONE symmetry, all 65,536 codes are "
           "included. Function configurations remain unchanged.\n\n";
      auto bo_x_exhaustive =
        xrt::bo(
          device,
          EXHAUSTIVE_N * sizeof(data_t),
          kernel.group_id(0)
        );
      auto bo_y_exhaustive =
        xrt::bo(
          device,
          EXHAUSTIVE_N * sizeof(data_t),
          kernel.group_id(1)
        );
      auto x_exhaustive_buf =
        bo_x_exhaustive.map<data_t*>();
      auto y_exhaustive_buf =
        bo_y_exhaustive.map<data_t*>();
      const std::vector<TestCase> exhaustive_tests =
        build_tests(EXHAUSTIVE_N);
      struct ExhaustiveRow {
        TestCase tc;
        ExhaustiveMetrics metrics;
      };
      std::vector<ExhaustiveRow> exhaustive_summary;
      for (const auto& tc : exhaustive_tests) {
        // Square is a symmetry sanity check, not one of the target nonlinear
        // functions requested for the exhaustive numerical validation.
        if (tc.function_name == "square") {
          continue;
        }
        const ExhaustiveMetrics m =
          run_exhaustive_validation(
            kernel,
            bo_x_exhaustive,
            bo_y_exhaustive,
            x_exhaustive_buf,
            y_exhaustive_buf,
            tc
          );
        exhaustive_summary.push_back({tc, m});
        std::cout
          << "----------------------------------------\n";
        std::cout
          << "function="
          << tc.function_name
          << "\n";
        std::cout
          << "nominal_benchmark_range=["
          << tc.xmin
          << ","
          << tc.xmax
          << "]\n";
        std::cout
          << "exhaustive_input_range=[-16,15.999511719]\n";
        std::cout
          << "exhaustive_codes_sent="
          << m.total_codes_sent
          << "\n";
        std::cout
          << "exhaustive_codes_included_in_metrics="
          << m.sample_count
          << "\n";
        std::cout
          << "exhaustive_codes_excluded_from_metrics="
          << m.excluded_code_count
          << "\n";
        std::cout
          << "exhaustive_mae="
          << m.mae
          << "\n";
        std::cout
          << "exhaustive_rmse="
          << m.rmse
          << "\n";
        std::cout
          << "exhaustive_max_abs_err="
          << m.max_abs_err
          << "\n";
        std::cout
          << "exhaustive_x_at_max_abs_err="
          << m.x_at_max_abs_err
          << "\n";
        std::cout
          << "exhaustive_signed_code_at_max_abs_err="
          << m.signed_code_at_max_abs_err
          << "\n";
        std::cout
          << "exhaustive_raw_code_at_max_abs_err="
          << static_cast<unsigned int>(m.raw_code_at_max_abs_err)
          << "\n";
        std::cout
          << "exhaustive_y_hw_at_max_abs_err="
          << m.y_hw_at_max_abs_err
          << "\n";
        std::cout
          << "exhaustive_y_ref_at_max_abs_err="
          << m.y_ref_at_max_abs_err
          << "\n";
        // Explicit Q5.11 minimum-value check. The raw code is 0x8000 and the
        // represented value is exactly -16.0. Its mathematical magnitude +16
        // is outside the positive Q5.11 range.
        std::cout
          << "MIN_Q511_CHECK\n";
        std::cout
          << "min_q511_raw_code=0x8000\n";
        std::cout
          << "min_q511_input="
          << m.min_q511_input
          << "\n";
        std::cout
          << "min_q511_positive_magnitude_representable=0\n";
        std::cout
          << "min_q511_symmetry_requires_magnitude="
          << (m.min_q511_requires_magnitude ? 1 : 0)
          << "\n";
        std::cout
          << "min_q511_included_in_accuracy_metrics="
          << (m.min_q511_included_in_metrics ? 1 : 0)
          << "\n";
        std::cout
          << "min_q511_y_hw="
          << m.min_q511_y_hw
          << "\n";
        std::cout
          << "min_q511_y_ref="
          << m.min_q511_y_ref
          << "\n";
        std::cout
          << "min_q511_abs_err="
          << m.min_q511_abs_err
          << "\n\n";
      }
      std::cout
        << "EXHAUSTIVE_SUMMARY\n";
      std::cout
        << "Function"
        << "\tNominalXmin"
        << "\tNominalXmax"
        << "\tCodesSent"
        << "\tCodesIncluded"
        << "\tCodesExcluded"
        << "\tMAE"
        << "\tRMSE"
        << "\tMaxAbsErr"
        << "\tXAtMaxAbsErr"
        << "\tSignedCodeAtMax"
        << "\tRawCodeAtMax"
        << "\tYhwAtMax"
        << "\tYrefAtMax"
        << "\tMinQ511Yhw"
        << "\tMinQ511Yref"
        << "\tMinQ511AbsErr"
        << "\tMinQ511RequiresMagnitude"
        << "\tMinQ511IncludedInMetrics"
        << "\n";
      for (const auto& row : exhaustive_summary) {
        const auto& tc = row.tc;
        const auto& m = row.metrics;
        std::cout
          << tc.function_name
          << "\t"
          << tc.xmin
          << "\t"
          << tc.xmax
          << "\t"
          << m.total_codes_sent
          << "\t"
          << m.sample_count
          << "\t"
          << m.excluded_code_count
          << "\t"
          << m.mae
          << "\t"
          << m.rmse
          << "\t"
          << m.max_abs_err
          << "\t"
          << m.x_at_max_abs_err
          << "\t"
          << m.signed_code_at_max_abs_err
          << "\t"
          << static_cast<unsigned int>(m.raw_code_at_max_abs_err)
          << "\t"
          << m.y_hw_at_max_abs_err
          << "\t"
          << m.y_ref_at_max_abs_err
          << "\t"
          << m.min_q511_y_hw
          << "\t"
          << m.min_q511_y_ref
          << "\t"
          << m.min_q511_abs_err
          << "\t"
          << (m.min_q511_requires_magnitude ? 1 : 0)
          << "\t"
          << (m.min_q511_included_in_metrics ? 1 : 0)
          << "\n";
      }
      std::cout
        << "========================================\n";
    }
    std::cout
      << "INTERPRETATION_NOTES\n";
    std::cout
      << "xclbin_program_time_ms measures device programming/loading "
         "of the bitstream.\n";
    std::cout
      << "CfgWrite_ms approximates runtime parameter configuration "
         "overhead through XRT set_arg calls.\n";
    std::cout
      << "Kernel_ms measures kernel start + wait time for one CAMTA "
         "execution over N samples.\n";
    std::cout
      << "The host sends original x values. Symmetry/reconstruction "
         "is handled inside CAMTA.\n";
    std::cout
      << "The nonlinear approximation cases use the validated runtime "
         "configurations reported by the offline search.\n";
    std::cout
      << "ReLU is included as a functional demonstration of ZERO and "
         "IDENTITY region modes.\n";
    std::cout
      << "HardTanh is included as a functional demonstration of CONST and "
         "IDENTITY region modes.\n";
    std::cout
      << "Square is included only as an EVEN-symmetry sanity check.\n";
    std::cout
      << "The exhaustive validation is a separate numerical test and does "
         "not replace the 10,000-sample performance benchmark.\n";
    std::cout
      << "The exhaustive validation keeps the same function configurations "
         "and sends all 65,536 Q5.11 input codes to the hardware.\n";
    std::cout
      << "For symmetry modes that require input magnitude, Q5.11 value -16 "
         "(raw code 0x8000) is exercised and reported separately but excluded "
         "from MAE, RMSE, and MaxAbsErr because +16 cannot be represented in "
         "the same Q5.11 format.\n";
    std::cout
      << "For NONE symmetry, no Q5.11 input code is excluded from the "
         "exhaustive numerical metrics.\n";
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr
      << "ERROR: "
      << e.what()
      << "\n";
    return 1;
  }
}
