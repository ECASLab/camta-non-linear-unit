# CAMTA: Reconfigurable Multi-Region Activation Unit

CAMTA is a 16-bit reconfigurable multi-region activation unit for nonlinear function approximation in machine learning workloads. The unit approximates functions such as GeLU, tanh, sigmoid, Swish, and the exponential stage used in Softmax by reusing the same scalar datapath with runtime-configurable thresholds, coefficients, polynomial degrees, and region modes.

The current repository contains:

- a Vitis HLS implementation of the scalar CAMTA unit and Alveo kernel wrapper;
- an XRT host application for FPGA runtime and accuracy validation;
- manually isolated Verilog RTL for the scalar core used for ASIC-oriented synthesis;
- simple C++ and Verilog testbenches for functional checks.

CAMTA is intended to be a reusable activation-function macro rather than a function-specific evaluator. Its main design trade-off is exchanging some area and power overhead for runtime configurability and hardware reuse across multiple nonlinear functions without hardware resynthesis.

---

## Authors

- Carlos Soto-Porras
- Jose Fonseca-Cruz
- Pablo Ramirez-Morera
- Erick Obregon-Fonseca
- Luis G. Leon-Vega
- Jorge Castro-Godinez

School of Electronics Engineering  
Costa Rica Institute of Technology

---

## Architecture Overview

CAMTA partitions the input domain into three programmable regions using two independent thresholds:

```text
r0: x < L_left
r1: L_left <= x <= L_right
r2: x > L_right
```

For each region, CAMTA selects:

- four polynomial coefficients: `a3`, `a2`, `a1`, `a0`;
- a polynomial degree: `0`, `1`, `2`, or `3`;
- an execution mode: `HORNER`, `CONST`, `ZERO`, or `IDENTITY`.

In `HORNER` mode, the selected region is evaluated as a cubic polynomial:

```text
P(x) = a0 + a1*x + a2*x^2 + a3*x^3
```

using Horner's rule:

```text
P(x) = (((a3*x + a2)*x + a1)*x + a0)
```

The region mode controls the final output behavior:

| Mode | Output behavior |
|---|---|
| `HORNER` | Returns the Horner polynomial result. |
| `CONST` | Returns the selected `a0` coefficient. |
| `ZERO` | Returns zero. |
| `IDENTITY` | Returns the input `x`. |

This mode-controlled bypass avoids unnecessary polynomial evaluation in regions where the function can be represented as constant, zero, or identity-like behavior. For example, tanh and sigmoid can use constant tails, while GeLU and Swish can use residual/identity-like tails.

---

## Numeric Format

The HLS implementation uses fixed-point arithmetic:

```cpp
typedef ap_fixed<16,6>  data_t;
typedef ap_fixed<24,10> acc_t;
```

| Type | Description |
|---|---|
| `data_t` | 16-bit Q6.10 fixed-point type used for inputs, outputs, thresholds, and coefficients. |
| `acc_t` | 24-bit internal accumulator used inside the Horner evaluation chain. |

The external datapath uses a compact 16-bit Q6.10 representation, while the Horner chain uses a wider accumulator to reduce intermediate quantization effects before casting the result back to `data_t`.

---

## Repository Structure

```text
camta-non-linear-unit/
├── README.md
├── LICENSE
├── run_hls.tcl
├── HW/
│   ├── Makefile
│   ├── camta.cpp
│   ├── camta.h
│   ├── camta_types.h
│   ├── horner_core.cpp
│   └── horner_core.h
├── SW/
│   ├── Makefile
│   └── camta.cpp
├── src/
│   ├── camta.cpp
│   ├── camta.h
│   ├── camta_types.h
│   ├── horner_core.cpp
│   ├── horner_core.h
│   ├── camta_unit_core.v
│   ├── camta_horner_seq.v
│   ├── camta_region_select.v
│   └── camta_mode_bypass.v
├── tb/
│   ├── camta_tb.cpp
│   └── tb_camta_unit_core.v
└── scripts/
    └── compare_hls_solutions.py
```

### Main components

| Path | Description |
|---|---|
| `src/camta.cpp` | HLS source for `camta_unit` and scalar array wrapper `camta`. Used by `run_hls.tcl`. |
| `src/horner_core.cpp` | Degree-controlled Horner polynomial evaluator. |
| `src/camta_types.h` | Fixed-point types and mode definitions. |
| `src/*.v` | Isolated Verilog RTL modules for scalar core-level simulation and ASIC-oriented synthesis. |
| `HW/` | Alveo/Vitis kernel source and Makefile for generating the FPGA binary. |
| `SW/camta.cpp` | XRT host application used for runtime and numerical validation. |
| `tb/camta_tb.cpp` | Simple C++ functional testbench for the scalar HLS wrapper. |
| `tb/tb_camta_unit_core.v` | Verilog testbench for the isolated scalar RTL core. |
| `run_hls.tcl` | Vitis HLS script for synthesizing the scalar `camta_unit`. |
| `scripts/compare_hls_solutions.py` | Utility script for extracting and comparing HLS synthesis reports. |

