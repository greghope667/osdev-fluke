.PHONY: dirs all elf iso clean run debug
.DEFAULT_GOAL := all
MAKEFLAGS += --no-builtin-rules

### Customisation

CC ?= gcc
AS ?= gcc
LD ?= ld

CFLAGS ?= -Og -g3
CXXFLAGS ?= -Og -g3
ASMFLAGS ?= -g3
LIMINE_DATA ?= /usr/share/limine

### Build directories

dirs:
	mkdir -p bin build
	find src/ -type d -print0 | xargs -0 -I X mkdir -p build/X
	mkdir -p isodir/boot/limine
	mkdir -p isodir/EFI/BOOT

### C/ASM build config

FLAGS =\
	-Isrc \
	-Wall \
	-Wextra \
	-mgeneral-regs-only \
	-mno-red-zone \
	-mcmodel=kernel \
	-fno-asynchronous-unwind-tables \
	-fno-pie \
	-fno-omit-frame-pointer \
	-fno-stack-protector \
	-ffreestanding \
	-fbuiltin

ifneq (,$(findstring clang,$(CC)))
	FLAGS +=\
		--target=x86_64-elf \
		-mstack-alignment=8
else
	FLAGS +=\
		-mpreferred-stack-boundary=3
endif

CFLAGS += $(FLAGS) -std=gnu23
CXXFLAGS += $(FLAGS) -std=gnu++26 -fno-exceptions -fno-rtti -Wno-invalid-offsetof
LDFLAGS += -static --eh-frame-hdr -znoexecstack

C_SRCS = $(shell find src -name '*.c')
CXX_SRCS = $(shell find src -name '*.cxx')
ASM_SRCS = $(shell find src -name '*.s')
OBJS = $(ASM_SRCS:%.s=build/%.o) $(C_SRCS:%.c=build/%.o) $(CXX_SRCS:%.cxx=build/%.o)
DEPS = $(C_SRCS:%.c=build/%.d) $(CXX_SRCS:%.cxx=build/%.d)

-include $(DEPS)

build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -MMD -MF build/$*.d -o $@

build/%.o: %.cxx
	$(CC) $(CXXFLAGS) -c $< -MMD -MF build/$*.d -o $@

build/%.o: %.s
	$(AS) -x assembler-with-cpp $(ASMFLAGS) -c $< -o $@

bin/os.elf: $(OBJS) src/linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -T src/linker.ld -o $@
	nm -n bin/os.elf | c++filt -p > bin/symbols
	printf "\n" >> bin/symbols
	objcopy --update-section .symbols=bin/symbols bin/os.elf
	size $(OBJS) $@

isodir/boot/init: programs/init.c src/fluke.h
	$(CC) $(CFLAGS) -static -nostdlib $< -o $@

### ISO build

ISOFILES = \
	isodir/boot/os.elf \
	isodir/boot/init \
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
	find build/ -type f -delete
	rm -f bin/os.elf bin/os.iso isodir/boot/os.elf

run: iso
	qemu-system-x86_64 -serial stdio -cdrom bin/os.iso -no-reboot | tee log.txt

debug: elf iso
	konsole -e qemu-system-x86_64 -serial stdio -cdrom bin/os.iso -s -S -d int &
	@exec gdb bin/os.elf -q \
		-iex "set debuginfod enabled off" \
		-ex "target remote localhost:1234"
