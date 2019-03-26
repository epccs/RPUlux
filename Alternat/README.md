# Alternate Power Input

## Overview

Alternate power input is used to send current from a solar panel into a battery. 

The alternate power source needs to act as a current source, and the main power source needs to act like a battery. Do not attempt this with a bench power supply as the main power input. When the alternate power is enabled it must current limit and then charge the battery.

The DayNight state machine is used, it has two events that run a registered callback function. The day event turns on the alternate power input, while the night event turns it off. Adc channels are measured with a burst of interrupts that occurs every 100 millis. Near the time of the Adc burst, the mux channel PWR_V which is connected to an input voltage divider (a 1% 15.8k and a 0.1% 100k) is checked. This is more or less the battery voltage so can be used to turn off the Alternate power input when the battery has reached a limit, and then turn it back on when the voltage drops.

The hardware for this lacks training wheels, it is going to take some time to refine these ideas. The target battery is lead acid, this method fails with other types. 


## Wiring Needed

![Wiring](./Setup/AuxilaryWiring.png)


# Firmware Upload

With a serial port connection (see BOOTLOAD_PORT in Makefile) and optiboot installed on the RPUlux run 'make bootload' and it should compile and then flash the MCU.

``` 
sudo apt-get install make git gcc-avr binutils-avr gdb-avr avr-libc avrdude
git clone https://github.com/epccs/RPUlux/
cd /RPUux/Alternat
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
{"id":{"name":"Alternat","desc":"RPUlux (17323^0) Board /w atmega328p","avr-gcc":"5.4.0"}}
```

##  /0/trackalt?

This will turn on the Alternat Power after a delay (22.5 sec) and log the data to the serial output until the host sends a character.

``` 
/1/trackalt?
{"millis":"0","ADC3":"14.69""PWR_I":"0.000""PWR_V":"13.01"}
{"millis":"5000","ADC3":"14.69""PWR_I":"0.000""PWR_V":"13.01"}
{"millis":"10000","ADC3":"14.69""PWR_I":"0.000""PWR_V":"13.01"}
{"millis":"15000","ADC3":"14.69""PWR_I":"0.000""PWR_V":"13.01"}
{"millis":"20000","ADC3":"14.69""PWR_I":"0.000""PWR_V":"13.01"}
{"millis":"25000","ADC3":"13.51""PWR_I":"0.000""PWR_V":"13.08"}
{"millis":"30000","ADC3":"13.54""PWR_I":"0.000""PWR_V":"13.11"}
{"millis":"35000","ADC3":"13.54""PWR_I":"0.000""PWR_V":"13.11"}
{"millis":"40000","ADC3":"13.58""PWR_I":"0.000""PWR_V":"13.15"}
{"millis":"45000","ADC3":"13.58""PWR_I":"0.000""PWR_V":"13.15"}
{"millis":"50000","ADC3":"13.62""PWR_I":"0.000""PWR_V":"13.15"}
{"millis":"55000","ADC3":"13.62""PWR_I":"0.000""PWR_V":"13.19"}
```


