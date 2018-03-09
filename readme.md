# RPUlux 

From <https://github.com/epccs/RPUlux/>

[![Build Status](https://travis-ci.org/epccs/RPUlux.svg?branch=master)](https://travis-ci.org/epccs/RPUlux)

## Overview

Board with ATmega328p and six AL8805 constant current drivers and headers for multi-drop serial Shields

This programmable ATmega328p based board has headers for a [RPUpi] or [RPUadpt] mezzanine shield. User's firmware application can monitor the input current with a high side current sense (ZXCT1087) with ADC channel six, and the input voltage with a voltage divider on channel seven. Six AL8805 based constant current sources are provided for LED strings. The current sources are PWM modulated at 500Hz with the OCnA and OCnB outputs from Timer0, Timer1, and Timer2. 

[RPUpi]: https://github.com/epccs/RPUpi/
[RPUadpt]: https://github.com/epccs/RPUadpt/

[Forum](http://rpubus.org/bb/viewforum.php?f=19)

[HackaDay](https://hackaday.io/project/60522-rpulux)


## Status

![Status](./Hardware/status_icon.png "Status")

## [Hardware](./Hardware)

Hardware files include a schematic, bill of materials, and various notes for testing and evaluation. [Layout] files are seperate.

[Layout]: https://github.com/epccs/Eagle/


## Example

This example shows an RS-422 serial bus that allows multiple microcontroller boards to be connected to a single host computer serial port. It has an [RPUpi] shield that allows the Raspberry Pi Zero's hardware UART to connect as the host. The Pi Zero W has on board WiFi which I use for SSH connections and Samba file sharing. The other controller boards use an [RPUadpt] shield to daisy-chain the RS-422 with CAT5 cables. 

![MultiDrop](./Hardware/Documents/MultiDrop.png "RPUlux MultiDrop")

The above setup has six channels for LED lights. Each channel can be dimmed at up to 500Hz with a PWM controlled pin from the ATmega328p. To my eyes, the PWM values have eight intensity steps (1, 3, 7, 15, 31, 63, 127, 255). I do not see much difference between 126 and 127 or 250 and 255.


## AVR toolchain

The core files for this board are in the /lib folder. Each example has its files and a Makefile in its own folder. The toolchain is available as standard packages on Ubuntu and Raspbian. 

```
sudo apt-get install git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/RPUlux/
```

* [gcc-avr](http://packages.ubuntu.com/search?keywords=gcc-avr)
* [binutils-avr](http://packages.ubuntu.com/search?keywords=binutils-avr)
* [gdb-avr](http://packages.ubuntu.com/search?keywords=gdb-avr)
* [avr-libc](http://packages.ubuntu.com/search?keywords=avr-libc)
* [avrdude](http://packages.ubuntu.com/search?keywords=avrdude)

The software is a guide, it is in C because that works for me.


## Arduino IDE

An [example] using the Arduino IDE.

[example]: ./Arduino

