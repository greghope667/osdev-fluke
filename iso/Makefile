
.PHONY: dirs build run debug
.DEFAULT_GOAL := build
MAKEFLAGS += --no-builtin-rules

LIMINE_DATA ?= /usr/share/limine
KERNEL ?= ../kernel/kernel.elf

### Build directories

dirs:
	mkdir -p bin build/boot/limine/ build/EFI/BOOT/

### ISO build

PROGRAMS = init

ISOPROG = $(PROGRAMS:%=build/boot/%)

ISOFILES = \
	$(ISOPROG) \
	build/boot/kernel.elf \
	build/boot/limine/limine.conf \
	build/boot/limine/limine-bios.sys \
	build/boot/limine/limine-uefi-cd.bin \
	build/boot/limine/limine-bios-cd.bin \
	build/EFI/BOOT/BOOTX64.EFI \

build/boot/%: ../programs/%.c | dirs
	$(CC) -O $< -o $@

build/boot/limine/%: $(LIMINE_DATA)/% | dirs
	cp $< $@

build/EFI/BOOT/%: $(LIMINE_DATA)/% | dirs
	cp $< $@

build/boot/limine/limine.conf: limine.conf | dirs
	cp $< $@

build/boot/kernel.elf: $(KERNEL) | dirs
	strip $< -o $@

os.iso: $(ISOFILES) | dirs
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		build -o $@
	limine bios-install $@

### Named targets

build: os.iso

clean:
	rm -rf os.iso build

run: build
	qemu-system-x86_64 -serial stdio -cdrom os.iso -no-reboot | tee log.txt

debug: build
	konsole -e qemu-system-x86_64 -serial stdio -cdrom os.iso -s -S -d int &
	@exec gdb $(KERNEL) -q \
		-iex "set debuginfod enabled off" \
		-ex "target remote localhost:1234"
