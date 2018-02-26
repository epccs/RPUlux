# Digital Input/Output

## Overview

Digital is an interactive command line program that demonstrates control of the Digital input/output from ATmega328p pins PD3, PD5, PD6, PB1, PB2, and PB3 (3,5,6,9,10,11). These I/O's control the LED drivers.

## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
git clone https://github.com/epccs/RPUlux/
cd /RPUux/Digital
make bootload
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.

``` 
#exit is C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 


# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 


## /[rpu_address]/[command [arg]]

rpu_address is taken from the I2C address 0x29 (e.g. ../Uart/id.h get_Rpu_address() ). The value of rpu_address is used as a character in a string, which means don't use a null value (C strings are null terminated), but the ASCII value for '1' (0x31) is easy and looks nice, though I fear it will cause some confusion when it is discovered that the actual address value is 49.

The LED_BUILTIN is bliked fast (twice per second) if the I2C address is not found, also the rpu_address defaults to '0'. 

Commands and their arguments follow.


## /0/id? [name|desc|avr-gcc]

Identify is from ../Uart/id.h Id().

``` 
/1/id?
{"id":{"name":"Digital","desc":"RPUlux (17323^0) Board /w atmega328p","avr-gcc":"5.4.0"}}
```

##  /0/pMod 3|5|6|9|10|11,INPUT|OUTPUT    

Set the Data Direction Register (DDRx) bit that sets the pin as INPUT or OUTPUT.

``` 
/1/pMod 3,OUTPUT
{"PD3":"OUTPUT"}
/1/pMod 10,INPUT
{"PB2":"INPUT"}
```


##  /0/dWrt 3|5|6|9|10|11,HIGH|LOW    

Set the Port Data Register (PORTx) bit that drives the pin or if mode (e.g. Port Input Register bit) is set as an INPUT enables a pullup. Returns the Port Input Register PINx bit (e.g. same as read command)

```
/1/dWrt 3,LOW
{"PD3":"LOW"}
/1/dWrt 10,HIGH
{"PB2":"LOW"}
```

on RPUlux PB2 has a 8.45k pull-down so the ATmega328p pullup is not a logic HIGH, but it may set the analog voltage on the LED driver high enough so that it drives some current.


##  /0/dTog 3|5|6|9|10|11  

Toggle the Port Data Register (PORTx) bit if the Data Direction Register (DDRx) bit is set as an OUTPUT. Returns the Port Input Register PINx bit (e.g. same as read command)

``` 
/1/dTog 3
{"PD3":"HIGH"}
/1/dTog 3
{"PD3":"LOW"}
/1/dTog 3
{"PD3":"HIGH"}
```


##  /0/dRe? 3|5|6|9|10|11 

Read the Port Input Register (PINx) bit that was latched during last low edge of the system clock.

``` 
/1/dRe? 3
{"PD3":"HIGH"}
/1/dRe? 10
{"PB2":"LOW"}
```

