# Makefile to build light_ws2812 library examples
# This is not a very good example of a makefile - the dependencies do not work, therefore everything is rebuilt every time.

# Change these parameters for your device

F_CPU = 16000000UL
DEVICE = atmega328p

# Tools:
CC = avr-gcc

LIB = light_ws2812
TARGET = MSGEQ7
DEP = ws2812_config.h Light_WS2812/light_ws2812.h

CFLAGS = -g2 -I. -ILight_WS2812 -mmcu=$(DEVICE) -DF_CPU=$(F_CPU)
CFLAGS+= -Os -ffunction-sections -fdata-sections -fpack-struct -fno-move-loop-invariants -fno-tree-scev-cprop -fno-inline-small-functions
CFLAGS+= -Wall -Wno-pointer-to-int-cast
#CFLAGS+= -Wa,-ahls=$<.lst

LDFLAGS = -Wl,--relax,--section-start=.text=0,-Map=main.map

all: $(TARGET)

# TODO: Use AVRM flash.
flash: $(TARGET)
	avrdude -F -V -c arduino -p m328p -P /dev/ttyUSB0 -b 57600 -U flash:w:$(TARGET).hex

$(LIB): $(DEP)
	echo Building Library
	$(CC) $(CFLAGS) -o Objects/$@.o -c Light_WS2812/$@.c

$(TARGET): $(LIB)
	echo Building $@
	$(CC) $(CFLAGS) -o Objects/$@.o $@.c Light_WS2812/$^.c
	avr-objcopy -j .text  -j .data -O ihex Objects/$@.o $@.hex
	avr-objdump -d -S Objects/$@.o >Objects/$@.lss

size:
	avr-size Objects/$@.o

.PHONY:	clean

clean:
	rm -f *.hex Objects/*.o Objects/*.lss
