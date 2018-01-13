# Description

Some lessons I learned doing RPUlux.

# Table Of Contents:

3. ^0 Pull Down PWM Control
2. ^0 Pull Down Alternate Power Control
1. ^0 Solar Panel Zener brakedown failure


## ^0  Pull Down PWM Control

IO3, IO5, IO6, IO9, nSS, and MOSI are used for for PWM Control, and they need to be off by default (e.g. bootloading, power up,  or if the user does not set them).

The AL8805 CTRL pin has a 50kOhm resistance to its internal reference (e.g. about 2.5V) and it needs to be brought down to .4V to turn off the converter, which can be done with anything less than 9.5k Ohm. Note 1k Ohm would use 2.5mA at rest and 5mA when turned on I want to use a higher value to reduce the loss, I have 8.45k Ohm on hand. 

![PullDownPWMctrl](./PullDownPWMctrl.png "Pull Down PWM CNRL")

## ^0  Pull Down Alternate Power Control

IO8 is used for Alternate Power Control, it needs to be off by default (e.g. bootloading, or if the user does not set it).

## ^0  Solar Panel Zener brakedown failure

Shorting a solar panel without by-pass diodes will cause more than a 10V reverse voltage on a partly shaded cell. That cell will then experience Zener breakdown failure.

![Zener](./SolarReverseBreakdownIssue.png "Zener Brakedown")
