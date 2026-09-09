OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY { RAM (rwxa) : ORIGIN = 0x80000000, LENGTH = 128M }

SECTIONS {
    .text   : { *(.text.init) *(.text*) }   > RAM
    .rodata : { *(.rodata*) }               > RAM
    .data   : { *(.data*) }                 > RAM
    .bss    : {
        __bss_start = .;
        *(.bss*) *(COMMON)
        __bss_end = .;
    } > RAM

    . = ALIGN(16);
    . += 4096;
    _stack_top = .;
}