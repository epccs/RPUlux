# Hardware

## Overview

This board has an ATmega328p and and six AL8805 constant current LED drivers. Input power is 7 to 36 VDC. 

Bootloader options include [optiboot] and [xboot]. Serial bootloaders can't change the hardware fuse setting which reduces programming errors that can accidentally brick the controller. 

[optiboot]: https://github.com/MCUdude/optiboot_flash
[xboot]: https://github.com/alexforencich/xboot

## Inputs/Outputs/Functions

```
        ATmega328p programs are compiled with open source tools that run nearly everywhere.
        Input power can range from 7 to 36V DC.
        Reverse Power Protection with indicator LED and low loss.
        Shield VIN pin can be disabled (2).
        Alternate Input Power can be enabled (8).
        The high side VIN current sensor is connected to ADC6.
        Power VIN voltage is divided down and connected to ADC7.
        Six constant-current LED channels controlled with ATmega328p PWM pins.
        Each LED channel has an AL8805 buck converter with 0.3 Ohm set resistor for 330mA.
        A weak pull-up will set an analog voltage on the AL8805 control pin for 50 to 150mA.
        Three digital input/outputs (14, 15, 16) with level shift for connecting to a higher voltage.
        The digital inputs are also connected to ADC channels (ADC0, ADC1, ADC2).
        The digital inputs also have current sinking N-CH MOSFET that may be enabled (4, 7)
        Two 22mA current sources may be enabled (12, 13) for current loop sensors.
```

## Uses

```
        Six constant-current LED channels
        Each LED channel has diming control with a PWM line from the ATmega328p.
```

## Notice

```
        AREF from ATmega328p is not connected to the header.
        3V3 is not present on the board and the header pin is not connected.
        ADC4 and ADC5 are used for I2C and not connected to the analog header.
        Diming with PWM is slightly audibly perceptible.
```


# Table Of Contents

1. [Status](#status)
2. [Design](#design)
3. [Bill of Materials](#bill-of-materials)
4. [Assembly](#assembly)
5. [How To Use](#how-to-use)


# Status

![Status](./status_icon.png "RPUlux Status")

```
        ^2  Done: Design, Layout, BOM, Review*, Order Boards, Assembly, Testing, 
            WIP: Evaluation.
            Todo: 
            *during review the Design may change without changing the revision.
            location: 2018-28-3 on RGB firewood lamp.

        ^1  Done: Design, Layout, BOM, Review*, Order Boards, Assembly, Testing, 
            WIP: Evaluation.
            location: 2018-1-3 on 1200WH/600RD firewood lamp.
            location: 2018-20-2 test bench.

        ^0  Done: Design, Layout, BOM, Review*, Order Boards, Assembly, Testing, Evaluation.
            location: 2018-22-1 test bench.
            location: 2018-20-2 on RGB firewood lamp.
            location: 2018-28-3 scrap/salvage.
            
```

Debugging and fixing problems i.e. [Schooling](./Schooling/)

Setup and methods used for [Evaluation](./Evaluation/)


# Design

The board is 0.063 thick, FR4, two layer, 1 oz copper with ENIG (gold) finish.

![Top](./Documents/17323,Top.png "RPUlux Top")
![TAssy](./Documents/17323,TAssy.jpg "RPUlux Top Assy")
![Bottom](./Documents/17323,Bottom.png "RPUlux Bottom")
![BAssy](./Documents/17323,BAssy.jpg "RPUlux Bottom Assy")


## Mounting

```
DIN rail
```

## Electrical Schematic

![Schematic](./Documents/17323,Schematic.png "RPUlux Schematic")

## Testing

Check correct assembly and function with [Testing](./Testing/)


# Bill of Materials

The BOM is a CVS file(s), import it into a spreadsheet program like LibreOffice Calc (or Excel), or use a text editor.

Option | BOM's included
----- | ----- 
A. | [BRD] 
M. | [BRD] [SMD] [HDR] 
W. | [BRD] [SMD] [HDR] [PLUG]
Z. | [BRD] [SMD] [HDR] [PLUG] [DIN]

[BRD]: ./Design/17323BRD,BOM.csv
[SMD]: ./Design/17323SMD,BOM.csv
[HDR]: ./Design/17323HDR,BOM.csv
[PLUG]: ./Design/17323PLUG,BOM.csv
[DIN]: ./Design/17323DIN,BOM.csv


# Assembly

## SMD

The board is assembled with CHIPQUIK no-clean solder SMD291AX (RoHS non-compliant). 

The SMD reflow is done in a Black & Decker Model NO. TO1303SB which has the heating elements controlled by a Solid State Relay and an ATMega328p loaded with this [Reflow] firmware.

[Reflow]: ../Reflow


# How To Use

## Prevent overcharging a battery

I do not recomend the DIY power electronics options, I recomend a charge controller like the [SunSaver-6L] which provides a lot of protection. 

[SunSaver-6L]: https://www.solar-electric.com/ss-6l.html

The following is some DIY approaches to prevent overcharging a battery with a solar panel. They are risky so expect to damage somthing e.g. the battery, solar panel, RPUlux board. This means you may have to replace surface mount parts, the bill of materials will tell you what to use, and the assembly drawings show where they are used. With some luck Digikey or Mouser will have the parts in stock. Again don't do this if you don't want to rework SMD parts.

![SolarPwrOption_open](./Documents/SolarPwrOption_open.png "Solar Power Option Open Circuit")

First option is to write software that controls IO8 to open circuit the solar panel that is connected to the alternate power input. The alternate power input must be less than 1.5 amps it is for small solar panels which normaly lack by-pass diodes.

![SolarPwrOption_ssr](./Documents/SolarPwrOption_ssr.png "Solar Power Option Open Circuit with SSR")

Software that controls IO4 can operate an SSR to open circuit the solar panel. This option is for solar panels that lack by-pass diodes and produce over 1.5A short-circuit current. Note the onboard N-CH MOSFET will connect the solar pannel to the battery when it pulls down.


## LED options

I have no connection with this outfit but I am using their parts.

* [RGB] Cree XP-E2 350mA flux (67.2/107/23.5)
* [Color] Cree XP-E2 having colors of DWL 450:465:520:565:585:610:620
* [White] Cree XP-E2 350mA flux 116

[RGB]: https://www.ledsupply.com/leds/cree-xpe2-rgb-high-power-led
[Color]: https://www.ledsupply.com/leds/cree-xlamp-xp-e2-color-high-power-led-star
[White]: https://www.ledsupply.com/leds/cree-xlamp-xpe2-white-high-power-led

If the the White forward voltage is less than 3.25V then a string of three would have 9.75V and that seems to work with a 12V input.


# Original Ideas

This board combines ideas from Sparkfun's [PicoBuck], Arduinos [Uno], and my own take on board design and hardware interfacing. I use shields [RPUpi] and [RPUadpt] that I also designed, they daisy-chain serial from the Raspberry Pi Zero which acts as a host computer, e.g. it compiles firmware and then uploads the image to one of the control boards using a open source toolchain.

[PicoBuck]: https://www.sparkfun.com/products/13705
[Uno]: https://store.arduino.cc/usa/arduino-uno-rev3
[RPUpi]: https://github.com/epccs/RPUpi/
[RPUadpt]: https://github.com/epccs/RPUadpt
