/*
 * DA2_Task1_C.c
 *
 * Created: 3/3/2023 1:36:28 PM
 * Author : david
 */ 

#define F_CPU 16000000UL  //defines clock freq = 16 MHz
#include "util/delay.h"
#include <avr/io.h>
#include <avr/interrupt.h>

int main(void)
{
	DDRB = (1<<5); //Set PB5 as an output
    while (1) {
		PORTB ^= (1<<5); //Toggle LED connected to pin 5
		_delay_ms(175);
	}
	
	return 0;
}

