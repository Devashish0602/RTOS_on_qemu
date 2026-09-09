CROSS   = riscv64-unknown-elf-
CC      = $(CROSS)gcc
OBJDUMP = $(CROSS)objdump

ARCH    = -march=rv32imac_zicsr -mabi=ilp32 -mcmodel=medany
CFLAGS  = $(ARCH) -Wall -Wextra -O0 -g -ffreestanding -nostdlib -fno-builtin
LDFLAGS = -T kernel.ld

SRCS = src/start.S src/uart.c src/main.c src/trap.c src/timer.c

INC = inc

all: kernel.elf

kernel.elf: $(SRCS) kernel.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -I $(INC) -o $@ $(SRCS)
	$(OBJDUMP) -d $@ > kernel.dis

run: kernel.elf
	qemu-system-riscv32 -machine virt -nographic -bios none -kernel kernel.elf

debug: kernel.elf
	qemu-system-riscv32 -machine virt -nographic -bios none -kernel kernel.elf -s -S

clean:
	rm -f kernel.elf kernel.dis