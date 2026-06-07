#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cmath>
#include <functional>
#include <numeric>
#include <algorithm>
#include <stdexcept>

#include "experimental/xrt_bo.h"
#include "experimental/xrt_device.h"
#include "experimental/xrt_kernel.h"

#include "../HW/camta_types.h"

enum class SymMode {
  NONE = 0,
  ODD = 1,
  COMPLEMENT = 2,
  RESIDUAL = 3
};

struct RegionPoly {
  int deg;
  int mode;
  data_t a3, a2, a1, a0;
};

struct TestCase {
  std::string name;
  int N;
  double xmin;
  double xmax;
  double L_left;
  double L_right;
  RegionPoly r0, r1, r2;
  std::function<double(double)> ref_fn;
  SymMode sym_mode = SymMode::NONE;
};

struct SoftmaxCase {
  std::string name;
  int N;
  double xmin;
  double xmax;
  double L_left;
  double L_right;
  RegionPoly r0, r1, r2;
};

struct Metrics {
  double cfg_write_ms = 0.0;
  double h2d_ms = 0.0;
  double kernel_ms = 0.0;
  double d2h_ms = 0.0;
  double total_ms = 0.0;

  double mse = 0.0;
  double rmse = 0.0;
  double mae = 0.0;
  double max_abs_err = 0.0;
};

struct SummaryRow {
  std::string name;
  int N = 0;
  double xmin = 0.0;
  double xmax = 0.0;
  double L_left = 0.0;
  double L_right = 0.0;
  SymMode sym_mode = SymMode::NONE;

  double avg_cfg_write_ms = 0.0;
  double avg_h2d_ms = 0.0;
  double avg_kernel_ms = 0.0;
  double avg_d2h_ms = 0.0;
  double avg_total_ms = 0.0;
  double throughput_samples_per_sec = 0.0;
  double cfg_over_kernel_pct = 0.0;

  double avg_kernel_us = 0.0;
  double ns_per_sample = 0.0;
  double cycles_at_250MHz = 0.0;

  double avg_mse = 0.0;
  double avg_rmse = 0.0;
  double avg_mae = 0.0;
  double avg_max_abs_err = 0.0;
};

struct SoftmaxSummaryRow {
  std::string name;
  int N = 0;
  double xmin = 0.0;
  double xmax = 0.0;

  double avg_cfg_write_ms = 0.0;
  double avg_h2d_ms = 0.0;
  double avg_kernel_ms = 0.0;
  double avg_d2h_ms = 0.0;
  double avg_total_ms = 0.0;
  double throughput_samples_per_sec = 0.0;
  double avg_kernel_us = 0.0;
  double ns_per_sample = 0.0;

  double avg_mse = 0.0;
  double avg_rmse = 0.0;
  double avg_mae = 0.0;
  double avg_max_abs_err = 0.0;
};

static const char* sym_mode_name(SymMode mode)
{
  switch (mode) {
    case SymMode::NONE:       return "none";
    case SymMode::ODD:        return "odd";
    case SymMode::COMPLEMENT: return "complement";
    case SymMode::RESIDUAL:   return "residual";
    default:                  return "unknown";
  }
}

static const char* region_mode_name(int mode)
{
  switch (mode) {
    case CAMTA_MODE_HORNER:   return "horner";
    case CAMTA_MODE_CONST:    return "const";
    case CAMTA_MODE_ZERO:     return "zero";
    case CAMTA_MODE_IDENTITY: return "identity";
    default:                  return "unknown";
  }
}

static double ref_gelu(double x)
{
  const double pi = 3.14159265358979323846;
  const double c = std::sqrt(2.0 / pi);
  return 0.5 * x * (1.0 + std::tanh(c * (x + 0.044715 * x * x * x)));
}

static double ref_tanh(double x)
{
  return std::tanh(x);
}

static double ref_sigmoid(double x)
{
  return 1.0 / (1.0 + std::exp(-x));
}

static double ref_swish(double x)
{
  return x / (1.0 + std::exp(-x));
}

static double host_input_transform(double x, SymMode mode)
{
  if (mode == SymMode::NONE) {
    return x;
  }

  return std::abs(x);
}