---

## Build Requirements

The project assumes a Xilinx/AMD FPGA development environment with:

- Vitis HLS / Vitis;
- XRT runtime and development headers;
- a compatible Alveo platform, configured by default for `xilinx_u55c_gen3x16_xdma_3_202210_1` in `HW/Makefile`;
- a C++17 compiler;
- Python 3 for the report comparison utility.

Before building on a server, load the corresponding Vitis and XRT environment scripts according to the local installation.

---

## Running Vitis HLS for the Scalar Unit

From the repository root:

```bash
vitis_hls -f run_hls.tcl
```

The script synthesizes the scalar top function:

```tcl
set top_name "camta_unit"
```

By default, it targets a 4 ns clock period. The target FPGA part and clock period can be overridden through environment variables:

```bash
export PART=xck26-sfvc784-2LV-c
export CLOCK_PERIOD=4
vitis_hls -f run_hls.tcl
```

The HLS script runs `csynth_design` and then attempts to execute `scripts/compare_hls_solutions.py` to summarize synthesis reports across available solutions.

---

## Building the Alveo Kernel

From the `HW/` directory:

```bash
cd HW
make build
```

Important Makefile settings:

```make
TARGET := hw
PLATFORM ?= xilinx_u55c_gen3x16_xdma_3_202210_1
KERNEL_NAME := camta
KERNEL_FREQ := 250
HLS_FILES := camta.cpp horner_core.cpp
```

The expected output binary is:

```text
HW/package.hw/kernels.xclbin
```

To clean generated files:

```bash
make cleanall
```

---

## Building and Running the XRT Host

Build the host application from `SW/`:

```bash
cd SW
make
```

Run all timing and functional tests:

```bash
./camta ../HW/package.hw/kernels.xclbin --mode all
```

Supported modes:

```bash
./camta ../HW/package.hw/kernels.xclbin --mode all
./camta ../HW/package.hw/kernels.xclbin --mode timing
./camta ../HW/package.hw/kernels.xclbin --mode functional
```

Equivalent short options are also supported:

```bash
./camta ../HW/package.hw/kernels.xclbin -all
./camta ../HW/package.hw/kernels.xclbin -timing
./camta ../HW/package.hw/kernels.xclbin -functional
```

The host executes one warm-up run and five measured runs. It reports detailed per-test timing, throughput, MSE, RMSE, MAE, maximum absolute error, and final summary tables.

---

## Runtime Validation Cases

The host validates the following nonlinear functions using `N = 10000` samples:

| Function | Evaluation range | Best configuration used in paper |
|---|---:|---|
| GeLU | `[-8, 8]` | residual symmetry, Horner/Horner/Identity |
| tanh | `[-4, 4]` | odd symmetry, Horner/Horner/Const |
| sigmoid | `[-8, 8]` | complement symmetry, Horner/Horner/Const |
| Swish | `[-8, 8]` | residual symmetry, Horner/Horner/Identity |
| Softmax-assisted exp | `[-8, 0]` | Const/Horner/Horner |

The Softmax experiment is reported as CAMTA-assisted Softmax. CAMTA approximates only the exponential stage over shifted inputs in `[-8, 0]`; summation and normalization are performed by the host.

---

## Selected Configurations

The following configurations correspond to the best reported `N = 10000` validation cases:

| Function | Range | `L_left` | `L_right` | Symmetry mode | Region modes | Degrees |
|---|---:|---:|---:|---|---|---|
| GeLU | `[-8, 8]` | `1.50` | `3.00` | residual | H/H/I | 3/3/1 |
| tanh | `[-4, 4]` | `1.25` | `3.50` | odd | H/H/C | 3/3/0 |
| sigmoid | `[-8, 8]` | `2.50` | `4.50` | complement | H/H/C | 3/3/0 |
| Swish | `[-8, 8]` | `2.00` | `6.50` | residual | H/H/I | 3/3/1 |
| Softmax-assisted exp | `[-8, 0]` | `-4.00` | `-1.00` | none | C/H/H | 0/3/3 |

Legend:

- H: `HORNER`
- C: `CONST`
- I: `IDENTITY`

---

