
_start:
    /* setup stack pointer */
    ldr sp, =0x4030CDFC
    mov r3, r0
    /* zero out bss */
    ldr     r0, =__bss_start__
    ldr     r1, =__bss_size__
    add     r1, r0
    mov     r2, #0
0:
    cmp     r0, r1
    strlt   r2, [r0], #4
    blt     0b

    mov     r0, r3
    b       main
