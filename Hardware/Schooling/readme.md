# Description

Some lessons I learned doing RPUlux.

# Table Of Contents:

1. ^0 PWR_V divider
1. ^0 Add defaults on IO Controls
1. ^0 Pull Down Alternate Power Control
1. ^0 Solar Panel Zener brakedown failure


## ^0  PWR_V divider

R4 changed from 1.5k to 15.8k Ohm. This is actually the original value I had on the last version of RPUno that was assembled. I decided to try the 1.5k since it would work well
with the 1.1V bandgap referance that I am now calibrating. Unfortuanly a 1% 100k in series with a 1.5k Ohm makes a divider that has a relationship problem. The uncertantly of the 100k is +/-1k and that is nearly the size of the sense resistor. The 100k should be 0.1% if I want to divide it down like that.


## ^0  Add defaults on IO Controls

Controls should have a known defaut so the will do known things when bootloading, powering up, or if the user does not set them.

IO8 needs a 10k pull down to be defined as off by default. Also add 100k to Q1 zener for leakage current.

IO12 (CS0_EN or MISO) and IO13 (CS1_EN or SCK) needs a 10k pull down to be defined as off by default.

IO4 and IO7 needs a 10k pull down so the NMOS SINK is defined as off by default.

IO3, IO5, IO6, IO9, nSS, and MOSI are used for for PWM Control and they should to be off by default.

The AL8805 CTRL pin has a 50kOhm resistance to its internal reference (e.g. about 2.5V) and it needs to be brought down to .4V to turn off the converter, which can be done with anything less than 9.5k Ohm. Note 1k Ohm would use 2.5mA at rest and 5mA when turned on I want to use a higher value to reduce the loss, I have 8.45k Ohm on hand. 

![PullDownPWMctrl](./PullDownPWMctrl.png "Pull Down PWM CNRL")

## ^0  Pull Down Alternate Power Control

IO8 is used for Alternate Power Control, it needs to be off by default (e.g. bootloading, or if the user does not set it).

## ^0  Solar Panel Zener brakedown failure

Shorting a solar panel without by-pass diodes will cause more than a 10V reverse voltage on a partly shaded cell. That cell will then experience Zener breakdown failure.

![Zener](./SolarReverseBreakdownIssue.png "Zener Brakedown")
