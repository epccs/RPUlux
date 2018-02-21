/* 
RPUlux SelfTest
Copyright (C) 2018 Ronald Sutherland

This Library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License 
along with the SelfTest.  If not, see http://www.gnu.org/licenses/.
*/ 

#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/io.h>
#include "../lib/timers.h"
#include "../lib/uart.h"
#include "../lib/twi.h"
#include "../lib/adc.h"
#include "../lib/rpu_mgr.h"
#include "../lib/pin_num.h"
#include "../lib/pins_board.h"
#include "../Adc/references.h"

#define BLINK_DELAY 1000UL

// Save the Value of the References for ADC converter 
// measure AVCC and put it hear in uV 
#define REF_EXTERN_AVCC 4986100UL
// ref_intern_1v1_uV is calculated based on the above value and the ICP1 PL resistor


#define R1 33.333

static unsigned long blink_started_at;
static unsigned long blink_delay;
static char rpu_addr;
static uint8_t passing;

void setup(void) 
{
    // Turn Off Current Sources
    pinMode(CS0_EN,OUTPUT);
    digitalWrite(CS0_EN,LOW);
    pinMode(CS1_EN,OUTPUT);
    digitalWrite(CS1_EN,LOW);

    // Turn Off VOUT to shield (e.g. disconnect VIN from shield)
    pinMode(SHLD_VIN_EN,OUTPUT);
    digitalWrite(SHLD_VIN_EN,LOW);
    
    // Turn Off NMOS sinks on ADC0 and ADC1
    pinMode(ADC0_SINK,OUTPUT);
    digitalWrite(ADC0_SINK,LOW);
    pinMode(ADC1_SINK,OUTPUT);
    digitalWrite(ADC1_SINK,LOW);

    // Initialize Timers, ADC, and clear bootloader, Arduino does these with init() in wiring.c
    initTimers(); //Timer0 Fast PWM mode, Timer1 & Timer2 Phase Correct PWM mode.
    init_ADC_single_conversion(EXTERNAL_AVCC); // warning AREF must not be connected to anything
    init_uart0_after_bootloader(); // bootloader may have the UART setup

    /* Initialize UART, it returns a pointer to FILE so redirect of stdin and stdout works*/
    stdout = stdin = uartstream0_init(BAUD);

    /* Initialize I2C, with the internal pull-up*/
    twi_init(TWI_PULLUP);

    // Enable global interrupts to start TIMER0 and UART
    sei(); 

    blink_started_at = millis();

    rpu_addr = get_Rpu_address();
    blink_delay = BLINK_DELAY;

    // blink fast if a default address from RPU manager not found
    if (rpu_addr == 0)
    {
        rpu_addr = '0';
        blink_delay = BLINK_DELAY/4;
    }
    
    // start with this referance, note it is not in EEPROM
    ref_extern_avcc_uV = REF_EXTERN_AVCC;
}

