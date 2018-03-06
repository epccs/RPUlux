/*
AmpHr is a command line controled demonstration of how to estimate the charge used from a power source.
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
#include "../lib/timers.h"
#include "../lib/uart.h"
#include "../lib/parse.h"
#include "../lib/adc.h"
#include "../lib/twi.h"
#include "../lib/rpu_mgr.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../Uart/id.h"
#include "../Adc/analog.h"
#include "../DayNight/day_night.h"
#include "../Pwm/pwm.h"
#include "../Digital/digital.h"
#include "chrg_accum.h"

// how fast does the discharge reading change? (it can be fast)
// at 16MHz the adc clock runs at 125kHz (see the core file ../lib/adc.c)
// the first reading takes 25 ADC clocks and the next takes 13 ADC clocks.
// since the channel changes with each new reading it always takes 25 ADC clocks for each reading.
// which means the max reading rate is about 5000 per sec.
// Fill the adc array every 10mSec with readings on all channels (e.g. 800 reading per sec, ) 
// The ADC is off 84% of the time so most of the power saving is relized.
#define ADC_DELAY_MILSEC 10UL
static unsigned long adc_started_at;

// 22mA current sources enabled with CS0_EN and CS1_EN which are defined in ../lib/pins_board.h
#define STATUS_LED CS0_EN
#define DAYNIGHT_STATUS_LED CS1_EN
#define DAYNIGHT_BLINK 500UL
static unsigned long day_status_blink_started_at;

#define BLINK_DELAY 1000UL
static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;

void ProcessCmd()
{ 
    if ( (strcmp_P( command, PSTR("/id?")) == 0) && ( (arg_count == 0) || (arg_count == 1)) )
    {
        Id("AmpHr"); // ../Uart/id.c
    }
    if ( (strcmp_P( command, PSTR("/day?")) == 0) && ( (arg_count == 0 ) ) )
    {
        Day(); // ../DayNight/day_night.c
    }
    if ( (strcmp_P( command, PSTR("/analog?")) == 0) && ( (arg_count >= 1 ) && (arg_count <= 5) ) )
    {
        Analog(); // ../Adc/analog.c
    }
    if ( (strcmp_P( command, PSTR("/pwm")) == 0) )
    {
        Pwm(); // ../Pwm/pwm.c
    }
    if ( (strcmp_P( command, PSTR("/pMod")) == 0) && ( (arg_count == 2 ) ) )
    {
        Mode(); // ../Digital/digital.c
    }
    if ( (strcmp_P( command, PSTR("/dWrt")) == 0) && ( (arg_count == 2 ) ) )
    {
        Write(); // ../Digital/digital.c
    }
    if ( (strcmp_P( command, PSTR("/dTog")) == 0) && ( (arg_count == 1 ) ) )
    {
        Toggle(); // ../Digital/digital.c
    }
    if ( (strcmp_P( command, PSTR("/dRe?")) == 0) && ( (arg_count == 1 ) ) )
    {
        Read(); // ../Digital/digital.c
    }
    if ( (strcmp_P( command, PSTR("/charge?")) == 0) && ( (arg_count == 0 ) ) )
    {
        Charge(); // ./chrg_accum.c
    }
}

//At start of each day do this
void callback_for_day_attach(void)
{
    // I don't have a defaut (not sure how to do that) so this is needed.
    return;
}

//At start of each night do this
void callback_for_night_attach(void)
{
    // setup AmpHr accumulators and load Adc calibration reference
    if (!init_ChargAccumulation(PWR_I)) // ../AmpHr/power_storage.c
    {
        blink_delay = BLINK_DELAY/4;
    }
}

void setup(void) 
{
    pinMode(STATUS_LED,OUTPUT);
    digitalWrite(STATUS_LED,HIGH);

    pinMode(DAYNIGHT_STATUS_LED,OUTPUT);
    digitalWrite(DAYNIGHT_STATUS_LED,HIGH);

    // set pwm pin DDR
    pinMode(CH1,OUTPUT); // DDRD |= (1<<PD3)
    pinMode(CH2,OUTPUT); // DDRD |= (1<<PD5)
    pinMode(CH3,OUTPUT); // DDRD |= (1<<PD6)
    pinMode(CH4,OUTPUT); // DDRB |= (1<<PB1)
    pinMode(CH5,OUTPUT); // DDRB |= (1<<PB2)
    pinMode(CH6,OUTPUT); // DDRB |= (1<<PB3)

    // Initialize Timers, ADC, and clear bootloader, Arduino does these with init() in wiring.c
    initTimers(); //Timer0 Fast PWM mode, Timer1 & Timer2 Phase Correct PWM mode.
    init_ADC_single_conversion(EXTERNAL_AVCC); // warning AREF must not be connected to anything
    init_uart0_after_bootloader(); // bootloader may have the UART setup

    // put ADC in Auto Trigger mode and fetch an array of channels
    enable_ADC_auto_conversion(BURST_MODE);
    adc_started_at = millis();

    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);
    
    /* Initialize I2C, with the internal pull-up 
        note: I2C scan will stop without a pull-up on the bus */
    twi_init(TWI_PULLUP);

    /* Clear and setup the command buffer, (probably not needed at this point) */
    initCommandBuffer();

    // Enable global interrupts to start TIMER0 and UART ISR's
    sei(); 
    
    blink_started_at = millis();
    day_status_blink_started_at = millis();
    
    rpu_addr = get_Rpu_address();
    blink_delay = BLINK_DELAY;
    
    // blink fast if a default address from RPU manager not found
    if (rpu_addr == 0)
    {
        rpu_addr = '0';
        blink_delay = BLINK_DELAY/4;
    }
    
    // set callbacks for DayNight state machine, i.e. work to do.
    Day_AttachWork(callback_for_day_attach);
    Night_AttachWork(callback_for_night_attach);
}

