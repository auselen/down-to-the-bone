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
    //HW_REG_SET(RTC_BASE + 0x48, 0x0);
    //while (HW_REG_GET(RTC_BASE + 0x44) & 0x1) ;
    //HW_REG_SET(RTC_BASE + 0x48, 0x1);
    //HW_REG_SET(INTC_BASE + 0xD4, 0x1 << 11);
    int sir_irq = HW_REG_GET(INTC_BASE + 0x40);
    //uart_putf("sir_irq %x\n", sir_irq);
    if (sir_irq & 0xFFFFFF80)
        return;
    if ((sir_irq & 0x7F) == 75) {
        while (HW_REG_GET(RTC_BASE + 0x44) & 0x1) ;
        //uart_putf("\n44 %x\n", HW_REG_GET(RTC_BASE + 0x44));
        if ((HW_REG_GET(RTC_BASE + 0x44) & 0x4) == 0x4) {
        //HW_REG_SET(INTC_BASE + 0xCC, 0x1 << 11);
            irq_count++;
        HW_REG_SET(INTC_BASE + 0xD0, 0x1 << 11);
        }
        //    ;
        //HW_REG_CLRBITS(RTC_BASE + 0x48, 0x4);
        //uart_putf("INT MASK %x", HW_REG_GET(INTC_BASE + 0xC0));
        //HW_REG_SET(INTC_BASE + 0xD4, 0x1 << 11);
    }
    //uart_putf("\nrtc status %x", HW_REG_GET(RTC_BASE + 0x44));
    //uart_putf(" rtc int %x", HW_REG_GET(RTC_BASE + 0x48));
    //uart_putf("IRQ! %d", HW_REG_GET(INTC_BASE + 0x40) & 0x7F);
    //rtc_irq();
    //HW_REG_SET(RTC_BASE + 0x48, 0x4);
    //HW_REG_SET(INTC_BASE + 0x48, 0x1);
    //asm volatile("DSB");
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

void config_ddr_x(void);


const char *CRYSTAL_FREQS[] = {"19.2", "24", "25", "26"};

void main(BootParams_t *bootcfg) {
    int val;
    int i = 0;
    leds_init();
    timer_init();
    uart_init();
    uart_putf("\n\nBooting...\n");
    asm("MRC p15, 0, %[v], c0, c0, 0" : [v] "=r" (val));
    uart_putf("main id register %x\n", val);
    asm("MRS %[v], CPSR" : [v] "=r" (val));
    uart_putf("CPSR register %x\n", val);
    val &= ~0x80;
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
    
    
    /*
    	config_ddr_x is 'semi-hardcoded' version of u-boot DDR3L init
    	
    	Currently, this does not seem to work properly if binary
    	is downloaded 1st time via XMODEM, assuming BBB is cold-started
    	into DL mode with boot-button pressed.
    	However, if binary is downloaded 2nd time, assuming BBB is set
    	to DL mode just via reset-button, it will work...
    */
    uart_putf("config_ddr_x enter...\n");
    config_ddr_x();
    uart_putf("config_ddr_x exit\n");
    
    {
	    #include <stdint.h>
	    uint32_t	*p = (uint32_t*)0x80000000;
	  	uint32_t	*endp = (uint32_t*)(0x80000000+512*1024*1024);
//	  	uint32_t	*endp = (uint32_t*)(0x80000000+64*1024*1024);
	  	int			i=0;
	  	
	  	uart_putf("start DDR3 ram test, from 0x%x to 0x%x\n",p,endp);
	  	
	  	uart_putf("    writing...\n");
	  	while ( p < endp )
	  	{
		  	*p = (uint32_t)i;
		  	p++;
		  	i++;
	  	}
	  	
	  	i=0;
	  	p = (uint32_t*)0x80000000;

	  	uart_putf("    checking...\n");
	  	while ( p < endp )
	  	{
		  	if ( *p != (uint32_t)i )
		  	{
			  	uart_putf("mem check error\n");
			  	uart_putf("    address : 0x%x\n",p);
			  	uart_putf("    expected: 0x%x\n",i);
			  	uart_putf("    actual  : 0x%x\n",*p);
		  		p = endp + 8;
	  		}
		  	else
		  	{
		  		p++;
		  		i++;
  			}
	  	}
	  	
	  	if ( p != (endp + 8) )
	  		uart_putf("DDR3 ram test OK\n");
	  	else
	  		uart_putf("DDR3 ram test FAIL\n");
    }

    
    rtc_init();
    rtc_irq();
    while (1) {
        leds_set(1 << (i++ % 4));
        int sec = HW_REG_GET(RTC_BASE);
        int min = HW_REG_GET(RTC_BASE + 4);
        int hour = HW_REG_GET(RTC_BASE + 8);
        uart_putf("\rtime %d%d:%d%d:%d%d irq_count:_%d_ i:%d",
            (hour >> 4) & 0x3, hour & 0xF,
            (min >> 4) & 0x7, min & 0xF,
            (sec >> 4) & 0x7, sec & 0xF,
            irq_count, i);
        //sleep();
    }
}
