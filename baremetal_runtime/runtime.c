#define NULL 0

#define HW_REG_GET(addr) (*(volatile unsigned int *) (addr))
#define HW_REG_GETB(addr) (*(volatile unsigned char *) (addr))
#define HW_REG_SET(addr, val) (*(volatile unsigned int *) (addr) = (val))
#define HW_REG_SETB(addr, val) (*(volatile unsigned char *) (addr) = (val))
#define HW_REG_CLRBITS(addr, clr) (*(volatile unsigned int *) (addr) = (*(volatile unsigned int *) (addr)) & ~(clr))
#define HW_REG_SETBITS(addr, set) (*(volatile unsigned int *) (addr) = (*(volatile unsigned int *) (addr)) | (set))
#define HW_REG_MODBITS(addr, clr, set) (*(volatile unsigned int *) (addr) = ((*(unsigned int *) (addr) & ~(clr)) | (set)))

#define CM_PER_GPIO1_CLKCTRL    0x44e000AC
#define CM_PER_TIMER7_CLKCTRL   0x44E0007C
#define CM_PER_L4HS_CLKSTCTRL   0x44E0011C
#define CM_PER_UART0_CLKCTRL    0x44E0006C

#define CM_WKUP_CLKSTCTRL       0x44E00400
#define CM_WKUP_UART0_CLKCTRL   0x44E004B4

#define GPIO1_OE                0x4804C134
#define GPIO1_SETDATAOUT        0x4804C194
#define GPIO1_CLEARDATAOUT      0x4804C190

#define DMTIMER7_TCRR           0x4804A03C
#define DMTIMER7_TCLR           0x4804A038

#define CONF_UART0_RXD          0x44E10970
#define CONF_UART0_TXD          0x44E10974

#define UART0_SYSC              0x44E09054
#define UART0_SYSS              0x44E09058
#define UART0_BASE              0x44E09000

#define CONTROL_MODULE_BASE     0x44E10000
#define CONTROL_STATUS          0x44E10040

#define ROM_CODE_VERSION        0x0002BFFC

int __aeabi_idiv(int num, int denum) {
    int q = 0;

    while (num > denum) {
        num -= denum;
        q++;
    }

   return q;
}

void leds_init() {
    HW_REG_SET(CM_PER_GPIO1_CLKCTRL, 0x40002);
    HW_REG_CLRBITS(GPIO1_OE, 0xf<<21);
}

void leds_set(int mask) {
    HW_REG_SET(GPIO1_SETDATAOUT, (mask & 0xf)<<21);
    HW_REG_SET(GPIO1_CLEARDATAOUT, (~mask & 0xf)<<21);
}

void timer_init() {
    HW_REG_SET(CM_PER_TIMER7_CLKCTRL, 0x2);
    leds_set(3); /* removing this halts device */
    HW_REG_SET(DMTIMER7_TCLR, 0x3);
}

void uart_init() {
    leds_set(0xf);
    /* set uart mux config */
    HW_REG_SET(CONF_UART0_RXD, (0x1<<4)|(0x1<<5));
    HW_REG_SET(CONF_UART0_TXD, 0x0);
    leds_set(1);
    /* setup clocks */
    HW_REG_MODBITS(CM_WKUP_CLKSTCTRL, 0x3, 0x2);
    HW_REG_MODBITS(CM_PER_L4HS_CLKSTCTRL, 0x3, 0x2);
    HW_REG_MODBITS(CM_WKUP_UART0_CLKCTRL, 0x3, 0x2);
    leds_set(3);
    /* uart soft reset */
    HW_REG_SETBITS(UART0_SYSC, 0x2);
    leds_set(7);
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
                default:
                    uart_putc('?');
            }
        } else {
            uart_putc(*fmt++);
        }
    }
}

void sleep() {
    int start = HW_REG_GET(DMTIMER7_TCRR);
    volatile unsigned int *cur = (unsigned int *) DMTIMER7_TCRR;
    while (*cur - start < 0x01000000) ;
}

typedef struct {
    int reserved;
    void *desc;
    char device;
    char reason;
    char reserved2;
} /* __attribute__((packed))*/ BootParams_t;

BootParams_t Empty_params;

const char *CRYSTAL_FREQS[] = {"19.2", "24", "25", "26"};

void main(BootParams_t *bootcfg) {
    int i = 0;
    leds_init();
    timer_init();
    uart_init();
    uart_putf("rom code version 0x%x\n", HW_REG_GET(ROM_CODE_VERSION));
    uart_putf("control_status 0x%x\n", HW_REG_GET(CONTROL_STATUS));
    uart_putf("crystal freq %s MHz\n", CRYSTAL_FREQS[(HW_REG_GET(CONTROL_STATUS) >> 22) & 0x3]);
    uart_putf("bootcfg 0x%x\n desc  : 0x%x\n device: 0x%x\n reason: 0x%x\n", bootcfg,
        bootcfg->desc, bootcfg->device, bootcfg->reason);
    *bootcfg = Empty_params;
    while (1) {
        leds_set(1 << (i % 4));
        uart_putc('a' + (i % ('z' - 'a')) /i);
        sleep();
        i++;
    }
}
