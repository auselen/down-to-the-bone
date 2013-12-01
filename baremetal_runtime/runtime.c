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
    int val;
    asm volatile("mov %[val], sp" : [val] "=r" (val));
    uart_putf("sp %x\n", val);
    asm("MRS %[v], CPSR" : [v] "=r" (val));
    uart_putf("CPSR register %x\n", val);
    uart_putf("undefined instruction, skipping\n");
}

void handler_undefined_entry() __attribute__ ((naked));
void handler_undefined_entry() {
    asm volatile("SRSFD #0x1B!\n");
    handler_undefined();
    asm volatile("RFEFD sp!\n");
}

volatile int irq_count;
//void handler_irq() __attribute__ ((noinline));
void handler_irq() {
    if ((HW_REG_GET(INTC_BASE + 0x40) & 0x7F) == 75)
        irq_count++;
    uart_putf("IRQ! %d", HW_REG_GET(INTC_BASE + 0x40) & 0x7F);
}

void handler_irq_entry() __attribute__ ((naked));
void handler_irq_entry() {
    asm volatile(
        "STMFD SP!, {R0-R12, LR}\n"
        "MRS R11, SPSR\n"
        "BL handler_irq\n"
        "LDR r0, =0x48200000\n" 
        "LDR r1, =0x1\n"
        "STR r1, [r0, #0x48]\n"
        "DSB\n"
        "MSR SPSR, R11\n"
        "LDMFD SP!, {R0-R12, LR}\n"
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
    int sp;
    leds_init();
    timer_init();
    uart_init();
    rtc_init();
    uart_putf("\n\nBooting...\n");
    asm("MRC p15, 0, %[v], c0, c0, 0" : [v] "=r" (val));
    uart_putf("main id register %x\n", val);
    asm("MRS %[v], CPSR" : [v] "=r" (val));
    uart_putf("CPSR register %x\n", val);
    val &= ~0xC0;
    asm("MSR CPSR, %[v]" : : [v] "r" (val));
    uart_putf("CPSR register %x\n", val);
    uart_putf("rom code version 0x%x\n", HW_REG_GET(ROM_CODE_VERSION));
    uart_putf("control_status 0x%x\n", HW_REG_GET(CONTROL_STATUS));
    uart_putf("crystal freq %s MHz\n", CRYSTAL_FREQS[(HW_REG_GET(CONTROL_STATUS) >> 22) & 0x3]);
    uart_putf("lr 0x%x\n", *bootcfg);
    uart_putf("bootcfg 0x%x\n desc  : 0x%x\n device: 0x%x\n reason: 0x%x\n", bootcfg,
        bootcfg->desc, bootcfg->device, bootcfg->reason);
    *bootcfg = Empty_params;
    HW_REG_SET(0x4030CE24, handler_undefined_entry);
    HW_REG_SET(0x4030CE38, handler_irq_entry);
    //asm volatile(".word 0xf000f0e7");
    rtc_irq();
    while (1) {
        leds_set(1 << (i++ % 4));
        int sec = HW_REG_GET(RTC_BASE);
        int min = HW_REG_GET(RTC_BASE + 4);
        int hour = HW_REG_GET(RTC_BASE + 8);
        asm volatile("mov %[sp], sp" : [sp] "=r" (sp));
        asm("MRS %[v], CPSR" : [v] "=r" (val));
        uart_putf("\rtime %d%d:%d%d:%d%d cpsr:%x irq_count:_%d_ %x",
            (hour >> 4) & 0x3, hour & 0xF,
            (min >> 4) & 0x7, min & 0xF,
            (sec >> 4) & 0x7, sec & 0xF,
            val, irq_count, sp);
        sleep();
    }
}
