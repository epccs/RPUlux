# Description

Some lessons I learned doing RPUlux.

# Table Of Contents:

1. ^2 AVR weak pull-up and AL8805 analog CTRL
1. ^2 AL8805 CTRL Pin Driven with AVR Weak Pull Up 
1. ^2 AL8805 Calculator
1. ^2 AL8805 still running at 165mA with 0.3 Ohm sense
1. ^1 AL8805 design, mfg layout
1. ^0 SMPS design , current sense resistor
1. ^0 PWR_V divider
1. ^0 Add defaults on IO Controls
1. ^0 Pull Down Alternate Power Control
1. ^0 Solar Panel Zener brakedown failure


## ^2 AVR weak pull-up and AL8805 analog CTRL

I was having problems with my [SelfTest] where I was only seeing about half the expected current from an AL8805. 

[SelfTest]: https://github.com/epccs/RPUlux/tree/master/SelfTest

An 8.45k Ohm pull-down was used to turn off the LED's during boot loading and when the AVR is in RESET. Form the AL8805 CTRL perspective the pull-down sums a divided amount from the AVR 5V through a 35k Ohm (weak pull-up mode) and from the 2.5V reference on AL8805 through 50k Ohm.

![8k45PullDown](./WeakPullUpWith8k45pull-down.png "8k45 Pull-Down")

With a 3.74k Ohm pull-down and a weak pull-up from the AVR the switch mode converter runs with an analog setting in the range 50mA to 150mA (approximate). 

![3k74PullDown](./WeakPullUpWith3k74pull-down.png "3k74 Pull-Down")

When the AVR pin is in the push-pull mode and set HIGH the constant current converter provides the expected amount of current. The weak pull-up mode from the AVR set an analog voltage on the AL8805 CTRL pin that allows it to run at a reduced current without any audible switching noise (e.g. the 500Hz).

```
...
Terminal ready
RPUlux Self Test date: Mar 27 2018
avr-gcc --version: 5.4.0
I2C provided address 0x31 from serial bus manager
adc reading for PWR_V: 359
PWR at: 12.812 V
ADC0 GN LED /W SINK on and CS*_EN off: 0.000 V
ADC1 RD LED /W SINK on and CS*_EN off: 0.000 V
ADC2 R1 /W CS*_EN off: 0.000 V
ADC3 R1 /W CS*_EN off: 0.000 V
CS0 curr source on R1: 0.022 A
Green LED fwd V: 2.245 V
CS1 curr source on R1: 0.022 A
Red LED fwd V: 2.133 V
   ADC2 reading used to calculate ref_intern_1v1_uV: 696 A
   calculated ref_intern_1v1_uV: 1088918 uV
REF_EXTERN_AVCC old value was in eeprom: 4948800 uV
REF_INTERN_1V1 old value was in eeprom: 1078310 uV
REF_EXTERN_AVCC saved in eeprom: 4986100 uV
REF_INTERN_1V1 saved in eeprom: 1088918 uV
PWR_I with !CS1_EN use INTERNAL_1V1: 0.013 A
PWR_I with CH1 LED, 1V1, 1sec: 0.121 A
PWR_I with CH1 LED, 1V1, 3sec: 0.121 A
Approximate CH1 curr on a 3.2V LED: 0.344 A
Approximate CH1 curr /w weak pull-up on a 3.2V LED: 0.091 A
PWR_I with CH2 LED, 1V1, 1sec: 0.119 A
PWR_I with CH2 LED, 1V1, 3sec: 0.119 A
Approximate CH2 curr on a 3.2V LED: 0.337 A
Approximate CH2 curr /w weak pull-up on a 3.2V LED: 0.091 A
PWR_I with CH3 LED, 1V1, 1sec: 0.117 A
PWR_I with CH3 LED, 1V1, 3sec: 0.117 A
Approximate CH3 curr on a 3.2V LED: 0.331 A
Approximate CH3 curr /w weak pull-up on a 3.2V LED: 0.088 A
PWR_I with CH4 LED, 1V1, 1sec: 0.119 A
PWR_I with CH4 LED, 1V1, 3sec: 0.118 A
Approximate CH4 curr on a 3.2V LED: 0.336 A
Approximate CH4 curr /w weak pull-up on a 3.2V LED: 0.090 A
PWR_I with CH5 LED, 1V1, 1sec: 0.117 A
PWR_I with CH5 LED, 1V1, 3sec: 0.117 A
Approximate CH5 curr on a 3.2V LED: 0.332 A
Approximate CH5 curr /w weak pull-up on a 3.2V LED: 0.088 A
PWR_I with CH6 LED, 1V1, 1sec: 0.120 A
PWR_I with CH6 LED, 1V1, 3sec: 0.120 A
Approximate CH6 curr on a 3.2V LED: 0.342 A
Approximate CH6 curr /w weak pull-up on a 3.2V LED: 0.092 A
[PASS]
```


