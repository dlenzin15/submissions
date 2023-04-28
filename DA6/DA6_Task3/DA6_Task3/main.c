#define F_CPU 16000000UL					// Define F_CPU to 16 MHz
#define PERIOD 1/F_CPU

#define SHIFT_REGISTER DDRB
#define SHIFT_PORT PORTB
#define DATA (1<<PB3) //MOSI (SI)
#define LATCH (1<<PB2) //SS (RCK)
#define CLOCK (1<<PB5) //SCK (SCK)

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

void init_IO(void){
	//Setup IO
	SHIFT_REGISTER |= (DATA | LATCH | CLOCK); //Set control pins as outputs
	SHIFT_PORT &= ~(DATA | LATCH | CLOCK); //Set control pins low
}
void init_SPI(void){
	//Setup SPI
	SPCR0 = (1<<SPE) | (1<<MSTR); //Start SPI as Master
}
void spi_send(unsigned char byte){
	SPDR0 = byte; //Shift in some data
	while(!(SPSR0 & (1<<SPIF))); //Wait for SPI process to finish
}

/* Segment byte maps for numbers 0 to 9 */
const uint8_t SEGMENT_MAP[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99,
0x92, 0x82, 0xF8, 0X80, 0X90};
/* Byte maps to select digit 1 to 4 */
const uint8_t SEGMENT_SELECT[] = {0xF1, 0xF2, 0xF4, 0xF8};
	
int main(void)
{
	char outs[72];
	sei();
	InitTimer3();
	StartTimer3();
	
	/* set PD2 as input */
	DDRD &= ~(1 << DDD2);              /* Make INT0 pin as Input */
	PORTD |= (1 << DDD2);              // turn On the Pull-up
	
	/* Make PWM, AIN1, AIN2, STBY outputs */
	DDRD |= (1 << DDD6) | (1 << DDD5) | (1 << DDD1); 
	DDRC |= (1<<DDC4);
	
	// We are manually setting the direction
	PORTD &= ~(1 << PD5); //set AIN2 low
	PORTD |= (1 << PD1); //set AIN1 and STBY high
	PORTC |= (1 << PC4);
	EIMSK |= (1 << INT0) | (1 << INT1); /* enable INT0 and INT1 */
	MCUCR |= (1 << ISC01) | (1 << ISC11) |
	(1 << ISC10); /* INT0 - falling edge, INT1 - raising edge */
		
	// WE are not using the ADC for speed - just manually setting the value
	ADC_Init(); // Initialize ADC
	TCNT0 = 0;  // Set timer0 count zero
	TCCR0A |= (1 << WGM00) | (1 << WGM01) | (1 << COM0A1);
	TCCR0B |=
	(1 << CS00) | (1 << CS01); // Set Fast PWM with Fosc/64 Timer0 clock 
	OCR0A = 30;
	
	init_IO();
	init_SPI();
	while(1)
	{
		OCR0A = (ADC_Read(0)/4);			// Read ADC and map it into 0-255 to write in OCR0 register

		// Convert ticks to RPM
		float rpms = (float)PERIOD * (float)revTickAvg * 1000.0 * 4.0;
		int rpms7seg_tens = (int)rpms / 10;
		int rpms7seg_ones = (int)rpms % 10;
		
		for (int i = 0; i < 10; i++)
		{
			//Pull LATCH low (start the SPI transfer!)
			SHIFT_PORT &= ~LATCH;
			//Send the tens digit to sevenseg
			spi_send((unsigned char)SEGMENT_MAP[rpms7seg_tens]);
			spi_send((unsigned char)0xF4);
			SHIFT_PORT |= LATCH;
			SHIFT_PORT &= ~LATCH;
			_delay_ms(10);
		
			//Send the ones digit to sevenseg
			//SHIFT_PORT &= ~LATCH;
			spi_send((unsigned char)SEGMENT_MAP[rpms7seg_ones]);
			spi_send((unsigned char)0xF8);
			SHIFT_PORT |= LATCH;
			SHIFT_PORT &= ~LATCH;
			_delay_ms(10);
		}
	}
}
