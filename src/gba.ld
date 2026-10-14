OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY {
    rom   : ORIGIN = 0x08000000, LENGTH = 32M
    iwram : ORIGIN = 0x03000000, LENGTH = 31K
    ewram : ORIGIN = 0x02000000, LENGTH = 256K
}

SECTIONS {
    .text : {
        KEEP(*(.crt0))
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7 .glue_7t .vfp11_veneer .v4_bx)
        . = ALIGN(4);
    } > rom

    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        . = ALIGN(4);
        KEEP(*(.init)) KEEP(*(.fini))
        . = ALIGN(4);
    } > rom

    .ARM.exidx : { *(.ARM.exidx*) } > rom
    .init_array : { KEEP(*(.init_array*)) } > rom
    .fini_array : { KEEP(*(.fini_array*)) } > rom

    .data : {
        __data_start = .;
        *(.iwram_code)
        *(.iwram .iwram.*)
        *(.data .data.*)
        . = ALIGN(4);
        __data_end = .;
    } > iwram AT > rom
    __data_lma = LOADADDR(.data);

    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss .bss.* COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > iwram

    .ewram : {
        __ewram_start = .;
        *(.ewram .ewram.*)
        . = ALIGN(4);
        __ewram_end = .;
    } > ewram AT > rom
    __ewram_lma = LOADADDR(.ewram);

    .sbss (NOLOAD) : {
        __sbss_start = .;
        *(.sbss .sbss.*)
        . = ALIGN(4);
        __sbss_end = .;
        end = .; _end = .; __end__ = .;
    } > ewram

    /DISCARD/ : { *(.comment) *(.ARM.attributes) }
}
