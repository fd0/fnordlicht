# Programmer used for In System Programming
ISP_PROG = dapa
# device the ISP programmer is connected to
ISP_DEV = /dev/parport0
# Programmer used for serial programming (using the bootloader)
SERIAL_PROG = butterfly
# device the serial programmer is connected to
SERIAL_DEV = /dev/ttyS0
# device name (for avr-gcc)
MCU = atmega8

# programs
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AS = avr-as
CP = cp
RM = rm -f
AVRDUDE = avrdude

-include config.mk

# flags for the compiler
CFLAGS += -g -Os -finline-limit=800 -mmcu=$(MCU)

ifneq ($(DEBUG),)
	CFLAGS += -Wall -W -Wchar-subscripts -Wmissing-prototypes
	CFLAGS += -Wmissing-declarations -Wredundant-decls
	CFLAGS += -Wstrict-prototypes -Wshadow -Wbad-function-cast
	CFLAGS += -Winline -Wpointer-arith -Wsign-compare -Werror
	CFLAGS += -Wunreachable-code -Wdisabled-optimization
	CFLAGS += -Wcast-align -Wwrite-strings -Wnested-externs -Wundef
	CFLAGS += -Wa,-adhlns=$(basename $@).lst
	CFLAGS += -DDEBUG
endif

all:

clean:
	$(RM) *.hex *.o *.lst

interactive-isp:
	$(AVRDUDE) -p m8 -c $(ISP_PROG) -P $(ISP_DEV) -t

interactive-serial:
	$(AVRDUDE) -p m8 -c $(SERIAL_PROG) -P $(SERIAL_DEV) -t

.PHONY: all clean interactive-isp interactive-serial

program-isp-%: %.hex
	$(AVRDUDE) -p m8 -c $(ISP_PROG) -P $(ISP_DEV) -U f:w:$<

program-serial-%: %.hex
	$(AVRDUDE) -p m8 -c $(SERIAL_PROG) -P $(SERIAL_DEV) -U f:w:$<

%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@

%.lst: %
	$(OBJDUMP) -h -S $< > $@
