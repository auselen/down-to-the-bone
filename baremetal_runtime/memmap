
/* memmap */
MEMORY
{
   l3_ram : ORIGIN = 0x402F0400, LENGTH = 0x0001b400
}

SECTIONS
{
   .text : { *(.text*) } > l3_ram
   __data_rom_start__ = .;
   .data : {
    __data_start__ = .;
    *(.data*)
   } > l3_ram
   __data_end__ = .;
   __data_size__ = __data_end__ - __data_start__;
   .bss  : {
   __bss_start__ = .;
   *(.bss*)
   } > l3_ram
   __bss_end__ = .;
   __bss_size__ = __bss_end__ - __bss_start__;
}
