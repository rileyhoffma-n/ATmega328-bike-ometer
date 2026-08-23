/*
 * buttons.h
 *
 * Created: 7/22/2026 5:26:50 PM
 *  Author: shelj
 */ 


#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000UL
#include <util/delay.h>

/**** GLOBALS ****/
extern volatile uint8_t pressedFlag;
extern volatile uint8_t buttonState;

/**** MACROS ****/
#define debounce_delay 8	//to be used in delay_ms

/**** CALLABLE FUNCTIONS ****/
void buttons_init(void);	//sets up the ports and pin change interrupts for the buttons
void buttons_eval(void);

/**** INTERNAL FUNCTIONS ****/



#endif /* BUTTONS_H_ */