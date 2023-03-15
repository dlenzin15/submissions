/*
 * DA3_Task1C.c
 *
 * Created: 3/10/2023 6:07:02 PM
 * Author : david
 */ 

#define F_CPU 16000000UL  //defines clock freq = 16 MHz
#include "util/delay.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/*******************Calculations:******************************/
/* 2/3000 = 0.666 ms. Target frequency = 3000/2 = 1500 Hz     */
/* OCR2A = (16 MHz / (2*64*1500)) = 83.3 Rounding down to 83  */
/*															  */
/*	However, this generates a pulse every 0.333 ms to make	  */
/*	a period of 0.666 ms.									  */
/*		                                                      */	
/* 83*2 = 166.												  */
/**************************************************************/

#define OCR2A_VALUE 166
#define OVERFLOW_MAX 2001	// (1.333s / 0.666 ms) = 2001.5

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
	DDRB |= (1<<2);			//Set PB2 to an output
	PORTB |= (1<<2);		//Initially turn LED off
	
	TCCR2A |= (1 << WGM21);	//Set Timer1 to CTC mode
	TCCR2B |= (1 << CS22);	//Set prescalar to 64
	OCR2A = OCR2A_VALUE;	//Load compare register value
	
	TIMSK2 |= (1 << OCIE2A);	//Set interrupt on compare match
	sei();						// enable interrupts

    while (1);					//Wait for interrupts
	return 0;	
}

