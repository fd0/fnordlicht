include ../avr.mk
include ../config.mk

TARGET=keyboard_test
OBJECTS += $(patsubst %.c,%.o,$(shell echo *.c))
HEADERS += $(shell echo *.h)

all: $(TARGET).hex $(TARGET).eep.hex $(TARGET).lss

$(TARGET): $(OBJECTS) $(TARGET).o

%.o: $(HEADERS)

install: program-serial-$(TARGET) program-serial-eeprom-$(TARGET)

clean: clean-$(TARGET)

clean-$(TARGET):
	$(RM) $(TARGET) $(OBJECTS)

bootstrap: fuse install-bootloader install

keyboard_test: keyboard_test.o