## ^2 AL8805 CTRL Pin Driven with AVR Weak Pull Up 

During testing, I have been using the SelfTest program, while also looking at bench instruments. It turns out the reason I was seeing 165mA from the AL8805 was that I did not set the AVR pin into a push-pull mode (e.g. pinMode(CH1,OUTPUT) ), as a result when the pin was set HIGH it enables a weak pull-up, which combines an 8.45k Ohm pull down with the AL8805 reference (2.5V) and 50k Ohm internal resistance. The result is about 1.2V on the AL8805 CTRL pin which sets the current about half the value I was expecting. 

After changing the SelfTest so AVR pins are used in the push-pull mode the LED channels now have the expected current.

```
...
Terminal ready
RPUlux Self Test date: Mar 21 2018
avr-gcc --version: 5.4.0
I2C provided address 0x31 from serial bus manager
adc reading for PWR_V: 361
PWR at: 12.883 V
ADC0 GN LED /W SINK on and CS*_EN off: 0.000 V
ADC1 RD LED /W SINK on and CS*_EN off: 0.000 V
ADC2 R1 /W CS*_EN off: 0.000 V
ADC3 R1 /W CS*_EN off: 0.000 V
CS0 curr source on R1: 0.022 A
Green LED fwd V: 2.245 V
CS1 curr source on R1: 0.022 A
Red LED fwd V: 2.128 V
   ADC2 reading used to calculate ref_intern_1v1_uV: 692 A
   calculated ref_intern_1v1_uV: 1088007 uV
REF_EXTERN_AVCC old value was in eeprom: 4948800 uV
REF_INTERN_1V1 old value was in eeprom: 1078310 uV
REF_EXTERN_AVCC saved in eeprom: 4986100 uV
REF_INTERN_1V1 saved in eeprom: 1088007 uV
PWR_I with !CS1_EN use INTERNAL_1V1: 0.013 A
PWR_I with CH1 LED, 1V1, 1sec: 0.121 A
PWR_I with CH1 LED, 1V1, 3sec: 0.120 A
Approximate CH1 curr on a 3.2V LED: 0.343 A
PWR_I with CH2 LED, 1V1, 1sec: 0.119 A
PWR_I with CH2 LED, 1V1, 3sec: 0.118 A
Approximate CH2 curr on a 3.2V LED: 0.336 A
PWR_I with CH3 LED, 1V1, 1sec: 0.116 A
PWR_I with CH3 LED, 1V1, 3sec: 0.116 A
Approximate CH3 curr on a 3.2V LED: 0.330 A
PWR_I with CH4 LED, 1V1, 1sec: 0.118 A
PWR_I with CH4 LED, 1V1, 3sec: 0.118 A
Approximate CH4 curr on a 3.2V LED: 0.335 A
PWR_I with CH5 LED, 1V1, 1sec: 0.117 A
PWR_I with CH5 LED, 1V1, 3sec: 0.116 A
Approximate CH5 curr on a 3.2V LED: 0.331 A
PWR_I with CH6 LED, 1V1, 1sec: 0.121 A
PWR_I with CH6 LED, 1V1, 3sec: 0.120 A
Approximate CH6 curr on a 3.2V LED: 0.344 A
[PASS]
```

## ^2 AL8805 Calculator

