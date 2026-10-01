@ Cartridge entry point. The header bytes are filled in by gbafix after linking.
    .section .crt0, "ax"
    .global _start
    .arm
_start:
    b rom_start
    .fill 188, 1, 0

rom_start:
    mov r0, #0x12            @ IRQ mode stack
    msr cpsr_c, r0
    ldr sp, =0x03007FA0
    mov r0, #0x1F            @ system mode stack
    msr cpsr_c, r0
    ldr sp, =0x03007F00

    ldr r0, =__data_lma      @ copy initialised data to IWRAM
    ldr r1, =__data_start
    ldr r2, =__data_end
1:  cmp r1, r2
    ldrlt r3, [r0], #4
    strlt r3, [r1], #4
    blt 1b

    ldr r1, =__bss_start     @ clear bss
    ldr r2, =__bss_end
    mov r3, #0
2:  cmp r1, r2
    strlt r3, [r1], #4
    blt 2b

    ldr r3, =main
    mov lr, pc
    bx r3
3:  b 3b
    .pool
