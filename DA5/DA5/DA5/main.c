/*
 * DA5.c
 *
 * Created: 4/12/2023 10:31:35 AM
 * Author : david
 */ 

// Definitions
#define F_CPU 16000000UL			
#define TRIGGER_PIN PB2
#define ECHO_PIN PB0
#define CONTROL_PIN PB1
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

// Included Files
#include <avr/io.h>		
#include <util/delay.h>						
#include <stdio.h>
#include <avr/interrupt.h>

// Function Declarations
void UART_init();
void timer_init();
void UART_transmit_string();
void Wait();
uint32_t calculateDistance();


void UART_init(unsigned int ubrr)
{
	//Set baud rate
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	
	//Enable transmitter and receiver and receiver interrupt
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	
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

void timer_init() 
{
	//Configure TIMER1
	TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);        //NON Inverted PWM
	TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10); //PRESCALER=64 MODE 14(FAST PWM)

	ICR1=4999;  //fPWM=50Hz (Period = 20ms Standard).
}

//Simple Wait Function
void Wait()
{
	uint8_t i;
	for(i=0;i<2;i++)
	{
		_delay_loop_2(0);
	}

}

uint32_t calculateDistance()
{
	PORTB &= (~(1<<TRIGGER_PIN));
	_delay_us(2);	// Pull trigger low before pulse
	
	/* Give 10 ms trigger pulse on trig. pin to HC-SR04 */
	PORTB |= (1<<TRIGGER_PIN);
	_delay_ms(10);
	PORTB &= (~(1<<TRIGGER_PIN));
		
	// Measure duration of pulse on echoPin
	unsigned long duration = 0;
	while (!(PINB & (1 << ECHO_PIN))); // Wait for echo to go high
	while ((PINB & (1 << ECHO_PIN))) 
		duration++; // Measure pulse width
	
	uint32_t distance= (uint32_t)duration*0.034/2;
		
	return distance;
}

void main()
{
	// Set data directions
	DDRB|=(1<<CONTROL_PIN) | (1<<TRIGGER_PIN);   //PWM Pins as Out
	PORTD &=  ~(1<<TRIGGER_PIN); // Pull down
	DDRB &= ~(1<<ECHO_PIN); // Set trigger pin to input
	
	timer_init();
	UART_init(MYUBRR);
	
	int angle = 0;
	OCR1A = 97;		// Initialize motor to 0 degrees
	
	while(1)
	{
		uint32_t distance = 0;
		char buffer[100];
		sprintf(buffer, "Distance = %d cm\n", distance);
		UART_transmit_string(buffer);
		
		while (OCR1A < 535)
		{
			OCR1A += 5; // Increment every 2 degrees until we reach 180 degrees
			angle += 2;
			Wait();
			distance = calculateDistance();
			sprintf(buffer, "%d,%d.", angle, distance);
			UART_transmit_string(buffer);
		}
		
		OCR1A = 535; // 180 degrees
		angle = 180;
		
		while (OCR1A > 97)
		{
			OCR1A -= 5; // decrement every 2 degrees until we reach 0 degrees
			angle -= 2;
			Wait();
			distance = calculateDistance();
			sprintf(buffer, "%d,%d.", angle, distance);
			UART_transmit_string(buffer);
		}
		OCR1A = 97; // 0 degrees
	}
}

