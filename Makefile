
# microcontroller and project specific settings
TARGET = fnordlicht
F_CPU = 16000000UL
MCU = atmega8

SUBDIRS = at_keyboard
OBJECTS += $(patsubst %.c,%.o,$(shell echo *.c)) at_keyboard/at_keyboard.o
HEADERS += $(shell echo *.h) at_keyboard/at_keyboard.h
CFLAGS += -Werror
LDFLAGS += -L/usr/local/avr/avr/lib

include avr.mk


.PHONY: all $(SUBDIRS)

all: $(TARGET).hex $(TARGET).eep.hex $(TARGET).lss

$(TARGET): $(OBJECTS) $(TARGET).o

%.o: $(HEADERS)

$(SUBDIRS):
	$(MAKE) -e -C $@

bootloader.hex:
	$(MAKE) -C boot/v0_7
	cp boot/v0_7/main.hex bootloader.hex

.PHONY: install

install: program-serial-$(TARGET) program-serial-eeprom-$(TARGET)

.PHONY: clean clean-$(TARGET) clean-$(SUBDIRS) clean-botloader

clean: clean-$(TARGET) clean-$(SUBDIRS) clean-bootloader

clean-$(TARGET):
	$(RM) $(TARGET) $(OBJECTS)

clean-$(SUBDIRS):
	$(MAKE) -C $(patsubst clean-%,%,$@) clean

clean-bootloader:
	$(MAKE) -C boot/v0_7 clean

.PHONY: bootstrap fuse install-bootloader

bootstrap: fuse install-bootloader install

fuse-atmega8:
	$(AVRDUDE) -p m8 -c $(ISP_PROG) -P $(ISP_DEV) -U hfuse:w:0xD0:m
	$(AVRDUDE) -p m8 -c $(ISP_PROG) -P $(ISP_DEV) -U lfuse:w:0xE0:m

install-bootloader: bootloader.hex program-isp-bootloader

