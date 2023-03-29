/*
 * DA4.c
 *
 * Created: 3/27/2023 10:52:05 AM
 * Author : David Lenzin, 2001654470
 */ 

/*******************************************************************/
/* Calculations for blink timer:								   */
/* Desired blinking time is 1 kHz, and 50% duty cycle			   */
/* OCR0A_VALUE = (16 MHz / (2*64*1000))-1 = 124.				   */
/* 1 kHz = 1 ms. 1s = 1000 ms. OVERFLOW_MAX = 1000 for a 1s period */
/*******************************************************************/

//Macros
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define OCR0A_VALUE 124
#define OVERFLOW_MAX 1000
#define F_CPU 16000000UL	//Define clock speed at 8 MHz

// Included Files
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Global Variables
unsigned int blink_enable = 0;		// Variable to enable the blinking LED at PB3
volatile unsigned int adc_temp = 0;	// Variable to track the ADC values

//Function declarations:
void UART_init(unsigned int);
void timer_init(void);
void led_init(void);
void adc_init(void);
void turn_off_led(void);
void turn_on_led(void);
void help_menu(void);
void UART_transmit_string(char *);
void read_adc(void);

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

void timer_init(void)
{
	TCCR0A |= (1<<WGM01);		//Set Timer1 to CTC mode
	TCCR0B |= (1<<CS01) | (1<<CS00);	//Set prescaler to 1024
	OCR0A = 124;				//Load compare register value
	TIMSK0 |= (1 << OCIE0A);	//Set interrupt on compare match
}

void led_init(void)
{
	DDRB |= (1<<3) | (1<<5);	//Set PB5 and PB3 to outputs for LED
	PORTB |= (1<<3) | (1<<5);	//Turn off LEDs initially
}

void adc_init(void)
{
	// AVcc with external capacitor on AREF pin. Right adjusted result. ADC0 selected for PC0
	ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (0<<MUX2) | (0<<MUX1) | (0<<MUX0);
	
	// Set the prescaler to 32. Don't enable the ADC until it is needed
	ADCSRA = (0<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (1<<ADPS0);
}

void turn_on_led(void)
{
	PORTB &= ~(1<<5); //Turn on PB5 LED
}

void turn_off_led(void)
{
	PORTB |= (1<<5); //Turn off PB5 LED
}

void help_menu(void)
{
	UART_transmit_string("***********************************DA4 Help Screen:***********************************\n");
	UART_transmit_string("'h'\t Displays a help screen to lists all keys and functionalities\n");
	UART_transmit_string("'o'\t Turns on LED connected to PB5\n");
	UART_transmit_string("'O'\t Turns off LED connected to PB5\n");
	UART_transmit_string("'b'\t Blinks the LED connected to PB3 continuously every second\n");
	UART_transmit_string("'P'\t Turns off the LED connected to PB3\n");
	UART_transmit_string("'a'\t Reads and displays the ADC value from the potentiometer connected to AC0/PC0\n");
	UART_transmit_string("\t ^ Please note that the ADC must be stopped before taking other inputs\n");
	UART_transmit_string("'A'\t Stops reading the ADC value from the potentiometer connected to AC0/PC0\n");
	UART_transmit_string("**************************************************************************************\n\n");
}

ISR(USART0_RX_vect) 
{
	// Get data from the USART data register
	unsigned char data = UDR0;
	switch(data)
	{
		case 'h':
			if (!(ADCSRA & (1 << ADEN)))	// Break if ADC is being read
				help_menu();
			break;
		case 'o':
			if (!(ADCSRA & (1 << ADEN)))	// Break if ADC is being read
				turn_on_led();
			break;
		case 'O':
			if (!(ADCSRA & (1 << ADEN)))	// Break if ADC is being read
				turn_off_led();
			break;
		case 'b':
			blink_enable = 1;
			break;
		case 'P':
			if (!(ADCSRA & (1 << ADEN)))	// Break if ADC is being read
				blink_enable = 0;
			PORTB |= (1<<3); //Turn LED off
			break;
		case 'a':
			ADCSRA |= (1<<ADEN);	// Enable the ADC to start reading potentiometer
			break;
		case 'A':
			ADCSRA &= ~(1<<ADEN);	// Disable the ADC to stop reading potentiometer
			break;
		default:
			break;
	}
}

ISR(TIMER0_COMPA_vect) 
{
	static int overflows = 0;	// Variable to track timer overflows
	if (blink_enable == 1){		// Check if blink has been enabled
		overflows++;			// Increment overflows until we reach the value that gives us 1 second
		if (overflows == OVERFLOW_MAX) {	
			PORTB ^= (1<<3);	//Toggle the PB3 LED
			overflows = 0;
		}
	}
}	

void UART_transmit_string(char *data) {
	while ((*data != '\0')) {	// Check if NULL char
		while (!(UCSR0A & (1 <<UDRE0)));	// Wait for register to be 
		UDR0 = *data;	// Store data in the data register
		data++;
	}
}

void read_adc(void)
{
	ADCSRA |= (1<<ADSC); // Start a conversion
	adc_temp = ADC;		// Read the ADC value into the temp variable to be used later
}


int main(void)
{
	UART_init(MYUBRR);	//Initialize USART
	UART_transmit_string("Connected!\n");	//Welcome message
	timer_init();		//Initialize Timer
	led_init();			//Initialize LEDs
	adc_init();			//Initialize ADC
	help_menu();		//Print the help menu upon startup/reboot
    
	while (1) {
		if (ADCSRA & (1 << ADEN))	//Check if ADC has been enabled
		{
			char buffer[8];		//8-bit buffer to read ADC
			read_adc();
			snprintf(buffer, sizeof(buffer), "%d\r\n", adc_temp);	//Read the adc value into the buffer
			UART_transmit_string(buffer);	//Send the adc value to the terminal
			_delay_ms(100);	//Delay for 0.10 seconds
		}
	}
	return 0;
}

