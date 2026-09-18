CROSS   := aarch64-none-elf-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy

BUILD_DIR := build

CFLAGS  := -std=c23 -ffreestanding -mgeneral-regs-only -O2 \
           -Wall -Wextra -Wpedantic -Werror -Iinclude
LDFLAGS := -nostdlib -T linker.ld

SRCS := $(wildcard src/*.c src/*.S)
OBJS := $(patsubst src/%,$(BUILD_DIR)/%.o,$(SRCS))
HDRS := $(wildcard include/*.h)

all: $(BUILD_DIR)/kernel8.img

$(BUILD_DIR)/kernel8.img: $(BUILD_DIR)/kernel8.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/kernel8.elf: $(OBJS) linker.ld
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.c.o: src/%.c $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.S.o: src/%.S | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
