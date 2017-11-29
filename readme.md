# RPUlux 

From <https://github.com/epccs/RPUlux/>

## Overview

Board with ATmega328p and six AL8805 constant current drivers and headers for multi-drop serial Shields

This programmable ATmega328p based board has headers for a [RPUpi] or [RPUadpt] mezzanine shield. User's firmware application can monitor the input current with a high side current sense (ZXCT1087) with ADC channel six, and the input voltage with a voltage divider on channel seven. Six AL8805 based constant current sources are provided for LED strings. The current sources are PWM modulated at 500Hz with the OCnA and OCnB outputs from Timer0, Timer1, and Timer2. 

[RPUpi]: https://github.com/epccs/RPUpi/
[RPUadpt]: https://github.com/epccs/RPUadpt/


## Status

![Status](./Hardware/status_icon.png "Status")

## [Hardware](./Hardware)

Hardware files include a schematic, bill of materials, and various notes for testing and evaluation. [Layout] files are seperate.

[Layout]: https://github.com/epccs/Eagle/


## Example

This example shows an RS-422 serial bus that allows multiple microcontroller boards to be connected to a single host computer serial port. It has an [RPUpi] shield that allows the Raspberry Pi Zero's hardware UART to connect as the host. The Pi Zero W has on board WiFi which I use for SSH connections and Samba file sharing. The other controller boards use an [RPUadpt] shield to daisy-chains the RS-422 with CAT5 cables. 

![MultiDrop](./Hardware/Documents/MultiDrop.png "RPUlux MultiDrop")

The above setup has six RGB lights. Each color has an eight bit 500Hz PWM value controled with the ATmega328p.

The solar charging shown is risky, so I plan to try a small solar panel that will not provide enough current to damage the battery to work on my software. A bench supply with a fairly small capacitor may also work, but it may damage the shorting FET. The idea is that once the battery is charged to about 14.7V then I will short the PV panel so that current does not charge the battery. The ADC0 (or ADC1) can sort of measure how much current is shorted by measuring the voltage across the Rds_on of the shorting FET.


## AVR toolchain

The core files for this board are in the /lib folder. Each example has its files and a Makefile in its own folder. The toolchain is available as standard packages on Ubuntu and Raspbian. 

```
sudo apt-get install git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/Punica/
```

* [gcc-avr](http://packages.ubuntu.com/search?keywords=gcc-avr)
* [binutils-avr](http://packages.ubuntu.com/search?keywords=binutils-avr)
* [gdb-avr](http://packages.ubuntu.com/search?keywords=gdb-avr)
* [avr-libc](http://packages.ubuntu.com/search?keywords=avr-libc)
* [avrdude](http://packages.ubuntu.com/search?keywords=avrdude)

The software is a guide, it is in C because that works for me.
