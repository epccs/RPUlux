/*
pwm is part of Capture, it uses a command line arguemnt to set pwm duty (OCR2A), 
Copyright (C) 2016 Ronald Sutherland

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

For a copy of the GNU General Public License use
http://www.gnu.org/licenses/gpl-2.0.html
*/
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h> 
#include "../lib/parse.h"
#include "pwm.h"

/* use ../lib/timers.c to initTimers().
   Timer0 is in Fast PWM Mode and will count from 0x00 to 0xFF then start at 0x00 again, it runs twice as fast as
   Timer1 and Timer2 which are in Phase-correct PWM mode where the timer counts up from 0 to 255 then counts down from 255 to 0. 
   The duty can be controled from 0 to 255 by setting the OCR2A or OCR2B register.
   And the output frequency is 16 MHz / 64 / 255 / 2 = 490.196Hz   */
void Pwm(void)
{
    int duty;
    
    if (arg_count == 2)
    {
        duty = atoi(arg[1]);
        if ( (command_done == 10) && (duty >=0) && (duty <= 255) )
        {
            if (strcmp_P( arg[0], PSTR("oc0a")) == 0)
            {
                // set Data Direction Register (its what pinMode(11, OUTPUT) does)
#if defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega168__) || defined(__AVR_ATmega48P__) || \
    defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328P__) 
                // if ( !(DDRD & (1<<PD6)) ) DDRD |= (1<<PD6); // use pMod in Digital to set DDR bit

                // connect PD6 pin to OC0A output (pwm) from timer 0 channel A, Clear on Compare Output or Fast PWM mode.
                if ( !(TCCR0A & (1<<COM0A1)) ) 
                {
                    TCCR0A |= (1<<COM0A1);
                }
#elif defined(__AVR_ATmega164P__) || defined(__AVR_ATmega324P__) \
    || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
                // if ( !(DDRB & (1<<PB3)) ) DDRB |= (1<<PB3);

                // connect PB3 pin to OC0A output (pwm) from timer 0 channel A, Clear on Compare Output or Fast PWM mode.
                if ( !(TCCR0A & (1<<COM0A1)) ) 
                {
                    TCCR0A |= (1<<COM0A1);
                }
#else
#   error I do not know where OC0A is on your MCU, check its Datasheet and then fix this file
#endif
                
#if defined (OCR0A)
                OCR0A = (uint8_t)(duty & 0xFF);
#else
#   error your MCU does not have the Output Compare Register OCR0A
#endif
                
                printf_P(PSTR("{\"pwm\":{\"OCR0A\":\"%d\"}}\r\n"),OCR0A);
                initCommandBuffer();
            }
            else if (strcmp_P( arg[0], PSTR("oc0b")) == 0)
            {
                // set Data Direction Register (its what pinMode(3, OUTPUT) does more or less does)
#if defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega168__) || defined(__AVR_ATmega48P__) || \
    defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328P__) 
                // if ( !(DDRD & (1<<PD5)) ) DDRD |= (1<<PD5);

                // connect PD5 pin to OC0B output (pwm) from timer 0 channel B, Clear on Compare Output or Fast PWM mode
                if ( !(TCCR0A & (1<<COM0B1)) ) 
                {
                    TCCR0A |= (1<<COM0B1);
                }
#elif defined(__AVR_ATmega164P__) || defined(__AVR_ATmega324P__) \
    || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
                // if ( !(DDRD & (1<<PB4)) ) DDRD |= (1<<PB4);

                // connect PB4 pin to OC0B output (pwm) from timer 0 channel B, Clear on Compare Output or Fast PWM mode
                if ( !(TCCR0A & (1<<COM0B1)) ) 
                {
                    TCCR0A |= (1<<COM0B1);
                }
#else
#   error I do not know where OC0B is on your MCU, check its Datasheet and then fix this file
#endif
                
#if defined (OCR0B)
                OCR0B = (uint8_t)(duty & 0xFF);
#else
#   error your MCU does not have the Output Compare Register OCR0B
#endif
                printf_P(PSTR("{\"pwm\":{\"OCR0B\":\"%d\"}}\r\n"),OCR0B);
                initCommandBuffer();
            }
            else if (strcmp_P( arg[0], PSTR("oc1a")) == 0)
            {
                // set Data Direction Register (its what pinMode(11, OUTPUT) does)
#if defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega168__) || defined(__AVR_ATmega48P__) || \
    defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328P__) 
                // if ( !(DDRB & (1<<PB1)) ) DDRB |= (1<<PB1);

                // connect PB1 pin to OC1A output (pwm) from timer 1 channel A, Clear on Compare Output or Fast PWM mode.
                if ( !(TCCR1A & (1<<COM1A1)) ) 
                {
                    TCCR1A |= (1<<COM1A1);
                }
#elif defined(__AVR_ATmega164P__) || defined(__AVR_ATmega324P__) \
    || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
                // if ( !(DDRD & (1<<PD5)) ) DDRD |= (1<<PD5);

                // connect PD5 pin to OC1A output (pwm) from timer 1 channel A, Clear on Compare Output or Fast PWM mode.
                if ( !(TCCR1A & (1<<COM1A1)) ) 
                {
                    TCCR1A |= (1<<COM1A1);
                }
#else
#   error I do not know where OC1A is on your MCU, check its Datasheet and then fix this file
#endif
                
#if defined (OCR1A)
                OCR1A = (uint8_t)(duty & 0xFF);
#else
#   error your MCU does not have the Output Compare Register OCR1A
#endif
                
                printf_P(PSTR("{\"pwm\":{\"OCR1A\":\"%d\"}}\r\n"),OCR1A);
                initCommandBuffer();
            }
            else if (strcmp_P( arg[0], PSTR("oc1b")) == 0)
            {
#if defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega168__) || defined(__AVR_ATmega48P__) || \
    defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328P__) 
                // if ( !(DDRB & (1<<PB2)) ) DDRB |= (1<<PB2);

                // connect PDB2pin to OC1B output (pwm) from timer 1 channel B, Clear on Compare Output or Fast PWM mode
                if ( !(TCCR0A & (1<<COM0B1)) ) 
                {
                    TCCR0A |= (1<<COM0B1);
                }
