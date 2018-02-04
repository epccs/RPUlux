/*
chrg_accum is part of AmpHr, it is to track how much current is used.
Copyright (C) 2017 Ronald Sutherland

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
#include <util/atomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/timers.h"
#include "../lib/pins_board.h"
#include "../Adc/references.h"
#include "chrg_accum.h"

// ADC6 is a high side current sense of the input current
// converted to current with analogRead(PWR_I)*(5.0/1024.0)/(0.068*50.0))
// the fine accumulator is for the adc reading * millis counts (e.g.  it is in mSec and needs scaled)

static unsigned long chrgTmrStartOfAccum;
static unsigned long chrgTmrStarted;
#define CHRG_ACCUMULATION_DELAY_MILSEC 20UL

static unsigned long chrg_accum;
static unsigned long chrg_accum_fine;

#define SERIAL_PRINT_DELAY_MILSEC 60000UL
static unsigned long serial_print_started_at;

// Accumulated Charge in mAHr
float ChargeAccum(void)
{
    return chrg_accum * ((ref_extern_avcc_uV/1.0E6)/1024.0)/(0.068*50.0)/3.6;
}

void Charge(void)
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
        
        serial_print_started_at = millis();
        printf_P(PSTR("{\"CHRG_mAHr\":"));
        command_done = 11;
    }
    else if ( (command_done == 11) )
    { 
        printf_P(PSTR("\"%1.2f\","),ChargeAccum() );
        command_done = 16;
    }
    else if ( (command_done == 16) )
    { 
        printf_P(PSTR("\"ACCUM_Sec\":"));
        command_done = 17;
    }
    else if ( (command_done == 17) )
    { 
        printf_P(PSTR("\"%1.2f\""),((chrgTmrStarted - chrgTmrStartOfAccum)/1000.0));
        printf_P(PSTR("}\r\n"));
        command_done = 18;
    }
    else if ( (command_done == 18) ) 
    { // delay between JSON printing
        unsigned long kRuntime= millis() - serial_print_started_at;
        if ((kRuntime) > ((unsigned long)SERIAL_PRINT_DELAY_MILSEC))
        {
            command_done = 10; /* This keeps looping output forever (until a Rx char anyway) */
        }
    }
}

/* check power accumulation
    charge accumulation is an integer value of the ADC*(millis()/1000) and will need scaled
*/
void CheckChrgAccumulation(void) 
{
    unsigned long kRuntime= millis() - chrgTmrStarted;
    if ((kRuntime) > ((unsigned long)CHRG_ACCUMULATION_DELAY_MILSEC))
    {
        chrg_accum_fine += (analogRead(PWR_I) * CHRG_ACCUMULATION_DELAY_MILSEC); 
        chrgTmrStarted += CHRG_ACCUMULATION_DELAY_MILSEC;
    }
    else
    {
        // check if fine accumulator has enough to add it to the Sec based accumulater
        if (chrg_accum_fine > 1000) 
        {
            ++chrg_accum;
            chrg_accum_fine -= 1000;
        }
    }
}

/* The charge and discharge accumulation values need to be zeroed at the start of the day
*/
uint8_t init_ChargAccumulation(void) 
{
    chrg_accum = 0;
    chrg_accum_fine = 0;
    chrgTmrStarted = millis();
    chrgTmrStartOfAccum = chrgTmrStarted;
    
    // laod reference calibration from eeprom
    if ( ! LoadAnalogRefFromEEPROM() )
    {
        return 0;
    }
    
    return 1;
}