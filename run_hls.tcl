# =========================================
# CAMTA Vitis HLS Script
# =========================================
 
#open_project camta_hls -reset
open_project camta_hls
 
# Top function
set top_name "camta"
set_top $top_name
 
# Design files
add_files src/camta.cpp
add_files src/horner_core.cpp
 
# Header files
add_files src/camta.h
add_files src/camta_types.h
add_files src/horner_core.h
 
# Testbench
add_files -tb tb/camta_tb.cpp
 
# Create solution
set datetime_str [clock format [clock seconds] -format "%Y%m%d_%H%M%S"]
set solution_name "solution_${top_name}_${datetime_str}"
open_solution $solution_name -flow_target vitis
 
# FPGA target (puedes cambiarlo)
# Parte FPGA (Kria KV260)
if { [info exists ::env(PART) ] } {
  set part $::env(PART)
} else {
  set part "xck26-sfvc784-2LV-c"
}

set_part $part
 
# Clock
if { [info exists ::env(CLOCK_PERIOD) ] } {
  set clk_period $::env(CLOCK_PERIOD)
} else {
  set clk_period 4
}

create_clock -period $clk_period -name default
 
# Opciones similares al ejemplo original
config_rtl -deadlock_detection sim
config_interface -m_axi_addr64
config_interface -m_axi_conservative_mode=1
config_interface -m_axi_auto_max_ports=0

# Exportar como kernel XO
config_export -format xo -ipname horner_core

# ==============================
# Simulación y síntesis
# ==============================

# (activar si quieres sim C)
csim_design -clean
 
# Run synthesis
csynth_design

# Ejecutar comparación de soluciones después de la síntesis
set compare_script ""
set script_candidates [list \
	[file normalize [file join [pwd] "scripts" "compare_hls_solutions.py"]] \
	[file normalize [file join [pwd] ".." "scripts" "compare_hls_solutions.py"]] \
]

foreach candidate $script_candidates {
	if { [file exists $candidate] } {
		set compare_script $candidate
		break
	}
}

if { $compare_script ne "" } {
	puts "Running post-synthesis comparison script..."

	set py_cmd "python"
	if { [catch {exec $py_cmd --version} py_check] } {
		set py_cmd "py"
	}

	if {
		[catch {
			exec $py_cmd $compare_script --project-dir [pwd] --top camta --report-glob "camta_hls/solution*/syn/report/*_csynth.rpt"
		} compare_output]
	} {
		puts "Warning: comparison script failed."
		puts $compare_output
	} else {
		puts $compare_output
	}
} else {
	puts "Warning: comparison script not found in expected locations"
}
 
# Export reports
export_design -format ip_catalog
 
exit
