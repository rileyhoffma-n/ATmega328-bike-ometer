/*
 * timer1.c
 *
 * Created: 7/7/2026 8:56:33 PM
 *  Author: shelj
 */ 

#include "timer1.h"

/**** GLOBALS ****/
volatile uint16_t ticksPerRev;
volatile uint16_t timeRevEnded;
uint64_t speedNumerator;
volatile uint8_t wheelRevCompleted;
volatile uint64_t rawSpeed;	//the speed in mph x 100

/**** FUNCTIONS ****/
void init_tcnt1(){	//initialize TCNT1 for normal mode, clk_io/256 prescale, falling edge trigger
	cli();			//disable global interrupts
	DDRB &= ~0x01;	//PB0 is an input for ICP1
	PORTB |= 0x01;	//enable internal pullup on ICP1
	TCCR1B = 0x04;	//choose falling edge trigger and clk/256 source
	TIMSK1 = 0x20;	//enable ICF interrupt
	sei();			//enable global interrupts
}

void updateSpeedNumerator(){	//to be called if the user changes the wheel circumference
	speedNumerator = RAW_NUMERATOR * 8000;	//THE 8000 IS A STANDIN FOR THE EEPROM GLOBAL VARIABLE OF THE 27.5 in radius, WHEEL CIRCUMFERENCE x 100
}

void calculateRawSpeed(){	//raw speed is mph * 100
	//this should mimic the equation [ (100 * wheelcircumference) * (F_CPU) * (3600)] / [ (prescale256) * (63360) * (#ofticks) ] = mph * 100 (with no decimal)
		
		cli();	//make sure that ticks per rev or the flag can't be changed while you are using it to calculate
		wheelRevCompleted = 0;	//reset the revolution flag
		uint64_t atomicTicks = ticksPerRev;
		sei();

		uint64_t speedDenominator = (uint64_t)RAW_DENOMINATOR * atomicTicks;	//update the denominator based on time of last revolution
	    rawSpeed = speedNumerator / speedDenominator;		//calcuate raw speed (should be mph*100)
}

void decay_raw_speed(){	
	//checks if the tire has slowed down since last measurement, by seeing if more time has elapsed between pulses
	uint16_t previousElapsed;	//previous pulse width
	uint16_t previousStart;	//ending timestamp of previous revolution
	uint16_t currentStamp;	//the current timestamp
	
	cli();	//atomize in case a pulse occurs during this function
	previousElapsed = ticksPerRev;	//previous pulse width
	previousStart = timeRevEnded;	//ending timestamp of previous revolution
	currentStamp = TCNT1;	//the current timestamp
	sei();
	
	uint16_t sinceLastPulse = currentStamp - previousStart;	//see how much time has currently elapsed since last pulse
	
	if(sinceLastPulse <= previousElapsed){
		return; //do nothing if the current pulse is shorter than the last pulse
	}
	else{	//if the current time elapsed is longer than the previous pulse, then you have slowed down, and decay is necessary
		//first, determine the amount of the excess time in relation to previous pulse
		uint16_t extraTicks = sinceLastPulse - previousElapsed;	//see how much extra time has elapsed compared to previous pulse
		uint16_t factor;	//will be different depending on what is larger, extraTime or previousElapsed
		
		if(extraTicks < previousElapsed){	//if the extra time is a fraction of the previous pulse, then less decay will be needed for now
			//roughly determine the factor previous/extra
			factor = previousElapsed / extraTicks;	//(integer division for rough estimate) factor will be the denominator, eg: extra is 1/factor of the previous pulse
			
			if(factor > 10){	//if the extra time is 1/10 or less of the previous pulse (x < 1/10)
				rawSpeed = percent_decay(99, rawSpeed);	//reduce the speed to 99% of what it was
			}
			else if(factor > 5){	//if the extra time is 1/10 < x < 1/5
				rawSpeed = percent_decay(98, rawSpeed);	//reduce the speed to 98% of what it was
			}
			else if(factor > 3){	//if the extra time is 1/5 < x < 1/3
				rawSpeed = percent_decay(98, rawSpeed);	//reduce the speed to 98% of what it was
			}
			else if(factor > 2){	//if the extra time is 1/3 < x < 1/2
				rawSpeed = percent_decay(98, rawSpeed);	//reduce the speed to 98% of what it was
			}
			else{	//if the extra time is 1/2 < x < 1
				rawSpeed = percent_decay(97, rawSpeed);	//reduce the speed to 97% of what it was
			}
		}
		else{	//if the extra time is more than the previous pulse, heavy decay is warranted
			//roughly determine the factor extra/previous
			factor = extraTicks / previousElapsed;
			
			if(factor < 2){	//if the extra time 1 < x < 2
				rawSpeed = percent_decay(95, rawSpeed);	//reduce the speed to 85% of what it was
			}
			else if(factor < 3){	//if the extra time is 2 < x < 3
				rawSpeed = percent_decay(90, rawSpeed);	//reduce the speed to 80% of what it was
			}
			else{	//if the extra time is x > 3
				rawSpeed = percent_decay(0, rawSpeed);	//reduce the speed to 0 MPH
			}
		}
	}
	
}

uint64_t percent_decay(uint8_t percent, uint64_t number){	//returns roughly the percent of the 64 bit number w/o using floats
	return( (number * percent) / 100);
}

ISR(TIMER1_CAPT_vect){	//ISR for ICF1
	uint16_t timeRevStarted = timeRevEnded;	//the current revolution started when the previous one ended
	timeRevEnded = ICR1;	//the current revolution end time is displayed by the counter, so store it
	ticksPerRev = timeRevEnded - timeRevStarted;
	
	wheelRevCompleted = 0xFF;	//set the wheel rev flag to indicate that a new revolution has been completed
}
