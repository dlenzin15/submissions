/*
 * DA3_Complete.c
 *
 * Created: 3/14/2023 6:29:49 PM
 * Author : david
 */ 

#define F_CPU 16000000UL  //defines clock freq = 16 MHz
#include "util/delay.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define DELAY_TCNT_VALUE 20		//TCNT = ((16 MHz / 256)*(1/3000)) - 1 = 19.83. Rounding up to 20. Results in a delay of 0.320 ms
#define DELAY_COUNTER 3125		// Timer delay is 0.320 ms. 1 second / 0.320 ms = 3125

#define TCNT1L_VALUE 0x06
#define TCNT1H_VALUE 0xFF

#define OCR2A_VALUE 166
#define OVERFLOW_MAX 2001	// (1.333s / 0.666 ms) = 2001.5


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

ISR(TIMER2_COMPA_vect)
{
	static int overflows = 0;	//Counter to track overflows
	overflows++;				//Increment the counter everytime the interrupt is triggered
	if (overflows == OVERFLOW_MAX) {
		PORTB ^= (1<<2);	//Toggle the LED
		overflows = 0;		//Reset the counter
	}
}


int main(void)
{
	//Initialize Ports:
	DDRB |= (1<<4) | (1<<3) | (1<<2);	//Set PB4, PB3, and PB2 to output
	PORTB |= (1<<4) | (1<<3) | (1<<2);	// Turn LEDs off initially
	
	//Initialize Timers
	//Timer 0
	TCCR0A = 0;				// Set timer to Normal mode
	TCCR0B |= (1 << CS02);	// set prescalar to 256
	TCNT0 = 0x00;			//Initialize the TCNT register to start the timer
	
	//Timer 1
	TCCR1B |= (1 << CS11) | (1 << CS10); //Set prescalar to 64. Mode is normal by default
	TCNT1L = TCNT1L_VALUE;
	TCNT1H = TCNT1H_VALUE;	//Load the TCNT1 Register with 0xFF06
	
	//Timer 2
	TCCR2A |= (1 << WGM21);	//Set Timer1 to CTC mode
	TCCR2B |= (1 << CS22);	//Set prescalar to 64
	OCR2A = OCR2A_VALUE;	//Load compare register value
	
	//Initialize interrupts
	TIMSK1 |= (1 << TOIE1);		//Enable Timer1 overflow interrupt
	TIMSK2 |= (1 << OCIE2A);	//Set interrupt on compare match
	sei();						// enable interrupts
	
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

