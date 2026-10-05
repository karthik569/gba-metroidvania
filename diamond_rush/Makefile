CC      := arm-none-eabi-gcc
AS      := arm-none-eabi-as
OBJCOPY := arm-none-eabi-objcopy
PYTHON  := python3

TARGET  := diamond_rush
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

ASSET_PNGS := $(wildcard assets/*.png)

.PHONY: all clean assets

all: $(TARGET).gba

assets: $(SRC_DIR)/asset_data.c

$(SRC_DIR)/asset_data.c $(INC_DIR)/asset_data.h: $(ASSET_PNGS) tools/build_assets.py
	$(PYTHON) tools/build_assets.py

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
