CROSSCOMPILE ?= arm-linux-gnueabihf-

CC=$(CROSSCOMPILE)gcc
AS=$(CROSSCOMPILE)as
LD=$(CROSSCOMPILE)ld

CFLAGS = -mcpu=cortex-a8 -marm -Wall -O2 -nostdlib -nostartfiles -ffreestanding -fstack-usage -Wstack-usage=256

OBJS=startup.o runtime.o io.o ddr3_init_x.o

TARGET=runtime

all : $(TARGET).bin

#startup.o : startup.s
#	$(CROSSCOMPILE)as startup.s -o startup.o

#runtime.o : runtime.c
#	$(CROSSCOMPILE)gcc $(CFLAGS) -c runtime.c -o runtime.o

#io.o : io.c
#	$(CROSSCOMPILE)gcc $(CFLAGS) -c io.c -o io.o

$(TARGET).bin : memmap $(OBJS)
	$(LD) -o $(TARGET) -T memmap $(OBJS)
	$(CROSSCOMPILE)objdump -M reg-names-raw -D $(TARGET) > $(TARGET).lst
	$(CROSSCOMPILE)objcopy $(TARGET) $(TARGET).bin -O binary

clean :
	-@rm *.o *.lst *.bin *.su runtime 2>/dev/null || true
