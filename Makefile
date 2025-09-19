# Makefile for flashing only
# This runs on the host machine where OpenOCD is available

PROJECT := docker-demo
BUILD_DIR := build
OPENOCD_CFGS := -f tools/openocd/interface-stlink.cfg -f tools/openocd/target-stm32l4x.cfg
ELF_FILE := $(BUILD_DIR)/$(PROJECT).elf
all: flash

flash:
	openocd $(OPENOCD_CFGS) -c "program $(ELF_FILE) verify reset exit"

.PHONY: all flash