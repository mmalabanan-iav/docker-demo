# --- project names/paths ---
PROJECT        := docker-demo
SRC_DIR        := src
BUILD_DIR      := build
LINKER_DIR     := linker
LIBOPENCM3_DIR := libopencm3

# --- toolchain ---
PREFIX  := arm-none-eabi
CC      := $(PREFIX)-gcc
LD      := $(PREFIX)-gcc
AS      := $(PREFIX)-gcc
OBJCOPY := $(PREFIX)-objcopy
SIZE    := $(PREFIX)-size
GDB     := gdb-multiarch

# --- defs/flags ---
CPUFLAGS := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
DEFS     := -DSTM32L4

CFLAGS  := -Os -g3 -Wall -Wextra -ffunction-sections -fdata-sections \
           $(CPUFLAGS) $(DEFS) \
           -I$(SRC_DIR) \
           -I$(LIBOPENCM3_DIR)/include

LDFLAGS := $(CPUFLAGS) -Wl,--gc-sections -nostartfiles \
           -T $(LINKER_DIR)/stm32l432.ld \
           -L $(LIBOPENCM3_DIR)/lib
LDLIBS  := -lopencm3_stm32l4

# --- sources/objects ---
CSRCS := $(wildcard $(SRC_DIR)/*.c)
COBJS := $(CSRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
AOBJS := $(ASMS:$(SRC_DIR)/%.s=$(BUILD_DIR)/%.o)
OBJS  := $(AOBJS) $(COBJS)

# --- rules ---
all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).bin
		$(SIZE) $(BUILD_DIR)/$(PROJECT).elf

$(BUILD_DIR):
	mkdir -p $@

# build libopencm3 for stm32/l4 family
$(LIBOPENCM3_DIR)/lib/libopencm3_stm32l4.a:
	@if [ ! -f $@ ]; then \
		echo ">>> Building libopencm3 for STM32L4"; \
		$(MAKE) -C $(LIBOPENCM3_DIR) "TARGETS=stm32/l4 DEVICE=stm32l432kcu6 CFLAGS=$(CFLAGS)"; \
	fi

# assemble startup
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	$(AS) $(CPUFLAGS) $(DEFS) -x assembler-with-cpp -c $< -o $@

# compile C
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# link
$(BUILD_DIR)/$(PROJECT).elf: $(LIBOPENCM3_DIR)/lib/libopencm3_stm32l4.a $(OBJS) $(LINKER_DIR)/stm32l432.ld
	$(LD) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# binary
$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJCOPY) -O binary $< $@

# --- host flash/debug helpers (not run in Docker) ---
OPENOCD_CFGS = -f tools/openocd/interface-stlink.cfg -f tools/openocd/target-stm32l4x.cfg

flash: $(BUILD_DIR)/$(PROJECT).elf
	openocd $(OPENOCD_CFGS) -c "program $< verify reset exit"

# gdb-server:
# 	openocd $(OPENOCD_CFGS) -c "init; reset halt"

# gdb-load: $(BUILD_DIR)/$(PROJECT).elf
# 	$(GDB) $< -ex "target extended-remote localhost:3333" \
# 	           -ex "monitor reset halt" \
# 	           -ex load \
# 	           -ex "monitor reset init"

# --- cleanup ---
clean:
	rm -rf $(BUILD_DIR)

print-%:
	@echo "$($*)"

.PHONY: all clean flash 
# .PHONY: gdb-server gdb-load
