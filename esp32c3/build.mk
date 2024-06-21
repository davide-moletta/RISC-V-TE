PROG        ?= firmware
ARCH        ?= esp32c3
RISC_V_TE   ?= $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)
ESPUTIL     ?= $(RISC_V_TE)/esputil/esputil
CFLAGS      ?= -W -Wall -Wextra -Werror -Wundef -Wshadow -pedantic \
               -Wdouble-promotion -fno-common -Wconversion \
               -march=rv32imc -mabi=ilp32 \
               -Os -ffunction-sections -fdata-sections -fno-builtin-printf \
               -I. -I$(RISC_V_TE)/$(ARCH) $(EXTRA_CFLAGS)
LINKFLAGS   ?= -T$(RISC_V_TE)/$(ARCH)/link.ld -nostdlib -nostartfiles -Wl,--gc-sections $(EXTRA_LINKFLAGS)
CWD         ?= $(realpath $(CURDIR))
FLASH_ADDR  ?= 0  # 2nd stage bootloader flash offset
DOCKER      ?= docker run -it --rm -v $(CWD):$(CWD) -v $(RISC_V_TE):$(RISC_V_TE) -w $(CWD) mdashnet/riscv
TOOLCHAIN   ?= $(DOCKER) riscv-none-elf
SRCS        ?= $(RISC_V_TE)/$(ARCH)/boot.c $(SOURCES) $(EXTRA_SOURCES)

build: $(PROG).bin

$(PROG).elf: $(SRCS)
	$(TOOLCHAIN)-gcc  $(CFLAGS) $(SRCS) $(LINKFLAGS) -o $@
#	$(TOOLCHAIN)-size $@

$(PROG).bin: $(PROG).elf $(ESPUTIL)
	$(ESPUTIL) mkbin $(PROG).elf $@

flash: $(PROG).bin $(ESPUTIL)
	$(ESPUTIL) flash $(FLASH_ADDR) $(PROG).bin

monitor: $(ESPUTIL)
	$(ESPUTIL) monitor

$(MDK)/esputil/esputil.c:
	git submodule update --init --recursive

$(ESPUTIL): $(RISC_V_TE)/esputil/esputil.c
	make -C $(RISC_V_TE)/esputil esputil

clean:
	@rm -rf *.{bin,elf,map,lst,tgz,zip,hex} $(PROG)*