void blink(void)
{
    unsigned long kRuntime = millis() - blink_started_at;
    if ( kRuntime > blink_delay)
    {
        digitalToggle(STATUS_LED);
        
        // next toggle 
        blink_started_at += blink_delay; 
    }
}

void blink_day_status(void)
{
    unsigned long kRuntime = millis() - day_status_blink_started_at;
    uint8_t state = DayState();
    if ( ( (state == DAYNIGHT_EVENING_DEBOUNCE_STATE) || \
           (state == DAYNIGHT_MORNING_DEBOUNCE_STATE) ) && \
           (kRuntime > (DAYNIGHT_BLINK/2) ) )
    {
        digitalToggle(DAYNIGHT_STATUS_LED);
        
        // set for next toggle 
        day_status_blink_started_at += DAYNIGHT_BLINK/2; 
    }
    if ( ( (state == DAYNIGHT_DAY_STATE) ) && \
           (kRuntime > (DAYNIGHT_BLINK) ) )
    {
        digitalWrite(DAYNIGHT_STATUS_LED,HIGH);
        
        // set for next toggle 
        day_status_blink_started_at += DAYNIGHT_BLINK; 
    }
    if ( ( (state == DAYNIGHT_NIGHT_STATE) ) && \
           (kRuntime > (DAYNIGHT_BLINK) ) )
    {
        digitalWrite(DAYNIGHT_STATUS_LED,LOW);
        
        // set for next toggle 
        day_status_blink_started_at += DAYNIGHT_BLINK; 
    }
    if ( ( (state == DAYNIGHT_FAIL_STATE) ) && \
           (kRuntime > (DAYNIGHT_BLINK/8) ) )
    {
        digitalToggle(DAYNIGHT_STATUS_LED);
        
        // set for next toggle 
        day_status_blink_started_at += DAYNIGHT_BLINK/8; 
    }
}

void adc_burst(void)
{
    unsigned long kRuntime= millis() - adc_started_at;
    if ((kRuntime) > ((unsigned long)ADC_DELAY_MILSEC))
    {
        enable_ADC_auto_conversion(BURST_MODE); // ../lib/adc.c
        adc_started_at += ADC_DELAY_MILSEC; 
    } 
}

int main(void) 
{
    setup();

    while(1) 
    { 
        // use STATUS_LED to show if I2C has a bus manager
        blink();

        // use DAYNIGHT_STATUS_LED to show day_state
        blink_day_status();

        // Check Day Light is a function that operates a day-night state machine.
        CheckDayLight(); // ../DayNight/day_night.c

        // delay between ADC burst
        adc_burst();

        // check how much current went through high side sensor
        CheckChrgAccumulation(PWR_I);
    
        // check if character is available to assemble a command, e.g. non-blocking
        if ( (!command_done) && uart0_available() ) // command_done is an extern from parse.h
        {
            // get a character from stdin and use it to assemble a command
            AssembleCommand(getchar()); // ../lib/parse.c

            // address is an ascii value, warning: a null address would terminate the command string. 
            StartEchoWhenAddressed(rpu_addr); // ../lib/parse.c
        }
        
        // check if a character is available, and if so flush transmit buffer and nuke the command in process.
        // A multi-drop bus can have another device start transmitting after getting an address byte so
        // the first byte is used as a warning, it is the onlly chance to detect a possible collision.
        if ( command_done && uart0_available() )
        {
            // dump the transmit buffer to limit a collision 
            uart0_flush(); // ../lib/uart.c
            initCommandBuffer(); // ../lib/parse.c
        }

        // finish echo of the command line befor starting a reply (or the next part of a reply)
        if ( command_done && (uart0_availableForWrite() == UART_TX0_BUFFER_SIZE) )
        {
            if ( !echo_on  )
            { // this happons when the address did not match 
                initCommandBuffer(); // ../lib/parse.c
            }
            else
            {
                if (command_done == 1)  
                {
                    findCommand(); // ../lib/parse.c
                    command_done = 10;
                }
                
                // do not overfill the serial buffer since that blocks looping, e.g. process a command in 32 byte chunks
                if ( (command_done >= 10) && (command_done < 250) )
                {
                     ProcessCmd();
                }
                else 
                {
                    initCommandBuffer(); // ../lib/parse.c
                }
            }
         }
    }        
    return 0;
}
