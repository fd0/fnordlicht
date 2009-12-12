####################################################
# fnordlicht-ng Makefile
####################################################

# ATTENTION:
# Any of these variables are overridden with the values from the file
# "config.mk", you can tune your settings there.  The file "config.mk" is not
# under version control.
# Just run 'make' to create a "config.mk" with the default settings

# hardware type, possible values (default is unset):
# ATTENTION: please configure in config.mk, this is just included here for reference!
# - fnordlicht -- original fnordlicht hardware
# - fnordlichtmini -- fnordlichtmini hardware
#HARDWARE = fnordlicht

# controller
MCU = atmega8

# frequency
F_CPU = 16000000UL

# main application name (without .hex)
# eg 'test' when the main function is defined in 'test.c'
TARGET = fboot

# c sourcecode files
# eg. 'test.c foo.c foobar/baz.c'
SRC = $(wildcard *.c)

# asm sourcecode files
# eg. 'interrupts.S foobar/another.S'
ASRC = $(wildcard *.S)

# headers which should be considered when recompiling
# eg. 'global.h foobar/important.h'
HEADERS = $(wildcard *.h)

# include directories (used for both, c and asm)
# eg '. usbdrv/'
INCLUDES =


# use more debug-flags when compiling
DEBUG = 0

# default baudrate
CONFIG_SERIAL_BAUDRATE = 19200

# avrdude programmer protocol
PROG = usbasp
# avrdude programmer device
DEV = usb
# further flags for avrdude
AVRDUDE_FLAGS =

CFLAGS += -DHARDWARE_$(HARDWARE)=1
CFLAGS += -DCONFIG_SERIAL_BAUDRATE=$(CONFIG_SERIAL_BAUDRATE)

# use a custom linker script
LDFLAGS += -L$(CURDIR)/ldscripts

.PHONY: all

# main make target (moved up here because of the config.mk target)
all: $(TARGET).hex

# create config.mk (if it does not exist yet)
$(CURDIR)/config.mk:
	@$(CP) config.mk.template config.mk
	@echo "===================================================="
	@echo "created file $@"
	@echo 'please tune your settings (especially $$HARDWARE)'
	@echo "there, then run 'make' again"
	@echo "===================================================="
	@exit 1

# include config file
-include $(CURDIR)/config.mk

# bootloader section start
# (see datasheet)
ifeq ($(MCU),atmega8)
	# atmega8 with 1024 words bootloader:
	# bootloader section starts at 0xc00 (word-address) == 0x1800 (byte-address)
	#BOOT_SECTION_START = 0x1800
	#
	# atmega8 with 512 words bootloader:
	# bootloader section starts at 0xe00 (word-address) == 0x1c00 (byte-address)
	BOOT_SECTION_START = 0x1c00
	# 7kb data memory (raw memory minus bootloader space)
	DATA_MEM = 7

	# use custom linker script
	LDFLAGS += -T avr4.x
endif
ifeq ($(MCU),atmega88)
	# atmega88 with 1024 words bootloader:
	# bootloader section starts at 0xc00 (word-address) == 0x1800 (byte-address)
	#BOOT_SECTION_START = 0x1800
	#
	# atmega88 with 512 words bootloader:
	# bootloader section starts at 0xe00 (word-address) == 0x1c00 (byte-address)
	BOOT_SECTION_START = 0x1c00
	# 7kb data memory (raw memory minus bootloader space)
	DATA_MEM = 7

	# use custom linker script
	LDFLAGS += -T avr4.x
endif
ifeq ($(MCU),atmega168)
	# atmega168 with 1024 words bootloader:
	# bootloader section starts at 0x1c00 (word-address) == 0x3800 (byte-address)
	#BOOT_SECTION_START = 0x3800
	#
	# atmega168 with 512 words bootloader:
	# bootloader section starts at 0x1e00 (word-address) == 0x3c00 (byte-address)
	BOOT_SECTION_START = 0x3c00
	# 15kb data memory (raw memory minus bootloader space)
	DATA_MEM = 15

	# use custom linker script
	LDFLAGS += -T avr5.x
endif

LDFLAGS += -Wl,--section-start=.text=$(BOOT_SECTION_START)
CFLAGS += -DBOOT_SECTION_START=$(BOOT_SECTION_START)


