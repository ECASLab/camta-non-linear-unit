catch {::common::set_param -quiet hls.xocc.mode csynth};

# ==============================
# Parámetros por defecto
# ==============================

# Parte FPGA (Kria KV260)
if { [info exists ::env(PART) ] } {
  set part $::env(PART)
} else {
  set part "xck26-sfvc784-2LV-c"
}

# Frecuencia objetivo: 250 MHz
# Periodo = 4 ns
if { [info exists ::env(CLOCK_PERIOD) ] } {
  set clk_period $::env(CLOCK_PERIOD)
} else {
  set clk_period 4
}

# ==============================
# Crear proyecto
# ==============================

set top_name "horner_core"

# Por defecto no resetear para conservar soluciones historicas y compararlas.
if { [info exists ::env(RESET_PROJECT)] && $::env(RESET_PROJECT) eq "1" } {
  open_project horner_core -reset
} else {
  open_project horner_core
}

set_top $top_name

# Archivos fuente
add_files "./HornerCore.cpp"
add_files -tb "./HornerCore_tb.cc"

# ==============================
# Crear solución
# ==============================

set datetime_str [clock format [clock seconds] -format "%Y%m%d_%H%M%S"]
set solution_name "solution_${top_name}_${datetime_str}"
open_solution $solution_name -flow_target vitis
set_part $part

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
csynth_design

# Ejecutar comparacion de soluciones despues de la sintesis
set compare_script ""
set script_candidates [list \
  [file normalize [file join [pwd] ".." "scripts" "compare_hls_solutions.py"]] \
  [file normalize [file join [pwd] "scripts" "compare_hls_solutions.py"]] \
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
      exec $py_cmd $compare_script --project-dir [pwd] --report-glob "horner_core*/solution*/syn/report/*_csynth.rpt"
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

close_project

puts "HLS completed successfully"
exit
