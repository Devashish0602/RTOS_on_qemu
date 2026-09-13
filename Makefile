CROSS   = riscv64-unknown-elf-
CC      = $(CROSS)gcc
OBJDUMP = $(CROSS)objdump

ARCH    = -march=rv32imac_zicsr -mabi=ilp32 -mcmodel=medany
CFLAGS  = $(ARCH) -Wall -Wextra -O0 -g -ffreestanding -nostdlib -fno-builtin
LDFLAGS = -T kernel.ld

SRCDIR = src


STARTUP := $(SRCDIR)/start.S
SRCS := $(wildcard $(SRCDIR)/*.c)



INC = inc

all: kernel.elf

kernel.elf: $(STARTUP) $(SRCS) kernel.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -I $(INC) -o $@ $(STARTUP) $(SRCS) 
	$(OBJDUMP) -d $@ > kernel.dump

run: kernel.elf
	qemu-system-riscv32 -machine virt -nographic -bios none -kernel kernel.elf

debug: kernel.elf
	qemu-system-riscv32 -machine virt -nographic -bios none -kernel kernel.elf -s -S

clean:
	rm -f kernel.elf kernel.dump