set SynModuleInfo {
  {SRCNAME horner_core MODELNAME horner_core RTLNAME horner_core IS_TOP 1
    SUBMODULES {
      {MODELNAME horner_core_mac_muladd_16s_16s_26ns_26_4_1 RTLNAME horner_core_mac_muladd_16s_16s_26ns_26_4_1 BINDTYPE op TYPE all IMPL dsp_slice LATENCY 3 ALLOW_PRAGMA 1}
      {MODELNAME horner_core_control_s_axi RTLNAME horner_core_control_s_axi BINDTYPE interface TYPE interface_s_axilite}
    }
  }
}
