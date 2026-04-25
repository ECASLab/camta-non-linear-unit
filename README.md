# CAMTA: Configurable Multi-Region Activation Unit

**CAMTA** is a configurable FPGA accelerator for approximating nonlinear activation functions using multi-region polynomial evaluation. The design is intended for FPGA-based machine learning inference workloads where activation functions such as **GeLU**, **tanh**, **sigmoid**, **Swish**, and exponential-like mappings must be evaluated efficiently without redesigning the hardware for every function.

CAMTA partitions the input domain into three programmable regions and evaluates one polynomial per region using **Horner's rule**. The accelerator uses a 32-lane vectorized datapath over a 512-bit AXI interface, allowing multiple 16-bit fixed-point samples to be processed per memory word.

---

## Authors

- Carlos Soto-Porras
- Jose Fonseca-Cruz
- Pablo Ramirez-Morera
- Erick Obregon-Fonseca

Instituto Tecnologico de Costa Rica  
FPGA Design Course Project

---

## Project Overview

The main goal of CAMTA is to provide a **runtime-configurable nonlinear approximation engine** for FPGA deployment. Instead of implementing a dedicated hardware block for a single activation function, CAMTA reuses the same architecture and changes its behavior through configuration parameters:

- Segmentation threshold `L`.
- Polynomial degree for each region.
- Polynomial coefficients for each region.
- Vector length `N`.

This makes the architecture function-agnostic at the hardware level. Switching from one activation function to another only requires changing the configuration values, not modifying or resynthesizing the RTL.

---

## Architecture

CAMTA is organized as a three-region piecewise polynomial approximation unit:

```text
Input vector
    |
    v
512-bit AXI input word
    |
    v
Unpack 32 fixed-point samples
    |
    v
Per-lane region selection
    |
    v
Coefficient and degree selection
    |
    v
Horner polynomial evaluation
    |
    v
Pack 32 fixed-point outputs
    |
    v
512-bit AXI output word
```

Each 512-bit memory word contains **32 samples** of 16 bits each. Internally, CAMTA instantiates 32 parallel evaluation lanes. Each lane independently:

1. Unpacks one input sample.
2. Selects the active region.
3. Selects the corresponding polynomial coefficients and degree.
4. Evaluates the polynomial using Horner's rule.
5. Packs the output sample back into the output AXI word.

---

## Region Selection

The input domain is divided using a programmable threshold `L`:

| Region | Condition | Purpose |
|---|---|---|
| Left region | `x < -L` | Negative tail or left-side behavior |
| Center region | `-L <= x <= L` | Main nonlinear transition region |
| Right region | `x > L` | Positive tail or right-side behavior |

Each region has its own polynomial degree `d_r` and coefficient set:

```text
{a3_r, a2_r, a1_r, a0_r}
```

This allows CAMTA to approximate functions with different curvature profiles by changing only the configuration registers.

---

## Polynomial Evaluation

For a selected region `r`, CAMTA approximates the output using a cubic polynomial:

```text
P_r(x) = a0_r + a1_r*x + a2_r*x^2 + a3_r*x^3
```

The polynomial is evaluated using Horner's rule:

```text
P_r(x) = (((a3_r*x + a2_r)*x + a1_r)*x + a0_r)
```

Horner's rule was selected because it maps naturally to FPGA hardware as a regular multiply-accumulate chain. This improves compatibility with HLS synthesis, simplifies pipelining, and avoids explicitly computing `x^2` and `x^3` as separate operations.

The design supports effective polynomial degrees from 0 to 3. Depending on the selected degree, unused stages can be bypassed or ignored.

---

## Numeric Format

The current implementation uses fixed-point arithmetic:

```cpp
typedef ap_fixed<16,6>  data_t;
typedef ap_fixed<24,10> acc_t;
typedef ap_uint<512>   word_t;
```

| Type | Description |
|---|---|
| `data_t` | 16-bit Q6.10 fixed-point type used for input samples, output samples, and coefficients. |
| `acc_t` | Wider internal accumulator used inside the Horner evaluation chain. |
| `word_t` | 512-bit AXI memory word. |

The external interface uses a compact 16-bit Q6.10 representation, while the internal Horner chain uses wider arithmetic to reduce intermediate quantization effects before casting the final result back to the interface format.

---

## Repository Structure

