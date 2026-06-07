# =========================================
# CAMTA Vitis HLS Script
# Pure scalar unit synthesis
# =========================================

# Project
set project_name "camta_unit_hls"

# open_project $project_name -reset
open_project $project_name

# Top function
set top_name "camta_unit"
set_top $top_name

# Design files
add_files src/camta.cpp -cflags "-Isrc"
add_files src/horner_core.cpp -cflags "-Isrc"

# Header files
add_files src/camta.h
add_files src/camta_types.h
add_files src/horner_core.h

# Testbench
# Para camta_unit no usamos testbench acá porque es unidad escalar pura.
# El testbench anterior aplica mejor para set_top camta.
# add_files -tb tb/camta_tb.cpp -cflags "-Isrc -Itb"

# Create solution
set datetime_str [clock format [clock seconds] -format "%Y%m%d_%H%M%S"]
set solution_name "solution_${top_name}_${datetime_str}"
open_solution $solution_name -flow_target vitis

# FPGA target
# Parte FPGA Kria KV260 por defecto
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

# Opciones RTL
config_rtl -deadlock_detection sim

# Nota:
# Para camta_unit no hay interfaces m_axi porque no es kernel con memoria.
# Estas opciones se dejan comentadas porque aplican al wrapper camta.
# config_interface -m_axi_addr64
# config_interface -m_axi_conservative_mode=1
# config_interface -m_axi_auto_max_ports=0

# Export configuration
config_export -format xo -ipname $top_name

# ==============================
# Simulación y síntesis
# ==============================

# Para camta_unit no corremos csim_design con este testbench.
# El testbench actual llama a camta(), no a camta_unit().
# csim_design -clean

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

	set report_glob "${project_name}/solution*/syn/report/*_csynth.rpt"

	if {
		[catch {
			exec $py_cmd $compare_script --project-dir [pwd] --top $top_name --report-glob $report_glob
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

# Export reports / IP
export_design -format ip_catalog

exit