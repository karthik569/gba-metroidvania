    .arm
    .section .crt0, "ax"
    .global _start

_start:
    b       rom_header_end

    @ Nintendo Logo (156 bytes)
    .fill   156, 1, 0

    @ Game Title (12 bytes)
    .ascii  "BRK_BREAKER\0"

    @ Game Code (4 bytes)
    .ascii  "ABKE"

    @ Maker Code (2 bytes)
    .ascii  "01"

    @ Fixed Value (0x96)
    .byte   0x96

    @ Main unit code
    .byte   0x00

    @ Device type
    .byte   0x00

    @ Reserved (7 bytes)
    .fill   7, 1, 0

    @ Software version
    .byte   0x00

    @ Complement checksum
    .byte   0x00

    @ Reserved (2 bytes)
    .byte   0x00, 0x00

rom_header_end:
    @ Set up IRQ Stack
    mov     r0, #0x12       @ IRQ mode
    msr     cpsr, r0
    ldr     sp, =__sp_irq

    @ Set up System Stack
    mov     r0, #0x1F       @ System mode
    msr     cpsr, r0
    ldr     sp, =__sp_usr

    @ Copy .data from ROM to EWRAM
    ldr     r0, =_sidata
    ldr     r1, =_sdata
    ldr     r2, =_edata
copy_data_loop:
    cmp     r1, r2
    ldrlt   r3, [r0], #4
    strlt   r3, [r1], #4
    blt     copy_data_loop

    @ Copy .iwram from ROM to IWRAM
    ldr     r0, =_siwram
    ldr     r1, =_siwram_start
    ldr     r2, =_eiwram_end
copy_iwram_loop:
    cmp     r1, r2
    ldrlt   r3, [r0], #4
    strlt   r3, [r1], #4
    blt     copy_iwram_loop

    @ Zero .bss in EWRAM
    mov     r0, #0
    ldr     r1, =_sbss
    ldr     r2, =_ebss
zero_bss_loop:
    cmp     r1, r2
    strlt   r0, [r1], #4
    blt     zero_bss_loop

    @ Call main()
    ldr     r3, =main
    bx      r3

hang:
    b       hang
