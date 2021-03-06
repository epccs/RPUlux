/*
AVR asynchronous interrupt-driven TWI/I2C C library 
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

Original 2006 Nicholas Zambetti 
Modified in 2012 by Todd Krein (todd@krein.org) to implement repeated starts
Modified in 2016 by Ronald Sutherland (ronald.sutherlad@gmail) to use as a C library with avr-libc dependency
*/

#include <stdbool.h>
#include <avr/interrupt.h>
#include <compat/twi.h>

#include "twi.h"

static volatile uint8_t twi_state;
static volatile uint8_t twi_slarw;
static volatile uint8_t twi_sendStop;			// should the transaction end with a stop
static volatile uint8_t twi_inRepStart;			// in the middle of a repeated start

// used to initalize the Transmit functions in case they are not used.
void transmit_default(void)
{
    return;
}

typedef void (*PointerToTransmit)(void);

// used to initalize the Receive functions in case they are not used.
void receive_default(uint8_t *rxBuffer, int rxBufferIndex)
{
    // ignore the received data
    // the receive event happens once after an I2C stop or repeated-start
    // that will need the slave to have data ready to transmit
    // repeated-start is usd for atomic operation e.g. prevents others from using bus 
    return;
}

typedef void (*PointerToReceive)(uint8_t*, int);

static PointerToTransmit twi_onSlaveTransmit = transmit_default;
static PointerToReceive twi_onSlaveReceive = receive_default;

static uint8_t twi_masterBuffer[TWI_BUFFER_LENGTH];
static volatile uint8_t twi_masterBufferIndex;
static volatile uint8_t twi_masterBufferLength;

static uint8_t twi_txBuffer[TWI_BUFFER_LENGTH];
static volatile uint8_t twi_txBufferIndex;
static volatile uint8_t twi_txBufferLength;

static uint8_t twi_rxBuffer[TWI_BUFFER_LENGTH];
static volatile uint8_t twi_rxBufferIndex;

static volatile uint8_t twi_error;

/* init twi pins and set bitrate */
void twi_init(uint8_t pull_up)
{
    // initialize state
    twi_state = TWI_READY;
    twi_sendStop = 1;		// default value
    twi_inRepStart = 0;

    // Do not use pull-up for twi pins if the MCU is running at a higher voltage.
    // e.g. if MCU has 5V and others have 3.3V do not use the pull-up. 
    if (pull_up) 
    {
#if defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || \
        defined(__AVR_ATmega168__) || defined(__AVR_ATmega48P__) || \
        defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || \
        defined(__AVR_ATmega328P__) 
        // digitalWrite(SDA, 1) without the function call
        DDRC &= ~(1 << DDC4);  // clear the ddr bit to set as an input
        PORTC |= (1 << PORTC4);  // write a one to the port bit to enable the pull-up

        // digitalWrite(SCL, 1)
        DDRC &= ~(1 << DDC5);
        PORTC |= (1 << PORTC5); 
#else
#error "no I2C definition for MCU available"
#endif
    }

    // initialize twi prescaler and bit rate
    TWSR &= ~((1<<TWPS0));
    TWSR &= ~((1<<TWPS1));
    TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;

    /* twi bit rate formula from atmega128 manual pg 204
    SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR))
    note: TWBR should be 10 or higher for master mode
    It is 72 for a 16mhz Wiring board with 100kHz TWI */

    // enable twi module, acks, and twi interrupt
    TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
}

/* disables twi pins */
void twi_disable(void)
{
    // disable twi module, acks, and twi interrupt
    TWCR &= ~((1<<TWEN) | (1<<TWIE) | (1<<TWEA));

    // deactivate internal pullups for twi.
#if defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega168__) || defined(__AVR_ATmega48P__) || \
    defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328P__) 
    // digitalWrite(SDA, 0) without the function call
    PORTC &= ~(1 << PORTC4);  // clear the port bit to disable the pull-up

    // digitalWrite(SCL, 0)
    PORTC &= ~(1 << PORTC5); 
#else
#error "no I2C definition for MCU available"
#endif
}

/* init slave address and enable interrupt */
void twi_setAddress(uint8_t address)
{
    // set twi slave address (skip over TWGCE bit)
    TWAR = address << 1;
}

/* set twi bit rate */
void twi_setFrequency(uint32_t frequency)
{
    TWBR = ((F_CPU / frequency) - 16) / 2;

    /* twi bit rate formula from atmega128 manual pg 204
    SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR))
    note: TWBR should be 10 or higher for master mode
    It is 72 for a 16mhz Wiring board with 100kHz TWI */
}

