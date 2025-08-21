################################################################################
# Target
################################################################################
TARGET = firmware

################################################################################
# Toolchain
################################################################################
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
SIZE = $(PREFIX)size

################################################################################
# Source files
################################################################################
C_SOURCES = $(wildcard src/*.c)
C_SOURCES += $(wildcard drivers/*.c)

# For now, we don't have assembly files, but this is a good placeholder
ASM_SOURCES =

################################################################################
# Includes and Defines
################################################################################
INCLUDES = -Iinclude -Idrivers

# Add defines here, e.g. -DSTM32F030x6
DEFINES = -DSTM32F030x6

################################################################################
# Compiler and Linker Flags
################################################################################
# CPU specific flags
CPU_FLAGS = -mcpu=cortex-m0 -mthumb

# Compiler flags
CFLAGS = $(CPU_FLAGS) -Og -g3 -Wall -Wextra -Wpedantic
CFLAGS += $(DEFINES)
CFLAGS += $(INCLUDES)
CFLAGS += -std=gnu11
CFLAGS += -ffunction-sections -fdata-sections

# Linker flags
# NOTE: A linker script is required for a real project.
# We will proceed without one for now, but it will be needed later.
LDFLAGS = $(CPU_FLAGS) -TSTM32F030K6T6_FLASH.ld -Wl,-Map=$(TARGET).map,--cref -Wl,--gc-sections

################################################################################
# Build artifacts
################################################################################
OBJS = $(C_SOURCES:.c=.o) $(ASM_SOURCES:.s=.o)
BUILD_DIR = build
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(OBJS)))

################################################################################
# Rules
################################################################################
.PHONY: all clean flash

all: $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	@echo "----------------"
	@echo "SIZE:"
	$(SIZE) $<
	@echo "----------------"

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: drivers/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET).elf $(TARGET).bin $(TARGET).map

# Placeholder for flashing with st-link
flash: all
	st-flash --reset write $(TARGET).bin 0x8000000
