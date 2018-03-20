# Description

This shows the setup and method used for evaluation of RPUlux.

# Table of References


# Table Of Contents:

1. ^1 Demo Setup
1. ^1 AL8805 Curr Sense is Unlike Datasheet


## ^1 Demo Setup

Build a demo setup. I used some firewood to make a lamp, it has a hole drilled from the top to the bottome (that is no easy task to get alligned). I soldered some copper grounding wire togather to act as a frame and a heat spreader for the LED's. There are two [RD] and four [WH] strings each having three Cree XPE2 LED. The three up board configuration needs to disipate 3 Watts of heat at 330mA, whith will get to hot without some sort of heat spreader. 

[RD]: https://www.ledsupply.com/leds/cree-xlamp-xp-e2-color-high-power-led-star
[WH]: https://www.ledsupply.com/leds/cree-xlamp-xpe2-white-high-power-led

A Raspbery Pi Zero running Raspian stretch lite is installed on an RPUpi^3 shield (it has a WiFi dongle). 

This setup was let run a few weeks with a red led channel set to PWM at 32. The battery charger seems to keep, but since the firmware is not smart about turning off the Pi Zero I did not want to risk a demanding load. 

![DemoSetup](./RPUlux^1+RPUpi^3+LA12V+CH1=3xRD "Demo Setup")

Although the AL8805 is not doing what the datasheet says it does seem to operate correctly.


## ^1 AL8805 Curr Sense is Unlike Datasheet

The datasheet example layout shows switching current (e.g. AC) flowing between the AL8805 sense pins. My ^1 design does not follow the datasheet layout. I find the sense resistor size is about half of what the datasheet suggests.

Using 0.15 Ohm (rather than 0.3 Ohm) gives 330mA on the LED channels. 

The next version ^2 will let switching current flow under the sense resistor to the VIN bypass, so it will use 0.3 Ohm to run at 330mA.