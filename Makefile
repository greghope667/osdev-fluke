.PHONY: dirs all kernel iso run debug
.DEFAULT_GOAL := all

### Customisation

LIMINE_DATA ?= /usr/share/limine

### Build directories

dirs:
	mkdir -p bin isodir/boot/limine/ isodir/EFI/BOOT/

### ISO build

ISOFILES = \
	isodir/boot/kernel.elf \
	isodir/boot/init \
	isodir/boot/limine/limine.conf \
	isodir/boot/limine/limine-bios.sys \
	isodir/boot/limine/limine-uefi-cd.bin \
	isodir/boot/limine/limine-bios-cd.bin \
	isodir/EFI/BOOT/BOOTX64.EFI \

CFLAGS += \
	-O \
	-isystem include \
	-ffreestanding \
	-fbuiltin

isodir/boot/init: programs/init.c | dirs
	$(CC) $(CFLAGS) -static -nostdlib $< -o $@

isodir/boot/limine/%: $(LIMINE_DATA)/% | dirs
	cp $< $@

isodir/EFI/BOOT/%: $(LIMINE_DATA)/% | dirs
	cp $< $@

isodir/boot/limine/limine.conf: kernel/limine.conf | dirs
	cp $< $@

kernel/kernel.elf:
	$(MAKE) $(MAKEFLAGS) -C kernel/ build

isodir/boot/kernel.elf: kernel/kernel.elf | dirs
	strip $< -o $@

bin/os.iso: $(ISOFILES) | dirs
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		isodir -o $@
	limine bios-install $@

### Named targets

kernel: kernel/kernel.elf
iso: bin/os.iso
all: kernel iso

run: iso
	qemu-system-x86_64 -serial stdio -cdrom bin/os.iso -no-reboot | tee log.txt

debug: kernel iso
	konsole -e qemu-system-x86_64 -serial stdio -cdrom bin/os.iso -s -S -d int &
	@exec gdb kernel/kernel.elf -q \
		-iex "set debuginfod enabled off" \
		-ex "target remote localhost:1234"
