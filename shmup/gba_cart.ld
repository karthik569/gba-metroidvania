/* Linker script for Game Boy Advance Cartridges */
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY {
    rom   (rx)  : ORIGIN = 0x08000000, LENGTH = 32M
    ewram (rwx) : ORIGIN = 0x02000000, LENGTH = 256K
    iwram (rwx) : ORIGIN = 0x03000000, LENGTH = 32K
}

__sp_usr = 0x03007F00;
__sp_irq = 0x03007FA0;

SECTIONS {
    .crt0 : {
        KEEP(*(.crt0))
    } > rom

    .text : {
        *(.text .text.*)
        *(.gnu.linkonce.t.*)
        . = ALIGN(4);
    } > rom

    .rodata : {
        *(.rodata .rodata.*)
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
    } > rom

    /* Initialized data placed in ROM, loaded to EWRAM on startup */
    _sidata = LOADADDR(.data);
    .data : {
        _sdata = .;
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(4);
        _edata = .;
    } > ewram AT > rom

    /* Uninitialized data in EWRAM */
    .bss (NOLOAD) : {
        _sbss = .;
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > ewram

    /* Fast IWRAM code/data */
    _siwram = LOADADDR(.iwram);
    .iwram : {
        _siwram_start = .;
        *(.iwram .iwram.*)
        . = ALIGN(4);
        _eiwram_end = .;
    } > iwram AT > rom

    /* Discard unused sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.ARM.attributes)
    }
}
