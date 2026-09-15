.PHONY: \
	download \
	sysroot \
	extract-gcc extract-binutils \
	configure-binutils configure-gcc \
	build-binutils build-gcc \
	install-binutils install-gcc

GCC_VER ?= gcc-16.2.0
BU_VER ?= binutils-2.47

PREFIX ?= $(HOME)/opt/fluke-cross
SYSROOT ?= $(HOME)/opt/fluke-sysroot

$(GCC_VER).tar.xz:
	wget https://mirrorservice.org/sites/sourceware.org/pub/gcc/releases/$(GCC_VER)/$(GCC_VER).tar.xz

$(BU_VER).tar.xz:
	wget https://sourceware.org/pub/binutils/releases/$(BU_VER).tar.xz

download: $(GCC_VER).tar.xz $(BU_VER).tar.xz

extract-gcc:
	tar xf $(GCC_VER).tar.xz
	mv $(GCC_VER) gcc
	git apply --directory gcc gcc.patch

extract-binutils:
	tar xf $(BU_VER).tar.xz
	mv $(BU_VER) binutils
	git apply --directory binutils binutils.patch

configure-binutils:
	mkdir -p build-binutils
	cd build-binutils && ../binutils/configure \
		--target=x86_64-fluke \
		--prefix="$(PREFIX)" \
		--with-sysroot="$(SYSROOT)" \
		--disable-multilib \
		--disable-nls \
		--disable-werror

configure-gcc:
	mkdir -p build-gcc
	cd build-gcc && ../gcc/configure \
		--target=x86_64-fluke \
		--prefix="$(PREFIX)" \
		--with-sysroot="$(SYSROOT)" \
		--disable-multilib \
		--disable-nls \
		--disable-libssp \
		--disable-libsanitizer \
		--disable-libquadmath \
		--disable-fixincludes \
		--disable-libgomp \
		--disable-gcov \
		--disable-libstdcxx \
		--enable-languages=c,c++ \
		--disable-werror

build-binutils:
	$(MAKE) $(MAKEFLAGS) -C build-binutils

build-gcc:
	$(MAKE) $(MAKEFLAGS) -C build-gcc

install-binutils:
	make -C build-binutils install

install-gcc:
	make -C build-gcc install
