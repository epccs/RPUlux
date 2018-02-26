# Description

This shows the setup and method used for evaluation of RPUlux.

# Table of References


# Table Of Contents:

1. ^1 AL8805 Curr Sense is Unlike Datasheet


## ^1 AL8805 Curr Sense is Unlike Datasheet

The datasheet example layout shows switching current (e.g. AC) flowing between the AL8805 sense pins. My ^1 design does not follow the datasheet layout. I find the sense resistor size is about half of what the datasheet suggests.

Using 0.15 Ohm (rather than 0.3 Ohm) gives 330mA on the LED channels. 

The next version ^2 will let switching current flow under the sense resistor to the VIN bypass, so it will use 0.3 Ohm to run at 330mA.