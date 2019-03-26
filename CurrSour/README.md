# Current Source

## Overview

There are two 22mA current sources CS0..CS1 controlled with ATmega328p pins PB4, PB5.


# Firmware Upload

With a serial port connection (see BOOTLOAD_PORT in Makefile) and optiboot installed on the RPUlux run 'make bootload' and it should compile and then flash the MCU.

``` 
sudo apt-get install make git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/RPUlux/
cd /RPUlux/CurrSour
make bootload
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk).


``` 
#exit is C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 


# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 


## /\[rpu_address\]/\[command \[arg\]\]

rpu_address is taken from the I2C address 0x29 (e.g. get_Rpu_address() which is included form ../lib/rpu_mgr.h). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated) as an adddress. The ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

Commands and their arguments follow.


## /0/id? \[name|desc|avr-gcc\]

identify 

``` 
/1/id?
{"id":{"name":"CurrSour","desc":"RPUlux (17323^2) Board /w atmega328p","avr-gcc":"5.4.0"}}
```

##  /0/cs 0..1,ON|OFF

Turn on (or off) 22mA current sources CS0..CS3

``` 
/1/cs 1,ON
{"PB5":"ON"}
/1/cs 1,OFF
{"PB5":"OFF"}
```


##  /0/showstat ON|OFF

A current source can be used to show status, but it is not ON by default.  

``` 
/1/showstat ON
{"SHOW":"ON"}
/1/showstat OFF
{"SHOW":"OFF"}
```