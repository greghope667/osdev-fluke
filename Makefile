.PHONY: all clean kernel install-headers libc install-libc iso qemu-run
.DEFAULT_GOAL := all

### Customisation

export SYSROOT ?= $(HOME)/opt/fluke-sysroot

### Named targets

all: kernel install-headers install-libc

clean:
	make -C kernel clean
	make -C system/libc clean
	make -C iso clean

kernel:
	$(MAKE) $(MAKEFLAGS) -C kernel

install-headers:
	mkdir -p "$(SYSROOT)/usr/include" "$(SYSROOT)/usr/lib" "$(SYSROOT)/usr/bin"
	cp -R --update include "$(SYSROOT)/usr"

libc: install-headers
	$(MAKE) $(MAKEFLAGS) -C system/libc

install-libc: libc
	make -C system/libc install

iso: all
	make -C iso build

qemu-run: all
	make -C iso run
