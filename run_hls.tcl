# =========================================
# CAMTA Vitis HLS Script
# =========================================
 
open_project camta_hls -reset
 
# Top function
set_top camta
 
# Design files
add_files src/camta.cpp
add_files src/horner_core.cpp
 
# Header files
add_files src/camta.h
add_files src/horner_core.h
 
# Testbench
add_files -tb tb/camta_tb.cpp
 
# Create solution
open_solution "solution1"
 
# FPGA target (puedes cambiarlo)
set_part {xck26-sfvc784-2LV-c}
 
# Clock
create_clock -period 4 -name default
 
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
 
# Export reports
export_design -format ip_catalog
 
exit