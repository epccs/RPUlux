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
 * along with the DigitalIO Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef Pins_Board_h
#define Pins_Board_h

#define NUM_DIGITAL_PINS            20
#define NUM_ANALOG_INPUTS        8

#define digitalPinHasPWM(p)         ((p) == 3 || (p) == 5 || (p) == 6 || (p) == 9 || (p) == 10 || (p) == 11)

// UART is for serial communication (never use these pins)
#define RX 0 
#define TX 1

// VIN  pin shield power control
#define SHLD_VIN_EN 2

// LED Channel
#define CH1 3

// NMOS SINK on ADC0 plugable
#define ADC0_SINK 4

// LED Channel
#define CH2 5
#define CH3 6

// NMOS SINK on plugable connector
#define ADC1_SINK 7

// ALTernat power ENable
#define ALT_EN 8

// LED Channel
#define CH4 9
#define CH5 10
#define CH6 11

// Current Source Enable
#define CS0_EN 12
#define CS1_EN 13
#define LED_BUILTIN 13 

//ADC0 thur ADC2 can also be used for digital IO from the plugable connector
#define DIO14 14
#define DIO15 15
#define DIO16 16
// DIO17 does not go to a plugable connector

// I2C
#define SDA 18
#define SCL 19

// ADC channels
// Values range from 0 to 1023 for 1024 slots which each reperesents 1/1024 of the reference. The last slot has some issues
// https://forum.arduino.cc/index.php?topic=303189.0 

// referance include 
//          AVCC which is the SMPS. It is about 5V but changes somewhat with temperature and input voltage.
//          1V1 internal badgap, which is measured during a selftest and placed in the eeprom.

// ADC0 is at the same pin that was defined as digital 14, it could have been defined as digital 0 but this matches the Uno numbers
// its voltage is analogRead(ADC0)*(<referance>/1024.0)
#define ADC0 0

// ADC1 is at the same pin that was defined as digital 15
// its voltage is analogRead(ADC1)*(<referance>/1024.0)
#define ADC1 1

// ADC2 is at the same pin that was defined as digital 16
#define ADC2 2
#define ADC3 3

// ADC4 and ADC5 are used for I2C with the RPUadpt/RPUftdi/RPUpi shields
#define ADC4 4
#define ADC5 5

// ADC6 voltage is analogRead(PWR_I)*(<referance>/1024.0)/(0.018*50.0)
#define PWR_I 6 

// ADC7 or input voltage is analogRead(PWR_V)*(<referance>/1024.0)*(101.5/1.5)
#define PWR_V 7

// note ADC6 and ADC7 do not have digital hardware on a 328p so can only be used as analog channels

#endif // Pins_Board_h
