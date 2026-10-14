@ MonsterMons GBA startup code
    .section .crt0, "ax"
    .arm
    .global _start
_start:
    b       rom_start
    .fill   156, 1, 0               @ Nintendo logo (patched by gbafix.py)
    .fill   12, 1, 0                @ title
    .fill   4, 1, 0                 @ game code
    .fill   2, 1, 0                 @ maker
    .byte   0x96                    @ fixed
    .byte   0, 0                    @ unit, device
    .fill   7, 1, 0
    .byte   0                       @ version
    .byte   0                       @ complement
    .fill   2, 1, 0

rom_start:
    mov     r0, #0x12               @ IRQ mode
    msr     cpsr_c, r0
    ldr     sp, =0x03007FA0
    mov     r0, #0x1F               @ System mode
    msr     cpsr_c, r0
    ldr     sp, =0x03007F00

    @ copy .data / .iwram code to IWRAM
    ldr     r0, =__data_lma
    ldr     r1, =__data_start
    ldr     r2, =__data_end
1:  cmp     r1, r2
    ldrlt   r3, [r0], #4
    strlt   r3, [r1], #4
    blt     1b

    @ zero .bss
    ldr     r1, =__bss_start
    ldr     r2, =__bss_end
    mov     r3, #0
2:  cmp     r1, r2
    strlt   r3, [r1], #4
    blt     2b

    @ copy .ewram data
    ldr     r0, =__ewram_lma
    ldr     r1, =__ewram_start
    ldr     r2, =__ewram_end
3:  cmp     r1, r2
    ldrlt   r3, [r0], #4
    strlt   r3, [r1], #4
    blt     3b

    @ zero .sbss (ewram bss)
    ldr     r1, =__sbss_start
    ldr     r2, =__sbss_end
    mov     r3, #0
4:  cmp     r1, r2
    strlt   r3, [r1], #4
    blt     4b

    @ install IRQ handler
    ldr     r0, =irq_handler
    ldr     r1, =0x03007FFC
    str     r0, [r1]

    ldr     r0, =main
    mov     lr, pc
    bx      r0
    b       .

    .section .iwram_code, "ax"
    .arm
    .align 2
    .global irq_handler
irq_handler:
    mov     r2, #0x04000000
    add     r2, r2, #0x200
    ldr     r1, [r2]                @ IE | (IF << 16)
    and     r0, r1, r1, lsr #16     @ IE & IF
    strh    r0, [r2, #2]            @ acknowledge IF
    ldr     r3, =0x03007FF8         @ BIOS IRQ flags for IntrWait
    ldrh    r1, [r3]
    orr     r1, r1, r0
    strh    r1, [r3]
    bx      lr
    .pool
