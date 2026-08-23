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
	DDRC &= ~0x0E;	//make PC1-PC3 inputs (PCINT9-PCINT11)
	
	PORTC |= (1 << PINC1) | (1 << PINC2) | (1 << PINC3);	//Enable internal pull ups (buttons should be LOW when PRESSED)
	
	PCICR |= (1 << PCIE1);	//enable PCINT one to trigger interrupt
	
	PCMSK1 |= (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11);	//enable PCINT9-11
	
	pressedFlag = 0x00;	//clear the flag
}

void buttons_eval(void){
	_delay_ms(debounce_delay);	//wait for buttons to settle
	buttonState = (~PINC) & (0x0E);	// make buttonState be 0000xxx0	where x = 1 for pressed, x = 0 for NOT pressed
	pressedFlag = 0x00;	//clear the pressed flag after evaluation
}

ISR(PCINT1_vect){	//The ISR for a button press
	pressedFlag = 0xFF;	//set the button flag to indicate that a button has been pressed
}
