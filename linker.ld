ENTRY(_start)

/*No page aligining for now, but will have to have that later for SURE*/

/* ".__.*" pattern allows for matching subsections like .text.startup, .rodata.str1.8 and so on  */

SECTIONS 
{
    . = 0x80000;

    .text : {
        KEEP(*(.text.boot))
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss (NOLOAD) : {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(8);
        /*I'm aliging the bss because we zero it in start.S and the loop assumes multiple of 8 bss*/
        /*eventually ill definitelly want WHOLE PAGE alignment for EVERY section*/
        __bss_end = .;
    }

    . = ALIGN(16);
    . = . + 0x4000; /*16KB stack*/
    __stack_top = .;
}