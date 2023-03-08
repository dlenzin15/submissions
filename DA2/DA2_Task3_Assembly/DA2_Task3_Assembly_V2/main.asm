;
; DA2_Task3_Assembly.asm
;
; Created: 3/7/2023 9:04:30 PM
; Author : david
;

.equ F_CPU = 16000000
.include "m328pbdef.inc"		;Include the header file

.org 0x0000
jmp main

.org 0x0002						; External interrupt request 0 vector
jmp INT0_ISR

main:
	ldi r16, (1<<5)		
	out DDRB, r16				; Set PB5 as an output 
	out PORTB, r16				; Initially turn off the LED

	ldi r17, (1<<2)
	OUT PORTD, r17				; Pull up for INTO pin

	ldi r18, (1<<INT0)			; load the bit mask for INT0 into r16
	out EIMSK, r18				; enable interrupts on external pin INT0
	
	ldi r18, 0x03				; load the value 0x03 into r16
	sts EICRA, r18				; set the rising edge of INT0 to generate an interrupt request

	sei							; enable interrupts globally

	loop:
		rjmp loop				; Infinite Loop

INT0_ISR:
	ldi r16, ~(1<<5)			; Turn on the LED
	out PORTB, r16
	
	; Delay loops to generate a delay of 3.5 seconds
    ldi  r21, 2
    ldi  r22, 29
    ldi  r23, 23
    ldi  r24, 133
	L1: dec  r24
		brne L1
		dec  r23
		brne L1
		dec  r22
		brne L1
		dec  r21
		brne L1
		
		ldi r16, (1<<5)			; Turn off the LED
		out PORTB, r16
		reti					; Return from Interrupt

	
