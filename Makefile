#——————————————————————————————————————————————————————
# Makefile for Raspberry Pi Pico (RP2040) projects
#
# Usage:
#   make         # configure + build PicoGreg.uf2
#   make clean   # delete build/
#   make flash   # copy UF2 onto Pico (in BOOTSEL mode)
#——————————————————————————————————————————————————————

 # Where you cloned the Pico SDK (no spaces!):
 PICO_SDK_PATH ?= $(HOME)/pico/pico-sdk

 BUILD_DIR := build
 PROJECT   := PicoGreg

 .PHONY: all clean flash
 all: $(BUILD_DIR)/$(PROJECT).uf2

 # Build rule
 $(BUILD_DIR)/$(PROJECT).uf2: | $(BUILD_DIR)
	cd $(BUILD_DIR) && \
	cmake \
	  -DPICO_SDK_PATH=$(PICO_SDK_PATH) \
	  -DCMAKE_BUILD_TYPE=Release \
	  ..
	@echo "→ Building…"
	cd $(BUILD_DIR) && \
	cmake --build . -- -j$(shell nproc)

 # Ensure build directory exists
 $(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

 # Wipe out the build directory
 clean:
	rm -rf $(BUILD_DIR)

 # Flash the Pico (adjust mount point if yours differs)
 flash: all
	@echo "→ Flashing UF2 onto Pico…"
	cp $(BUILD_DIR)/$(PROJECT).uf2 /media/$(USER)/RPI-RP2/

#---------------------------------------
# Additional flags for pico_i2c_slave
CFLAGS += -I$(CURDIR)/pico_i2c_slave/include      # Add header path :contentReference[oaicite:8]{index=8}
SRCS   += pico_i2c_slave/i2c_slave/i2c_slave.c     # Compile the I2C slave source :contentReference[oaicite:9]{index=9}
LDFLAGS += -lhardware_i2c                          # Ensure I2C SDK lib is linked :contentReference[oaicite:10]{index=10}

