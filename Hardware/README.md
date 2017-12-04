# Hardware

## Overview

This board has an ATmega328p and and six AL8805 constant current LED drivers. The reset is TBD.

Bootloader options include [optiboot] and [xboot]. Serial bootloaders can't change the hardware fuse setting which reduces programming errors that can accidentally brick the controller. 

[optiboot]: https://github.com/Optiboot/optiboot
[xboot]: https://github.com/alexforencich/xboot

## Inputs/Outputs/Functions

```
        ATmega328p programs are compiled with open source tools that run nearly everywhere.
        Input power can range from 7 to 36V DC
        High side current sense of input power is connected to ADC6.
        Input power voltage is divided down and connected to ADC7.
        Three digital or analog input/outputs (D14/ADC0,D15/ADC1,D16/ADC3) with level conversion.
        IO4, IO7, and IO8 control current sinking N-CH MOSFET connected to ADC0, ADC1, and ADC2.
        IO12 and IO13 each control 22mA current source.
```

## Uses

```
        Six AL8805 buck converters drive constant current through LED channels
        Each LED channel has a 500Hz PWM control line from the ATmega328p.
        Allows control of two RGB color lighting sources.
        Sink solar panel power when the battery is full, and measure approximate current with Rds_on.
```

## Notice

```
        AREF from ATmega328p is not connected to the header.
        3V3 is not present on the board, the header pin is not connected.
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
        ^0  Done: Design, Layout, BOM,
            WIP: Review*,
            Todo: Order Boards, Assembly, Testing, Evaluation.
            *during review the Design may change without changing the revision.
```

Debugging and fixing problems i.e. [Schooling](./Schooling/)

Setup and methods used for [Evaluation](./Evaluation/)


# Design

The board is 0.063 thick, FR4, two layer, 1 oz copper with ENIG (gold) finish.

![Top](./Documents/17323,Top.png "RPUlux Top")
![TAssy](./Documents/17323,TAssy.jpg "RPUlux Top Assy")
![Bottom](./Documents/17323,Bottom.png "RPUlux Bottom")
![BAssy](./Documents/17323,BAssy.jpg "RPUlux Bottom Assy")

## Electrical Parameters (Typical)

```
TBD
```

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

## Stopping charge from solar panel

A few approaches to prevent overcharging a battery with a solar panel.

![SolarPwrOptions](./Documents/SolarPwrOptions.png "RPUlux Solar Power Options")

Using IO4 and IO7 to control onboard N-CH MOSFET it is possible to short the solar panel. The blocking diode needs to prevent the battery from supplying current to the short (and block night or dark discharge current).

It is also possible to use IO4 to open circuit the solar panel. This requires more off-board parts but is really a question of comfort level. Most people seem to have a bad reaction to seeing the idea of shorting the solar panel, so I think it is important to give other options.

Another problem is not getting enough sun, which needs to be handled by software by not running the lights at night or the Raspberry Pi. 

An even better option for charge control is to use something like [SunSaver-6L] which will provide a lot of protection. This removes the need to write some of the control software, which if done wrong may damage the battery or hardware on the board. 

[SunSaver-6L]: https://www.solar-electric.com/ss-6l.html