.PHONY: dirs all elf iso clean run debug
.DEFAULT_GOAL := all

### Customisation

CFLAGS ?= -Og -g3
CC ?= gcc
LIMINE_DATA ?= /usr/share/limine

### Build directories

dirs:
	mkdir -p bin
	mkdir -p build/src
	mkdir -p isodir/boot/limine
	mkdir -p isodir/EFI/BOOT

### C/ASM build config

CFLAGS +=\
	-std=gnu23 \
	-Wall \
	-Wextra \
	-static \
	-nostdlib \
	-mgeneral-regs-only \
	-mno-red-zone \
	-mcmodel=kernel \
	-fno-pie \
	-nostdlib \
	-fno-omit-frame-pointer \
	-fno-stack-protector \
	-ffreestanding

ifneq (,$(findstring clang,$(CC)))
	CFLAGS +=\
		--target=x86_64-elf \
		-mstack-alignment=3
else
	CFLAGS +=\
		-mpreferred-stack-boundary=3
endif

C_SRCS = $(wildcard src/*.c)
ASM_SRCS = $(wildcard src/*.s)
OBJS = $(ASM_SRCS:%.s=build/%.o) $(C_SRCS:%.c=build/%.o)
DEPS = $(C_SRCS:%.c=build/%.d)

-include $(DEPS)

build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -MMD -MF build/$*.d -o $@

build/%.o: %.s
	$(CC) -c $< -o $@

bin/os.elf: $(OBJS) src/linker.ld
	$(CC) $(CFLAGS) $(OBJS) -T src/linker.ld -o $@
	nm -n bin/os.elf > bin/symbols
	printf "\n" >> bin/symbols
	objcopy --update-section .symbols=bin/symbols bin/os.elf
	size $(OBJS) $@

### ISO build

ISOFILES = \
	isodir/boot/os.elf \
	isodir/boot/limine/limine.conf \
	isodir/boot/limine/limine-bios.sys \
	isodir/boot/limine/limine-uefi-cd.bin \
	isodir/boot/limine/limine-bios-cd.bin \
	isodir/EFI/BOOT/BOOTX64.EFI \

isodir/boot/limine/%: $(LIMINE_DATA)/%
	cp $< $@

isodir/EFI/BOOT/%: $(LIMINE_DATA)/%
	cp $< $@

isodir/boot/limine/limine.conf: src/limine.conf
	cp $< $@

isodir/boot/os.elf: bin/os.elf
	strip $< -o $@

bin/os.iso: $(ISOFILES)
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		isodir -o $@
	limine bios-install $@

### Named targets

elf: bin/os.elf
iso: bin/os.iso
all: elf iso

clean:
	rm -f $(OBJS) $(DEPS) bin/os.elf bin/os.iso isodir/boot/os.elf

run: iso
	qemu-system-x86_64 -serial stdio -cdrom bin/os.iso | tee log.txt

debug: elf iso
	konsole -e qemu-system-x86_64 -serial stdio -cdrom bin/os.iso -s -S &
	@exec gdb bin/os.elf -q \
		-iex "set debuginfod enabled off" \
		-ex "target remote localhost:1234"
