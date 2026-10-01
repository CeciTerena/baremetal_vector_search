CROSS   := aarch64-none-elf-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy

LINUX_CC := aarch64-none-linux-gnu-gcc

BUILD_DIR   := build
SD_BM       := $(BUILD_DIR)/sd_bm
SD_LINUX    := $(BUILD_DIR)/sd_linux
BUILD_LINUX := $(BUILD_DIR)/linux

COMMON_CFLAGS := -std=c23 -O2 -Wall -Wextra -Wpedantic -Werror -Iinclude

CFLAGS  := $(COMMON_CFLAGS) -ffreestanding
LDFLAGS := -nostdlib -T linker.ld

LINUX_CFLAGS  := $(COMMON_CFLAGS)
LINUX_LDFLAGS := -static

SRCS := $(wildcard src/*.c src/*.S)
OBJS := $(patsubst src/%,$(BUILD_DIR)/%.o,$(SRCS))
HDRS := $(wildcard include/*.h)

SHARED_SRCS := src/search.c src/bench.c src/dataset.c src/checksum.c
LINUX_SRCS  := $(SHARED_SRCS) $(wildcard linux/*.c)
LINUX_OBJS  := $(patsubst %.c,$(BUILD_LINUX)/%.o,$(LINUX_SRCS))

all: sd_bm sd_linux

sd_bm: $(SD_BM)/baremetal.img $(SD_BM)/config.txt $(SD_BM)/data.bin

$(SD_BM)/baremetal.img: $(BUILD_DIR)/kernel8.elf | $(SD_BM)
	$(OBJCOPY) -O binary $< $@

$(SD_BM)/config.txt: boot/config_bm.txt | $(SD_BM)
	cp $< $@

$(SD_BM)/data.bin: data/data.bin | $(SD_BM)
	cp $< $@

$(BUILD_DIR)/kernel8.elf: $(OBJS) linker.ld
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.c.o: src/%.c $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.S.o: src/%.S | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR) $(SD_BM):
	mkdir -p $@

sd_linux: $(SD_LINUX)/bench $(SD_LINUX)/config.txt $(SD_LINUX)/cmdline.txt $(SD_LINUX)/data.bin \
          $(SD_LINUX)/run.sh

$(SD_LINUX)/bench: $(LINUX_OBJS) | $(SD_LINUX)
	$(LINUX_CC) $(LINUX_LDFLAGS) -o $@ $(LINUX_OBJS)

$(SD_LINUX)/config.txt: boot/config_linux.txt | $(SD_LINUX)
	cp $< $@

$(SD_LINUX)/cmdline.txt: boot/cmdline_linux.txt | $(SD_LINUX)
	cp $< $@

$(SD_LINUX)/data.bin: data/data.bin | $(SD_LINUX)
	cp $< $@

$(SD_LINUX)/run.sh: boot/run.sh | $(SD_LINUX)
	cp $< $@

$(BUILD_LINUX)/%.o: %.c $(HDRS)
	mkdir -p $(@D)
	$(LINUX_CC) $(LINUX_CFLAGS) -c -o $@ $<

$(SD_LINUX):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all sd_bm sd_linux clean
