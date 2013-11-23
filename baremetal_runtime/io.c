#include "io.h"

void leds_init() {
    HW_REG_SET(CM_PER_GPIO1_CLKCTRL, 0x40002);
    HW_REG_CLRBITS(GPIO1_OE, 0xf<<21);
}

void leds_set(int mask) {
    HW_REG_SET(GPIO1_SETDATAOUT, (mask & 0xf)<<21);
    HW_REG_SET(GPIO1_CLEARDATAOUT, (~mask & 0xf)<<21);
}

void uart_init() {
    /* set uart mux config */
    HW_REG_SET(CONF_UART0_RXD, (0x1<<4)|(0x1<<5));
    HW_REG_SET(CONF_UART0_TXD, 0x0);
    /* setup clocks */
    HW_REG_MODBITS(CM_WKUP_CLKSTCTRL, 0x3, 0x2);
    HW_REG_MODBITS(CM_PER_L4HS_CLKSTCTRL, 0x3, 0x2);
    HW_REG_MODBITS(CM_WKUP_UART0_CLKCTRL, 0x3, 0x2);
    leds_set(7); /* removing this halts device */
    /* uart soft reset */
    HW_REG_SETBITS(UART0_SYSC, 0x2);
    while ((HW_REG_GET(UART0_SYSS) & 0x1) != 0x1)
        ;
    /* disable smart idle */
    HW_REG_SETBITS(UART0_SYSC, (0x1 << 0x3));
    /* uart init */
    while (!(HW_REG_GETB(UART0_BASE + 20) & 0x40))
        ;
    HW_REG_SETB(UART0_BASE + 4, 0);
    HW_REG_SETB(UART0_BASE + 32, 7);
    HW_REG_SETB(UART0_BASE + 12, ~0x7c);
    HW_REG_SETB(UART0_BASE + 0, 0);
    HW_REG_SETB(UART0_BASE + 4, 0);
    HW_REG_SETB(UART0_BASE + 12, 3);
    HW_REG_SETB(UART0_BASE + 16, 3);
    HW_REG_SETB(UART0_BASE + 8, 7);
    HW_REG_SETB(UART0_BASE + 12, ~0x7c);
    HW_REG_SETB(UART0_BASE + 0, 26);
    HW_REG_SETB(UART0_BASE + 4, 0);
    HW_REG_SETB(UART0_BASE + 12, 3);
    HW_REG_SETB(UART0_BASE + 32, 0);
}

void uart_putc(int c) {
    if (c == '\n')
        uart_putc('\r');

    while ((HW_REG_GETB(UART0_BASE + 20) & 32) == 0)
        ;
    HW_REG_SETB(UART0_BASE + 0, c);
}

void uart_putf(const char *fmt, ...) {
    int *stack_head = __builtin_frame_address(0);
    stack_head += 2; // skip fmt, skip stack_head

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt++) {
                case 'c':
                    uart_putc(*stack_head++);
                    break;
                case 's': {
                    const char *s = (char *) *stack_head++;
                    while (*s) {
                        uart_putc(*s++);
                    }
                    break;
                }
                case 'x': {
                    int num = *stack_head++;
                    int shift = 28;
                    while (shift >= 0) {
                        int hd = (num >> shift) & 0xf;
                        if (hd > 9)
                            hd += 'A' - 10;
                        else
                            hd += '0';
                        uart_putc(hd);
                        shift -= 4;
                    }
                    break;
                }
                case 'd': {
                    int num = *stack_head++;
                    char buf[16];
                    char *s = buf + (sizeof(buf) / sizeof(buf[0]));
                    char *e = s;

                    do {
                        *--s = '0' + num % 10;
                    } while (num /= 10);

                    while (s <= e)
                        uart_putc(*s++);

                    break;
                }
                default:
                    uart_putc('?');
            }
        } else {
            uart_putc(*fmt++);
        }
    }
}

