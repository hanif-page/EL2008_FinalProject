# --- KONFIGURASI PERANGKAT KERAS ---
MCU = atmega328p
F_CPU = 16000000UL
PORT = /dev/cu.usbmodem11301
BAUD_UPLOAD = 115200

# --- KONFIGURASI TOOLCHAIN ---
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -Wall -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I./include

# --- FILE TARGET ---
TARGET = firmware
SRCS = src/main.c src/database.c
OBJS = $(SRCS:.c=.o)

# --- ATURAN KOMPILASI ---
all: $(TARGET).hex

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# --- ATURAN UPLOAD KE ARDUINO ---
upload: $(TARGET).hex
	avrdude -F -V -c arduino -p $(MCU) -P $(PORT) -b $(BAUD_UPLOAD) -U flash:w:$<

# --- PEMBERSIHAN ---
clean:
	rm -f src/*.o *.elf *.hex