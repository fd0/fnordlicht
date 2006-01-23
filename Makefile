# Default options.
# =====================================================================
# You can create a config.mk to override the defaults.
-include config.mk
#
# Programmer used for In System Programming
ISP_PROG ?= dapa
# device the ISP programmer is connected to
ISP_DEV ?= /dev/parport0

# Programmer used for serial programming
# (using the bootloader)
SERIAL_PROG ?= butterfly

# device the serial programmer is connected to
SERIAL_DEV ?= /dev/ttyUSB0

# device name (for avr-gcc)
MCU = atmega8

# compiler
CC = avr-gcc

# flags for the compiler
CFLAGS = -g -Os -mmcu=${MCU} -Wall -Wstrict-prototypes -Wunreachable-code

# Targets
# =====================================================================

all: fnordlicht.hex

debug: CFLAGS+=-DDEBUG
debug: fnordlicht.hex

clean:
	rm -f *.hex *.list *.map *.obj *.cof *.o *.i *.s *.lst *.elf *.lss tags
	cd boot/v0_7 && make clean

tags: *.c
	ctags *.c

install-fnordlicht: prog-serial-fnordlicht

install-bootloader: prog-isp-bootloader

fuse:
	avrdude -p m8 -c $(ISP_PROG) -P $(ISP_DEV) -U hfuse:w:0xD0:m
	avrdude -p m8 -c $(ISP_PROG) -P $(ISP_DEV) -U lfuse:w:0xE0:m

bootstrap: fuse bootloader.hex prog-isp-bootloader install-fnordlicht

interactive-isp:
	avrdude -p m8 -c $(ISP_PROG) -P $(ISP_DEV) -t

interactive-serial:
	avrdude -p m8 -c $(SERIAL_PROG) -P $(SERIAL_DEV) -t

prog-isp-%: %.hex
	avrdude -p m8 -c $(ISP_PROG) -P $(ISP_DEV) -U f:w:$<

prog-serial-%: %.hex
	avrdude -p m8 -c $(SERIAL_PROG) -P $(SERIAL_DEV) -U f:w:$<

bootloader.hex:
	cd boot/v0_7 && make clean && make
	cp boot/v0_7/main.hex bootloader.hex

%.o: %.c tags
	${CC} -Wa,-adhlns=$(basename $@).lst ${CFLAGS} -c $<

fnordlicht.elf: static_scripts.o fnordlicht.o
	${CC} ${CFLAGS} -o $@ static_scripts.o fnordlicht.o

%.elf: %.o
	${CC} ${CFLAGS} -o $@ $<

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom $< $@

%.lss: %.elf
	avr-objdump -h -S $< > $@
