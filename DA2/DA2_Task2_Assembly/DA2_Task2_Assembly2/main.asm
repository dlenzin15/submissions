;
; DA2_Task2_Assembly.asm
;
; Created: 3/6/2023 9:39:36 PM
; Author : david
;

.include "m328pbdef.inc"	; Include the header file for the Atmega328PB board

ldi r16, (1<<5)		
out DDRB, r16				; Set PB5 as an output 
out PORTB, r16				; Initially turn off the LED

loop:
	sbis PINC, 1			; check if switch at PC1 is pressed
    rjmp led_on				; jump to button_pressed if it is
	rjmp loop				; Loop forever

led_on:
	ldi r16, ~(1<<5)		; Turn on the LED
	out PORTB, r16			
	rjmp delay				; Jump to delay function

delay:						; Delay function to generate a delay of 1.75 seconds
    ldi  r21, 143
    ldi  r22, 12
    ldi  r23, 66
L1: dec  r23
    brne L1
    dec  r22
    brne L1
    dec  r21
    brne L1
	
	ldi r16, (1<<5)			; Turn off the LED
	out PORTB, r16			
	rjmp loop				; Return to loop






