#include <stdint.h>
#include "mmu.h"
#include "uart.h"

#define MEM_TYPE_DEVICE 0
#define MEM_TYPE_NORMAL 1

// block descriptors --> tell what (and where) exactly that region of memory is/can do.
#define DESC_VALID_BLOCK (1ULL << 0)            // [bits 1:0] valid entry, block type
#define DESC_MEM_TYPE(i) ((uint64_t)(i) << 2)   // [AttrIndx] which memory type
#define DESC_EL2_RES1 (1ULL << 6)               // [AP[1]] must be 1 at EL2
#define DESC_SHARED_CORES (3ULL << 8)           // [SH] inner shareable: caches coherent across cores
#define DESC_ACCESSED (1ULL << 10)              // [AF] access flag: 0 = fault on first use
#define DESC_NO_EXECUTE (1ULL << 54)            // [XN] never fetch instructions from here

#define GB (1ULL << 30)

// MAIR_EL2: defines the memory types
#define MAIR_DEVICE 0x00ULL
#define MAIR_NORMAL 0xFFULL
#define MAIR_VALUE ((MAIR_DEVICE << (8 * MEM_TYPE_DEVICE)) | (MAIR_NORMAL << (8 * MEM_TYPE_NORMAL)))

// TCR_EL2: how translation works
#define TCR_T0SZ_4GB (32ULL)        // [T0SZ] address space = 2^(64-32) = 4 GB -> walk starts at L1
#define TCR_WALK_WB (5ULL << 8)     // [IRGN0, ORGN0] table walks read through the caches
#define TCR_WALK_SH (3ULL << 12)    // [SH0] table memory is inner shareable
#define TCR_GRANULE_4K (0ULL << 14) // [TG0] 4 KB table format
#define TCR_PA_32BIT (0ULL << 16)   // [PS] physical addresses are 32 bits (4 GB)
#define TCR_EL2_RES1 ((1ULL << 31) | (1ULL << 23))  // must be 1 at EL2
#define TCR_VALUE (TCR_T0SZ_4GB | TCR_WALK_WB | TCR_WALK_SH | TCR_GRANULE_4K | TCR_PA_32BIT | TCR_EL2_RES1)

#define SCTLR_MMU (1ULL << 0)       // [M] translation on
#define SCTLR_ALIGN (1ULL << 1)     // [A] fault on unaligned access (OFF)
#define SCTLR_DCACHE (1ULL << 2)    // [C] data cache on
#define SCTLR_ICACHE (1ULL << 12)   // [I] instruction cache on
#define SCTLR_WXN (1ULL << 19)      // [WXN] writable => not executable: must be OFF (writing and executing in the same block)


// 4KB aligned
static alignas(4096) uint64_t l1_table[4];
// each entry is a block descriptor (4 entries, 1GB each)

// 1 block = 1 GB.
// my pi only has 1GB usable in RAM for the CPU. (948mb more specifically)
// but the adress space can of course be much larger. The mem mapped IO has its adresses near 4GB. 
// for that purpose, ill cover a 4GB adress space with the table.

// Because the mapping is one to one (virtual = physical) (again to keep things simple) 
// ill map the first GB (0-1) for everything real (because thats the actual ammount of RAM these is), and the "last GB" (3-4) for the HW registers.  

// entries 1 - 2 stay invalid because nothing is actually wired to adresses 1-3GB. 
static void build_table(void) {
    l1_table[0] = 0 * GB | DESC_VALID_BLOCK | DESC_MEM_TYPE(MEM_TYPE_NORMAL) | DESC_EL2_RES1 | DESC_SHARED_CORES | DESC_ACCESSED;
    l1_table[3] = 3 * GB | DESC_VALID_BLOCK | DESC_MEM_TYPE(MEM_TYPE_DEVICE) | DESC_EL2_RES1 | DESC_ACCESSED | DESC_NO_EXECUTE;
}

static uint64_t current_el(void) {
    uint64_t el;
    __asm__ volatile("mrs %0, CurrentEL" : "=r"(el));
    return (el >> 2) & 3;
}

static void set_memory_types(void) {
    __asm__ volatile("msr mair_el2, %0" :: "r"(MAIR_VALUE));
}

static void set_translation_control(void) {
    __asm__ volatile("msr tcr_el2, %0" :: "r"(TCR_VALUE));
}

static void set_table_base(void) {
    __asm__ volatile("msr ttbr0_el2, %0" :: "r"((uint64_t)l1_table));
}

// thrwoing away any cached translations / instructions from before
static void forget_old_translations(void) {
    __asm__ volatile("dsb ish\n\t"
                     "tlbi alle2\n\t"
                     "ic iallu\n\t"
                     "dsb ish\n\t"
                     "isb" ::: "memory");
}

static void turn_on_mmu_and_caches(void) {
    uint64_t sctlr;
    __asm__ volatile("mrs %0, sctlr_el2" : "=r"(sctlr));
    sctlr |= SCTLR_MMU | SCTLR_DCACHE | SCTLR_ICACHE;
    sctlr &= ~(SCTLR_ALIGN | SCTLR_WXN);
    // turns on the MMU and instruction + data caches
    __asm__ volatile("msr sctlr_el2, %0\n\t"
                     "isb" :: "r"(sctlr) : "memory");
}

void mmu_enable(void) {
    if (current_el() != 2) {
        uart_print("MMU: not at EL2. caches OFF\n");
        return;
    }
    build_table();
    set_memory_types();
    set_translation_control();
    set_table_base();
    forget_old_translations();
    turn_on_mmu_and_caches();
}