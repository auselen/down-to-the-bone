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

void handler_undefined() {
    uart_putf("undefined instruction, skipping\n");
}

void handler_undefined_entry() __attribute__ ((naked));
void handler_undefined_entry() {
    asm volatile("SRSFD sp!, #0x1B\n");
    handler_undefined();
    asm volatile("RFEFD sp!\n");
}

volatile int irq_count;
volatile int hour, min, sec;
void handler_irq() {
    int sir_irq = HW_REG_GET(INTC_BASE + 0x40);
    if (sir_irq & 0xFFFFFF80)
        return;
    if ((sir_irq & 0x7F) == 75) {
        irq_count++;
        sec = HW_REG_GET(RTC_BASE);
        min = HW_REG_GET(RTC_BASE + 4);
        hour = HW_REG_GET(RTC_BASE + 8);
    }
}

void handler_irq_entry() __attribute__ ((naked));
void handler_irq_entry() {
    asm volatile(
        "STMFD SP!, {R0-R3, R11, LR}\n"
        "MRS R11, SPSR\n"
        "BL handler_irq\n"
        "LDR r0, =0x48200000\n" 
        "LDR r1, =0x1\n"
        "STR r1, [r0, #0x48]\n"
        "DSB\n"
        "MSR SPSR, R11\n"
        "LDMFD SP!, {R0-R3, R11, LR}\n"
        "SUBS PC, LR, #4"
    );
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
    uart_putf("\n\nBooting...\n");
    asm volatile("MRC p15, 0, %[v], c0, c0, 0" : [v] "=r" (val));
    uart_putf("main id register %x\n", val);
    asm volatile("CPSIE i");
    asm volatile("MRS %[v], CPSR" : : [v] "r" (val));
    uart_putf("CPSR register %x\n", val);
    uart_putf("rom code version 0x%x\n", HW_REG_GET(ROM_CODE_VERSION));
    uart_putf("control_status 0x%x\n", HW_REG_GET(CONTROL_STATUS));
    uart_putf("crystal freq %s MHz\n", CRYSTAL_FREQS[(HW_REG_GET(CONTROL_STATUS) >> 22) & 0x3]);
    uart_putf("bootcfg 0x%x\n desc  : 0x%x\n device: 0x%x\n reason: 0x%x\n", bootcfg,
        bootcfg->desc, bootcfg->device, bootcfg->reason);
    *bootcfg = Empty_params;
    HW_REG_SET(0x4030CE24, handler_undefined_entry);
    HW_REG_SET(0x4030CE38, handler_irq_entry);
    asm volatile(".word 0xe7f000f0");
    rtc_init();
    rtc_irq();
    while (1) {
        leds_set(1 << (i++ % 4));
        uart_putf("\rtime %d%d:%d%d:%d%d irq_count:_%d_ i:%d",
            (hour >> 4) & 0x3, hour & 0xF,
            (min >> 4) & 0x7, min & 0xF,
            (sec >> 4) & 0x7, sec & 0xF,
            irq_count, i);
        sleep();
    }
}
