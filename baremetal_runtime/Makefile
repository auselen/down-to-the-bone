CROSSCOMPILE ?= /home/auselen/bin/gcc-linaro-arm-linux-gnueabihf-4.8-2013.09_linux/bin/arm-linux-gnueabihf-

CFLAGS = -mcpu=cortex-a8 -marm -Wall -O2 -nostdlib -nostartfiles -ffreestanding

all : runtime.bin

startup.o : startup.s
	$(CROSSCOMPILE)as startup.s -o startup.o

runtime.o : runtime.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c runtime.c -o runtime.o

runtime.bin : memmap startup.o runtime.o
	$(CROSSCOMPILE)ld -o runtime -T memmap startup.o runtime.o
	$(CROSSCOMPILE)objdump -D runtime > runtime.lst
	$(CROSSCOMPILE)objcopy runtime runtime.bin -O binary

clean :
	rm *.o *.lst *.bin runtime
