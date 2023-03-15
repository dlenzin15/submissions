/*
 * DA3_Task1B.c
 *
 * Created: 3/10/2023 5:15:19 PM
 * Author : david
 */ 

#define F_CPU 16000000UL  //defines clock freq = 16 MHz
#include "util/delay.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/************************ Calculations **************************************/
/* Top value: ((16 MHz / 64) * 0.999 ms) - 1 = 248.75. Rounding up to 249	*/
/* TCNT1 Value: (2^16 - 1) = 65535. 65535 - 249 = 65286, which is 0xFF06	*/
/****************************************************************************/

#define TCNT1L_VALUE 0x06
#define TCNT1H_VALUE 0xFF

ISR(TIMER1_OVF_vect)
{
	static int overflows = 0;	//Counter to track to overflows
	overflows++;
	if (overflows >= 1000)		//1000 overflows is approximately 1 second
	{
		PORTB ^= (1<<3);		//Toggle the LED
		overflows = 0;			//Reset counter
	}
	
	TCNT1L = TCNT1L_VALUE;
	TCNT1H = TCNT1H_VALUE;	//Load the TCNT1 Register with 0xFF06 
		
}

int main(void)
{
	DDRB |= (1<<3);		//Set PB5 as an output
	PORTB |= (1<<3);	//Turn off LED initially
	
	TCCR1B |= (1 << CS11) | (1 << CS10); //Set prescalar to 64. Mode is normal by default
	TCNT1L = TCNT1L_VALUE;	
	TCNT1H = TCNT1H_VALUE;	//Load the TCNT1 Register with 0xFF06
	
	TIMSK1 |= (1 << TOIE1); //Enable Timer1 overflow interrupt
	
	sei();	//Enable global interrupts
	
    while (1);	//Wait for interrupts
	
	return 0; 
}

