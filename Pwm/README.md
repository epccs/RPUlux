# Pwm 

## Overview

Timer**n** **n** = 0, 1, 2 each have two output compare pins OC**n**A and OC**n**B that can generate PWM waveform. 

Timer0 is set for Fast PWM Mode see ATmega328 datasheet 15.73 (page 99)

Timer1 and Timer2 are set in Phase Correct PWM Mode see ATmega328 datasheet 15.7.4 (page 100)

Timer0 prescaler is 64 and it counts up to 256 which gives a frequency of 976.56Hz.

Timer1 and Timer2 have a prescaler of 64 and count up to 255 and then down to zero which gives a frequency of 490.2Hz

Using phase-correct PWM on Timer0 means that it overflows half as often resulting in different millis() behavior than is currently in place. Unfortunatly the LED drivers are only rated for 500Hz PWM frequency so this will need fixed.


## Firmware Upload

With a serial port connection (set the BOOT_PORT in Makefile) and optiboot installed on the RPUno run 'make bootload' and it should compile and then flash the MCU.

``` 
git clone https://github.com/epccs/RPUlux/
cd /RPUux/Pwm
make bootload
...
avrdude done.  Thank you.
``` 

Now connect with picocom (or ilk). Note I am often at another computer doing this through SSH. The Samba folder is for editing the files from Windows.

``` 
#exit picocom with C-a, C-x
picocom -b 38400 /dev/ttyUSB0
``` 

or log the terminal session

``` 
script -f -c "picocom -b 38400 /dev/ttyUSB0" stuff,capture.log
``` 


# Commands

Commands are interactive over the serial interface at 38400 baud rate. The echo will start after the second character of a new line. 

## /0/id? [name|desc|avr-gcc]

identify 

``` 
/1/id?
{"id":{"name":"Capture","desc":"RPUno (14140^7) Board /w atmega328p","avr-gcc":"4.9"}}
```

## /0/pwm CH1..CH6,0..255

Pulse width modulation using Compare Match with Fast or Phase-Correct PWM mode. 

``` 
/0/pwm CH6,127
{"pwm":{"CH6":"127"}}
``` 


