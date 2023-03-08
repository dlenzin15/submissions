/*
 * DA2_Task2_C.c
 *
 * Created: 3/6/2023 7:29:13 PM
 * Author : david
 */ 

#define F_CPU 16000000UL 
#include <avr/io.h>
#include <util/delay.h>


int main(void)
{
    DDRC &= ~(1<<1); //Set PC5 to an input, which is connected to switch 1
	PORTC |= (1<<1);	//Pull-up resistor
	
	DDRB |= (1<<5); //Set PB5 as an output, which is connected to LED
	PORTB |= (1<<5); //Initially turn LED off
    while (1) 
    {
		if (!(PINC & (1 << 1))) {	//If switch is pressed
			PORTB &= ~(1 << 5);	//Turn on LED
			_delay_ms(1750);	//Delay for 1.75 seconds
			PORTB |= (1 << 5);	//Turn off the LED
		}
		PORTB |= (1 << 5);	//Keep the LED to off if button not pressed
    }
}

