# AVR-GCC Makefile
PROJECT=avr_sensor
SOURCES=main.c uart.c crc8.c onewire.c  ds18x20.c uart_addon.c gpio_hw.c
CC=avr-gcc
OBJCOPY=avr-objcopy
MMCU=atmega8

CFLAGS=-mmcu=$(MMCU) -Wall -Os -DOW_ONE_BUS


$(PROJECT).hex: $(PROJECT).out
	$(OBJCOPY) -O ihex $(PROJECT).out $(PROJECT).hex
	$(OBJCOPY) -O binary $(PROJECT).out $(PROJECT).bin

$(PROJECT).out: $(SOURCES) 
	$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)

program: $(PROJECT).hex
	avrdude -p m8 -c usbasp -e -U flash:w:$(PROJECT).hex
clean:
	rm -f $(PROJECT).o
	rm -f $(PROJECT).out
	rm -f $(PROJECT).map
	rm -f $(PROJECT).hex
	rm -f $(PROJECT).bin
