#define NULL 0

#include "io.h"

void timer_init() {
    HW_REG_SET(CM_PER_TIMER7_CLKCTRL, 0x2);
    leds_set(3); /* removing this halts device */
    HW_REG_SET(DMTIMER7_TCLR, 0x3);
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
    int val;
    int i = 0;
    leds_init();
    timer_init();
    uart_init();
    rtc_init();
    asm("MRC p15, 0, %[v], c0, c0, 0" : [v] "=r" (val));
    uart_putf("main id register %x\n", val);
    uart_putf("rom code version 0x%x\n", HW_REG_GET(ROM_CODE_VERSION));
    uart_putf("control_status 0x%x\n", HW_REG_GET(CONTROL_STATUS));
    uart_putf("crystal freq %s MHz\n", CRYSTAL_FREQS[(HW_REG_GET(CONTROL_STATUS) >> 22) & 0x3]);
    uart_putf("bootcfg 0x%x\n desc  : 0x%x\n device: 0x%x\n reason: 0x%x\n", bootcfg,
        bootcfg->desc, bootcfg->device, bootcfg->reason);
    *bootcfg = Empty_params;
    while (1) {
        leds_set(1 << (i % 4));
        int sec = HW_REG_GET(RTC_BASE);
        int min = HW_REG_GET(RTC_BASE + 4);
        int hour = HW_REG_GET(RTC_BASE + 8);
        uart_putf("\rtime %d%d:%d%d:%d%d", (hour >> 4) & 0x3, hour & 0xF,
            (min >> 4) & 0x7, min & 0xF,
            (sec >> 4) & 0x7, sec & 0xF);
        sleep();
        i++;
    }
}