```text
proj_final_FPGA_CAMTA-main/
├── README.md
├── run_hls.tcl
├── src/
│   ├── camta.cpp
│   ├── camta.h
│   ├── camta_types.h
│   ├── horner_core.cpp
│   └── horner_core.h
├── tb/
│   └── camta_tb.cpp
├── scripts/
│   └── compare_hls_solutions.py
├── HornerCore/
│   ├── HornerCore.cpp
│   ├── HornerCorePipeline.cpp
│   ├── HornerCore.tcl
│   ├── HornerCore_tb.cc
│   └── HornerCorePipeline_tb.cc
└── camta_hls/
    └── solution*/
        ├── syn/report/
        └── impl/
```

### Main Files

| Path | Description |
|---|---|
| `src/camta.cpp` | Top-level CAMTA accelerator. Handles AXI access, lane unpacking, region selection, Horner evaluation, and output packing. |
| `src/camta_types.h` | Fixed-point and AXI word type definitions, plus lane pack/unpack helper functions. |
| `src/horner_core.cpp` | Horner polynomial evaluation core. |
| `tb/camta_tb.cpp` | C++ testbench for functional validation. |
| `run_hls.tcl` | Vitis HLS automation script. |
| `scripts/compare_hls_solutions.py` | Utility script for extracting and comparing HLS synthesis metrics. |
| `HornerCore/` | Standalone Horner core exploration, including baseline and pipelined versions. |

---

## HLS Implementation Details

The top-level CAMTA function uses AXI memory interfaces for input and output buffers, plus AXI-Lite control registers for configuration parameters:

```cpp
#pragma HLS INTERFACE m_axi     offset=slave port=x_in  bundle=gmem0 depth=1024
#pragma HLS INTERFACE m_axi     offset=slave port=y_out bundle=gmem1 depth=1024
#pragma HLS INTERFACE s_axilite port=return
```

The main word-processing loop is pipelined with an initiation interval target of one cycle:

```cpp
#pragma HLS PIPELINE II=1
```

The lane loop is unrolled to exploit the 32-lane vectorized datapath:

```cpp
#pragma HLS UNROLL
```

This organization prioritizes throughput by processing one 512-bit word per pipeline iteration once the pipeline is filled.

---

## Running Vitis HLS

From the repository root:

```bash
vitis_hls -f run_hls.tcl
```

Default target configuration:

```tcl
set part "xck26-sfvc784-2LV-c"
set clk_period 4
```

This corresponds to the AMD Kria KV260 target device with a 4 ns clock period, equivalent to 250 MHz.

The target part and clock can also be configured through environment variables:

```bash
export PART=xck26-sfvc784-2LV-c
export CLOCK_PERIOD=4
vitis_hls -f run_hls.tcl
```

PowerShell example:

```powershell
$env:PART="xck26-sfvc784-2LV-c"
$env:CLOCK_PERIOD="4"
vitis_hls -f run_hls.tcl
```

---

## Experimental Setup

The architecture was evaluated with Vitis HLS 2023.2 and XRT-based host execution at 250 MHz. The evaluation considered the following nonlinear functions:

- GeLU
- tanh
- sigmoid
- Swish
- exponential

For each function, CAMTA was configured with a different set of:

- Three-region coefficients.
- Region degrees.
- Segmentation threshold `L`.

Runtime measurements were obtained using one warm-up run and five measured runs. Numerical accuracy was evaluated against a floating-point software reference using:

- MSE
- RMSE
- MAE
- Maximum absolute error

---

## Synthesis Results

The final vectorized CAMTA implementation meets the 250 MHz target with timing margin.

| Metric | Value |
|---|---:|
| Target clock period | 4.00 ns |
| Estimated clock period | 2.92 ns |
| Target frequency | 250 MHz |
| Top-level latency | 157 to 1180 cycles |
| Top-level latency time | 0.628 us to 4.720 us |
| `word_loop` initiation interval | 1 cycle |
| `word_loop` iteration latency | 14 cycles |
| BRAM18K | 116 |
| DSP | 96 |
| FF | 27,656 |
| LUT | 33,507 |

Approximate resource utilization on the target FPGA:

| Resource | Utilization |
|---|---:|
| BRAM | 40% |
| DSP | 7% |
| FF | 11% |
| LUT | 28% |

These results reflect the main architectural trade-off of CAMTA: the design uses more replicated hardware than a scalar implementation, but this enables wide-word, lane-parallel execution and high throughput.

---

## Runtime and Accuracy Results

Representative runtime and numerical results are summarized below.

