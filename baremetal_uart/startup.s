/*
Outputs alphabet over serial.
*/

.equ CM_PER_GPIO1_CLKCTRL, 0x44e000AC
.equ GPIO1_OE, 0x4804C134
.equ GPIO1_SETDATAOUT, 0x4804C194
.equ CONF_UART0_RXD, 0x44E10970
.equ CONF_UART0_TXD, 0x44E10974
.equ CM_WKUP_CLKSTCTRL, 0x44E00400
.equ CM_PER_L4HS_CLKSTCTRL, 0x44E0011C
.equ CM_WKUP_UART0_CLKCTRL, 0x44E004B4
.equ CM_PER_UART0_CLKCTRL, 0x44E0006C
.equ UART0_SYSC, 0x44E09054
.equ UART0_SYSS, 0x44E09058
.equ UART0_BASE, 0x44E09000

_start:
    /* set clock for GPIO1, TRM 8.1.12.1.29 */
    ldr r0, =CM_PER_GPIO1_CLKCTRL
    ldr r1, =0x40002
    str r1, [r0]

    /* set pin 21 for output, led USR0, TRM 25.3.4.3 */
    ldr r0, =GPIO1_OE
    ldr r1, [r0]
    bic r1, r1, #(7<<21)
    str r1, [r0]

    /* logical 1 turns on the led, TRM 25.3.4.2.2.2 */
    ldr r0, =GPIO1_SETDATAOUT
    ldr r1, =(1<<21)
    str r1, [r0]

    /* set uart mux config */
    ldr r0, =CONF_UART0_RXD
    ldr r1, =(0x1<<4)|(0x1<<5)
    str r1, [r0]
    ldr r0, =CONF_UART0_TXD
    ldr r1, =0x0
    str r1, [r0]

    /* setup_clocks_for_console */
    ldr r0, =CM_WKUP_CLKSTCTRL
    ldr r1, [r0]
    and r1, #~0x3
    orr r1, #0x2
    str r1, [r0]
    ldr r0, =CM_PER_L4HS_CLKSTCTRL
    ldr r1, [r0]
    and r1, #~0x3
    orr r1, #0x2
    str r1, [r0]
    ldr r0, =CM_WKUP_UART0_CLKCTRL
    ldr r1, [r0]
    and r1, #~0x3
    orr r1, #0x2
    str r1, [r0]
    ldr r0, =CM_PER_UART0_CLKCTRL
    ldr r1, [r0]
    and r1, #~0x3
    orr r1, #0x2
    str r1, [r0]

    /* UART soft reset */
    ldr r0, =UART0_SYSC
    ldr r1, [r0]
    orr r1, #0x2
    str r1, [r0]
    ldr r0, =UART0_SYSS
.uart_soft_reset:
    ldr r1, [r0]
    ands r1, #0x1
    beq uart_soft_reset
    /* turn off smart idle */
    ldr r0, =UART0_SYSC
    ldr r1, [r0]
    orr r1, #(0x1 << 0x3)
    str r1, [r0]

    /* initialize UART */
    ldr r0, =UART0_BASE
    ldr r1, =26
.uart_init:
    ldrb    r3, [r0, #20]
    uxtb    r3, r3
    tst     r3, #0x40
    beq     .uart_init
    mov     r3, #0
    strb    r3, [r0, #4]
    mov     r3, #7
    strb    r3, [r0, #32]
    mvn     r3, #0x7c
    strb    r3, [r0, #12]
    mov     r3, #0
    strb    r3, [r0]
    strb    r3, [r0, #4]
    mov     r3, #3
    strb    r3, [r0, #12]
    strb    r3, [r0, #16]
    mov     r3, #7
    strb    r3, [r0, #8]
    mvn     r3, #0x7c
    strb    r3, [r0, #12]
    uxtb    r3, r1
    strb    r3, [r0]
    ubfx    r1, r1, #8, #8
    strb    r1, [r0, #4]
    mov     r3, #3
    strb    r3, [r0, #12]
    mov     r3, #0
    strb    r3, [r0, #32]

    /* turn on second led */
    ldr r2, =GPIO1_SETDATAOUT
    ldr r1, =(1<<22)
    str r1, [r2]

    ldr     r1, =UART0_BASE
    ldr     r0, ='A'
.loop:
    cmp     r0, #'Z'
    movgt   r0, #'A'
    bl .uart_putc
    mov     r3, r0
    ldr     r0, ='\r'
    bl .uart_putc
    ldr     r0, ='\n'
    bl .uart_putc
    mov     r0, r3
    add     r0, #1
    b .loop

.uart_putc:
    ldrb    r2, [r1, #20]
    uxtb    r2, r2
    tst     r2, #32
    beq     .uart_putc
    strb    r0, [r1]
    bx      lr
