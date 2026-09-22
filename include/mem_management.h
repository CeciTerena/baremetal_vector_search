#ifndef MEMMANAGE_H
#define MEMMANAGE_H

#include <stddef.h>
#include <stdint.h>

extern char __heap_start[]; // from linker.ld
extern char __heap_end[];
#define HEAP_END ((uintptr_t)__heap_end)
#define HEAP_START ((uintptr_t)__heap_start)
#define DATASET_BASE HEAP_END // NEEDS TO MATCH INITRAMFS IN CONFIG.TXT
#define KERNEL 0x80000 // NEEDS TO MATCH LINKER.LD!

#define BUMP_ALLOC_OK 0
#define BUMP_ALLOC_ERR 1

typedef struct bump_allocator bump_allocator;

struct bump_allocator {
  uintptr_t base;
  size_t total_size;
  size_t used;
};

static inline uint64_t align16(uint64_t x) {
    return (x + 15) & ~(uint64_t)15;
}

static inline void bump_allocator_init(bump_allocator *allocator) {
  allocator->base = HEAP_START;
  allocator->total_size = HEAP_END - HEAP_START;
  allocator->used = 0;
}

uint32_t bump_alloc(bump_allocator *allocator, size_t size, void **ptr); //the ptr that gets returned is 16 byte aligned. 
void bump_reset(bump_allocator *allocator);

#endif
