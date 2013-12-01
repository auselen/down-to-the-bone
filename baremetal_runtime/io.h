#ifndef IO_H
#define IO_H

#define HW_REG_GET(addr) (*(volatile unsigned int *) (addr))
#define HW_REG_GETB(addr) (*(volatile unsigned char *) (addr))
#define HW_REG_SET(addr, val) (*(volatile unsigned int *) (addr) = (unsigned int) (val))
#define HW_REG_SETB(addr, val) (*(volatile unsigned char *) (addr) = (unsigned char) (val))
#define HW_REG_CLRBITS(addr, clr) (*(volatile unsigned int *) (addr) = (*(volatile unsigned int *) (addr)) & ~(clr))
#define HW_REG_SETBITS(addr, set) (*(volatile unsigned int *) (addr) = (*(volatile unsigned int *) (addr)) | (set))
#define HW_REG_MODBITS(addr, clr, set) (*(volatile unsigned int *) (addr) = ((*(unsigned int *) (addr) & ~(clr)) | (set)))

#define CM_PER_GPIO1_CLKCTRL    0x44e000AC
#define CM_PER_TIMER7_CLKCTRL   0x44E0007C
#define CM_PER_L4HS_CLKSTCTRL   0x44E0011C
#define CM_PER_UART0_CLKCTRL    0x44E0006C
#define CM_RTC_RTC_CLKCTRL      0x44E00800
#define CM_RTC_CLKSTCTRL        0x44E00804

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

#define RTC_BASE                0x44E3E000

#define INTC_BASE               0x48200000

void leds_init();
void leds_set(int mask);

void uart_init();
void uart_putc(int c);
void uart_putf(const char *fmt, ...);

void rtc_init();
void rtc_irq();

#endif