| Function | N | Kernel time [us] | Throughput [MSamples/s] | RMSE | MAE |
|---|---:|---:|---:|---:|---:|
| GeLU | 10000 | 88.22 | 113.3787 | 0.0225 | 0.0128 |
| tanh | 4096 | 90.17 | 45.4339 | 0.0639 | 0.0360 |
| sigmoid | 4096 | 87.67 | 44.6428 | 0.0393 | 0.0241 |
| Swish | 4096 | 84.12 | 48.6855 | 0.0905 | 0.0607 |
| exp, `L = 1.25` | 1024 | 86.54 | 11.8329 | 16.73 | 4.42 |
| exp, `L = 1.25` | 4096 | 89.95 | 45.5373 | 16.64 | 4.37 |
| exp, `L = 0.35`, range `[-1, 1]` | 1024 | 85.01 | 12.0467 | 0.001 | 0.002 |
| exp, `L = 0.35`, range `[-1, 1]` | 4096 | 86.72 | 47.2367 | 0.001 | 0.002 |

The results show that CAMTA is especially effective for bounded smooth activation functions. The exponential function is more challenging over a wide interval such as `[-4, 4]` because its large dynamic range is difficult to approximate using a fixed three-region cubic model under Q6.10 constraints. However, when the exponential range is restricted to `[-1, 1]`, the approximation error is significantly reduced.

---

## Comparison with Related Approaches

CAMTA should be interpreted as a configurable approximation engine rather than a function-specific exact evaluator.

Compared with specialized CORDIC-based accelerators, CAMTA requires more area but offers runtime flexibility across multiple activation functions. Compared with PLAC-style piecewise linear approximation, CAMTA can achieve competitive accuracy for several bounded functions while keeping a unified hardware datapath.

| Function | Metric | CAMTA | PLAC | CORDIC |
|---|---|---:|---:|---:|
| `log2(1 + x)` | MAE | `1.11e-3` | `2.15e-4` | - |
| `tanh(x)` | MAE | `1.66e-3` | `5.55e-3` | - |
| `sigmoid(x)` | MAE | `1.90e-3` | `5.65e-3` | - |
| Softmax | RMSE | `2.68e-4` | - | `~3e-5` to `4e-5` |

This comparison highlights CAMTA's design point: it trades minimum area and exact evaluation for architectural reusability and runtime configurability.

---

## Main Design Trade-offs

### Strengths

- Runtime reconfiguration through coefficients, region degrees, and threshold `L`.
- Same hardware core can approximate multiple nonlinear functions.
- 32-lane vectorized datapath aligned with a 512-bit AXI interface.
- Fully pipelined word-level processing with initiation interval of 1 cycle.
- Good throughput for large batches, exceeding 113 MSamples/s for GeLU at `N = 10000`.
- Competitive accuracy for bounded activation functions.

### Limitations

- Higher resource utilization than scalar or function-specific implementations.
- Approximation quality depends strongly on the selected coefficients and input range.
- Rapidly growing functions such as exponential are difficult to approximate over wide domains with only three cubic regions.
- Fixed kernel launch and memory-transfer overheads dominate for small batch sizes.

---

## Development Notes

The final architecture described in the report is the packed 512-bit AXI version with 32 lanes. Older scalar testbench or header variants may appear in the repository as intermediate development artifacts. When validating or extending the final version, the testbench and function declaration should use the packed `word_t` interface and the helper functions in `camta_types.h`:

```cpp
camta_pack_lane(word, lane, value);
camta_unpack_lane(word, lane);
```

---

## Future Work

Potential extensions include:

1. **Asymmetric segmentation:** replace the single symmetric threshold `L` with independent thresholds `L-` and `L+` to better approximate asymmetric functions such as GeLU and ELU.
2. **Early-exit saturation:** bypass the Horner chain in regions where functions such as `tanh` or `sigmoid` are already saturated.
3. **Mixed precision per region:** use different fixed-point formats depending on the curvature and dynamic range of each region.
4. **Formal coefficient optimization:** integrate minimax or least-squares fitting constrained to the Q6.10 quantization grid.
5. **Full system integration:** integrate the exported HLS kernel into a complete Vivado/Vitis acceleration flow for the target FPGA platform.

---

## AI Tools Usage

AI tools were used as auxiliary support during the development of the project. Their use was limited to:

- Testbench generation and debugging support.
- Suggestions for block diagram organization.
- Troubleshooting implementation issues.
- Improving clarity, organization, and technical writing.

The architectural decisions, implementation, experiments, interpretation of results, and academic responsibility remain with the authors.

---

## License

This repository is intended for academic use as part of an FPGA design course project.
