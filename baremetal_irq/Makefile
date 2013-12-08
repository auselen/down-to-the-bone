CROSSCOMPILE ?= arm-linux-gnueabihf-

CFLAGS = -mcpu=cortex-a8 -marm -Wall -O2 -nostdlib -nostartfiles -ffreestanding -fstack-usage -Wstack-usage=256

all : runtime.bin

startup.o : startup.s
	$(CROSSCOMPILE)as startup.s -o startup.o

runtime.o : runtime.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c runtime.c -o runtime.o

io.o : io.c
	$(CROSSCOMPILE)gcc $(CFLAGS) -c io.c -o io.o

runtime.bin : memmap startup.o runtime.o io.o
	$(CROSSCOMPILE)ld -o runtime -T memmap startup.o runtime.o io.o
	$(CROSSCOMPILE)objdump -M reg-names-raw -D runtime > runtime.lst
	$(CROSSCOMPILE)objcopy runtime runtime.bin -O binary

clean :
	-@rm *.o *.lst *.bin *.su runtime 2>/dev/null || true
