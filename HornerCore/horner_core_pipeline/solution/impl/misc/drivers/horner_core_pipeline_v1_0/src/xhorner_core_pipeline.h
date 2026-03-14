// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.2 (64-bit)
// Tool Version Limit: 2023.10
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XHORNER_CORE_PIPELINE_H
#define XHORNER_CORE_PIPELINE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xhorner_core_pipeline_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
#ifdef SDT
    char *Name;
#else
    u16 DeviceId;
#endif
    u64 Control_BaseAddress;
} XHorner_core_pipeline_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XHorner_core_pipeline;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XHorner_core_pipeline_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XHorner_core_pipeline_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XHorner_core_pipeline_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XHorner_core_pipeline_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifdef SDT
int XHorner_core_pipeline_Initialize(XHorner_core_pipeline *InstancePtr, UINTPTR BaseAddress);
XHorner_core_pipeline_Config* XHorner_core_pipeline_LookupConfig(UINTPTR BaseAddress);
#else
int XHorner_core_pipeline_Initialize(XHorner_core_pipeline *InstancePtr, u16 DeviceId);
XHorner_core_pipeline_Config* XHorner_core_pipeline_LookupConfig(u16 DeviceId);
#endif
int XHorner_core_pipeline_CfgInitialize(XHorner_core_pipeline *InstancePtr, XHorner_core_pipeline_Config *ConfigPtr);
#else
int XHorner_core_pipeline_Initialize(XHorner_core_pipeline *InstancePtr, const char* InstanceName);
int XHorner_core_pipeline_Release(XHorner_core_pipeline *InstancePtr);
#endif

void XHorner_core_pipeline_Start(XHorner_core_pipeline *InstancePtr);
u32 XHorner_core_pipeline_IsDone(XHorner_core_pipeline *InstancePtr);
u32 XHorner_core_pipeline_IsIdle(XHorner_core_pipeline *InstancePtr);
u32 XHorner_core_pipeline_IsReady(XHorner_core_pipeline *InstancePtr);
void XHorner_core_pipeline_Continue(XHorner_core_pipeline *InstancePtr);
void XHorner_core_pipeline_EnableAutoRestart(XHorner_core_pipeline *InstancePtr);
void XHorner_core_pipeline_DisableAutoRestart(XHorner_core_pipeline *InstancePtr);
u32 XHorner_core_pipeline_Get_return(XHorner_core_pipeline *InstancePtr);

void XHorner_core_pipeline_Set_x(XHorner_core_pipeline *InstancePtr, u32 Data);
u32 XHorner_core_pipeline_Get_x(XHorner_core_pipeline *InstancePtr);
void XHorner_core_pipeline_Set_a3(XHorner_core_pipeline *InstancePtr, u32 Data);
u32 XHorner_core_pipeline_Get_a3(XHorner_core_pipeline *InstancePtr);
void XHorner_core_pipeline_Set_a2(XHorner_core_pipeline *InstancePtr, u32 Data);
u32 XHorner_core_pipeline_Get_a2(XHorner_core_pipeline *InstancePtr);
void XHorner_core_pipeline_Set_a1(XHorner_core_pipeline *InstancePtr, u32 Data);
u32 XHorner_core_pipeline_Get_a1(XHorner_core_pipeline *InstancePtr);
void XHorner_core_pipeline_Set_a0(XHorner_core_pipeline *InstancePtr, u32 Data);
u32 XHorner_core_pipeline_Get_a0(XHorner_core_pipeline *InstancePtr);

void XHorner_core_pipeline_InterruptGlobalEnable(XHorner_core_pipeline *InstancePtr);
void XHorner_core_pipeline_InterruptGlobalDisable(XHorner_core_pipeline *InstancePtr);
void XHorner_core_pipeline_InterruptEnable(XHorner_core_pipeline *InstancePtr, u32 Mask);
void XHorner_core_pipeline_InterruptDisable(XHorner_core_pipeline *InstancePtr, u32 Mask);
void XHorner_core_pipeline_InterruptClear(XHorner_core_pipeline *InstancePtr, u32 Mask);
u32 XHorner_core_pipeline_InterruptGetEnabled(XHorner_core_pipeline *InstancePtr);
u32 XHorner_core_pipeline_InterruptGetStatus(XHorner_core_pipeline *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