/* attempt to become twi master and read bytes from a device
 * Input    address: 7bit i2c device address
 *          data: pointer to byte array
 *          length: number of bytes to read into array
 *          sendStop: Boolean indicating whether to send a stop at the end
 * Output   number of bytes read
 */
uint8_t twi_readFrom(uint8_t address, uint8_t* data, uint8_t length, uint8_t sendStop)
{
    uint8_t i;

    // ensure data will fit into buffer
    if(TWI_BUFFER_LENGTH < length)
    {
        return 0;
    }

    // wait until twi is ready, become master receiver
    while(TWI_READY != twi_state)
    {
        continue;
    }
    twi_state = TWI_MRX;
    twi_sendStop = sendStop;
    // reset error state (0xFF.. no error occured)
    twi_error = 0xFF;

    // initialize buffer iteration vars
    twi_masterBufferIndex = 0;
    twi_masterBufferLength = length-1;  // This is not intuitive, read on...
    // On receive, the previously configured ACK/NACK setting is transmitted in
    // response to the received byte before the interrupt is signalled. 
    // Therefor we must actually set NACK when the _next_ to last byte is
    // received, causing that NACK to be sent in response to receiving the last
    // expected byte of data.

    // build sla+w, slave device address + w bit
    twi_slarw = TW_READ;
    twi_slarw |= address << 1;

    if (true == twi_inRepStart) 
    {
        // if we're in the repeated start state, then we've already sent the start,
        // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
        // We need to remove ourselves from the repeated start state before we enable interrupts,
        // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
        // up. Also, don't enable the START interrupt. There may be one pending from the 
        // repeated start that we sent ourselves, and that would really confuse things.
        twi_inRepStart = false;			// remember, we're dealing with an ASYNC ISR
        do 
        {
            TWDR = twi_slarw;
        } while(TWCR & (1<<TWWC));
        TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);	// enable INTs, but not START
    }
    else
    {
        // send start condition
        TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWEA) | (1<<TWINT) | (1<<TWSTA);
    }
        
    // wait for read operation to complete
    while(TWI_MRX == twi_state)
    {
        continue;
    }

    if (twi_masterBufferIndex < length)
    {
        length = twi_masterBufferIndex;
    }

    // copy twi buffer to data
    for(i = 0; i < length; ++i)
    {
        data[i] = twi_masterBuffer[i];
    }
	
    return length;
}

/* attempt to become twi bus master and write bytes to a device
 * Input    address: 7bit i2c device address
 *          data: pointer to byte array
 *          length: number of bytes in array
 *          wait: boolean indicating to wait for write or not
 *          sendStop: boolean indicating whether or not to send a stop at the end
 * Output   0 .. success
 *          1 .. length to long for buffer
 *          2 .. address send, NACK received
 *          3 .. data send, NACK received
 *          4 .. other twi error (lost bus arbitration, bus error, ..)
 */
uint8_t twi_writeTo(uint8_t address, uint8_t* data, uint8_t length, uint8_t wait, uint8_t sendStop)
{
    uint8_t i;

    // ensure data will fit into buffer
    if(TWI_BUFFER_LENGTH < length)
    {
        return 1;
    }

    // wait until twi is ready, become master transmitter
    while(TWI_READY != twi_state)
    {
        continue;
    }
    twi_state = TWI_MTX;
    twi_sendStop = sendStop;
    // reset error state (0xFF.. no error occured)
    twi_error = 0xFF;

    // initialize buffer iteration vars
    twi_masterBufferIndex = 0;
    twi_masterBufferLength = length;
  
    // copy data to twi buffer
    for(i = 0; i < length; ++i)
    {
        twi_masterBuffer[i] = data[i];
    }
  
    // build sla+w, slave device address + w bit
    twi_slarw = TW_WRITE;
    twi_slarw |= address << 1;
  
    // if we're in a repeated start, then we've already sent the START
    // in the ISR. Don't do it again.
    //
    if (true == twi_inRepStart) {
        // if we're in the repeated start state, then we've already sent the start,
        // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
        // We need to remove ourselves from the repeated start state before we enable interrupts,
        // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
        // up. Also, don't enable the START interrupt. There may be one pending from the 
        // repeated start that we sent outselves, and that would really confuse things.
        twi_inRepStart = false;			// remember, we're dealing with an ASYNC ISR
        do 
        {
            TWDR = twi_slarw;				
        } while(TWCR & (1<<TWWC));
        TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);	// enable INTs, but not START
    }
    else
    {
        // send start condition
        TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE) | (1<<TWSTA);	// enable INTs
    }

    // wait for write operation to complete
    while(wait && (TWI_MTX == twi_state))
    {
        continue;
    }
  
    if (twi_error == 0xFF)
        return 0;	// success
    else if (twi_error == TW_MT_SLA_NACK)
        return 2;	// error: address send, nack received
    else if (twi_error == TW_MT_DATA_NACK)
        return 3;	// error: data send, nack received
    else
        return 4;	// other twi error
}

