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

open_project horner_core -reset
set_top horner_core

# Archivos fuente
add_files "./HornerCore.cpp"
add_files -tb "./HornerCore_tb.cc"

# ==============================
# Crear solución
# ==============================

open_solution solution -flow_target vitis
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

close_project

puts "HLS completed successfully"
exit
