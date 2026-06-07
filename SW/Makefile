# Host source and executable
SOURCES := camta.cpp
EXECUTABLES := camta

CXX ?= g++

# Buscar includes de HLS/Vitis desde variables de entorno ya cargadas
XILINX_HLS_INC   := $(if $(XILINX_HLS),-I$(XILINX_HLS)/include,)
XILINX_VITIS_INC := $(if $(XILINX_VITIS),-I$(XILINX_VITIS)/include,)

CFLAGS := -I/opt/xilinx/xrt/include \
          -Wall -std=c++17 -O0 -g \
          -I. -I./hls-types/include \
          $(XILINX_HLS_INC) $(XILINX_VITIS_INC)

LDFLAGS := -L/opt/xilinx/xrt/lib -pthread -lxrt_core -lxrt_coreutil

.PHONY: all clean

all: $(EXECUTABLES)

$(EXECUTABLES): %: %.cpp
        $(CXX) -o $@ $< $(CFLAGS) $(LDFLAGS)

clean:
        $(RM) $(EXECUTABLES)