/*
alternat is a library that racks ADC3, ADC6, and ADC7 after Aux input is enabled. 
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
#include <util/atomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../Adc/references.h"
#include "alternat.h"

#define SERIAL_PRINT_DELAY_MILSEC 5000UL
#define ALT_ENABLE_DELAY_MILSEC 22500UL
static unsigned long serial_print_started_at;
static unsigned long alt_tracking_started_at;

static uint8_t alt_enabled;


void TrackAlt(void)
{
    if ( (command_done == 10) )
    {
        // laod reference calibration or show an error if they are not in eeprom
        if ( ! LoadAnalogRefFromEEPROM() )
        {
            printf_P(PSTR("{\"err\":\"AdcRefNotInEeprom\"}\r\n"));
            initCommandBuffer();
            return;
        }

        alt_tracking_started_at = millis();
        serial_print_started_at = millis();
        alt_enabled = 0;
        command_done = 11;
    }
    else if ( (command_done == 11) )
    { 
        printf_P(PSTR("{\"millis\":\"%lu\","),(serial_print_started_at-alt_tracking_started_at));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    { 
        printf_P(PSTR("\"ADC3\":"));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    { 
         printf_P(PSTR("\"%1.2f\""),(analogRead(ADC3)*(ref_extern_avcc_uV/1.0E6)/1024.0)*(115.8/15.8) );
        command_done = 14;
    }
        else if ( (command_done == 14) )
    { 
        printf_P(PSTR("\"PWR_I\":"));
        command_done = 15;
    }
    else if ( (command_done == 15) )
    { 
         printf_P(PSTR("\"%1.3f\""),(analogRead(PWR_I)*((ref_extern_avcc_uV/1.0E6)/1024.0)/(0.018*50.0)));
        command_done = 16;
    }
        else if ( (command_done == 16) )
    { 
        printf_P(PSTR("\"PWR_V\":"));
        command_done = 17;
    }
    else if ( (command_done == 17) )
    { 
         printf_P(PSTR("\"%1.2f\""),(analogRead(PWR_V)*((ref_extern_avcc_uV/1.0E6)/1024.0)*(115.8/15.8)));
        command_done = 18;
    }
    else if ( (command_done == 18) )
    { 
         printf_P(PSTR("}\r\n"));
        command_done = 19;
    }
    else if ( (command_done == 19) ) 
    { 
        // delay Alternat enable
        if (!alt_enabled)
        {
            unsigned long kRuntime= millis() - alt_tracking_started_at;
            if ((kRuntime) > ((unsigned long)ALT_ENABLE_DELAY_MILSEC))
            {
                digitalWrite(ALT_EN,HIGH); // turn on the ALT power input
                alt_enabled =1;
            }
        }
        // delay between JSON printing
        unsigned long kRuntime= millis() - serial_print_started_at;
        if ((kRuntime) > ((unsigned long)SERIAL_PRINT_DELAY_MILSEC))
        {
            serial_print_started_at+= SERIAL_PRINT_DELAY_MILSEC;
            command_done = 11; /* This keeps looping output forever (until a Rx char anyway) */
        }
    }
}