####################################################
# 'make' configuration
####################################################
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AS = avr-as
SIZE = avr-size
CP = cp
RM = rm -f
RMDIR = rm -rf
MKDIR = mkdir
AVRDUDE = avrdude

# flags for automatic dependency handling
DEPFLAGS = -MD -MP -MF .dep/$(@F).d

# flags for the compiler (for .c files)
CFLAGS += -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=gnu99 -fshort-enums $(DEPFLAGS)
CFLAGS += $(addprefix -I,$(INCLUDES))
CFLAGS += -fno-split-wide-types
CFLAGS += --param inline-call-cost=2  -finline-limit=3 -fno-inline-small-functions
CFLAGS += -Wl,--relax

# flags for the compiler (for .S files)
ASFLAGS += -g -mmcu=$(MCU) -DF_CPU=$(F_CPU) -x assembler-with-cpp $(DEPFLAGS)
ASFLAGS += $(addprefix -I,$(INCLUDES))

# flags for the linker
LDFLAGS += -mmcu=$(MCU)

# fill in object files
OBJECTS += $(SRC:.c=.o)
OBJECTS += $(ASRC:.S=.o)

# include more debug flags, if $(DEBUG) is 1
ifeq ($(DEBUG),1)
	CFLAGS += -Wall -W -Wchar-subscripts -Wmissing-prototypes
	CFLAGS += -Wmissing-declarations -Wredundant-decls
	CFLAGS += -Wstrict-prototypes -Wshadow -Wbad-function-cast
	CFLAGS += -Winline -Wpointer-arith -Wsign-compare
	CFLAGS += -Wunreachable-code -Wdisabled-optimization
	CFLAGS += -Wcast-align -Wwrite-strings -Wnested-externs -Wundef
	CFLAGS += -Wa,-adhlns=$(basename $@).lst
	CFLAGS += -DCONFIG_DEBUG=1
endif

####################################################
# avrdude configuration
####################################################
ifeq ($(MCU),atmega8)
	AVRDUDE_MCU=m8
endif
ifeq ($(MCU),atmega48)
	AVRDUDE_MCU=m48
endif
ifeq ($(MCU),atmega88)
	AVRDUDE_MCU=m88
endif
ifeq ($(MCU),atmega168)
	AVRDUDE_MCU=m168
endif

AVRDUDE_FLAGS += -p $(AVRDUDE_MCU)

####################################################
# make targets
####################################################

.PHONY: hardware-check clean distclean avrdude-terminal

# check if HARDWARE is set
hardware-check:
ifeq ($(HARDWARE),)
	@echo 'please edit config.mk and set $$HARDWARE'
	@exit 1
endif

$(TARGET).elf: hardware-check $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

# all objects (.o files) and config.mk
$(OBJECTS): $(HEADERS) config.mk

# remove all compiled files
clean:
	$(RM) $(foreach ext,elf hex eep.hex map,$(TARGET).$(ext)) \
		$(foreach file,$(patsubst %.o,%,$(OBJECTS)),$(foreach ext,o lst lss,$(file).$(ext)))

# additionally remove the dependency makefile and config.mk
distclean: clean
	$(RMDIR) .dep
	$(RM) config.mk

# avrdude-related targets
install program: program-$(TARGET)

avrdude-terminal:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(PROG) -P $(DEV) -t

program-%: %.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(PROG) -P $(DEV) -U flash:w:$< -U lock:w:0x2f:m

program-eeprom-%: %.eep.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(PROG) -P $(DEV) -U eeprom:w:$<

# special programming targets
%.hex: %.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	@echo "===================================================="
	@echo "$@ compiled for hardware: $(HARDWARE)"
	@echo "using controller $(MCU)"
	@echo -n "size for $< is "
	@$(SIZE) -A $@ | grep '\.sec1' | tr -s ' ' | cut -d" " -f2
	@echo "===================================================="

%.eep.hex: %.elf
	$(OBJCOPY) --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex -j .eeprom $< $@

%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

.PHONY: fuses-atmega8-fnordlichtmini fuses-lock

fuses-atmega8-fnordlichtmini:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(PROG) -P $(DEV) -U lfuse:w:0xff:m -U hfuse:w:0xda:m

fuses-lock:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(PROG) -P $(DEV) -U lock:w:0x2f:m

-include $(shell $(MKDIR) .dep 2>/dev/null) $(wildcard .dep/*)