/*  fill slave tx buffer with data that will be used by slave tx event callback
 * Input    data: pointer to byte array
 *          length: number of bytes in array
 * Output   1 length too long for buffer
 *          2 not slave transmitter
 *          0 ok
 */
uint8_t twi_transmit(const uint8_t* data, uint8_t length)
{
    uint8_t i;

    // ensure data will fit into buffer
    if(TWI_BUFFER_LENGTH < length)
    {
        return 1;
    }
  
    // ensure we are currently a slave transmitter
    if(TWI_STX != twi_state)
    {
        return 2;
    }
  
    // set length and copy data into tx buffer
    twi_txBufferLength = length;
    for(i = 0; i < length; ++i)
    {
        twi_txBuffer[i] = data[i];
    }
  
    return 0;
}

/* set function called durring a slave read operation
 * Input    function: callback function to use
 */
void twi_attachSlaveRxEvent( void (*function)(uint8_t*, int) )
{
    twi_onSlaveReceive = function;
}

/* sets function called before a slave write operation
 * Input    function: callback function to use
 */
void twi_attachSlaveTxEvent( void (*function)(void) )
{
    twi_onSlaveTransmit = function;
}

/* send byte or ready to receive signal
 * Input    ack: byte indicating to ack or to nack
 */
void twi_reply(uint8_t ack)
{
    // transmit master read ready signal, with or without ack
    if(ack)
    {
        TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA);
    }
    else
    {
        TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT);
    }
}

/* relinquishe bus master status */
void twi_stop(void)
{
    // send stop condition
    TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWEA) | (1<<TWINT) | (1<<TWSTO);

    // wait for stop condition to be exectued on bus
    // TWINT is not set after a stop condition!
    while(TWCR & (1<<TWSTO))
    {
        continue;
    }

    // update twi state
    twi_state = TWI_READY;
}

/* release bus */
void twi_releaseBus(void)
{
    // release bus
    TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWEA) | (1<<TWINT);

    // update twi state
    twi_state = TWI_READY;
}

