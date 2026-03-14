// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.2 (64-bit)
// Tool Version Limit: 2023.10
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
// control
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read/COR)
//        bit 4  - ap_continue (Read/Write/SC)
//        bit 7  - auto_restart (Read/Write)
//        bit 9  - interrupt (Read)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0 - enable ap_done interrupt (Read/Write)
//        bit 1 - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        bit 1 - ap_ready (Read/TOW)
//        others - reserved
// 0x10 : Data signal of ap_return
//        bit 15~0 - ap_return[15:0] (Read)
//        others   - reserved
// 0x18 : Data signal of x
//        bit 15~0 - x[15:0] (Read/Write)
//        others   - reserved
// 0x1c : reserved
// 0x20 : Data signal of a3
//        bit 15~0 - a3[15:0] (Read/Write)
//        others   - reserved
// 0x24 : reserved
// 0x28 : Data signal of a2
//        bit 15~0 - a2[15:0] (Read/Write)
//        others   - reserved
// 0x2c : reserved
// 0x30 : Data signal of a1
//        bit 15~0 - a1[15:0] (Read/Write)
//        others   - reserved
// 0x34 : reserved
// 0x38 : Data signal of a0
//        bit 15~0 - a0[15:0] (Read/Write)
//        others   - reserved
// 0x3c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XHORNER_CORE_PIPELINE_CONTROL_ADDR_AP_CTRL   0x00
#define XHORNER_CORE_PIPELINE_CONTROL_ADDR_GIE       0x04
#define XHORNER_CORE_PIPELINE_CONTROL_ADDR_IER       0x08
#define XHORNER_CORE_PIPELINE_CONTROL_ADDR_ISR       0x0c
#define XHORNER_CORE_PIPELINE_CONTROL_ADDR_AP_RETURN 0x10
#define XHORNER_CORE_PIPELINE_CONTROL_BITS_AP_RETURN 16
#define XHORNER_CORE_PIPELINE_CONTROL_ADDR_X_DATA    0x18
#define XHORNER_CORE_PIPELINE_CONTROL_BITS_X_DATA    16
#define XHORNER_CORE_PIPELINE_CONTROL_ADDR_A3_DATA   0x20
#define XHORNER_CORE_PIPELINE_CONTROL_BITS_A3_DATA   16
#define XHORNER_CORE_PIPELINE_CONTROL_ADDR_A2_DATA   0x28
#define XHORNER_CORE_PIPELINE_CONTROL_BITS_A2_DATA   16
#define XHORNER_CORE_PIPELINE_CONTROL_ADDR_A1_DATA   0x30
#define XHORNER_CORE_PIPELINE_CONTROL_BITS_A1_DATA   16
#define XHORNER_CORE_PIPELINE_CONTROL_ADDR_A0_DATA   0x38
#define XHORNER_CORE_PIPELINE_CONTROL_BITS_A0_DATA   16

