# AVR-GCC Makefile
PROJECT=mag-rw
SOURCES=mag-rw.c usart.c timer.c
CC=avr-gcc
OBJCOPY=avr-objcopy
MMCU=atmega16

AVRDUDE = /home/lucian/fac/pm/programator/src/avrdude-5.5/avrdude -C /etc/avrdude.conf -c usbtiny -p m16 #-F  #-B 250

CFLAGS=-mmcu=$(MMCU) -O0 -Wall# -Wl,-u,vfprintf
CPPFLAGS=-I./
LDLIBS=#-lprintf_min

$(PROJECT).hex: $(PROJECT).out
	$(OBJCOPY) -O ihex $(PROJECT).out $(PROJECT).hex

$(PROJECT).out: $(SOURCES) 
	$(CC) $(CFLAGS) $(CPP_FLAGS) $(LDFLAGS) $(LDLIBS) -o $(PROJECT).out $(SOURCES)

flash: $(PROJECT).hex
	$(AVRDUDE) -U flash:w:$(PROJECT).hex
flash_read:
	$(AVRDUDE) -F -u -U eeprom:r:flash_out.hex:i
clean:
	rm -f $(PROJECT).o
	rm -f $(PROJECT).out
	rm -f $(PROJECT).map
	rm -f $(PROJECT).hex
