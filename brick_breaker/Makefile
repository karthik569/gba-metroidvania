CC      := arm-none-eabi-gcc
AS      := arm-none-eabi-as
OBJCOPY := arm-none-eabi-objcopy
PYTHON  := python3

TARGET  := brick_breaker
BUILD   := build
SRC_DIR := src
INC_DIR := include

CFLAGS  := -mcpu=arm7tdmi -mtune=arm7tdmi -mthumb -mthumb-interwork \
           -O2 -Wall -Wextra -fomit-frame-pointer -ffast-math \
           -nostartfiles -I$(INC_DIR)

LDFLAGS := -T gba_cart.ld -nostartfiles

C_SRCS  := $(wildcard $(SRC_DIR)/*.c)
S_SRCS  := $(wildcard $(SRC_DIR)/*.s)
OBJS    := $(patsubst $(SRC_DIR)/%.c, $(BUILD)/%.o, $(C_SRCS)) \
           $(patsubst $(SRC_DIR)/%.s, $(BUILD)/%.o, $(S_SRCS))

.PHONY: all clean

all: $(TARGET).gba

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRC_DIR)/%.s | $(BUILD)
	$(AS) -mcpu=arm7tdmi -mthumb-interwork $< -o $@

$(BUILD)/%.o: $(SRC_DIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS) gba_cart.ld
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	$(PYTHON) tools/fix_header.py $@
	@echo "============================================="
	@echo "SUCCESS: $@ is built and ready to play!"
	@echo "============================================="

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).gba
