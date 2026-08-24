/*
 * timer0.h
 *
 * Created: 7/17/2026 4:35:32 PM
 *  Author: shelj
 */ 


#ifndef TIMER0_H_
#define TIMER0_H_

#include <avr/io.h>
#include <avr/interrupt.h>

/**** GLOBALS ****/
extern volatile uint16_t numTwoMsTicks;	//number of 2ms ticks that have elapsed since the one second flag was set
extern volatile uint8_t oneSecondFlag;	//used to indicate when one second has elapsed, is set by the OCR0A ISR, should be cleared by main
extern volatile uint8_t hundredMillisecondFlag;	
extern volatile uint8_t numHundredMsTicks;

extern uint8_t secondsRide;
extern uint8_t minutesRide;
extern uint8_t hoursRide;

/**** MACROS ****/

/**** CALLABLE FUNCTIONS ****/
void init_tcnt0();
void increase_ride_time();
void increase_lifetime_time();


/**** INTERNAL FUNCTIONS ****/



#endif /* TIMER0_H_ */