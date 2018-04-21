# Serial Bootloader

## Overview

AVR microcontrollers are initialy programmed through an In-circuit Serial Programmer (ICSP/ISP), however with this firmware loaded in the microcontroller then programing can be done over a serial interface (TX/RX) without the ICSP programmer. This is called serial bootloading. 

![Simplified Core Logic](https://raw.githubusercontent.com/epccs/RPUno/master/Bootloader/SimplifiedCoreLogic.png "Simplified Core Logic")

To install a serial bootloader requires an ISP tool. Arduino has an [ArduinoISP] sketch that I load into a Arduino Uno to use as an ISP tool.

[ArduinoISP]: https://github.com/arduino/Arduino/blob/master/build/shared/examples/11.ArduinoISP/ArduinoISP/ArduinoISP.ino

I use a two step process. First run "make fuse" which will unlock the boot flash section and set fuses (LOW, HIGH, EXTENDED). The changed fuse setting will switch operation form the internal oscilator to the exteranl crystal, if the crystal is missing the MCU will be locked up. The second step is "make isp" which will load the bootloader and lock the boot flash section. The lock prohibits the Store Program Memory (SPM) instruction from running in the boot flash section (i.e. it only works for the bootloader, the ISP will still upload). 

    WARNING: programing these fuse valuse will lock up the MCU when the crystal is missing or damaged.


## Firmware Upload

With ISP tool connected.

``` 
git clone https://github.com/epccs/RPUlux/
cd /RPUlux/Bootloader
make fuse
...
avrdude done.  Thank you.
make isp
...
avrdude done.  Thank you.
``` 

This bootloader was from RPUno which was from Optiboot
