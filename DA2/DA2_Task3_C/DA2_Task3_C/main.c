/*
 * DA2_Task3_C.c
 *
 * Created: 3/6/2023 8:36:47 PM
 * Author : david
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


ISR(INT0_vect) {
	PORTB &= ~(1<<5);	//Turn LED on
	_delay_ms(3500);	//Delay for 3.5 seconds
	PORTB |= (1<<5);	//Turn LED off
}

int main(void)
{
	PORTD |= (1<<2);	//Activate Pull-up resistor for INT0 pin
	
	DDRB |= (1<<5); //Set PB5 as an output, which is connected to LED
	PORTB |= (1<<5); //Initially turn LED off
	
	EIMSK = (1 << INT0); //Enable interrupts on external pin INT0
	EICRA = 0x03; //The rising edge of INT0 generate an interrupt request
	sei();	//Enable interrupts
    
	while (1);
}

