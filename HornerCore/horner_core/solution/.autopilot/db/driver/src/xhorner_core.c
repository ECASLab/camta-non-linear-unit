// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.2 (64-bit)
// Tool Version Limit: 2023.10
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xhorner_core.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XHorner_core_CfgInitialize(XHorner_core *InstancePtr, XHorner_core_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XHorner_core_Start(XHorner_core *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_AP_CTRL) & 0x80;
    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XHorner_core_IsDone(XHorner_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XHorner_core_IsIdle(XHorner_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XHorner_core_IsReady(XHorner_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XHorner_core_Continue(XHorner_core *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_AP_CTRL) & 0x80;
    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_AP_CTRL, Data | 0x10);
}

void XHorner_core_EnableAutoRestart(XHorner_core *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XHorner_core_DisableAutoRestart(XHorner_core *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_AP_CTRL, 0);
}

u32 XHorner_core_Get_return(XHorner_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_AP_RETURN);
    return Data;
}
void XHorner_core_Set_x(XHorner_core *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_X_DATA, Data);
}

u32 XHorner_core_Get_x(XHorner_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_X_DATA);
    return Data;
}

void XHorner_core_Set_a3(XHorner_core *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_A3_DATA, Data);
}

u32 XHorner_core_Get_a3(XHorner_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_A3_DATA);
    return Data;
}

void XHorner_core_Set_a2(XHorner_core *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_A2_DATA, Data);
}

u32 XHorner_core_Get_a2(XHorner_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_A2_DATA);
    return Data;
}

void XHorner_core_Set_a1(XHorner_core *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_A1_DATA, Data);
}

u32 XHorner_core_Get_a1(XHorner_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_A1_DATA);
    return Data;
}

void XHorner_core_Set_a0(XHorner_core *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_A0_DATA, Data);
}

u32 XHorner_core_Get_a0(XHorner_core *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_A0_DATA);
    return Data;
}

void XHorner_core_InterruptGlobalEnable(XHorner_core *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_GIE, 1);
}

void XHorner_core_InterruptGlobalDisable(XHorner_core *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_GIE, 0);
}

void XHorner_core_InterruptEnable(XHorner_core *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_IER);
    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_IER, Register | Mask);
}

void XHorner_core_InterruptDisable(XHorner_core *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_IER);
    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_IER, Register & (~Mask));
}

void XHorner_core_InterruptClear(XHorner_core *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHorner_core_WriteReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_ISR, Mask);
}

u32 XHorner_core_InterruptGetEnabled(XHorner_core *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_IER);
}

u32 XHorner_core_InterruptGetStatus(XHorner_core *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHorner_core_ReadReg(InstancePtr->Control_BaseAddress, XHORNER_CORE_CONTROL_ADDR_ISR);
}

