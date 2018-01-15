/*
 * Pin definitions for RPUlux used with pin_num.h Digital IO library
 * Copyright (C) 2016 Ronald Sutherland
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the Arduino DigitalIO Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef Pins_Board_h
#define Pins_Board_h

#define NUM_DIGITAL_PINS            20
#define NUM_ANALOG_INPUTS        8
// analogInputToDigitalPin() takes an AVR analog channel number and returns the digital pin number otherwise -1.
// #define analogInputToDigitalPin(p)  ((p < 8) ? (p) + 14 : -1)
#define digitalPinHasPWM(p)         ((p) == 3 || (p) == 5 || (p) == 6 || (p) == 9 || (p) == 10 || (p) == 11)

// UART is for serial communication (never use these pins)
#define RX0 0 
#define TX0 1

// VIN  pin shield power control
#define SHLD_VIN_EN 2

// LED Channel
#define CH1 3

// NMOS SINK on ADC0 plugable
#define ADC0_SINK 4

// LED Channel
#define CH2 5
#define CH3 6

// NMOS SINK on ADC1 plugable
#define ADC1_SINK 7

// NMOS SINK on ADC2 plugable
#define ADC2_SINK 8

// LED Channel
#define CH4 9
#define CH5 10
#define CH6 11

// Current Source Enable
#define CS0_EN 12
#define CS1_EN 13
#define LED_BUILTIN 13 

// I2C
#define SDA 18
#define SCL 19

// ADC channels
// Values range from 0 to 1023 for 1024 slots which each reperesents 1/1024 of the reference. The last slot has some issues
// https://forum.arduino.cc/index.php?topic=303189.0 

// ADC0 has a Plugable input with a 20mA current source
// its voltage is analogRead(ADC0)*(5.0/1024.0)
#define ADC0 0

// ADC1 has a Plugable input with a 20mA current source
// its voltage is analogRead(ADC1)*(5.0/1024.0)
#define ADC1 1

#define ADC2 2
#define ADC3 3

// ADC4 and ADC5 are used for I2C with the RPUadpt/RPUftdi/RPUpi shields
#define ADC4 4
#define ADC5 5

// ADC6 voltage is analogRead(PV_V)*(5.0/1024.0)*(5.0/1024.0)/(0.068*50.0)
#define PWR_I 6 

// ADC7 or input voltage is analogRead(PWR_V)*(5.0/1024.0)*(115.8.0/15.8)
#define PWR_V 7

#endif // Pins_Board_h
