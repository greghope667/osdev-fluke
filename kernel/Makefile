.PHONY: dirs build clean
.DEFAULT_GOAL := build
MAKEFLAGS += --no-builtin-rules

### Customisation

CC ?= gcc
AS ?= as
LD ?= ld

CFLAGS ?= -Og -g3
CXXFLAGS ?= -Og -g3
ASMFLAGS ?= --gdwarf-5

### Build directories

dirs:
	find src/ -type d -print0 | xargs -0 -I X mkdir -p build/X

### C/ASM build config

FLAGS =\
	-Isrc \
	-isystem ../include \
	-Wall \
	-Wextra \
	-mgeneral-regs-only \
	-mno-red-zone \
	-mcmodel=kernel \
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
LDFLAGS += -static -znoexecstack

C_SRCS = $(shell find src -name '*.c')
CXX_SRCS = $(shell find src -name '*.cxx')
ASM_SRCS = $(shell find src -name '*.s')
OBJS = $(ASM_SRCS:%.s=build/%.o) $(C_SRCS:%.c=build/%.o) $(CXX_SRCS:%.cxx=build/%.o)
DEPS = $(C_SRCS:%.c=build/%.d) $(CXX_SRCS:%.cxx=build/%.d)

-include $(DEPS)

build/%.o: %.c | dirs
	$(CC) $(CFLAGS) -c $< -MMD -MF build/$*.d -o $@

build/%.o: %.cxx | dirs
	$(CC) $(CXXFLAGS) -c $< -MMD -MF build/$*.d -o $@

build/%.o: %.s | dirs
	cpp -fworking-directory $< | $(AS) $(ASMFLAGS) -o $@

kernel.elf: $(OBJS) src/linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -T src/linker.ld -o $@
	nm -n $@ | c++filt -p > build/symbols
	printf "\n" >> build/symbols
	objcopy --update-section .symbols=build/symbols $@
	size $(OBJS) $@

### Named targets

build: kernel.elf

clean:
	find build/src -type f -delete