## Representative FPGA Runtime and Accuracy Results

The following results correspond to the final `N = 10000` validation run on the Alveo flow:

| Function | Kernel time [us] | Throughput [MSamples/s] | RMSE | MaxAbsErr |
|---|---:|---:|---:|---:|
| GeLU | 183.59 | 54.47 | 0.00152 | 0.00501 |
| tanh | 185.74 | 53.84 | 0.00162 | 0.00582 |
| sigmoid | 185.89 | 53.79 | 0.00207 | 0.00803 |
| Swish | 185.42 | 53.93 | 0.00389 | 0.01344 |
| Softmax-assisted | 185.24 | 53.98 | 3.60e-6 | 1.08e-5 |

These timings correspond to the scalar FPGA wrapper execution and include kernel launch and memory-mapped execution overhead. They should not be interpreted as the intrinsic datapath latency of the CAMTA scalar unit.

---

## FPGA HLS Synthesis Summary

The scalar HLS synthesis result used in the paper reports:

| Metric | CAMTA scalar unit |
|---|---:|
| Target clock | 4.00 ns |
| Estimated clock | 2.465 ns |
| Datapath latency | 11 cycles |
| BRAM | 0 |
| DSP | 3 |
| FF | 802 |
| LUT | 1756 |

A CORDIC exponential core used as a comparison point reports 2 DSPs, 757 FFs, and 2676 LUTs. That comparison should be interpreted carefully because the CORDIC result corresponds only to an exponential evaluator, while CAMTA supports multiple nonlinear functions through runtime-configurable thresholds, coefficients, degrees, and region modes.

---

## ASIC-Oriented Synthesis Summary

The manually isolated scalar RTL core was synthesized using a TSMC 65 nm standard-cell flow at 250 MHz. The HLS-generated interface and wrapper logic were excluded so that the ASIC evaluation reflects only the arithmetic and control core.

| Metric | Value |
|---|---:|
| Clock period | 4.0 ns |
| Frequency | 250 MHz |
| Critical path | 2.62 ns |
| Total cell area | 6632.40 um^2 |
| Combinational area | 4555.60 um^2 |
| Noncombinational area | 2076.80 um^2 |
| Total cells | 1582 |
| Combinational cells | 1343 |
| Sequential cells | 236 |
| Horner core area | 4753.60 um^2 |
| Region-selection area | 164.00 um^2 |
| Mode-bypass area | 83.20 um^2 |
| Dynamic power | 1.1743 mW |
| Leakage power | 189.16 uW |
| Total power | 1.3634 mW |

The Horner core dominates the implementation area, while the region-selection and mode-bypass logic represent a small fraction of the total area. This supports the interpretation that most of the hardware cost is associated with the shared arithmetic datapath, and that the additional region configurability introduces limited area overhead relative to the polynomial evaluator.

---

## Verilog RTL Core

The `src/*.v` files provide a scalar RTL version of the CAMTA core:

| File | Description |
|---|---|
| `camta_unit_core.v` | Top-level scalar RTL core with start/done handshake. |
| `camta_horner_seq.v` | Sequential Horner evaluator using a Q10.14 accumulator. |
| `camta_region_select.v` | Region decoder for `L_left` and `L_right`. |
| `camta_mode_bypass.v` | Output mux for `HORNER`, `CONST`, `ZERO`, and `IDENTITY` modes. |

A simple Verilog testbench is provided in:

```text
tb/tb_camta_unit_core.v
```

---

## Limitations and Notes

- CAMTA is a configurable approximation unit, not a function-specific exact evaluator.
- Approximation quality depends on the selected coefficients, thresholds, degrees, modes, and input range.
- The Softmax result is CAMTA-assisted; CAMTA approximates the exponential stage, while the host performs reduction and normalization.
- The `run_hls.tcl` script synthesizes the scalar `camta_unit`, not the Alveo memory wrapper.
- The `HW/` Makefile builds the Alveo kernel wrapper named `camta`.
- The ASIC-oriented synthesis results correspond to the isolated scalar RTL core, excluding HLS-generated AXI/interface logic.

---

## Future Work

Planned extensions include:

- integrating CAMTA into a RISC-V-based accelerator architecture;
- adding input gating to reduce unnecessary Horner-core switching when `CONST`, `ZERO`, or `IDENTITY` modes are selected;
- improving backend timing robustness and hold-time closure for ASIC implementation;
- refining coefficient-generation and quantization-aware fitting flows for new nonlinear functions;
- evaluating multi-lane replication strategies for throughput-oriented accelerator designs.

---

## License

This repository is distributed under the license included in `LICENSE`.
