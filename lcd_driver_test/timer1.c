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
volatile uint8_t wheelRevCompleted;
volatile uint32_t rawSpeed;	//the speed in mph x 100 * 1000 (times 100k)
volatile uint8_t unitsFlag = 0x00;		//start in imperial units (0x00) by default
volatile uint32_t wheelCircumference = 86390;	//the circumference of the wheel * 1000 in inches
uint8_t wholeSpeed;
uint8_t fracSpeed;

volatile uint32_t rawRevPerHundredthMile = 0;
volatile uint8_t revPerHundredthMile = 0;	//is 1 + the number of FULL ROTATIONS of the wheel needed to travel 100th of a mile
volatile uint32_t leftoverPerHundredthMile = 0;	//(the fraction of a rotation EXTRA for every 1 + revPerHundredthMile completed) * 100,000
volatile uint32_t leftoverCounter = 0;	//tracks extra rotations * 100,000

volatile uint8_t revCounter = 0;		//keeps track of the completed revolutions, will be serviced often enough such that it remains below 255
volatile uint32_t hundredthsTraveled;	//the amount of hundredths of a mile that you've gone for this ride

/**** FUNCTIONS ****/
void init_tcnt1(){	//initialize TCNT1 for normal mode, clk_io/256 prescale, falling edge trigger
	cli();			//disable global interrupts
	DDRB &= ~0x01;	//PB0 is an input for ICP1
	PORTB |= 0x01;	//enable internal pullup on ICP1
	TCCR1B = 0x04;	//choose falling edge trigger and clk/256 source
	TIMSK1 = 0x20;	//enable ICF interrupt
	sei();			//enable global interrupts
}

void calculateRawSpeed(){	//raw speed is mph * 100
	//this should mimic the equation [ (100 * wheelcircumference) * (F_CPU) * (3600)] / [ (prescale256) * (63360) * (#ofticks) ] = mph * 100 (with no decimal)
		
		cli();	//make sure that ticks per rev or the flag can't be changed while you are using it to calculate
		wheelRevCompleted = 0;	//reset the revolution flag
		uint32_t atomicTicks = ticksPerRev;	//nothing should exceed 32 bits in these operations
		sei();
		
		//calculate the raw speed using fixed-point arithmetic
		rawSpeed = SPEED_FACTOR * wheelCircumference;	//should not exceed 32 bits if circumference is less than 193500
		rawSpeed = rawSpeed / atomicTicks;	//integer division is done last to preserve accuracy
		
	//ADD METRIC CONVERSION LATER, SHOULD STILL FIT INTO UINT32_T AT NORMAL SPEEDS
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
				rawSpeed = percent_decay(94, rawSpeed);	//reduce the speed to 80% of what it was
			}
			else{	//if the extra time is x > 3
				rawSpeed = percent_decay(0, rawSpeed);	//reduce the speed to 0 MPH
			}
		}
	}
	
}

uint64_t percent_decay(uint8_t percent, uint32_t number){	//returns roughly the percent of the 32 bit number w/o using floats
	return( (number * percent) / 100);
}

void interpret_rawSpeed(){	//updates the fracSpeed and wholeSpeed variables (for display purposes)
	  wholeSpeed = rawSpeed / 10000UL;
	  fracSpeed = (rawSpeed % 10000UL) / 1000UL;	//gets it to two decimal places only
}

ISR(TIMER1_CAPT_vect){	//ISR for ICF1
	uint16_t timeRevStarted = timeRevEnded;	//the current revolution started when the previous one ended
	timeRevEnded = ICR1;	//the current revolution end time is displayed by the counter, so store it
	ticksPerRev = timeRevEnded - timeRevStarted;
	
	wheelRevCompleted = 0xFF;	//set the wheel rev flag to indicate that a new revolution has been completed
	revCounter++;	//add a revolution to the count
}

void calculateFactorsForDistance(){	//takes circumference and updates the variables needed for distance counting, resets the extra upon wheel circumference change for simplicity
	rawRevPerHundredthMile = DISTANCE_FACTOR / wheelCircumference;		//is rev per 100th times 100,000, works even for wheels with 5 in diameter (extremely small)
	
	revPerHundredthMile = (rawRevPerHundredthMile / 100000UL) + 1;	//number of full revolutions to complete 100th of a mile WITH EXTRA
	leftoverPerHundredthMile = 100000UL - (rawRevPerHundredthMile % 100000UL);	//(the fraction of a rotation EXTRA for every 1 + revPerHundredthMile completed) * 100,000

	//the leftover target is rawREvPerHundredthMile
	leftoverCounter = 0;	//reset leftover counter after calculating data new wheel
}

void addDistance(){		//updates the amount of 100th miles you've gone
	cli();	//make it atomic since revCounter could change
	if(revCounter >= revPerHundredthMile){	//if you've gone 100th of a mile
		hundredthsTraveled++;	//add a hundredth to the counter
		revCounter = revCounter - revPerHundredthMile;	//decrease the counter to show that you've accounted for the previous batch of revolutions
		
		leftoverCounter = leftoverCounter + leftoverPerHundredthMile;	//add to the leftover counter
		if(leftoverCounter >= rawRevPerHundredthMile){	//if the leftovers are enough to earn an extra 100th mile
			hundredthsTraveled++;	//add another 100th
			leftoverCounter = leftoverCounter - rawRevPerHundredthMile;	//decrease the counter to show you've taken care of the leftover
		}
	}
	
	sei();	//re enable interrupts
}