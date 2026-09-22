#include "mem_management.h"
#include "uart.h"

uint32_t bump_alloc(bump_allocator *allocator, size_t requested_size, void **ptr) {
    uintptr_t start = align16(allocator->base + allocator->used);
    size_t offset = start - allocator->base;

    if (offset + requested_size > allocator->total_size) {
        uart_print("requested ammount could not be allocated\n");
        return BUMP_ALLOC_ERR;
    }
    *ptr = (void *)start;
    allocator->used = offset + requested_size;
    return BUMP_ALLOC_OK;
}
// TODO - make it more robust against overflows and whatnot. 

void bump_reset(bump_allocator *allocator) {
    allocator->used = 0;
}
