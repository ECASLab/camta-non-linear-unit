set moduleName camta
set isTopModule 1
set isCombinational 0
set isDatapathOnly 0
set isPipelined 1
set pipeline_type function
set FunctionProtocol ap_ctrl_hs
set isOneStateSeq 0
set ProfileFlag 0
set StallSigGenFlag 0
set isEnableWaveformDebug 1
set hasInterrupt 0
set DLRegFirstOffset 0
set DLRegItemOffset 0
set C_modelName {camta}
set C_modelType { int 16 }
set C_modelArgList {
	{ x int 16 regular  }
	{ L int 16 regular  }
	{ a3_r0 int 16 regular  }
	{ a2_r0 int 16 regular  }
	{ a1_r0 int 16 regular  }
	{ a0_r0 int 16 regular  }
	{ a3_r1 int 16 regular  }
	{ a2_r1 int 16 regular  }
	{ a1_r1 int 16 regular  }
	{ a0_r1 int 16 regular  }
	{ a3_r2 int 16 regular  }
	{ a2_r2 int 16 regular  }
	{ a1_r2 int 16 regular  }
	{ a0_r2 int 16 regular  }
}
set hasAXIMCache 0
set AXIMCacheInstList { }
set C_modelArgMapList {[ 
	{ "Name" : "x", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "L", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a3_r0", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a2_r0", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a1_r0", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a0_r0", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a3_r1", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a2_r1", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a1_r1", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a0_r1", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a3_r2", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a2_r2", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a1_r2", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "a0_r2", "interface" : "wire", "bitwidth" : 16, "direction" : "READONLY"} , 
 	{ "Name" : "ap_return", "interface" : "wire", "bitwidth" : 16} ]}
# RTL Port declarations: 
set portNum 21
set portList { 
	{ ap_clk sc_in sc_logic 1 clock -1 } 
	{ ap_rst sc_in sc_logic 1 reset -1 active_high_sync } 
	{ ap_start sc_in sc_logic 1 start -1 } 
	{ ap_done sc_out sc_logic 1 predone -1 } 
	{ ap_idle sc_out sc_logic 1 done -1 } 
	{ ap_ready sc_out sc_logic 1 ready -1 } 
	{ x sc_in sc_lv 16 signal 0 } 
	{ L sc_in sc_lv 16 signal 1 } 
	{ a3_r0 sc_in sc_lv 16 signal 2 } 
	{ a2_r0 sc_in sc_lv 16 signal 3 } 
	{ a1_r0 sc_in sc_lv 16 signal 4 } 
	{ a0_r0 sc_in sc_lv 16 signal 5 } 
	{ a3_r1 sc_in sc_lv 16 signal 6 } 
	{ a2_r1 sc_in sc_lv 16 signal 7 } 
	{ a1_r1 sc_in sc_lv 16 signal 8 } 
	{ a0_r1 sc_in sc_lv 16 signal 9 } 
	{ a3_r2 sc_in sc_lv 16 signal 10 } 
	{ a2_r2 sc_in sc_lv 16 signal 11 } 
	{ a1_r2 sc_in sc_lv 16 signal 12 } 
	{ a0_r2 sc_in sc_lv 16 signal 13 } 
	{ ap_return sc_out sc_lv 16 signal -1 } 
}
set NewPortList {[ 
	{ "name": "ap_clk", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "clock", "bundle":{"name": "ap_clk", "role": "default" }} , 
 	{ "name": "ap_rst", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "reset", "bundle":{"name": "ap_rst", "role": "default" }} , 
 	{ "name": "ap_start", "direction": "in", "datatype": "sc_logic", "bitwidth":1, "type": "start", "bundle":{"name": "ap_start", "role": "default" }} , 
 	{ "name": "ap_done", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "predone", "bundle":{"name": "ap_done", "role": "default" }} , 
 	{ "name": "ap_idle", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "done", "bundle":{"name": "ap_idle", "role": "default" }} , 
 	{ "name": "ap_ready", "direction": "out", "datatype": "sc_logic", "bitwidth":1, "type": "ready", "bundle":{"name": "ap_ready", "role": "default" }} , 
 	{ "name": "x", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "x", "role": "default" }} , 
 	{ "name": "L", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "L", "role": "default" }} , 
 	{ "name": "a3_r0", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a3_r0", "role": "default" }} , 
 	{ "name": "a2_r0", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a2_r0", "role": "default" }} , 
 	{ "name": "a1_r0", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a1_r0", "role": "default" }} , 
 	{ "name": "a0_r0", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a0_r0", "role": "default" }} , 
 	{ "name": "a3_r1", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a3_r1", "role": "default" }} , 
 	{ "name": "a2_r1", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a2_r1", "role": "default" }} , 
 	{ "name": "a1_r1", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a1_r1", "role": "default" }} , 
 	{ "name": "a0_r1", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a0_r1", "role": "default" }} , 
 	{ "name": "a3_r2", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a3_r2", "role": "default" }} , 
 	{ "name": "a2_r2", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a2_r2", "role": "default" }} , 
 	{ "name": "a1_r2", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a1_r2", "role": "default" }} , 
 	{ "name": "a0_r2", "direction": "in", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "a0_r2", "role": "default" }} , 
 	{ "name": "ap_return", "direction": "out", "datatype": "sc_lv", "bitwidth":16, "type": "signal", "bundle":{"name": "ap_return", "role": "default" }}  ]}

set RtlHierarchyInfo {[
	{"ID" : "0", "Level" : "0", "Path" : "`AUTOTB_DUT_INST", "Parent" : "", "Child" : ["1"],
		"CDFG" : "camta",
		"Protocol" : "ap_ctrl_hs",
		"ControlExist" : "1", "ap_start" : "1", "ap_ready" : "1", "ap_done" : "1", "ap_continue" : "0", "ap_idle" : "1", "real_start" : "0",
		"Pipeline" : "Aligned", "UnalignedPipeline" : "0", "RewindPipeline" : "0", "ProcessNetwork" : "0",
		"II" : "1",
		"VariableLatency" : "0", "ExactLatency" : "7", "EstimateLatencyMin" : "7", "EstimateLatencyMax" : "7",
		"Combinational" : "0",
		"Datapath" : "0",
		"ClockEnable" : "0",
		"HasSubDataflow" : "0",
		"InDataflowNetwork" : "0",
		"HasNonBlockingOperation" : "0",
		"IsBlackBox" : "0",
		"Port" : [
			{"Name" : "x", "Type" : "None", "Direction" : "I"},
			{"Name" : "L", "Type" : "None", "Direction" : "I"},
			{"Name" : "a3_r0", "Type" : "None", "Direction" : "I"},
			{"Name" : "a2_r0", "Type" : "None", "Direction" : "I"},
			{"Name" : "a1_r0", "Type" : "None", "Direction" : "I"},
			{"Name" : "a0_r0", "Type" : "None", "Direction" : "I"},
			{"Name" : "a3_r1", "Type" : "None", "Direction" : "I"},
			{"Name" : "a2_r1", "Type" : "None", "Direction" : "I"},
			{"Name" : "a1_r1", "Type" : "None", "Direction" : "I"},
			{"Name" : "a0_r1", "Type" : "None", "Direction" : "I"},
			{"Name" : "a3_r2", "Type" : "None", "Direction" : "I"},
			{"Name" : "a2_r2", "Type" : "None", "Direction" : "I"},
			{"Name" : "a1_r2", "Type" : "None", "Direction" : "I"},
			{"Name" : "a0_r2", "Type" : "None", "Direction" : "I"}]},
	{"ID" : "1", "Level" : "1", "Path" : "`AUTOTB_DUT_INST.grp_horner_core_fu_140", "Parent" : "0", "Child" : ["2", "3", "4"],
		"CDFG" : "horner_core",
		"Protocol" : "ap_ctrl_hs",
		"ControlExist" : "0", "ap_start" : "0", "ap_ready" : "0", "ap_done" : "0", "ap_continue" : "0", "ap_idle" : "0", "real_start" : "0",
		"Pipeline" : "Aligned", "UnalignedPipeline" : "0", "RewindPipeline" : "0", "ProcessNetwork" : "0",
		"II" : "1",
		"VariableLatency" : "0", "ExactLatency" : "5", "EstimateLatencyMin" : "5", "EstimateLatencyMax" : "5",
		"Combinational" : "0",
		"Datapath" : "1",
		"ClockEnable" : "1",
		"HasSubDataflow" : "0",
		"InDataflowNetwork" : "0",
		"HasNonBlockingOperation" : "0",
		"IsBlackBox" : "0",
		"Port" : [
			{"Name" : "x_val", "Type" : "None", "Direction" : "I"},
			{"Name" : "a3_val", "Type" : "None", "Direction" : "I"},
			{"Name" : "a2_val", "Type" : "None", "Direction" : "I"},
			{"Name" : "a1_val", "Type" : "None", "Direction" : "I"},
			{"Name" : "a0_val", "Type" : "None", "Direction" : "I"}]},
	{"ID" : "2", "Level" : "2", "Path" : "`AUTOTB_DUT_INST.grp_horner_core_fu_140.mul_20s_16s_36_1_0_U1", "Parent" : "1"},
	{"ID" : "3", "Level" : "2", "Path" : "`AUTOTB_DUT_INST.grp_horner_core_fu_140.mul_24s_16s_40_1_0_U2", "Parent" : "1"},
	{"ID" : "4", "Level" : "2", "Path" : "`AUTOTB_DUT_INST.grp_horner_core_fu_140.mul_24s_16s_40_1_0_U3", "Parent" : "1"}]}


set ArgLastReadFirstWriteLatency {
	camta {
		x {Type I LastRead 0 FirstWrite -1}
		L {Type I LastRead 0 FirstWrite -1}
		a3_r0 {Type I LastRead 0 FirstWrite -1}
		a2_r0 {Type I LastRead 0 FirstWrite -1}
		a1_r0 {Type I LastRead 0 FirstWrite -1}
		a0_r0 {Type I LastRead 0 FirstWrite -1}
		a3_r1 {Type I LastRead 0 FirstWrite -1}
		a2_r1 {Type I LastRead 0 FirstWrite -1}
		a1_r1 {Type I LastRead 0 FirstWrite -1}
		a0_r1 {Type I LastRead 0 FirstWrite -1}
		a3_r2 {Type I LastRead 0 FirstWrite -1}
		a2_r2 {Type I LastRead 0 FirstWrite -1}
		a1_r2 {Type I LastRead 0 FirstWrite -1}
		a0_r2 {Type I LastRead 0 FirstWrite -1}}
	horner_core {
		x_val {Type I LastRead 0 FirstWrite -1}
		a3_val {Type I LastRead 0 FirstWrite -1}
		a2_val {Type I LastRead 0 FirstWrite -1}
		a1_val {Type I LastRead 0 FirstWrite -1}
		a0_val {Type I LastRead 0 FirstWrite -1}}}

set hasDtUnsupportedChannel 0

set PerformanceInfo {[
	{"Name" : "Latency", "Min" : "7", "Max" : "7"}
	, {"Name" : "Interval", "Min" : "1", "Max" : "1"}
]}

set PipelineEnableSignalInfo {[
	{"Pipeline" : "0", "EnableSignal" : "ap_enable_pp0"}
]}

set Spec2ImplPortList { 
	x { ap_none {  { x in_data 0 16 } } }
	L { ap_none {  { L in_data 0 16 } } }
	a3_r0 { ap_none {  { a3_r0 in_data 0 16 } } }
	a2_r0 { ap_none {  { a2_r0 in_data 0 16 } } }
	a1_r0 { ap_none {  { a1_r0 in_data 0 16 } } }
	a0_r0 { ap_none {  { a0_r0 in_data 0 16 } } }
	a3_r1 { ap_none {  { a3_r1 in_data 0 16 } } }
	a2_r1 { ap_none {  { a2_r1 in_data 0 16 } } }
	a1_r1 { ap_none {  { a1_r1 in_data 0 16 } } }
	a0_r1 { ap_none {  { a0_r1 in_data 0 16 } } }
	a3_r2 { ap_none {  { a3_r2 in_data 0 16 } } }
	a2_r2 { ap_none {  { a2_r2 in_data 0 16 } } }
	a1_r2 { ap_none {  { a1_r2 in_data 0 16 } } }
	a0_r2 { ap_none {  { a0_r2 in_data 0 16 } } }
}

set maxi_interface_dict [dict create]

# RTL port scheduling information:
set fifoSchedulingInfoList { 
}

# RTL bus port read request latency information:
set busReadReqLatencyList { 
}

# RTL bus port write response latency information:
set busWriteResLatencyList { 
}

# RTL array port load latency information:
set memoryLoadLatencyList { 
}
