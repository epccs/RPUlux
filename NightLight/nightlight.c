/*
nightlight is a library that controls some lights with a timer and state machine. 
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
#include <avr/eeprom.h> 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "../lib/parse.h"
#include "../lib/timers.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../AmpHr/chrg_accum.h"
#include "nightlight.h"

//The EEPROM memory usage is as follows. 
#define EE_LED_BASE_ADDR 200
// each LED 1..3 has an array of settings offset by
#define EE_LED_ARRAY_OFFSET 20
// each setting is at this byte from the array offset
#define EE_LED_ID 0
#define EE_LED_DLY_STRT 2
#define EE_LED_RUNTIME 6
#define EE_LED_DELAY 10
#define EE_LED_MAHR_STP 14
#define EE_LED_CYCLES 18

#define SEC_IN_HR 3600UL
#define SEC_IN_6HR 21600UL
#define SEC_IN_DAY 86400UL

#define MAHR_NOT_SET 0xFFFFFFFFUL

typedef struct {
    unsigned long started_at;  // holds the start time of the present operation
    unsigned long delay_start_sec; // delay befor first set operation (one time)
    unsigned long runtime_sec; // delay after set operation and befor reset
    unsigned long delay_sec;   // delay after reset operation and befor next set operation if (cycles > 0)
    uint8_t cycle_state;
    uint16_t cycles; // keep cycling until zero
    uint32_t mahr_stop; // mAHr usage after start at which to stop the led
    unsigned long cycle_millis_start;
    unsigned long cycle_millis_stop;
    unsigned long cycle_millis_bank; // millis count accumulate or store
}  ledTimer;

static ledTimer led[LEDSTRING_COUNT];

typedef struct {
  uint8_t dio;  
} Led_Map;

//  LED STRING1 is at ledMap index zero not one.
static const Led_Map ledMap[LEDSTRING_COUNT] = {
    [0] = { .dio=3 }, // LED1 is controled with Digital IO3
    [1] = { .dio=5 }, // LED2 is controled with Digital IO5
    [2] = { .dio=6 }, // LED3 is controled with Digital IO6
    [3] = { .dio=9 }, // LED4 is controled with Digital IO9
    [4] = { .dio=10 }, // LED5 is controled with Digital IO10
    [5] = { .dio=11 }, // LED6 is controled with Digital IO11
};

uint8_t uint8_from_arg0 (void)
{
    // check that arg[0] is a digit 
    if ( ( !( isdigit(arg[0][0]) ) ) )
    {
        printf_P(PSTR("{\"err\":\"%sArg0_NaN\"}\r\n"),command[1]);
        return 0;
    }
    uint8_t arg0 = atoi(arg[0]);
    if ( ( arg0 < 1) || (arg0 > LEDSTRING_COUNT) )
    {
        printf_P(PSTR("{\"err\":\"%sArg0_OutOfRng\"}\r\n"),command[1]);
        return 0;
    }
    return arg0;
}

unsigned long ul_from_arg1 (unsigned long max)
{
    // check that arg[1] is a digit 
    if ( ( !( isdigit(arg[1][0]) ) ) )
    {
        printf_P(PSTR("{\"err\":\"%s1NaN\"}\r\n"),command[1]);
        return 0;
    }
    unsigned long ul = strtoul(arg[1], (char **)NULL, 10);
    if ( ( ul < 1) || (ul > max) )
    {
        printf_P(PSTR("{\"err\":\"%s1OtOfRng\"}\r\n"),command[1]);
        return 0;
    }
    return ul;
}

// arg[0] is led, arg[1] is delay_start
void DelayStart(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..SEC_IN_6HR 
        unsigned long delay_start = ul_from_arg1(SEC_IN_6HR);
        if (! delay_start)
        {
            initCommandBuffer();
            return;
        }
        // don't change a LED that is in use it needs to be stopped first
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        led[led_arg0-1].delay_start_sec = delay_start;
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_start_sec\":\"%lu\""),(led[led_arg0-1].delay_start_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"DlyStCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led, arg[1] is runtime
void RunTime(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..SEC_IN_6HR 
        unsigned long runtime = ul_from_arg1(SEC_IN_6HR);
        if (! runtime)
        {
            initCommandBuffer();
            return;
        }
        // don't change an led that is in use it needs to be stopped first
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        led[led_arg0-1].runtime_sec = runtime;
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"runtime_sec\":\"%lu\","),(led[led_arg0-1].runtime_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"RunTmCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led, arg[1] is delay
void Delay(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..SEC_IN_DAY 
        unsigned long delay = ul_from_arg1(SEC_IN_DAY);
        if (! delay)
        {
            initCommandBuffer();
            return;
        }
        // don't change an led that is in use it needs to be stopped first
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        led[led_arg0-1].delay_sec = delay;
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_sec\":\"%lu\""),(led[led_arg0-1].delay_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"DelayCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led, arg[1] is mahr_stop
void AHrStop(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        // and arg[1] value is 1..MAHR_NOT_SET 
        unsigned long mahr_stop = ul_from_arg1(MAHR_NOT_SET);
        if (! mahr_stop)
        {
            initCommandBuffer();
            return;
        }
        // don't change an led that is in use it needs to be stopped first
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        led[led_arg0-1].mahr_stop = mahr_stop;
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"mahr_stop\":\"%lu\","),(led[led_arg0-1].mahr_stop));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"AHrStpCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led, [arg[1] is cycles]
void Run(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        
        uint16_t cycles = led[led_arg0-1].cycles;
        if (arg[1]!=NULL)
        {
            // valid arg[1] value is 1..0xFFFF
            cycles = (uint16_t) (ul_from_arg1(0xFFFF));
            if (! cycles)
            {
                initCommandBuffer();
                return;
            }
        }

         // don't run a led that is in use it needs to be stopped first
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        led[led_arg0-1].cycle_state = 1;
        led[led_arg0-1].cycles = cycles;
        led[led_arg0-1].cycle_millis_bank = 0;
        led[led_arg0-1].started_at = millis(); //delay_start_sec is timed from now
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_start_sec\":\"%lu\","),(led[led_arg0-1].delay_start_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"runtime_sec\":\"%lu\","),(led[led_arg0-1].runtime_sec));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_sec\":\"%lu\","),(led[led_arg0-1].delay_sec));
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%u\""),(led[led_arg0-1].cycles));
        command_done = 15;
    }
    else if ( (command_done == 15) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        if (led[led_arg0-1].mahr_stop != MAHR_NOT_SET)
        {
            printf_P(PSTR(",\"mahr_stop\":\"%lu\""),(led[led_arg0-1].mahr_stop));
        }
        command_done = 16;
    }
    else if ( (command_done == 16) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"RunCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led, arg[1] is cycles
void Save(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        // valid arg[1] value is 1..0xFFFF 
        uint16_t cycles = ul_from_arg1(0xFFFF);
        if (! cycles)
        {
            initCommandBuffer();
            return;
        }
         // don't save a led that is in use
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        if ( eeprom_is_ready() )
        {
            led[led_arg0-1].cycle_state = 0;
            led[led_arg0-1].cycles = cycles;
            led[led_arg0-1].cycle_millis_bank = 0;
            uint16_t value = ((uint16_t) (led_arg0)) + 0x4C30; //ascii bytes for 'L1', 'L2'...
            eeprom_write_word( (uint16_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_ID), value);
            printf_P(PSTR("{\"LED%d\":{"),led_arg0);
            command_done = 11;
        }
    }
    else if ( (command_done == 11) )
    {  
        if ( eeprom_is_ready() )
        {
            uint8_t led_arg0 = atoi(arg[0]);
            uint32_t value = led[led_arg0-1].delay_start_sec;
            eeprom_write_dword( (uint32_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_DLY_STRT), value);
            printf_P(PSTR("\"delay_start_sec\":\"%lu\","),(value));
            command_done = 12;
        }
    }
    else if ( (command_done == 12) )
    {
        if ( eeprom_is_ready() )
        {
            uint8_t led_arg0 = atoi(arg[0]);
            uint32_t value = led[led_arg0-1].runtime_sec;
            eeprom_write_dword( (uint32_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_RUNTIME), value);
            printf_P(PSTR("\"runtime_sec\":\"%lu\","),(value));
            command_done = 13;
        }
    }
    else if ( (command_done == 13) )
    {  
        if ( eeprom_is_ready() )
        {
            uint8_t led_arg0 = atoi(arg[0]);
            uint32_t value = led[led_arg0-1].delay_sec;
            eeprom_write_dword( (uint32_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_DELAY), value);
            printf_P(PSTR("\"delay_sec\":\"%lu\","),(value));
            command_done = 14;
        }
    }
    else if ( (command_done == 14) )
    {
        if ( eeprom_is_ready() )
        {
            uint8_t led_arg0 = atoi(arg[0]);
            uint32_t value = led[led_arg0-1].mahr_stop;
            eeprom_write_dword( (uint32_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_MAHR_STP), value);
            if (led[led_arg0-1].mahr_stop != MAHR_NOT_SET)
            {
                printf_P(PSTR("\"mahr_stop\":\"%lu\","),(value));
            }
            command_done = 15;
        }
    }
    else if ( (command_done == 15) )
    {
        if ( eeprom_is_ready() )
        {
            uint8_t led_arg0 = atoi(arg[0]);
            uint16_t value = led[led_arg0-1].cycles;
            eeprom_write_word( (uint16_t *)((led_arg0-1)*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_CYCLES), value);
            printf_P(PSTR("\"cycles\":\"%u\""),(value));
            command_done = 16;
        }
    }
    else if ( (command_done == 16) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"SavCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led
void Load(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }

         // don't load a led that is in use
        if (led[led_arg0-1].cycle_state)
        {
            printf_P(PSTR("{\"err\":\"Led%dInUse\"}\r\n"),led_arg0);
            initCommandBuffer();
            return;
        }
        if ( eeprom_is_ready() )
        {
            if (LoadLedControlFromEEPROM(led_arg0))
            {
                led[led_arg0-1].cycle_millis_bank = 0;
                printf_P(PSTR("{\"LED%d\":{"),led_arg0);
                command_done = 11;
            }
            else
            {
                printf_P(PSTR("{\"err\":\"LdFailLed%dnoEEP\"}\r\n"),led_arg0);
                initCommandBuffer();
                return;
            }
        }
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_start_sec\":\"%lu\","),(led[led_arg0-1].delay_start_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"runtime_sec\":\"%lu\","),(led[led_arg0-1].runtime_sec));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_sec\":\"%lu\","),(led[led_arg0-1].delay_sec));
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {
        uint8_t led_arg0 = atoi(arg[0]);
        if (led[led_arg0-1].mahr_stop != MAHR_NOT_SET)
        {
            printf_P(PSTR("\"mahr_stop\":\"%lu\","),(led[led_arg0-1].mahr_stop));
        }
        command_done = 15;
    }
    else if ( (command_done == 15) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%u\""),(led[led_arg0-1].cycles));
        command_done = 16;
    }
    else if ( (command_done == 16) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"LdCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led
void Time(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"cycle_state\":\"%d\","),(led[led_arg0-1].cycle_state));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%u\","),(led[led_arg0-1].cycles));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"cycle_millis\":\"%lu\""),(led[led_arg0-1].cycle_millis_bank));
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"TmCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

// arg[0] is led
void Stop(void)
{
    if ( (command_done == 10) )
    {
        uint8_t led_arg0 = uint8_from_arg0();
        if (! led_arg0)
        {
            initCommandBuffer();
            return;
        }
        printf_P(PSTR("{\"LED%d\":{"),led_arg0);
        command_done = 11;
    }
    else if ( (command_done == 11) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        if (led[led_arg0-1].cycle_state)
        {
            led[led_arg0-1].delay_start_sec = 1;
            led[led_arg0-1].runtime_sec = 1; 
            led[led_arg0-1].delay_sec = 1;
            led[led_arg0-1].mahr_stop = MAHR_NOT_SET;
            led[led_arg0-1].cycles = 1;
        }
        printf_P(PSTR("\"delay_start_sec\":\"%lu\","),(led[led_arg0-1].delay_start_sec));
        command_done = 12;
    }
    else if ( (command_done == 12) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"runtime_sec\":\"%lu\","),(led[led_arg0-1].runtime_sec));
        command_done = 13;
    }
    else if ( (command_done == 13) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"delay_sec\":\"%lu\","),(led[led_arg0-1].delay_sec));
        command_done = 14;
    }
    else if ( (command_done == 14) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        printf_P(PSTR("\"cycles\":\"%u\""),(led[led_arg0-1].cycles));
        command_done = 15;
    }
    else if ( (command_done == 15) )
    {  
        uint8_t led_arg0 = atoi(arg[0]);
        if (led[led_arg0-1].mahr_stop != MAHR_NOT_SET)
        {
            printf_P(PSTR(",\"mahr_stop\":\"%lu\""),(led[led_arg0-1].mahr_stop));
        }
        command_done = 16;
    }
    else if ( (command_done == 16) )
    {
        printf_P(PSTR("}}\r\n"));
        initCommandBuffer();
    }
    else
    {
        printf_P(PSTR("{\"err\":\"StpCmdDnWTF\"}\r\n"));
        initCommandBuffer();
    }
}

#define LED_STATE_NOT_ACTIVE 0
#define LED_STATE_PRESTART 1
#define LED_STATE_SET 2
#define LED_STATE_RUNTIME 3
#define LED_STATE_RESET 4
#define LED_STATE_CYCCOUNT 5
#define LED_STATE_DELAY 6
/* operate the led states without blocking
    cycle_state 
    LED_STATE_NOT_ACTIVE = led not active
    LED_STATE_PRESTART = active, wait for pre start time (delay_start_sec), 
    LED_STATE_SET = trun on LED.
    LED_STATE_RUNTIME = wait for runTime with LED on 
    LED_STATE_RESET = turn off LED
    LED_STATE_CYCCOUNT = reduce the cycle count and stop if last one was done.
    LED_STATE_DELAY = wait for the delay time with LED off than set LED_STATE_SET.
*/ 
void LedControl() {
    for(int i = 0; i < LEDSTRING_COUNT; i++){
        // active, wait for start time (delay_start_sec),.
        if ((led[i].cycle_state == LED_STATE_PRESTART)) 
        {
            unsigned long kRuntime= millis() - led[i].started_at;
            if ((kRuntime) > ((unsigned long)led[i].delay_start_sec * 1000)) 
            {
                led[i].cycle_state = LED_STATE_SET;
                break;
            }
        }
  
        if ((led[i].cycle_state == LED_STATE_SET)) 
        {
            if(ledMap[i].dio < NUM_DIGITAL_PINS) // the compiler does not know if ledMap values are valid so test them 
                digitalWrite(ledMap[i].dio,HIGH); // Use a Digital IO to sink a current source;
            led[i].started_at = millis(); //start timing runtime
            led[i].cycle_millis_start = millis(); 
            led[i].cycle_state = LED_STATE_RUNTIME;
            break;
        }

        // wait for runTime.
        if (led[i].cycle_state == LED_STATE_RUNTIME) 
        {
            unsigned long kRuntime= millis() - led[i].started_at;
            if ((kRuntime) > ((unsigned long)led[i].runtime_sec * 1000)-1) 
            {
                led[i].cycle_millis_stop = millis(); // correction of -1 millis was added so timer shows expected value
                led[i].cycle_state = LED_STATE_RESET;
            }
            if (ChargeAccum(PWR_I) > led[i].mahr_stop) // ChargeAccum is from ../AmpHr/chrg_accum.c, warning this test takes a few machine cycles on an AVR
            {  
                led[i].cycle_state = LED_STATE_RESET;
                led[i].cycles = 0;
                break;
            }
        }

        if ((led[i].cycle_state == LED_STATE_RESET)) 
        {
            if(ledMap[i].dio < NUM_DIGITAL_PINS) 
                digitalWrite(ledMap[i].dio,LOW); // When a Digital IO with level shift goes high it opens the LED string and the current source saturates.
            led[i].cycle_millis_bank += (led[i].cycle_millis_stop - led[i].cycle_millis_start);
            led[i].started_at = millis(); //start timing for  delay (if it is needed)
            led[i].cycle_state = LED_STATE_CYCCOUNT;
            break;
        }

        if ((led[i].cycle_state == LED_STATE_CYCCOUNT)) 
        {
            if (led[i].cycles)
            {
                led[i].cycles = led[i].cycles -1;
            }
            if (led[i].cycles)
            {
                led[i].cycle_state = LED_STATE_DELAY;
                break;
            }
            else
            {
                led[i].started_at = 0;
                led[i].cycle_state = LED_STATE_NOT_ACTIVE;
                break;
            }
        }
        
        // delay after reset operation and befor next set operation
        if ((led[i].cycle_state == LED_STATE_DELAY)) 
        {
            unsigned long kRuntime= millis() - led[i].started_at;
            if ( (kRuntime) > ((unsigned long)led[i].delay_sec*1000UL) ) 
            {
                led[i].cycle_state = LED_STATE_SET;
                break;
            }
        }
    }
}

