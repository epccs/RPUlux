/*
  Initialize ATmega328p Timers  
  Copyright (c) 2016 Ronald S,. Sutherland

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/* 328pb issue with timer 3 and PWM: https://www.avrfreaks.net/forum/atmega328pb-timer-34-output-compare-pwm-issue
when you configure Timer 1 and Timer 3 identically for both channels with all the 
correct DDR bits set for output, you only get a PWM waveform on the first three 
output pins, OC1A, OC1B, and OC3A.  OC3B will only have a continous, low output.
To get the OC3B output to work correctly set the portD bit 2 pin value high: e.g., "PORTD |= 4;"
Timers OC3B or OC4B are used to accomplish a modulated signal, but that [hack] means PortD bit 2 
is used to control the combination of the two timer outputs.
*/

#include <avr/interrupt.h>
#include "timers.h"

// the prescaler is set so that Timer0 ticks every 64 clock cycles. 
// The overflow handler for fast pwm is called every 256 ticks, thus freq = 16MHz/64/256 = 976.5625Hz. 
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))

// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)

// fractional number of milliseconds per timer0 overflow. 
// shift right by three to fit number into a byte. 
// phase-correct should not lose precision at 16 MHz.
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

ISR(TIMER0_OVF_vect)
{
    // copy these to local variables so they can be stored in registers
    // (volatile variables must be read from memory on every access)
    unsigned long m = timer0_millis;
    unsigned char f = timer0_fract;

    m += MILLIS_INC;
    f += FRACT_INC;
    if (f >= FRACT_MAX) {
        f -= FRACT_MAX;
        m += 1;
    }

    timer0_fract = f;
    timer0_millis = m;
    timer0_overflow_count++;
}

unsigned long millis()
{
    unsigned long m;
    uint8_t oldSREG = SREG;

    // disable interrupts or value may be inconsistent
    cli();
    m = timer0_millis;
    SREG = oldSREG;

    return m;
}

// note phase-correct mode counts up and then down,  this only works with fast mode
unsigned long micros() {
    unsigned long m;
    uint8_t oldSREG = SREG;
    uint8_t t;
    
    cli(); 
    m = timer0_overflow_count;

    // copy of TCNT0 which gives direct access to read/write Timer0
    t = TCNT0;

    // TIFR0 is the Timer0 interrupt flag register, and TOV0 is the overflow flag in it.
    if ((TIFR0 & (1<<TOV0)) && (t < 255))
        m++;

    SREG = oldSREG;
    
    return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
}

// Prescale factor is 64
// Timer0 is in fast pwm mode.
// Timer1, Timser2 are in phase-correct mode.
// phase-correct is better for motors since it distributes the switching events and thus reduces the chance of sustained excitation of a resonance mode.
void initTimers()
{
    // Set Timer0 waveform generation mode for fast pwm
    TCCR0A = 0;
    TCCR0A |= (1<<WGM01);
    TCCR0A |= (1<<WGM00);
    
    // Set Timer0 prescale factor to 64
    TCCR0B = 0;  // OC0A/OC0B disconnected
    TCCR0B |= (1<<CS01);
    TCCR0B |= (1<<CS00);

    // enable Timer0 overflow interrupt
    TIMSK0 |= (1<<TOIE0);

    // Set Timer1 waveform generation mode for 8-bit phase correct
    TCCR1A = 0; // OC1A/OC1B disconnected
    TCCR1A |= (1<<WGM10);

    // set Timer1 prescale factor to 64 
    TCCR1B = 0;
    TCCR1B |= (1<<CS11); // what?
    TCCR1B |= (1<<CS10); // look at the ATmega328p datasheet!

    // configure timer 2 for phase correct pwm
    TCCR2A =0; // OC2A/OC2B disconnected
    TCCR2A |= (1<<WGM20);
    
    // Set Timer2 prescale factor to 64
    TCCR2B = 0; 
    TCCR2B |= (1<<CS22);
}
