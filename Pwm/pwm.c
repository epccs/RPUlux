/*
pwm is part of Capture, it uses a command line arguemnt to set pwm duty (OCR2A), 
Copyright (C) 2019 Ronald Sutherland

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
            if (strcmp_P( arg[0], PSTR("CH3")) == 0) // CH3 is on OC0A
            {
                // if ( !(DDRD & (1<<PD6)) ) DDRD |= (1<<PD6); // set pinMode in main, it only needs done once

                // connect PD6 pin to OC0A output (pwm) from timer 0 channel A, Clear on Compare Output or Fast PWM mode.
                if ( !(TCCR0A & (1<<COM0A1)) ) 
                {
                    TCCR0A |= (1<<COM0A1);
                }
                OCR0A = (uint8_t)(duty & 0xFF);
                printf_P(PSTR("{\"pwm\":{\"CH3\":\"%d\"}}\r\n"),OCR0A);
                initCommandBuffer();
            }
            else if (strcmp_P( arg[0], PSTR("CH2")) == 0) // CH2 is on OC0B
            {
                // if ( !(DDRD & (1<<PD5)) ) DDRD |= (1<<PD5);

                // connect PD5 pin to OC0B output (pwm) from timer 0 channel B, Clear on Compare Output or Fast PWM mode
                if ( !(TCCR0A & (1<<COM0B1)) ) 
                {
                    TCCR0A |= (1<<COM0B1);
                }
                OCR0B = (uint8_t)(duty & 0xFF);
                printf_P(PSTR("{\"pwm\":{\"CH2\":\"%d\"}}\r\n"),OCR0B);
                initCommandBuffer();
            }
            else if (strcmp_P( arg[0], PSTR("CH4")) == 0) // CH4 is on OC1A
            {
                // if ( !(DDRB & (1<<PB1)) ) DDRB |= (1<<PB1);

                // connect PB1 pin to OC1A output (pwm) from timer 1 channel A, Clear on Compare Output or Fast PWM mode.
                if ( !(TCCR1A & (1<<COM1A1)) ) 
                {
                    TCCR1A |= (1<<COM1A1);
                }
                OCR1A = (uint8_t)(duty & 0xFF);
                printf_P(PSTR("{\"pwm\":{\"CH4\":\"%d\"}}\r\n"),OCR1A);
                initCommandBuffer();
            }
            else if (strcmp_P( arg[0], PSTR("CH5")) == 0) // CH5 is on OC1B
            {
                // if ( !(DDRB & (1<<PB2)) ) DDRB |= (1<<PB2);

                // connect PB2 pin to OC1B output (pwm) from timer 1 channel B, Clear on Compare Output or Fast PWM mode
                if ( !(TCCR1A & (1<<COM1B1)) ) 
                {
                    TCCR1A |= (1<<COM1B1);
                }
                OCR1B = (uint8_t)(duty & 0xFF);
                printf_P(PSTR("{\"pwm\":{\"CH5\":\"%d\"}}\r\n"),OCR1B);
                initCommandBuffer();
            }
            else if (strcmp_P( arg[0], PSTR("CH6")) == 0) // CH6 is on OC2A
            {
                // if ( !(DDRB & (1<<PB3)) ) DDRB |= (1<<PB3);

                // connect PB3 pin to OC2A output (pwm) from timer 2 channel A, Clear on Compare Output or Fast PWM mode.
                if ( !(TCCR2A & (1<<COM2A1)) ) 
                {
                    TCCR2A |= (1<<COM2A1);
                }
                OCR2A = (uint8_t)(duty & 0xFF);
                printf_P(PSTR("{\"pwm\":{\"CH6\":\"%d\"}}\r\n"),OCR2A);
                initCommandBuffer();
            }
            else if (strcmp_P( arg[0], PSTR("CH1")) == 0) // CH1 is on OC2B
            {
                // if ( !(DDRD & (1<<PD3)) ) DDRD |= (1<<PD3);

                // connect PD3 pin to OC2B output (pwm) from timer 2 channel B, Clear on Compare Output or Fast PWM mode
                if ( !(TCCR2A & (1<<COM2B1)) ) 
                {
                    TCCR2A |= (1<<COM2B1);
                }
                OCR2B = (uint8_t)(duty & 0xFF);
                printf_P(PSTR("{\"pwm\":{\"CH1\":\"%d\"}}\r\n"),OCR2B);
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