ISR(TWI_vect)
{
    switch(TW_STATUS)
    {
        // All Master
        case TW_START:     // sent start condition
        case TW_REP_START: // sent repeated start condition
            // copy device address and r/w bit to output register and ack
            TWDR = twi_slarw;
            twi_reply(1);
            break;

        // Master Transmitter
        case TW_MT_SLA_ACK:  // slave receiver acked address
        case TW_MT_DATA_ACK: // slave receiver acked data
            // if there is data to send, send it, otherwise stop 
            if(twi_masterBufferIndex < twi_masterBufferLength)
            {
                // copy data to output register and ack
                TWDR = twi_masterBuffer[twi_masterBufferIndex++];
                twi_reply(1);
            }
            else
            {
                if (twi_sendStop)
                    twi_stop();
                else 
                {
                    twi_inRepStart = true;	// we're gonna send the START
                    // don't enable the interrupt. We'll generate the start, but we 
                    // avoid handling the interrupt until we're in the next transaction,
                    // at the point where we would normally issue the start.
                    TWCR = (1<<TWINT) | (1<<TWSTA)| (1<<TWEN) ;
                    twi_state = TWI_READY;
                }
            }
            break;
            
        case TW_MT_SLA_NACK:  // address sent, nack received
            twi_error = TW_MT_SLA_NACK;
            twi_stop();
            break;
        
        case TW_MT_DATA_NACK: // data sent, nack received
            twi_error = TW_MT_DATA_NACK;
            twi_stop();
            break;
        
        case TW_MT_ARB_LOST: // lost bus arbitration
            twi_error = TW_MT_ARB_LOST;
            twi_releaseBus();
            break;

        // Master Receiver
        case TW_MR_DATA_ACK: // data received, ack sent
            // put byte into buffer
            twi_masterBuffer[twi_masterBufferIndex++] = TWDR;
        case TW_MR_SLA_ACK:  // address sent, ack received
            // ack if more bytes are expected, otherwise nack
            if(twi_masterBufferIndex < twi_masterBufferLength)
            {
                twi_reply(1);
            }
            else
            {
                twi_reply(0);
            }
            break;
            
        case TW_MR_DATA_NACK: // data received, nack sent
            // put final byte into buffer
            twi_masterBuffer[twi_masterBufferIndex++] = TWDR;
            if (twi_sendStop)
                twi_stop();
            else 
            {
                twi_inRepStart = true;	// we're gonna send the START
                // don't enable the interrupt. We'll generate the start, but we 
                // avoid handling the interrupt until we're in the next transaction,
                // at the point where we would normally issue the start.
                TWCR = (1<<TWINT) | (1<<TWSTA)| (1<<TWEN) ;
                twi_state = TWI_READY;
            }    
            break;
            
        case TW_MR_SLA_NACK: // address sent, nack received
            twi_stop();
            break;
        
        // TW_MR_ARB_LOST handled by TW_MT_ARB_LOST case

        // Slave Receiver
        case TW_SR_SLA_ACK:   // addressed, returned ack
        case TW_SR_GCALL_ACK: // addressed generally, returned ack
        case TW_SR_ARB_LOST_SLA_ACK:   // lost arbitration, returned ack
        case TW_SR_ARB_LOST_GCALL_ACK: // lost arbitration, returned ack
            // enter slave receiver mode
            twi_state = TWI_SRX;
            // indicate that rx buffer can be overwritten and ack
            twi_rxBufferIndex = 0;
            twi_reply(1);
            break;
        
        case TW_SR_DATA_ACK:       // data received, returned ack
        case TW_SR_GCALL_DATA_ACK: // data received generally, returned ack
            // if there is still room in the rx buffer
            if(twi_rxBufferIndex < TWI_BUFFER_LENGTH)
            {
                // put byte in buffer and ack
                twi_rxBuffer[twi_rxBufferIndex++] = TWDR;
                twi_reply(1);
            }
            else
            {
                // otherwise nack
                twi_reply(0);
            }
            break;

        case TW_SR_STOP: // stop or repeated start condition received
            // ack future responses and leave slave receiver state
            twi_releaseBus();
            // put a null char after data if there's room
            if(twi_rxBufferIndex < TWI_BUFFER_LENGTH)
            {
                twi_rxBuffer[twi_rxBufferIndex] = '\0';
            }
            // callback to user defined callback
            twi_onSlaveReceive(twi_rxBuffer, twi_rxBufferIndex);
            // since we submit rx buffer to "wire" library, we can reset it
            twi_rxBufferIndex = 0;
            break;

        case TW_SR_DATA_NACK:       // data received, returned nack
        case TW_SR_GCALL_DATA_NACK: // data received generally, returned nack
            // nack back at master
            twi_reply(0);
            break;
        
        // Slave Transmitter
        case TW_ST_SLA_ACK:          // addressed, returned ack
        case TW_ST_ARB_LOST_SLA_ACK: // arbitration lost, returned ack
            // enter slave transmitter mode
            twi_state = TWI_STX;
            // ready the tx buffer index for iteration
            twi_txBufferIndex = 0;
            // set tx buffer length to be zero, to verify if user changes it
            twi_txBufferLength = 0;
            // request for txBuffer to be filled and length to be set
            // note: user must call twi_transmit(bytes, length) to do this
            twi_onSlaveTransmit();
            // if they didn't change buffer & length, initialize it
            if(0 == twi_txBufferLength)
            {
                twi_txBufferLength = 1;
                twi_txBuffer[0] = 0x00;
            }
            // transmit first byte from buffer, fall
        case TW_ST_DATA_ACK: // byte sent, ack returned
            // copy data to output register
            TWDR = twi_txBuffer[twi_txBufferIndex++];
            // if there is more to send, ack, otherwise nack
            if(twi_txBufferIndex < twi_txBufferLength)
            {
                twi_reply(1);
            }
            else
            {
                twi_reply(0);
            }
            break;
            
        case TW_ST_DATA_NACK: // received nack, we are done 
        case TW_ST_LAST_DATA: // received ack, but we are done already!
            // ack future responses
            twi_reply(1);
            // leave slave receiver state
            twi_state = TWI_READY;
            break;

        // All
        case TW_NO_INFO:   // no state information
            break;
        
        case TW_BUS_ERROR: // bus error, illegal stop/start
            twi_error = TW_BUS_ERROR;
            twi_stop();
            break;
    }
}