static double host_output_transform(double x_original, double y_kernel, SymMode mode)
{
  if (mode == SymMode::NONE) {
    return y_kernel;
  }

  const double a = std::abs(x_original);

  if (mode == SymMode::ODD) {
    return (x_original < 0.0) ? -y_kernel : y_kernel;
  }

  if (mode == SymMode::COMPLEMENT) {
    return (x_original < 0.0) ? (1.0 - y_kernel) : y_kernel;
  }

  if (mode == SymMode::RESIDUAL) {
    return (x_original < 0.0) ? (y_kernel - a) : y_kernel;
  }

  return y_kernel;
}

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

  if (tc.N <= 0) {
    throw std::runtime_error("Invalid test case: N must be positive.");
  }

  if (tc.L_left >= tc.L_right) {
    throw std::runtime_error("Invalid test case: L_left must be smaller than L_right.");
  }

  std::vector<double> x_original(tc.N);

  auto t_total_0 = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < tc.N; ++i) {
    double x = tc.xmin + (tc.xmax - tc.xmin) * (double)i / (double)(tc.N - 1);

    x_original[i] = x;

    double x_for_kernel = host_input_transform(x, tc.sym_mode);

    x_buf[i] = static_cast<data_t>(x_for_kernel);
    y_buf[i] = static_cast<data_t>(0);
  }

  auto t_h2d_0 = std::chrono::high_resolution_clock::now();
  bo_x_in.sync(XCL_BO_SYNC_BO_TO_DEVICE);
  auto t_h2d_1 = std::chrono::high_resolution_clock::now();

  auto t_cfg_0 = std::chrono::high_resolution_clock::now();

  auto run = kernel(
    bo_x_in,
    bo_y_out,
    tc.N,
    static_cast<data_t>(tc.L_left),
    static_cast<data_t>(tc.L_right),
    tc.r0.deg,
    tc.r1.deg,
    tc.r2.deg,
    tc.r0.mode,
    tc.r1.mode,
    tc.r2.mode,
    tc.r0.a3, tc.r0.a2, tc.r0.a1, tc.r0.a0,
    tc.r1.a3, tc.r1.a2, tc.r1.a1, tc.r1.a0,
    tc.r2.a3, tc.r2.a2, tc.r2.a1, tc.r2.a0
  );

  auto t_cfg_1 = std::chrono::high_resolution_clock::now();

  auto t_k_0 = std::chrono::high_resolution_clock::now();
  run.wait();
  auto t_k_1 = std::chrono::high_resolution_clock::now();

  auto t_d2h_0 = std::chrono::high_resolution_clock::now();
  bo_y_out.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  auto t_d2h_1 = std::chrono::high_resolution_clock::now();

  if (compute_error) {
    double mse = 0.0;
    double mae = 0.0;
    double max_abs_err = 0.0;

    for (int i = 0; i < tc.N; ++i) {
      double x_ref = x_original[i];
      double y_raw = static_cast<double>(y_buf[i]);
      double y_hw = host_output_transform(x_ref, y_raw, tc.sym_mode);
      double y_ref = tc.ref_fn(x_ref);

      double err = y_hw - y_ref;
      double abs_err = std::abs(err);

      mse += err * err;
      mae += abs_err;

      if (abs_err > max_abs_err) {
        max_abs_err = abs_err;
      }
    }

    mse /= (double)tc.N;
    mae /= (double)tc.N;

    m.mse = mse;
    m.rmse = std::sqrt(mse);
    m.mae = mae;
    m.max_abs_err = max_abs_err;
  }

  auto t_total_1 = std::chrono::high_resolution_clock::now();

  m.cfg_write_ms =
    std::chrono::duration<double, std::milli>(t_cfg_1 - t_cfg_0).count();
  m.h2d_ms =
    std::chrono::duration<double, std::milli>(t_h2d_1 - t_h2d_0).count();
  m.kernel_ms =
    std::chrono::duration<double, std::milli>(t_k_1 - t_k_0).count();
  m.d2h_ms =
    std::chrono::duration<double, std::milli>(t_d2h_1 - t_d2h_0).count();
  m.total_ms =
    std::chrono::duration<double, std::milli>(t_total_1 - t_total_0).count();

  return m;
}