// warning this sets all LED for 1 Second operation, 1 cycle, and sets cycle_state so they will run
void Reset_All_LED() {
    for(uint8_t i = 0; i < LEDSTRING_COUNT; i++)
    {
        led[i].started_at = millis(); // used to delay start
        led[i].delay_start_sec = i+i+i+1; // every 3 sec starting at 1, then 4, and 7
        led[i].runtime_sec = 1; 
        led[i].delay_sec = LEDSTRING_COUNT*3+1;
        led[i].mahr_stop = MAHR_NOT_SET;
        led[i].cycles = 1;
        led[i].cycle_state = 1;
        led[i].cycle_millis_bank = 0;
    }
}

uint8_t LoadLedControlFromEEPROM(uint8_t led_string) 
{
    uint16_t i = led_string-1;
    if (!led[i].cycle_state)
    {
        uint16_t id = eeprom_read_word((uint16_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_ID));
        if (id == (i+0x4C31) ) // 'L' is 0x4C and '1' is 0x31, thus L1, L2...
        {
            led[i].started_at = millis(); // delay start is from now.
            led[i].delay_start_sec =eeprom_read_dword((uint32_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_DLY_STRT)); 
            led[i].runtime_sec = eeprom_read_dword((uint32_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_RUNTIME));  
            led[i].delay_sec = eeprom_read_dword((uint32_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_DELAY)); 
            led[i].mahr_stop = eeprom_read_dword((uint32_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_MAHR_STP)); 
            led[i].cycles = eeprom_read_word((uint16_t*)(i*EE_LED_ARRAY_OFFSET+EE_LED_BASE_ADDR+EE_LED_CYCLES)); 
            led[i].cycle_millis_bank = 0;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

// return the cycle_state (e.g. tells if it is running)
uint8_t Live(uint8_t led_string) 
{
    uint16_t i = led_string-1;
    if (i<LEDSTRING_COUNT)
    {
        return led[i].cycle_state;
    }
    else return 0xFF; // not a valid LED string 
}

// start the LED if it is not running and return cycle state
uint8_t StartLed(uint8_t led_string) 
{
    uint16_t i = led_string-1;
    if (i<LEDSTRING_COUNT )
    {
        if (! led[i].cycle_state) led[i].cycle_state = 1;
        return led[i].cycle_state;
    }
    else return 0x0; // not a valid led string
}

// only use init at setup() not durring loop()
void init_Led(void) 
{
    for(int i = 0; i < LEDSTRING_COUNT; i++)
    {
        pinMode(ledMap[i].dio,OUTPUT);
        digitalWrite(ledMap[i].dio,LOW);
    }
}
