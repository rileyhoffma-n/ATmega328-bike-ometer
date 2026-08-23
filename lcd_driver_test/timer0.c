/*
 * timer0.c
 *
 * Created: 7/17/2026 4:35:48 PM
 *  Author: shelj
 */ 

#include "timer0.h"

/**** GLOBALS ****/
volatile uint16_t numTwoMsTicks;
volatile uint8_t numHundredMsTicks;
volatile uint8_t oneSecondFlag;
volatile uint8_t hundredMillisecondFlag;	


/**** FUNCTIONS ****/

void init_tcnt0(){	//initialize TCNT0 for normal mode, clk_io/8 prescale, reset on compare match, compare register at 249 (really 250 with the extra clock cycle)
	cli();
	
	TCCR0A = 0x02;	//not connected to any pins, CTC mode
	TCCR0B = 0x02;	//no force compare, CTC mode, prescale 8
	OCR0A = 0xF9;	//compare when timer counter reaches 249 (ISR triggers on cycle later at 250)
	TIMSK0 = 0x02;	//enable OCR0A interrupt
	
	numTwoMsTicks = 0;	//initialize the ticks to 0
	numHundredMsTicks = 0;
	oneSecondFlag = 0x00;	//initialize the flags to false
	hundredMillisecondFlag = 0x00;
	//This should trigger the interrupt every 2 milliseconds for a 1 MHz F_CPU
	
	sei();
}


ISR(TIMER0_COMPA_vect) {	//ISR for timer output compare A being triggered
	numTwoMsTicks++;
	
	if(numTwoMsTicks >= 50){
		hundredMillisecondFlag = 0xFF;	//set 100ms flag
		numHundredMsTicks++;	//increment 100ms ticks
		numTwoMsTicks = 0;	//reset the 2ms ticks
	}
	if(numHundredMsTicks >= 10){
		oneSecondFlag = 0xFF;	//set the 1 sec flag
		numHundredMsTicks = 0;	//reset 100 ms ticks
	}

}