Diodes Inc has a [Calculator] for the circuit (note I ran it on LibreOffice 6.0.2 so don't let the xls stop you) 

[Calculator]: https://www.diodes.com/assets/Calculators/WEB-AL8805-calculator-v1.1.xls

I want about 330mA. I am using a 33uH coil. 

![AL8805Calculator4RPUlux](./AL8805cal4RPUlux.png "AL8805 Calculator for RPUlux")


## ^2 AL8805 running at 165mA with 0.3 Ohm sense

The AL8805 layout overhaul on my RPUlux^2 board did not change anything. I feel like a punching bag. 

Adding 0.1uF bypass next to the AL8805 did not change anything noticabley.

![AL8805W5_bypass](./RPUlux^2_bypassTest.jpg "Bypass Next to AL8805")

![AL8805W5_bypass_ref](./RPUlux^2_bypassTestRef.png "Bypass Next to AL8805 Ref")

## ^1  AL8805 design, mfg layout

My [RPUlux] version ^1 [layout] has less noise on the 0V plane than the first version. The ADC readings from the MCU look right (I sort of figured those were noise related so I ignored that problem).

[RPUlux]: https://github.com/epccs/RPUlux/
[layout]: https://github.com/epccs/Eagle/tree/master/RPUlux

The problem is I am still getting 160mA with .3 Ohm sense from the LED driver (AL8805). Looking at the Diodes example circuit when the switch is off. I see the current flows through D1 and into C6 mostly (some goes to C3 bypass). 

![AL8805W5_layout_sw_off](./AL8805W5_layout_sw_off.png "AL8805W5 Example Layout Swith Off")

However when the switch is on the current flow is right under the sense.

![AL8805W5_layout_sw_on](./AL8805W5_layout_sw_on.png "AL8805W5 Example Layout Swith On")

Note ^2 does this but average LED current is still about half the expected value.

Replacing the sense resistor with 0.15 Ohm sets the current at about 330mA. On my ^1 layout the switch current flows out to a 4.7uF. 

![RPUlux^1_layout_sw_on](./RPUlux^1_layout_sw_on.png "RPUlux^1 Example Layout Swith On")

When the switch is off the current flows through the diode.

![RPUlux^1_layout_sw_off](./RPUlux^1_layout_sw_off.png "RPUlux^1 Example Layout Swith Off")

Leo (aka Wawa on the Arduino [Forum]) has shown what I think is a better solution. He is using a PT4115, which is about the same as AL8805.  The switching current looks to run under the sense resistor like in the datasheet, and since there is a possibility it is used for stability (like slope compensation) I better do that also.

[Forum]: https://forum.arduino.cc/index.php?topic=530545.0

![Leo's_Example_Layout](./PT4115_on_board_by_Leo.png "Leo's Example Layout")


## ^0  SMPS design, current sense resistor

Every control chip (AL8805) is a teacher. I am seeing about half the current (160mA) that I expect to see with a 0.3 Ohm sense resistor. 

After looking at the Sparkfun Femtobuck I think I see some ideas to try. 

This converter switches the current to 0V, but I need to send the return current into the 10 uF capacitors before it is and then connected to 0V. Because the switching current is on the ground plane I think it is causing the control chip switch on noise events rather than on the sense resistor ramp.

I also did not connect the sense resistor to the AL8805 VIN pin directly and that is causing some error.

The high side current sense is reading about half of what it should durrin SelfTest (DMM reads 60mA), and that is less (12.8V*.06A = .77W) than expected for an LED that should have 1W (e.g. 350mA*3.2V).

```
RPUlux Self Test date: Jan 21 2018
avr-gcc --version: 5.4.0
I2C provided address 0x31 from serial bus manager
adc reading for PWR_V: 359
PWR at: 12.864 V
ADC0 GN LED /W SINK on and CS*_EN off: 0.000 V
ADC1 RD LED /W SINK on and CS*_EN off: 0.000 V
ADC2 R1 /W CS*_EN off: 0.000 V
ADC3 R1 /W CS*_EN off: 0.000 V
CS0 curr source on R1: 0.022 A
Green LED fwd V: 2.239 V
CS1 curr source on R1: 0.023 A
Red LED fwd V: 2.132 V
   ADC2 reading used to calculate ref_intern_1v1_uV: 722 A
   calculated ref_intern_1v1_uV: 1074802 uV
REF_EXTERN_AVCC old value was in eeprom: 5006500 uV
REF_INTERN_1V1 old value was in eeprom: 1073325 uV
REF_EXTERN_AVCC saved in eeprom: 5006500 uV
REF_INTERN_1V1 saved in eeprom: 1074802 uV
PWR_I with CS1_EN and INTERNAL_1V1: 0.013 A
PWR_I with CH1 LED, 1V1, 1sec: 0.031 A
PWR_I with CH1 LED, 1V1, 3sec: 0.031 A
PWR_I with CH2 LED, 1V1, 1sec: 0.030 A
PWR_I with CH2 LED, 1V1, 3sec: 0.030 A
PWR_I with CH3 LED, 1V1, 1sec: 0.031 A
PWR_I with CH3 LED, 1V1, 3sec: 0.030 A
PWR_I with CH4 LED, 1V1, 1sec: 0.030 A
PWR_I with CH4 LED, 1V1, 3sec: 0.030 A
PWR_I with CH5 LED, 1V1, 1sec: 0.030 A
PWR_I with CH5 LED, 1V1, 3sec: 0.030 A
PWR_I with CH6 LED, 1V1, 1sec: 0.029 A
PWR_I with CH6 LED, 1V1, 3sec: 0.030 A
[PASS]
```

PWR_I reads 30mA while a DMM reads 60mA input current. The reading may be noise related, in any case noise improvements need done first.

I think the main issue is that the AL8805 switch current is sent right onto the 0V ground plane. Any place on the plane that the current attempts to flow develop a magnetic field that opposes the flow of that current. This means the current has to flow on the edge of the plane. Following the edge, outline shows that the magnetic fields from the switched current will make a mess all over the board.

![CircuitWithAL8805^0](./CircuitWithAL8805^0.png "Circuit With AL8805 ^0")

Moving things around some seems to give a little more room, so now I can swap the .1uF bypass for a 4.7uF. This also moves the things that are switching current flow (the Diode and the AL8805) to one side and the things with less noise to the other side of the AL8805 with its feedback inputs. The AL8805 0V pin (where the switched current is sent) is connected to an island poor that is connected to the bypass. This means most of the switched current is confined to the edges of this island plane and never goes through the via to the boards larger 0V plane.

![CircuitWithAL8805^1](./CircuitWithAL8805^1.png "Circuit With AL8805 ^1")


## ^0  PWR_V divider

R4 changed from 1.5k to 15.8k Ohm. This is actually the original value I had on the last version of RPUno that was assembled. I decided to try the 1.5k since it would work well
with the 1.1V bandgap referance that I am now calibrating. Unfortuanly a 1% 100k in series with a 1.5k Ohm makes a divider that has a relationship problem. The uncertantly of the 100k is +/-1k and that is nearly the size of the sense resistor. The 100k should be 0.1% if I want to divide it down like that.


## ^0  Add defaults on IO Controls

Controls should have a known defaut so they will do known things when bootloading, powering up, or if the user does not set them.

IO8 needs a 10k pull down to be defined as off by default. Also add 100k to Q1 zener for leakage current.

IO12 (CS0_EN or MISO) and IO13 (CS1_EN or SCK) needs a 10k pull down to be defined as off by default.

IO4 and IO7 needs a 10k pull down so the NMOS SINK is defined as off by default.

IO3, IO5, IO6, IO9, nSS, and MOSI are used for PWM Control and they should to be off by default.

The AL8805 CTRL pin has a 50kOhm resistance to its internal reference (e.g. about 2.5V) and it needs to be brought down to .4V to turn off the converter, which can be done with anything less than 9.5k Ohm. Note 1k Ohm would use 2.5mA at rest and 5mA when turned on I want to use a higher value to reduce the loss, I have 8.45k Ohm on hand. 

![PullDownPWMctrl](./PullDownPWMctrl.png "Pull Down PWM CNRL")

## ^0  Pull Down Alternate Power Control

IO8 is used for Alternate Power Control, it needs to be off by default (e.g. bootloading, or if the user does not set it).

## ^0  Solar Panel Zener brakedown failure

Shorting a solar panel without by-pass diodes will cause more than a 10V reverse voltage on a partly shaded cell. That cell will then experience Zener breakdown failure.

![Zener](./SolarReverseBreakdownIssue.png "Zener Brakedown")