#elif defined(__AVR_ATmega164P__) || defined(__AVR_ATmega324P__) \
    || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
                // if ( !(DDRD & (1<<PD4)) ) DDRD |= (1<<PD4);

                // connect PD4 pin to OC1B output (pwm) from timer 1 channel B, Clear on Compare Output or Fast PWM mode
                if ( !(TCCR1A & (1<<COM1B1)) ) 
                {
                    TCCR1A |= (1<<COM1B1);
                }
#else
#   error I do not know where OC1B is on your MCU, check its Datasheet and then fix this file
#endif
                
#if defined (OCR1B)
                OCR1B = (uint8_t)(duty & 0xFF);
#else
#   error your MCU does not have the Output Compare Register OCR1B
#endif
                printf_P(PSTR("{\"pwm\":{\"OCR1B\":\"%d\"}}\r\n"),OCR1B);
                initCommandBuffer();
            }
            else if (strcmp_P( arg[0], PSTR("oc2a")) == 0)
            {
                // set Data Direction Register (its what pinMode(11, OUTPUT) does)
#if defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega168__) || defined(__AVR_ATmega48P__) || \
    defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328P__) 
                // if ( !(DDRB & (1<<PB3)) ) DDRB |= (1<<PB3);

                // connect PB3 pin to OC2A output (pwm) from timer 2 channel A, Clear on Compare Output or Fast PWM mode.
                if ( !(TCCR2A & (1<<COM2A1)) ) 
                {
                    TCCR2A |= (1<<COM2A1);
                }
#elif defined(__AVR_ATmega164P__) || defined(__AVR_ATmega324P__) \
    || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
                // if ( !(DDRD & (1<<PD7)) ) DDRD |= (1<<PD7);

                // connect PD7 pin to OC2A output (pwm) from timer 2 channel A, Clear on Compare Output or Fast PWM mode.
                if ( !(TCCR2A & (1<<COM2A1)) ) 
                {
                    TCCR2A |= (1<<COM2A1);
                }
#else
#   error I do not know where OC2A is on your MCU, check its Datasheet and then fix this file
#endif
                
#if defined (OCR2A)
                OCR2A = (uint8_t)(duty & 0xFF);
#else
#   error your MCU does not have the Output Compare Register OCR2A
#endif
                
                printf_P(PSTR("{\"pwm\":{\"OCR2A\":\"%d\"}}\r\n"),OCR2A);
                initCommandBuffer();
            }
            else if (strcmp_P( arg[0], PSTR("oc2b")) == 0)
            {
                // set Data Direction Register (its what pinMode(3, OUTPUT) does more or less does)
#if defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega168__) || defined(__AVR_ATmega48P__) || \
    defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328P__) 
                // if ( !(DDRD & (1<<PD3)) ) DDRD |= (1<<PD3);

                // connect PD3 pin to OC2B output (pwm) from timer 2 channel B, Clear on Compare Output or Fast PWM mode
                if ( !(TCCR2A & (1<<COM2B1)) ) 
                {
                    TCCR2A |= (1<<COM2B1);
                }
#elif defined(__AVR_ATmega164P__) || defined(__AVR_ATmega324P__) \
    || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
                //Note PD6 has ICP1
                // if ( !(DDRD & (1<<PD6)) ) DDRD |= (1<<PD6);

                // connect PD6 pin to OC2B output (pwm) from timer 2 channel B, Clear on Compare Output or Fast PWM mode
                if ( !(TCCR2A & (1<<COM2B1)) ) 
                {
                    TCCR2A |= (1<<COM2B1);
                }
#else
#   error I do not know where OC2B is on your MCU, check its Datasheet and then fix this file
#endif
                
#if defined (OCR2B)
                OCR2B = (uint8_t)(duty & 0xFF);
#else
#   error your MCU does not have the Output Compare Register OCR2B
#endif
                printf_P(PSTR("{\"pwm\":{\"OCR2B\":\"%d\"}}\r\n"),OCR2B);
                initCommandBuffer();
            }
        }
        else
        {
            printf_P(PSTR("{\"err\":\"pwmRange_%d\"}\r\n"),duty);
            initCommandBuffer();
        }
    }
    else
    {
        printf_P(PSTR("{\"err\":\"pwmNeed2Arg\"}\r\n"));
        initCommandBuffer();
    }
}

