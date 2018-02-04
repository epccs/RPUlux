# Amp-Hour Counter

## Overview

ADC6 is connected to a high side current sense. The [Adc] firmware is included to provide the readings. This interactive command line program demonstrates how the ATmega328p can be used to estimate the discharge. 

[Adc]: ../Adc


# Start of Day 

The [day-night][../DayNight] state machine is used to clear the discharge accumulation at TBD.


# Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUlux run 'make bootload' and it should compile and then flash the MCU.

``` 
rsutherland@leek:~/Samba/hg/RPUlux/AmpHr$ make bootload
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.


``` 
#exit is C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 

or log the terminal session

``` 
script -f -c "picocom -b 38400 /dev/ttyUSB0" stuff.json
``` 


# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 


## /\[rpu_address\]/[command \[arg\]\]

rpu_address is taken from the I2C address 0x29 (e.g. get_Rpu_address() which is included form ../Uart/id.h). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated), but the ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

Commands and their arguments follow.


## /0/id? \[name|desc|avr-gcc\]

identify 

``` 
/1/id?
{"id":{"name":"AmpHr","desc":"RPUlux (17323^0) Board /w atmega328p","avr-gcc":"5.4.0"}}
```

##  /0/charge?

Report the discharge since the day-night state machine cleared the accumulation.

``` 
/1/charge?
{"CHRG_mAHr":"0.00","ACCUM_Sec":"205.20"}

/1/analog? 6,7
{"PWR_I":"0.005","PWR_V":"13.62"}
```

Note: The sense resistor is 0.018 Ohm on the ^0 board I am using, it needs to be changed to 0.068 Ohm.


## \[/0/day?\](../DayNight#0day)


## \[/0/analog? 0..7\[,0..7\[,0..7\[,0..7\[,0..7\]\]\]\]\](../Adc#0analog-0707070707)
