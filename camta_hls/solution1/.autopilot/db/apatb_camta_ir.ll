; ModuleID = 'C:/Users/carlo/OneDrive/Documents/Maestria/DAFPGA/Proyecto/Avances/proj_final_FPGA_CAMTA/camta_hls/solution1/.autopilot/db/a.g.ld.5.gdce.bc'
source_filename = "llvm-link"
target datalayout = "e-m:e-i64:64-i128:128-i256:256-i512:512-i1024:1024-i2048:2048-i4096:4096-n8:16:32:64-S128-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "fpga64-xilinx-none"

%"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>" = type { %"struct.ap_fixed_base<16, 6, true, AP_TRN, AP_WRAP, 0>" }
%"struct.ap_fixed_base<16, 6, true, AP_TRN, AP_WRAP, 0>" = type { %"struct.ssdm_int<16, true>" }
%"struct.ssdm_int<16, true>" = type { i16 }

; Function Attrs: argmemonly noinline norecurse readnone willreturn
define internal fastcc void @copy_in() unnamed_addr #0 {
entry:
  ret void
}

; Function Attrs: argmemonly noinline norecurse readnone willreturn
define internal fastcc void @copy_out() unnamed_addr #1 {
entry:
  ret void
}

declare void @apatb_camta_hw(%"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*)

; Function Attrs: argmemonly noinline norecurse readnone willreturn
define internal fastcc void @copy_back() unnamed_addr #1 {
entry:
  ret void
}

define void @camta_hw_stub_wrapper(%"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*) #2 {
entry:
  call void @copy_out()
  call void @camta_hw_stub(%"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %0, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %1, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %2, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %3, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %4, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %5, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %6, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %7, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %8, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %9, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %10, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %11, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %12, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %13, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %14)
  call void @copy_in()
  ret void
}

declare void @camta_hw_stub(%"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"*)

; Function Attrs: inaccessiblemem_or_argmemonly noinline willreturn
define void @apatb_camta_ir(%"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %ret, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %x, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %L, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a3_r0, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a2_r0, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a1_r0, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a0_r0, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a3_r1, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a2_r1, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a1_r1, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a0_r1, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a3_r2, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a2_r2, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a1_r2, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* nocapture readonly %a0_r2) #3 {
entry:
  call fastcc void @copy_in()
  %0 = alloca %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"
  call void @apatb_camta_hw(%"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %0, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %x, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %L, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a3_r0, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a2_r0, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a1_r0, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a0_r0, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a3_r1, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a2_r1, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a1_r1, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a0_r1, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a3_r2, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a2_r2, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a1_r2, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %a0_r2)
  %1 = load volatile %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>", %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %0
  call void @copy_back()
  %2 = load volatile %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>", %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %0
  store %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>" %2, %"struct.ap_fixed<16, 6, AP_TRN, AP_WRAP, 0>"* %ret
  ret void
}

attributes #0 = { argmemonly noinline norecurse readnone willreturn "fpga.wrapper.func"="copyin" }
attributes #1 = { argmemonly noinline norecurse readnone willreturn "fpga.wrapper.func"="copyout" }
attributes #2 = { "fpga.wrapper.func"="stub" }
attributes #3 = { inaccessiblemem_or_argmemonly noinline willreturn "fpga.wrapper.func"="wrapper" }

!llvm.dbg.cu = !{}
!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3}
!blackbox_cfg = !{!4}

!0 = !{!"clang version 7.0.0 "}
!1 = !{i32 2, !"Dwarf Version", i32 4}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{}