void test(void)
{
    // Info from some Predefined Macros
    printf_P(PSTR("RPUlux Self Test date: %s\r\n"), __DATE__);
    printf_P(PSTR("avr-gcc --version: %s\r\n"),__VERSION__);
    
    // I2C is used to read serial bus manager address 
    if (rpu_addr == '1')
    {
        printf_P(PSTR("I2C provided address 0x31 from serial bus manager\r\n"));
    } 
    else  
    { 
        passing = 0; 
        printf_P(PSTR(">>> I2C failed, or address not 0x31 from serial bus manager\r\n"));
        return;
    }

    // With current sources off measure input current
    _delay_ms(1000) ; // busy-wait to let the 1uF settle

    // Input voltage
    int adc_pwr_v = analogRead(PWR_V);
    printf_P(PSTR("adc reading for PWR_V: %d\r\n"), adc_pwr_v);
    float input_v = adc_pwr_v*((ref_extern_avcc_uV/1.0E6)/1024.0)*(115.8/15.8);
    printf_P(PSTR("PWR at: %1.3f V\r\n"), input_v);
    if (input_v > 14.0) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Input voltage is to high.\r\n"));
        return;
    }
    if (input_v < 12.0) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Input voltage is to low.\r\n"));
        return;
    }
    
    //current sources are off,  but ADC0 and ADC1 will float somewhat since they have an LED turn on NMOS to zero them
    digitalWrite(ADC0_SINK,HIGH);
    digitalWrite(ADC1_SINK,HIGH);
    _delay_ms(100); // busy-wait delay

    // measure ADC0..ADC3 
    float adc0_v = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    printf_P(PSTR("ADC0 GN LED /W SINK on and CS*_EN off: %1.3f V\r\n"), adc0_v);
    float adc1_v = analogRead(ADC1)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    printf_P(PSTR("ADC1 RD LED /W SINK on and CS*_EN off: %1.3f V\r\n"), adc1_v);
    float adc2_v = analogRead(ADC2)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    printf_P(PSTR("ADC2 R1 /W CS*_EN off: %1.3f V\r\n"), adc2_v);
    float adc3_v = analogRead(ADC3)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    printf_P(PSTR("ADC3 R1 /W CS*_EN off: %1.3f V\r\n"), adc3_v);
    if ( (adc0_v > 0.01)  || (adc1_v > 0.01) || (adc2_v > 0.01) || (adc3_v > 0.01))
    { 
        passing = 0; 
        printf_P(PSTR(">>> ADC is to high, is the self-test wiring right?\r\n"));
        return;
    }

    // enable CS0 
    digitalWrite(ADC0_SINK,LOW);
    digitalWrite(ADC1_SINK,LOW);
    digitalWrite(CS0_EN,HIGH);
    _delay_ms(100); // busy-wait delay
    
    // R1 hos CS0 on it
    float adc2_cs0_v = analogRead(ADC2)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    float adc2_cs0_i = adc2_cs0_v / R1;
    printf_P(PSTR("CS0 curr source on R1: %1.3f A\r\n"), adc2_cs0_i);
    if (adc2_cs0_i < 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS0 curr is to low.\r\n"));
        return;
    }
    if (adc2_cs0_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS0 curr is to high.\r\n"));
        return;
    }
    float adc0_cs0_v = analogRead(ADC0)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    float green_led_v = adc0_cs0_v - adc2_cs0_v;
    printf_P(PSTR("Green LED fwd V: %1.3f V\r\n"), green_led_v);
    digitalWrite(CS0_EN,LOW);

    // enable CS1
    digitalWrite(CS1_EN,HIGH);
    _delay_ms(100); // busy-wait delay
    
    // R1 hos CS1 on it
    float adc2_cs1_v = analogRead(ADC2)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    float adc2_cs1_i = adc2_cs1_v / R1;
    printf_P(PSTR("CS1 curr source on R1: %1.3f A\r\n"), adc2_cs1_i);
    if (adc2_cs1_i < 0.018) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS1 curr is to low.\r\n"));
    }
    if (adc2_cs1_i > 0.026) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> CS1 curr is to high.\r\n"));
    }
    float adc1_cs1_v = analogRead(ADC1)*((ref_extern_avcc_uV/1.0E6)/1024.0);
    float red_led_v = adc1_cs1_v - adc2_cs1_v;
    printf_P(PSTR("Red LED fwd V: %1.3f V\r\n"), red_led_v);

    //This is a good place to swap ADC referances and find the band-gap voltage
    init_ADC_single_conversion(INTERNAL_1V1); 
    _delay_ms(100); // busy-wait delay
    int adc2_used_for_ref_intern_1v1_uV = analogRead(ADC2);
    printf_P(PSTR("   ADC2 reading used to calculate ref_intern_1v1_uV: %d A\r\n"), adc2_used_for_ref_intern_1v1_uV);
    float _ref_intern_1v1_uV = 1.0E6*1024.0 * ((adc2_cs1_i * R1) / adc2_used_for_ref_intern_1v1_uV);
    uint32_t temp_ref_intern_1v1_uV = (uint32_t)_ref_intern_1v1_uV;
    printf_P(PSTR("   calculated ref_intern_1v1_uV: %lu uV\r\n"), temp_ref_intern_1v1_uV);
    uint32_t temp_ref_extern_avcc_uV = ref_extern_avcc_uV;
    
    // check for old referance values
    if (LoadAnalogRefFromEEPROM())
    {
        printf_P(PSTR("REF_EXTERN_AVCC old value was in eeprom: %lu uV\r\n"), ref_extern_avcc_uV);
        printf_P(PSTR("REF_INTERN_1V1 old value was in eeprom: %lu uV\r\n"), ref_intern_1v1_uV);
    }
    ref_extern_avcc_uV = temp_ref_extern_avcc_uV;
    ref_intern_1v1_uV = temp_ref_intern_1v1_uV;
    if ((ref_intern_1v1_uV > 1050000UL)  || (ref_intern_1v1_uV < 1150000UL) )
    {
        //while ( !WriteEeReferenceId() ) {};
        //while ( !WriteEeReferenceAvcc() ) {};
        //while ( !WriteEeReference1V1() ) {};
        printf_P(PSTR("REF_EXTERN_AVCC saved in eeprom: %lu uV\r\n"), ref_extern_avcc_uV);
        printf_P(PSTR("REF_INTERN_1V1 saved in eeprom: %lu uV\r\n"), ref_intern_1v1_uV);
    }
    else
    { 
        passing = 0; 
        printf_P(PSTR(">>> REF_* for ADC not saved in eeprom.\r\n"));
    }
    digitalWrite(CS1_EN,LOW);

    float input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CS1_EN and INTERNAL_1V1: %1.3f A\r\n"), input_i);
    if (input_i > 0.045) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Input curr is to high.\r\n"));
        return;
    }
    if (input_i < 0.005) 
    { 
        passing = 0; 
        printf_P(PSTR(">>> Input curr is to low.\r\n"));
        return;
    }

    // enable LED channel 1 
    digitalWrite(CH1,HIGH); // CH1 is defined in ../lib/pins_board.h
    _delay_ms(1000); // busy-wait delay so the 1uF cap has the average input current

    float ch1_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH1 LED, 1V1, 1sec: %1.3f A\r\n"), ch1_input_i);
    
    _delay_ms(2000); // busy-wait delay so the 1uF cap has the average input current
    ch1_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH1 LED, 1V1, 3sec: %1.3f A\r\n"), ch1_input_i);

    // enable LED channel2
    digitalWrite(CH1,LOW);
    digitalWrite(CH2,HIGH);
    _delay_ms(1000);

    float ch2_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH2 LED, 1V1, 1sec: %1.3f A\r\n"), ch2_input_i);
    
    _delay_ms(2000); 
    ch2_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH2 LED, 1V1, 3sec: %1.3f A\r\n"), ch2_input_i);

    // enable LED channel3
    digitalWrite(CH2,LOW);
    digitalWrite(CH3,HIGH);
    _delay_ms(1000);

    float ch3_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH3 LED, 1V1, 1sec: %1.3f A\r\n"), ch3_input_i);
    
    _delay_ms(2000); 
    ch3_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH3 LED, 1V1, 3sec: %1.3f A\r\n"), ch3_input_i);

    // enable LED channel4
    digitalWrite(CH3,LOW);
    digitalWrite(CH4,HIGH);
    _delay_ms(1000);

    float ch4_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH4 LED, 1V1, 1sec: %1.3f A\r\n"), ch4_input_i);
    
    _delay_ms(2000); 
    ch4_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH4 LED, 1V1, 3sec: %1.3f A\r\n"), ch4_input_i);

    // enable LED channel5
    digitalWrite(CH4,LOW);
    digitalWrite(CH5,HIGH);
    _delay_ms(1000);

    float ch5_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH5 LED, 1V1, 1sec: %1.3f A\r\n"), ch5_input_i);
    
    _delay_ms(2000); 
    ch5_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH5 LED, 1V1, 3sec: %1.3f A\r\n"), ch5_input_i);

    // enable LED channel6
    digitalWrite(CH5,LOW);
    digitalWrite(CH6,HIGH);
    _delay_ms(1000);

    float ch6_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH6 LED, 1V1, 1sec: %1.3f A\r\n"), ch6_input_i);
    
    _delay_ms(2000); 
    ch6_input_i = analogRead(PWR_I)*((ref_intern_1v1_uV/1.0E6)/1024.0)/(0.068*50.0);
    printf_P(PSTR("PWR_I with CH6 LED, 1V1, 3sec: %1.3f A\r\n"), ch6_input_i);

    //swap back to the AVCC referance 
    digitalWrite(CH6,LOW);
    init_ADC_single_conversion(EXTERNAL_AVCC); 
    _delay_ms(100); // busy-wait delay
    
    // final test status
    if (passing)
    {
        printf_P(PSTR("[PASS]\r\n"));
    }
    else
    {
        printf_P(PSTR("[FAIL]\r\n"));
    }
    printf_P(PSTR("\r\n\r\n\r\n"));
}

void led_setup_after_test(void)
{
    // Turn Off CS0 Curr Source
    pinMode(CS0_EN,OUTPUT);
    digitalWrite(CS0_EN,LOW);
    
    // Turn Off CS1 Curr Source
    pinMode(CS1_EN,OUTPUT);
    digitalWrite(CS1_EN,LOW);

    if (passing)
    {
        digitalWrite(CS0_EN,LOW);
    }
    else
    {
        digitalWrite(CS1_EN,LOW);
    }
}

void blink(void)
{
    unsigned long kRuntime = millis() - blink_started_at;
    if ( kRuntime > blink_delay)
    {
        if (passing)
        {
            digitalToggle(CS0_EN);
        }
        else
        {
            digitalToggle(CS1_EN);
        }
        
        // next toggle 
        blink_started_at += blink_delay; 
    }
}

int main(void)
{
    setup(); 
    
    passing = 1;
    test();
    led_setup_after_test();
    
    while (1) 
    {
        blink();
    }    
}

