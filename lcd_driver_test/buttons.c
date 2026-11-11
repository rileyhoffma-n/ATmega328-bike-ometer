/*
 * buttons.c
 *
 * Created: 7/22/2026 5:27:06 PM
 *  Author: shelj
 */ 

#include "buttons.h"

volatile uint8_t pressedFlag;
volatile uint8_t buttonState;

void buttons_init(void){
	DDRD &= ~0xE0;	//make PD5-PD7 inputs (PCINT21-PCINT23)
	
	PORTD |= (1 << PIND5) | (1 << PIND6) | (1 << PIND7);	//Enable internal pull ups (buttons should be LOW when PRESSED)
	
	PCICR |= (1 << PCIE2);	//enable PCINT one to trigger interrupt on pins 23:16
	
	PCMSK2 |= (1 << PCINT21) | (1 << PCINT22) | (1 << PCINT23);	//enable PCINT21-23
	
	pressedFlag = 0x00;	//clear the flag
}

void buttons_eval(void){
	_delay_ms(debounce_delay);	//wait for buttons to settle
	buttonState = (~PIND) & (0xE0);	// make buttonState be xxx00000	where x = 1 for pressed, x = 0 for NOT pressed
	pressedFlag = 0x00;	//clear the pressed flag after evaluation
}

ISR(PCINT1_vect){	//The ISR for a button press
	pressedFlag = 0xFF;	//set the button flag to indicate that a button has been pressed
}
