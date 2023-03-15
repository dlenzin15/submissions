/*
 * GccApplication1.c
 *
 * Created: 3/10/2023 3:26:15 PM
 * Author : david
 */ 

#define F_CPU 16000000UL  //defines clock freq = 16 MHz
#include "util/delay.h"
#include <avr/io.h>

#define DELAY_TCNT_VALUE 20		//TCNT = ((16 MHz / 256)*(1/3000)) - 1 = 19.83. Rounding up to 20. Results in a delay of 0.320 ms
#define DELAY_COUNTER 3125		// Timer delay is 0.320 ms. 1 second / 0.320 ms = 3125

int main(void)
{
	DDRB |= (1<<4);			//Set PB4 to an output
	PORTB |= (1<<4);		// Turn LED off initially
	TCCR0A = 0;				// Set timer to Normal mode
	TCCR0B |= (1 << CS02);	// set prescalar to 256
	TCNT0 = 0x00;			//Initialize the TCNT register to start the timer
    
	int overflows = 0;		//Counter to track the how many times the timer overflows
	while (1) 
    {
		if (TCNT0 == DELAY_TCNT_VALUE) {	
			overflows++;	//If the TCNT has overflown, increment the counter
			if (overflows >= DELAY_COUNTER) {	//If the timer has overflown enough times, reset the counter and toggle the LED
				overflows = 0;		
				PORTB ^= (1<<4);
			}
			TCNT0 = 0;			//Reset the TCNT register
		}
    }
	return 0;
}

