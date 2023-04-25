/*
 * DA6
 * David Lenzin, 2001654470
 */ 

#define F_CPU 16000000UL					// Define F_CPU to 16 MHz
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>							
#include <avr/interrupt.h>
#include <stdio.h>							
#include <util/delay.h>						

volatile uint8_t Direction = 0;

// Functions:
void ADC_Init(void);
int ADC_Read(char);
ISR(INT0_vect);

//UART functions for debugging
void UART_init(unsigned int);
void UART_transmit_string(char *);
 

void ADC_Init()								// ADC Initialization function 
{
	DDRC = 0x00;							// Make ADC port as input 
	ADCSRA = 0x87;							// Enable ADC, with freq/128  
	ADMUX = 0x40;							// Vref: Avcc, ADC channel: 0
}

int ADC_Read(char channel)					// ADC Read function 
{
	ADMUX = 0x40 | (channel & 0x07);		// set input channel to read
	ADCSRA |= (1<<ADSC);					// Start ADC conversion
	while (!(ADCSRA & (1<<ADIF)));			// Wait until end of conversion by polling ADC interrupt flag
	ADCSRA |= (1<<ADIF);					// Clear interrupt flag
	_delay_us(1);							// Wait a little bit
	return ADCW;							// Return ADC word
}

ISR(INT0_vect)
{
	TCCR0B |= (0<<CS00)|(0<<CS01);			// Set Fast PWM with Fosc/64 Timer0 clock
	_delay_us(5000);						// Software de-bouncing control delay 
	TCCR0B |= (1<<CS00)|(1<<CS01);			// Set Fast PWM with Fosc/64 Timer0 clock

}

void UART_init(unsigned int ubrr)
{
	//Set baud rate
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	
	//Enable transmitter and receiver and reciever interrupt
	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
	
	//Set frame format: 8 bits data, 1 stop bit
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
	
	sei();
}

void UART_transmit_string(char *data) {
	while ((*data != '\0')) {	// Check if NULL char
		while (!(UCSR0A & (1 <<UDRE0)));	// Wait for register to be
		UDR0 = *data;	// Store data in the data register
		data++;
	}
}

int main(void)
{
	DDRD &= ~(1<<PD2);					// Make INT0 pin as Input
	PORTD |= (1 << PD2);				// turn On the Pull-up
	DDRD |= (1<<PD6) | (1<<PD5) | (1<<PD4) | (1<<PD1); // Set AIN2, AIN1, STBY to outputs
	PORTD &= ~(1 << PD5); //set AIN2 low
	PORTD |= (1 << PB4) | (1 << PD1); //set AIN1 and STBY high	
	
	EICRA |= (1 << ISC01);  // set INT0 to trigger to falling edge
	EIMSK |= (1 << INT0);   // Turns on INT0
	sei();					// Enable Global Interrupt
	
	ADC_Init();				// Initialize ADC
	UART_init(MYUBRR);
	TCNT0 = 0;				// Set timer0 count zero
	TCCR0A |= (1<<WGM00)|(1<<WGM01)|(1<<COM0A1);
	TCCR0B |= (1<<CS00)|(1<<CS01);			// Set Fast PWM with Fosc/64 Timer0 clock 
	
	while(1)
	{
		OCR0A = (ADC_Read(0)/4);			// Read ADC and map it into 0-255 to write in OCR0 register 
		
		// Transmit to UART for debugging
		char buffer[100];		//Buffer to read ADC
		sprintf(buffer, "%d mV\r\n", OCR0A);	//Read the adc value into the buffer
		UART_transmit_string(buffer);	//Send the adc value to the terminal
		_delay_ms(100);	//Delay for 0.10 seconds
	}
}