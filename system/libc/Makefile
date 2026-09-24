.PHONY: dirs build install clean
.DEFAULT_GOAL := build
MAKEFLAGS += --no-builtin-rules

### Customisation

CC ?= gcc

CFLAGS ?= -O -g
CXXFLAGS ?= -O -g
ASMFLAGS ?= -g

### Build directories

dirs:
	find src/ -type d -print0 | xargs -0 -I X mkdir -p build/X

### C/ASM build config

FLAGS += \
	-Isrc \
	-Wall \
	-Wextra

CFLAGS += $(FLAGS) -std=gnu23 -Wmissing-prototypes
CXXFLAGS += $(FLAGS) -std=gnu++26 -fno-exceptions -fno-rtti -Wno-invalid-offsetof

C_SRCS = $(shell find src -name '*.c')
CXX_SRCS = $(shell find src -name '*.cxx')
ASM_SRCS = $(shell find src -name '*.S')
OBJS = $(ASM_SRCS:%.S=build/%.o) $(C_SRCS:%.c=build/%.o) $(CXX_SRCS:%.cxx=build/%.o)
DEPS = $(C_SRCS:%.c=build/%.d) $(CXX_SRCS:%.cxx=build/%.d)

-include $(DEPS)

build/%.o: %.c | dirs
	$(CC) $(CFLAGS) -c $< -MMD -MF build/$*.d -o $@

build/src/string/%.o: src/string/%.c | dirs
	$(CC) $(CFLAGS) -fno-builtin -c $< -MMD -MF build/src/string/$*.d -o $@

build/%.o: %.cxx | dirs
	$(CC) $(CXXFLAGS) -c $< -MMD -MF build/$*.d -o $@

build/%.o: %.S | dirs
	$(CC) -c $< -o $@

libc.a: $(OBJS)
	$(AR) $(ARFLAGS) -s $@ $?

crt1.o: crt/crt1.S
	$(CC) -c $< -o $@

### Named targets

build: crt1.o libc.a

install: build
	cp -R --update crt1.o libc.a "$(SYSROOT)/usr/lib"

clean:
	rm -rf crt1.o libc.a build
