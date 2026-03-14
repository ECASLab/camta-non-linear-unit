set SynModuleInfo {
  {SRCNAME horner_core_pipeline MODELNAME horner_core_pipeline RTLNAME horner_core_pipeline IS_TOP 1
    SUBMODULES {
      {MODELNAME horner_core_pipeline_mac_muladd_16s_16s_26ns_26_4_1 RTLNAME horner_core_pipeline_mac_muladd_16s_16s_26ns_26_4_1 BINDTYPE op TYPE all IMPL dsp_slice LATENCY 3 ALLOW_PRAGMA 1}
      {MODELNAME horner_core_pipeline_control_s_axi RTLNAME horner_core_pipeline_control_s_axi BINDTYPE interface TYPE interface_s_axilite}
    }
  }
}
