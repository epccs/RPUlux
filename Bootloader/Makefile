# Makefile
TARGET = optiboot_rpulux
OBJECTS = main.o

## Chip and project-specific global definitions
MCU = atmega328p
F_CPU = 16000000UL
BAUD = 38400UL
LED = B5
LED_START_FLASHES = 0
BOOT_FLASH_SECTION_START = 0x7e00
VERSION_PLACED_AT_FLASH_LOCATION = 0x7ffe

# the efuse uses the lower three bits, so some tools read it as 0x05.
# also the lockbit uses the lower five bits, so some tools read unlock as 0xFF and lock as 0xEF
# http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p&LOW=FF&HIGH=DE&EXTENDED=05&LOCKBIT=3F
LOW_FUSE = 0xFF
HIGH_FUSE = 0xDE
EXTENDED_FUSE = 0xFD
# avrdude -v <= 6.2 
#UNLOCK_FUSE = 0x3F
#LOCK_FUSE = 0x2F
# avrdude -v >= 6.3 : "1" is returned for unused bits for -c stk500v1
UNLOCK_FUSE = 0xFF
LOCK_FUSE = 0xEF

CPPFLAGS = -DF_CPU=$(F_CPU) -DBAUD=$(BAUD)
CPPFLAGS += -DLED_START_FLASHES=$(LED_START_FLASHES)
CPPFLAGS += $(LED_DATA_FLASH_CMD) -DLED=$(LED) 

# BIGBOOT adds extra features, but takes up 1K.
# CPPFLAGS += -DBIGBOOT=1

# SOFT_UART: AVR305 soft-UART instead of hardware UART
# CPPFLAGS += -DSOFT_UART=1

# LED_DATA_FLASH: Flash LED when transferring data
# CPPFLAGS += -DLED_DATA_FLASH=1

# UART: number (0..n) for devices with more than  one hardware uart (644P, 1284P, etc)
# CPPFLAGS += -DUART=0


## Cross-compilation
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size

# FTDI's USB to serial bridge shows as /dev/ttyUSB0, 
# Uno's serial bridge (an ATmega16U2) shows as /dev/ttyACM0  (a modem,?)
# Pi Zero on chip hardware serial shows as /dev/ttyAMA0 (hardware UART on a Linux system)
ISP_PORT = /dev/ttyACM0

## Compiler/linker options
CFLAGS  = -g -Wall -Os -fno-split-wide-types -mrelax -fno-caller-saves 

TARGET_ARCH = -mmcu=$(MCU)

LDFLAGS = -Wl,--section-start=.text=$(BOOT_FLASH_SECTION_START) -Wl,--section-start=.version=$(VERSION_PLACED_AT_FLASH_LOCATION) -Wl,--relax -nostartfiles -nostdlib
#-Wl,--gc-sections


all: $(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).hex $(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).lst

baudcheck: 
	- @$(CC) --version
	- @$(CC) $(CFLAGS) -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) -E baudcheck.c -o baudcheck.tmp.sh
	- @bash baudcheck.tmp.sh
	rm -f baudcheck.tmp.sh

# platforms support EEPROM and large bootloaders need the eeprom functions that
# are defined in libc (e.g. the -lc option), even though they were explicity remove with 
# -nostdlib to save space.
%.elf: $(OBJECTS) baudcheck
	$(CC) $(LDFLAGS) $(TARGET_ARCH) -o $@ $< -lc
	$(SIZE) -C --mcu=$(MCU) $@

fuse: 
	avrdude -v -p $(MCU) -c stk500v1 -P /dev/ttyACM0 -b 19200 -e -U lock:w:$(UNLOCK_FUSE):m -U lfuse:w:$(LOW_FUSE):m -U hfuse:w:$(HIGH_FUSE):m -U efuse:w:$(EXTENDED_FUSE):m

## set fuse like they are from factory, i.e. 8MHz internal, B.O.D. disabled.
oem: 
	avrdude -v -p atmega328p -c stk500v1 -P /dev/ttyACM0 -b 19200 -e -U lock:w:$(UNLOCK_FUSE):m -U lfuse:w:0x62:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m

# the Uno (with ArduinoISP sketch) is used as ICSP tool
# https://github.com/arduino/Arduino/blob/master/build/shared/examples/11.ArduinoISP/ArduinoISP/ArduinoISP.ino
# save the eeprom so it can be put back after bootloader install. Note the -e will set all EEPROM addresses to '0xFF'.
isp: $(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).hex
	avrdude -v -p $(MCU) -c stk500v1 -P $(ISP_PORT) -b 19200 -U eeprom:r:$(TARGET)_$(MCU)_eeprom.hex:i
	avrdude -v -p $(MCU) -c stk500v1 -P $(ISP_PORT) -b 19200 -e -U flash:w:$(TARGET)_$(MCU)_-b$(BAUD)_F_CPU$(F_CPU).hex -U lock:w:$(LOCK_FUSE):m

# recover a saved eeprom
 recoveree:
	avrdude -v -p $(MCU) -c stk500v1 -P $(ISP_PORT) -b 19200 -U eeprom:w:$(TARGET)_$(MCU)_eeprom.hex

clean:
	rm -f *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin *.hex *.tmp.sh

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O binary $< $@