static Metrics run_softmax_test(
  xrt::kernel& kernel,
  xrt::bo& bo_x_in,
  xrt::bo& bo_y_out,
  data_t* x_buf,
  data_t* y_buf,
  const SoftmaxCase& tc,
  bool compute_error
) {
  Metrics m;

  if (tc.N <= 0) {
    throw std::runtime_error("Invalid softmax case: N must be positive.");
  }

  if (tc.L_left >= tc.L_right) {
    throw std::runtime_error("Invalid softmax case: L_left must be smaller than L_right.");
  }

  std::vector<double> logits(tc.N);
  std::vector<double> ref_exp(tc.N);
  std::vector<double> hw_exp(tc.N);
  std::vector<double> ref_softmax(tc.N);
  std::vector<double> hw_softmax(tc.N);

  auto t_total_0 = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < tc.N; ++i) {
    double x = tc.xmin + (tc.xmax - tc.xmin) * (double)i / (double)(tc.N - 1);

    logits[i] = x;
    x_buf[i] = static_cast<data_t>(x);
    y_buf[i] = static_cast<data_t>(0);
  }

  auto t_h2d_0 = std::chrono::high_resolution_clock::now();
  bo_x_in.sync(XCL_BO_SYNC_BO_TO_DEVICE);
  auto t_h2d_1 = std::chrono::high_resolution_clock::now();

  auto t_cfg_0 = std::chrono::high_resolution_clock::now();

  auto run = kernel(
    bo_x_in,
    bo_y_out,
    tc.N,
    static_cast<data_t>(tc.L_left),
    static_cast<data_t>(tc.L_right),
    tc.r0.deg,
    tc.r1.deg,
    tc.r2.deg,
    tc.r0.mode,
    tc.r1.mode,
    tc.r2.mode,
    tc.r0.a3, tc.r0.a2, tc.r0.a1, tc.r0.a0,
    tc.r1.a3, tc.r1.a2, tc.r1.a1, tc.r1.a0,
    tc.r2.a3, tc.r2.a2, tc.r2.a1, tc.r2.a0
  );

  auto t_cfg_1 = std::chrono::high_resolution_clock::now();

  auto t_k_0 = std::chrono::high_resolution_clock::now();
  run.wait();
  auto t_k_1 = std::chrono::high_resolution_clock::now();

  auto t_d2h_0 = std::chrono::high_resolution_clock::now();
  bo_y_out.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  auto t_d2h_1 = std::chrono::high_resolution_clock::now();

  if (compute_error) {
    double ref_sum = 0.0;
    double hw_sum = 0.0;

    for (int i = 0; i < tc.N; ++i) {
      ref_exp[i] = std::exp(logits[i]);

      double y = static_cast<double>(y_buf[i]);

      if (y < 0.0) {
        y = 0.0;
      }

      hw_exp[i] = y;

      ref_sum += ref_exp[i];
      hw_sum += hw_exp[i];
    }

    if (hw_sum <= 0.0) {
      throw std::runtime_error("Invalid softmax case: CAMTA exp approximation sum is zero.");
    }

    double mse = 0.0;
    double mae = 0.0;
    double max_abs_err = 0.0;

    for (int i = 0; i < tc.N; ++i) {
      ref_softmax[i] = ref_exp[i] / ref_sum;
      hw_softmax[i] = hw_exp[i] / hw_sum;

      double err = hw_softmax[i] - ref_softmax[i];
      double abs_err = std::abs(err);

      mse += err * err;
      mae += abs_err;

      if (abs_err > max_abs_err) {
        max_abs_err = abs_err;
      }
    }

    mse /= (double)tc.N;
    mae /= (double)tc.N;

    m.mse = mse;
    m.rmse = std::sqrt(mse);
    m.mae = mae;
    m.max_abs_err = max_abs_err;
  }

  auto t_total_1 = std::chrono::high_resolution_clock::now();

  m.cfg_write_ms =
    std::chrono::duration<double, std::milli>(t_cfg_1 - t_cfg_0).count();
  m.h2d_ms =
    std::chrono::duration<double, std::milli>(t_h2d_1 - t_h2d_0).count();
  m.kernel_ms =
    std::chrono::duration<double, std::milli>(t_k_1 - t_k_0).count();
  m.d2h_ms =
    std::chrono::duration<double, std::milli>(t_d2h_1 - t_d2h_0).count();
  m.total_ms =
    std::chrono::duration<double, std::milli>(t_total_1 - t_total_0).count();

  return m;
}

static double avg(const std::vector<double>& v)
{
  if (v.empty()) {
    return 0.0;
  }

  return std::accumulate(v.begin(), v.end(), 0.0) / (double)v.size();
}

