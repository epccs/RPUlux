# Description

This shows the setup and method used for evaluation of RPUlux.

# Table of References


# Table Of Contents:

1. ^1 AL8805 Curr Sense is Unlike Datasheet


## ^1 AL8805 Curr Sense is Unlike Datasheet

The datasheet example layout shows switching current (e.g. AC) flowing between the AL8805 sense pins. Some (including myself) consider this to be bad practice. My design does not follow the datasheet layout so the sense input voltage matches the resistor voltage. Doing this a find the sense resistor size is about half of what the datasheet suggests, meaning that the set current threshold voltage seems to be about 50mV rather than 100mV. The hysteresis seems to be about 20% of the threshold voltage, so the current ramp looks as I would expect. 

Using 0.15 Ohm (rather than 0.3 Ohm) gives 330mA on the LED channels, which is what I was trying for. 

It is still early in the evaluation but I wanted to note that the sense resistor is different than what the datasheet indicates.