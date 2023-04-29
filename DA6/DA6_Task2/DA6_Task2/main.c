/*
 * DA6_Task2.c
 *
 * Created: 4/25/2023 7:38:26 PM
 * Author : david
 */ 

#define F_CPU 16000000UL					// Define F_CPU to 16 MHz
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define PERIOD 1/F_CPU

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <util/delay.h>

// capture Flag
volatile uint8_t Flag;
volatile uint8_t Direction = 0;
volatile uint32_t revTickAvg;

volatile uint32_t revTick; // Ticks per revolution
volatile uint32_t revCtr;  // Total elapsed revolutions
volatile uint32_t T1Ovs2;  // Overflows for small rotations

void ADC_Init() /* ADC Initialization function */
{
	DDRC = 0x00;   /* Make ADC port as input */
	ADCSRA = 0x87; /* Enable ADC, with freq/128  */
	ADMUX = 0x40;  /* Vref: Avcc, ADC channel: 0 */
}

int ADC_Read(char channel) /* ADC Read function */
{
	ADMUX = 0x40 | (channel & 0x07); /* set input channel to read */
	ADCSRA |= (1 << ADSC);           /* Start ADC conversion */
	while (!(ADCSRA & (1 << ADIF))); /* Wait until end of conversion by polling ADC interrupt flag */
	ADCSRA |= (1 << ADIF); /* Clear interrupt flag */
	_delay_us(1);          /* Wait a little bit */
	return ADCW;           /* Return ADC word */
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
	
}

void UART_transmit_string(char *data) {
	while ((*data != '\0')) {	// Check if NULL char
		while (!(UCSR0A & (1 <<UDRE0)));	// Wait for register to be
		UDR0 = *data;	// Store data in the data register
		data++;
	}
}

ISR(INT0_vect)
{
	TCCR0B |= (0<<CS00)|(0<<CS01);			// Set Fast PWM with Fosc/64 Timer0 clock
	_delay_us(5000);						// Software de-bouncing control delay
	TCCR0B |= (1<<CS00)|(1<<CS01);			// Set Fast PWM with Fosc/64 Timer0 clock
}

// Initialize timer
void InitTimer3(void) {
	// Set PE2 as input
	DDRE &= ~(1 << DDE2);
	PORTE |= (1 << DDE2);

	// Set Initial Timer value
	TCNT3 = 0;
	
	////First capture on rising edge
	TCCR3A = 0;
	TCCR3B = (0 << ICNC3) | (1 << ICES3);
	TCCR3C = 0;
	
	// Interrupt setup
	// ICIE3: Input capture
	// TOIE3: Timer1 overflow
	TIFR3 = (1 << ICF3) | (1 << TOV3);    // clear pending
	TIMSK3 = (1 << ICIE3) | (1 << TOIE3); // and enable
}

void StartTimer3(void) {
	// Start timer without pre-scaler
	TCCR3B |= (1 << CS30);
}

volatile uint32_t tickv, ticks;

// capture ISR
ISR(TIMER3_CAPT_vect) {
	tickv = ICR3; // save duration of last revolution
	revTickAvg = (uint32_t)tickv + ((uint32_t)T1Ovs2 * 0x10000L);
	revCtr++;  // add to revolution count
	TCNT3 = 0; // restart timer for next revolution
	T1Ovs2 = 0;
}

// Overflow ISR
ISR(TIMER3_OVF_vect) {
	// increment overflow counter
	T1Ovs2++;
}

int main(void) {
	char outs[72];
	UART_init(MYUBRR);
	sei();
	UART_transmit_string("Connected!\n"); // we're alive!
	InitTimer3();
	StartTimer3();
	UART_transmit_string("TIMER3 ICP Running \r\n");
	
	/* set PD2 as input */
	DDRD &= ~(1 << DDD2);                            /* Make INT0 pin as Input */
	PORTD |= (1 << DDD2);              // turn On the Pull-up
	DDRD |= (1 << DDD6) | (1 << DDD5) | (1<<DDD1); /* Make PWM, AIN1, AIN2, STBY outputs */
	DDRC |= (1<<DDC4)
	
	// We are manually setting the direction
	PORTD &= ~(1 << PD5); //set AIN2 low
	PORTD |= (1 << PD1); //set AIN1 and STBY high
	PORTC |= (1<<PC4);
	EIMSK |= (1 << INT0) | (1 << INT1); /* enable INT0 and INT1 */
	MCUCR |= (1 << ISC01) | (1 << ISC11) |
	(1 << ISC10); /* INT0 - falling edge, INT1 - raising edge */
	
	// WE are not using the ADC for speed - just manually setting the value
	ADC_Init(); /* Initialize ADC */
	TCNT0 = 0;  /* Set timer0 count zero */
	TCCR0A |= (1 << WGM00) | (1 << WGM01) | (1 << COM0A1);
	TCCR0B |=
	(1 << CS00) | (1 << CS01); /* Set Fast PWM with Fosc/64 Timer0 clock */
	OCR0A = 30;
	
	while (1) {
		OCR0A = (ADC_Read(0)/4);			// Read ADC and map it into 0-255 to write in OCR0 register 

		// Convert ticks to RPM
		float rpms = (float)PERIOD * (float)revTickAvg * 1000.0 * 2.0;
		
		// send Speed value to LCD or USART
		UART_transmit_string("RPMS =  ");
		sprintf(outs, "%.2f \n", rpms);
		UART_transmit_string(outs);
		_delay_ms(100);	
	}
}