int main(int argc, char** argv)
{
  try {
    std::string binaryFile = "../HW/package.hw/kernels.xclbin";
    std::string mode = "all";

    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];

      if (arg == "--mode" && i + 1 < argc) {
        mode = argv[++i];
      } else if (arg == "-all") {
        mode = "all";
      } else if (arg == "-timing") {
        mode = "timing";
      } else if (arg == "-functional") {
        mode = "functional";
      } else {
        binaryFile = arg;
      }
    }

    const bool do_functional = (mode == "functional" || mode == "all");
    const bool do_timing     = (mode == "timing" || mode == "all");

    const int maxN = 10000;
    const int N_COMMON = 10000;
    const int warmup_runs = 1;
    const int measured_runs = 5;

    std::vector<TestCase> tests = {
      {
        "gelu_baseline_L2p0",
        N_COMMON, -8.0, 8.0, -2.0, 2.0,
        {1, CAMTA_MODE_HORNER, 0.0, 0.0, -0.00292969, -0.01757812},
        {2, CAMTA_MODE_HORNER, 0.0, 0.26660156, 0.5, 0.02734375},
        {1, CAMTA_MODE_HORNER, 0.0, 0.0, 1.00292969, -0.01757812},
        ref_gelu
      },
      {
        "gelu_asym_Lm1p25_Lp2p0",
        N_COMMON, -8.0, 8.0, -1.25, 2.0,
        {2, CAMTA_MODE_HORNER, 0.0, -0.00488281, -0.05566406, -0.14453125},
        {3, CAMTA_MODE_HORNER, -0.04980469, 0.31835938, 0.54785156, 0.01464844},
        {1, CAMTA_MODE_HORNER, 0.0, 0.0, 1.00292969, -0.01757812},
        ref_gelu
      },
      {
        "gelu_hostsym_L1p75_L5p0",
        N_COMMON, -8.0, 8.0, 1.75, 5.0,
        {3, CAMTA_MODE_HORNER, -0.11230469, 0.46582031, 0.48632812, 0.00097656},
        {3, CAMTA_MODE_HORNER,  0.00683594, -0.08203125, 1.32324219, -0.41699219},
        {3, CAMTA_MODE_IDENTITY, 0.0, 0.0, 1.0, 0.0},
        ref_gelu,
        SymMode::RESIDUAL
      },
      {
        "gelu_hostsym_L1p5_L3p0_refit",
        N_COMMON, -8.0, 8.0, 1.5, 3.0,
        {3, CAMTA_MODE_HORNER, -0.115234375, 0.4736328125, 0.4814453125, 0.0009765625},
        {3, CAMTA_MODE_HORNER,  0.0078125,  -0.09765625,  1.3798828125, -0.478515625},
        {1, CAMTA_MODE_IDENTITY, 0.0, 0.0, 1.0, 0.0},
        ref_gelu,
        SymMode::RESIDUAL
      },

      {
        "tanh_baseline_L1p0",
        N_COMMON, -4.0, 4.0, -1.0, 1.0,
        {0, CAMTA_MODE_CONST, 0.0, 0.0, 0.0, -1.0},
        {3, CAMTA_MODE_HORNER, -0.333333, 0.0, 1.0, 0.0},
        {0, CAMTA_MODE_CONST, 0.0, 0.0, 0.0, 1.0},
        ref_tanh
      },
      {
        "tanh_opt_L1p75",
        N_COMMON, -4.0, 4.0, -1.75, 1.75,
        {0, CAMTA_MODE_CONST, 0.0, 0.0, 0.0, -0.98730469},
        {3, CAMTA_MODE_HORNER, -0.12890625, 0.0, 0.90136719, 0.0},
        {0, CAMTA_MODE_CONST, 0.0, 0.0, 0.0, 0.98730469},
        ref_tanh
      },
      {
        "tanh_hostsym_L0p75_L2p0",
        N_COMMON, -4.0, 4.0, 0.75, 2.0,
        {3, CAMTA_MODE_HORNER, -0.17578125, -0.09570312, 1.01757812, -0.00097656},
        {3, CAMTA_MODE_HORNER,  0.09472656, -0.59570312, 1.32714844, -0.06445312},
        {3, CAMTA_MODE_HORNER,  0.00781250, -0.08398438, 0.30078125, 0.63769531},
        ref_tanh,
        SymMode::ODD
      },
      {
        "tanh_hostsym_L1p25_L3p5_consttail",
        N_COMMON, -4.0, 4.0, 1.25, 3.5,
        {3, CAMTA_MODE_HORNER, -0.037109375, -0.26953125, 1.0751953125, -0.0048828125},
        {3, CAMTA_MODE_HORNER,  0.0263671875, -0.232421875, 0.69140625, 0.2998046875},
        {0, CAMTA_MODE_CONST,   0.0, 0.0, 0.0, 0.9990234375},
        ref_tanh,
        SymMode::ODD
      },

      {
        "sigmoid_baseline_L2p0",
        N_COMMON, -8.0, 8.0, -2.0, 2.0,
        {0, CAMTA_MODE_CONST, 0.0, 0.0, 0.0, 0.0},
        {1, CAMTA_MODE_HORNER, 0.0, 0.0, 0.25, 0.5},
        {0, CAMTA_MODE_CONST, 0.0, 0.0, 0.0, 1.0},
        ref_sigmoid
      },
      {
        "sigmoid_opt_L3p25",
        N_COMMON, -8.0, 8.0, -3.25, 3.25,
        {0, CAMTA_MODE_CONST, 0.0, 0.0, 0.0, 0.0078125},
        {3, CAMTA_MODE_HORNER, -0.00878906, 0.0, 0.22949219, 0.5},
        {0, CAMTA_MODE_CONST, 0.0, 0.0, 0.0, 0.9921875},
        ref_sigmoid
      },
      {
        "sigmoid_hostsym_L2p5_L4p5_consttail",
        N_COMMON, -8.0, 8.0, 2.5, 4.5,
        {3, CAMTA_MODE_HORNER, -0.001953125, -0.0341796875, 0.2685546875, 0.498046875},
        {3, CAMTA_MODE_HORNER,  0.00390625,  -0.0556640625, 0.2734375,    0.5283203125},
        {0, CAMTA_MODE_CONST,   0.0, 0.0, 0.0, 0.9970703125},
        ref_sigmoid,
        SymMode::COMPLEMENT
      },

      {
        "swish_baseline_L2p0",
        N_COMMON, -8.0, 8.0, -2.0, 2.0,
        {0, CAMTA_MODE_ZERO, 0.0, 0.0, 0.0, 0.0},
        {2, CAMTA_MODE_HORNER, 0.0, 0.25, 0.5, 0.0},
        {1, CAMTA_MODE_HORNER, 0.0, 0.0, 1.0, 0.0},
        ref_swish
      },
      {
        "swish_asym_Lm1p0_Lp3p75",
        N_COMMON, -8.0, 8.0, -1.0, 3.75,
        {2, CAMTA_MODE_HORNER, 0.0, -0.00878906, -0.12304688, -0.43261719},
        {3, CAMTA_MODE_HORNER, -0.03222656, 0.24023438, 0.52539062, 0.00097656},
        {1, CAMTA_MODE_HORNER, 0.0, 0.0, 1.01757812, -0.12597656},
        ref_swish
      },
      {
        "swish_hostsym_L2p0_L6p5_refit",
        N_COMMON, -8.0, 8.0, 2.0, 6.5,
        {3, CAMTA_MODE_HORNER, -0.0458984375, 0.2890625, 0.4873046875, 0.0009765625},
        {3, CAMTA_MODE_HORNER,  0.0009765625, -0.0263671875, 1.2158203125, -0.576171875},
        {1, CAMTA_MODE_IDENTITY, 0.0, 0.0, 1.0, 0.0},
        ref_swish,
        SymMode::RESIDUAL
      }
    };

    std::vector<SoftmaxCase> softmax_tests = {
      {
        "softmax_exp_camta_N10000_range_m8_0",
        N_COMMON, -8.0, 0.0,
        -4.0, -1.0,
        {0, CAMTA_MODE_CONST, 0.0, 0.0, 0.0, 0.0048828125},
        {3, CAMTA_MODE_HORNER, 0.015625, 0.1640625, 0.611328125, 0.826171875},
        {3, CAMTA_MODE_HORNER, 0.1025390625, 0.462890625, 0.9921875, 1.0}
      }
    };

    auto tprog0 = std::chrono::high_resolution_clock::now();
    auto device = xrt::device(0);
    auto uuid = device.load_xclbin(binaryFile);
    auto kernel = xrt::kernel(device, uuid, "camta");
    auto tprog1 = std::chrono::high_resolution_clock::now();

    double xclbin_program_time_ms =
      std::chrono::duration<double, std::milli>(tprog1 - tprog0).count();

    auto bo_x_in  = xrt::bo(device, maxN * sizeof(data_t), kernel.group_id(0));
    auto bo_y_out = xrt::bo(device, maxN * sizeof(data_t), kernel.group_id(1));

    auto x_buf = bo_x_in.map<data_t*>();
    auto y_buf = bo_y_out.map<data_t*>();

    std::vector<SummaryRow> summary;
    std::vector<SoftmaxSummaryRow> softmax_summary;

    std::cout << std::fixed << std::setprecision(9);
    std::cout << "xclbin_program_time_ms=" << xclbin_program_time_ms << "\n";
    std::cout << "mode=" << mode << "\n";
    std::cout << "warmup_runs=" << warmup_runs << "\n";
    std::cout << "measured_runs=" << measured_runs << "\n\n";

    for (const auto& tc : tests) {
      for (int i = 0; i < warmup_runs; ++i) {
        (void)run_test(kernel, bo_x_in, bo_y_out, x_buf, y_buf, tc, do_functional);
      }

      std::vector<double> cfg_list, h2d_list, kernel_list, d2h_list, total_list;
      std::vector<double> mse_list, rmse_list, mae_list, maxerr_list;

      for (int i = 0; i < measured_runs; ++i) {
        Metrics m = run_test(kernel, bo_x_in, bo_y_out, x_buf, y_buf, tc, do_functional);

        if (do_timing) {
          cfg_list.push_back(m.cfg_write_ms);
          h2d_list.push_back(m.h2d_ms);
          kernel_list.push_back(m.kernel_ms);
          d2h_list.push_back(m.d2h_ms);
          total_list.push_back(m.total_ms);
        }

        if (do_functional) {
          mse_list.push_back(m.mse);
          rmse_list.push_back(m.rmse);
          mae_list.push_back(m.mae);
          maxerr_list.push_back(m.max_abs_err);
        }
      }

      SummaryRow row;
      row.name = tc.name;
      row.N = tc.N;
      row.xmin = tc.xmin;
      row.xmax = tc.xmax;
      row.L_left = tc.L_left;
      row.L_right = tc.L_right;
      row.sym_mode = tc.sym_mode;

      if (do_timing) {
        row.avg_cfg_write_ms = avg(cfg_list);
        row.avg_h2d_ms = avg(h2d_list);
        row.avg_kernel_ms = avg(kernel_list);
        row.avg_d2h_ms = avg(d2h_list);
        row.avg_total_ms = avg(total_list);
        row.throughput_samples_per_sec =
          (row.avg_kernel_ms > 0.0) ? ((double)tc.N / (row.avg_kernel_ms * 1e-3)) : 0.0;
        row.cfg_over_kernel_pct =
          (row.avg_kernel_ms > 0.0) ? (100.0 * row.avg_cfg_write_ms / row.avg_kernel_ms) : 0.0;

        row.avg_kernel_us = row.avg_kernel_ms * 1000.0;
        row.ns_per_sample =
          (row.N > 0) ? (row.avg_kernel_ms * 1e6 / (double)row.N) : 0.0;
        row.cycles_at_250MHz =
          row.avg_kernel_ms * 1e-3 * 250e6;
      }

      if (do_functional) {
        row.avg_mse = avg(mse_list);
        row.avg_rmse = avg(rmse_list);
        row.avg_mae = avg(mae_list);
        row.avg_max_abs_err = avg(maxerr_list);
      }

      summary.push_back(row);

      std::cout << "========================================\n";
      std::cout << "test_name=" << tc.name << "\n";
      std::cout << "N=" << tc.N
                << " xmin=" << tc.xmin
                << " xmax=" << tc.xmax
                << " L_left=" << tc.L_left
                << " L_right=" << tc.L_right
                << " sym_mode=" << sym_mode_name(tc.sym_mode) << "\n";
      std::cout << "region_modes="
                << region_mode_name(tc.r0.mode) << "/"
                << region_mode_name(tc.r1.mode) << "/"
                << region_mode_name(tc.r2.mode) << "\n";
      std::cout << "deg_r0=" << tc.r0.deg
                << " deg_r1=" << tc.r1.deg
                << " deg_r2=" << tc.r2.deg << "\n";

      if (do_timing) {
        std::cout << "avg_cfg_write_ms=" << row.avg_cfg_write_ms << "\n";
        std::cout << "avg_h2d_ms=" << row.avg_h2d_ms << "\n";
        std::cout << "avg_kernel_ms=" << row.avg_kernel_ms << "\n";
        std::cout << "avg_kernel_us=" << row.avg_kernel_us << "\n";
        std::cout << "ns_per_sample=" << row.ns_per_sample << "\n";
        std::cout << "cycles_at_250MHz=" << row.cycles_at_250MHz << "\n";
        std::cout << "avg_d2h_ms=" << row.avg_d2h_ms << "\n";
        std::cout << "avg_total_ms=" << row.avg_total_ms << "\n";
        std::cout << "throughput_samples_per_sec=" << row.throughput_samples_per_sec << "\n";
        std::cout << "cfg_over_kernel_pct=" << row.cfg_over_kernel_pct << "\n";
      }

      if (do_functional) {
        std::cout << "avg_mse=" << row.avg_mse << "\n";
        std::cout << "avg_rmse=" << row.avg_rmse << "\n";
        std::cout << "avg_mae=" << row.avg_mae << "\n";
        std::cout << "avg_max_abs_err=" << row.avg_max_abs_err << "\n";
      }

      std::cout << "\n";
    }

    for (const auto& tc : softmax_tests) {
      for (int i = 0; i < warmup_runs; ++i) {
        (void)run_softmax_test(kernel, bo_x_in, bo_y_out, x_buf, y_buf, tc, do_functional);
      }

      std::vector<double> cfg_list, h2d_list, kernel_list, d2h_list, total_list;
      std::vector<double> mse_list, rmse_list, mae_list, maxerr_list;

      for (int i = 0; i < measured_runs; ++i) {
        Metrics m = run_softmax_test(kernel, bo_x_in, bo_y_out, x_buf, y_buf, tc, do_functional);

        if (do_timing) {
          cfg_list.push_back(m.cfg_write_ms);
          h2d_list.push_back(m.h2d_ms);
          kernel_list.push_back(m.kernel_ms);
          d2h_list.push_back(m.d2h_ms);
          total_list.push_back(m.total_ms);
        }

        if (do_functional) {
          mse_list.push_back(m.mse);
          rmse_list.push_back(m.rmse);
          mae_list.push_back(m.mae);
          maxerr_list.push_back(m.max_abs_err);
        }
      }

      SoftmaxSummaryRow row;
      row.name = tc.name;
      row.N = tc.N;
      row.xmin = tc.xmin;
      row.xmax = tc.xmax;

      if (do_timing) {
        row.avg_cfg_write_ms = avg(cfg_list);
        row.avg_h2d_ms = avg(h2d_list);
        row.avg_kernel_ms = avg(kernel_list);
        row.avg_d2h_ms = avg(d2h_list);
        row.avg_total_ms = avg(total_list);
        row.throughput_samples_per_sec =
          (row.avg_kernel_ms > 0.0) ? ((double)tc.N / (row.avg_kernel_ms * 1e-3)) : 0.0;
        row.avg_kernel_us = row.avg_kernel_ms * 1000.0;
        row.ns_per_sample =
          (row.N > 0) ? (row.avg_kernel_ms * 1e6 / (double)row.N) : 0.0;
      }

      if (do_functional) {
        row.avg_mse = avg(mse_list);
        row.avg_rmse = avg(rmse_list);
        row.avg_mae = avg(mae_list);
        row.avg_max_abs_err = avg(maxerr_list);
      }

      softmax_summary.push_back(row);

      std::cout << "========================================\n";
      std::cout << "softmax_test_name=" << tc.name << "\n";
      std::cout << "N=" << tc.N
                << " xmin=" << tc.xmin
                << " xmax=" << tc.xmax
                << " L_left=" << tc.L_left
                << " L_right=" << tc.L_right << "\n";

      if (do_timing) {
        std::cout << "avg_cfg_write_ms=" << row.avg_cfg_write_ms << "\n";
        std::cout << "avg_h2d_ms=" << row.avg_h2d_ms << "\n";
        std::cout << "avg_kernel_ms=" << row.avg_kernel_ms << "\n";
        std::cout << "avg_kernel_us=" << row.avg_kernel_us << "\n";
        std::cout << "ns_per_sample=" << row.ns_per_sample << "\n";
        std::cout << "avg_d2h_ms=" << row.avg_d2h_ms << "\n";
        std::cout << "avg_total_ms=" << row.avg_total_ms << "\n";
        std::cout << "throughput_samples_per_sec=" << row.throughput_samples_per_sec << "\n";
      }

      if (do_functional) {
        std::cout << "avg_mse=" << row.avg_mse << "\n";
        std::cout << "avg_rmse=" << row.avg_rmse << "\n";
        std::cout << "avg_mae=" << row.avg_mae << "\n";
        std::cout << "avg_max_abs_err=" << row.avg_max_abs_err << "\n";
      }

      std::cout << "\n";
    }

    std::cout << "========================================\n";
    std::cout << "FINAL_SUMMARY\n";

    if (do_timing) {
      std::cout << "Function"
                << "\tN"
                << "\tXmin"
                << "\tXmax"
                << "\tL_left"
                << "\tL_right"
                << "\tSymMode"
                << "\tCfgWrite_ms"
                << "\tKernel_ms"
                << "\tKernel_us"
                << "\tNsPerSample"
                << "\tCycles250MHz"
                << "\tThroughput_sps"
                << "\tCfgOverKernel_pct";

      if (do_functional) {
        std::cout << "\tMSE"
                  << "\tRMSE"
                  << "\tMAE"
                  << "\tMaxAbsErr";
      }

      std::cout << "\n";

      for (const auto& row : summary) {
        std::cout << row.name
                  << "\t" << row.N
                  << "\t" << row.xmin
                  << "\t" << row.xmax
                  << "\t" << row.L_left
                  << "\t" << row.L_right
                  << "\t" << sym_mode_name(row.sym_mode)
                  << "\t" << row.avg_cfg_write_ms
                  << "\t" << row.avg_kernel_ms
                  << "\t" << row.avg_kernel_us
                  << "\t" << row.ns_per_sample
                  << "\t" << row.cycles_at_250MHz
                  << "\t" << row.throughput_samples_per_sec
                  << "\t" << row.cfg_over_kernel_pct;

        if (do_functional) {
          std::cout << "\t" << row.avg_mse
                    << "\t" << row.avg_rmse
                    << "\t" << row.avg_mae
                    << "\t" << row.avg_max_abs_err;
        }

        std::cout << "\n";
      }
    } else if (do_functional) {
      std::cout << "Function"
                << "\tN"
                << "\tXmin"
                << "\tXmax"
                << "\tL_left"
                << "\tL_right"
                << "\tSymMode"
                << "\tMSE"
                << "\tRMSE"
                << "\tMAE"
                << "\tMaxAbsErr"
                << "\n";

      for (const auto& row : summary) {
        std::cout << row.name
                  << "\t" << row.N
                  << "\t" << row.xmin
                  << "\t" << row.xmax
                  << "\t" << row.L_left
                  << "\t" << row.L_right
                  << "\t" << sym_mode_name(row.sym_mode)
                  << "\t" << row.avg_mse
                  << "\t" << row.avg_rmse
                  << "\t" << row.avg_mae
                  << "\t" << row.avg_max_abs_err
                  << "\n";
      }
    }

    if (!softmax_summary.empty()) {
      std::cout << "========================================\n";
      std::cout << "SOFTMAX_SUMMARY\n";

      std::cout << "Function"
                << "\tN"
                << "\tXmin"
                << "\tXmax";

      if (do_timing) {
        std::cout << "\tKernel_ms"
                  << "\tKernel_us"
                  << "\tNsPerSample"
                  << "\tThroughput_sps";
      }

      if (do_functional) {
        std::cout << "\tMSE"
                  << "\tRMSE"
                  << "\tMAE"
                  << "\tMaxAbsErr";
      }

      std::cout << "\n";

      for (const auto& row : softmax_summary) {
        std::cout << row.name
                  << "\t" << row.N
                  << "\t" << row.xmin
                  << "\t" << row.xmax;

        if (do_timing) {
          std::cout << "\t" << row.avg_kernel_ms
                    << "\t" << row.avg_kernel_us
                    << "\t" << row.ns_per_sample
                    << "\t" << row.throughput_samples_per_sec;
        }

        if (do_functional) {
          std::cout << "\t" << row.avg_mse
                    << "\t" << row.avg_rmse
                    << "\t" << row.avg_mae
                    << "\t" << row.avg_max_abs_err;
        }

        std::cout << "\n";
      }
    }

    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 1;
  }
}