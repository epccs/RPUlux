# Amp-Hour Counter

## Overview

Accumulate ADC readings at timed intervals from a resistor sensor. This is an interactive command line program that demonstrates how the ATmega328p can be used to keep track of charge (or discharge). 

[Adc]: ../Adc


# Start of Day 

The [day-night][../DayNight] state machine is used to clear the discharge accumulation at TBD.


# Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUlux run 'make bootload' and it should compile and then flash the MCU.

``` 
git clone https://github.com/epccs/RPUlux/
cd /RPUux/AmpHr
make bootload
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
{"PWR_I":"0.153","PWR_V":"13.28"}
```

## \[/0/day?\](../DayNight#0day)


## \[/0/analog? 0..7\[,0..7\[,0..7\[,0..7\[,0..7\]\]\]\]\](../Adc#0analog-0707070707)


## \[/0/pwm CH1..CH6,0..255\](../Pwm#0pwm-ch1ch60255)


##  \[/0/pMod 14..16,INPUT|OUTPUT\](../Digital#0pmod-1416inputoutput)


##  \[/0/dWrt 14..16,HIGH|LOW\](../Digital#0dwrt-1416highlow)


##  \[/0/dTog 14..16\](../Digital#0dtog-1416)


##  \[/0/dRe? 14..16\](../Digital#0dre-1416)


