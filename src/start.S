.section ".text.boot"

.global _start

_start:
    // enabling floating point / SIMD. 
    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    and     x0, x0, #3
    cmp     x0, #2
    b.ne    fp_el1              // not at EL2

    mrs     x1, cptr_el2
    bic     x1, x1, #(1 << 10)  // clear TFP -> no FP/SIMD trap at EL2
    msr     cptr_el2, x1

    fp_el1:
    mov     x1, #(3 << 20)      // FPEN = 0b11 -> no trap at EL1/EL0
    msr     cpacr_el1, x1
    isb

    // each core has this mpird register that tells which core we are in
    mrs     x0, mpidr_el1       // move register from system register --> read MPIDR_EL1 into a general purpose register x0.
    and     x0, x0, #3          // mask to just the lower 2 bits (3 = 11, encodes cores 0-3)
    cbz     x0, core0_continue  // if x0 is 0, we are core 0, skip the parking loop

park_loop:
    wfe
    b       park_loop
    // cores 1-3 stay in an infinite loop and don't proceed. 

core0_continue:
    ldr     x0, =__stack_top    // load addr of stack_top symbol
    mov     sp, x0              // set stack ptr there. 

    ldr     x1, =__bss_start    // x1 = write poiunter, start of .bss
    ldr     x2, =__bss_end      // x2 = end addr to stop at

zero_bss:
    cmp     x1, x2          // have we reached the end? 
    b.ge    zero_bss_done   // if x1 >= x2 done, leave loop.
    str     xzr, [x1], #8   // write 0 bytes at x1, then x1 += 8
    b       zero_bss        // repeat

zero_bss_done:
    bl      main    // call C main. no return to here


// In a normal scenario main returns an int, passed to the OS as an exit code
// here, baremetal, I NEED TO HANDLE THAT EXIT CODE (lol)
// and for simplicity now, my main() will NOT return. 
// And if it does, ill just stall the core instead of executing anything else. 
// later on I'll handle this more correctly
// but for now main() just shouldn't return. 

hang:   
    wfe             // if main does return, park.
    b       hang
