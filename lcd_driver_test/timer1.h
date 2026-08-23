/*
 * timer1.h
 *
 * Created: 7/7/2026 8:56:18 PM
 *  Author: shelj
 */ 

#ifndef TIMER1_H_
#define TIMER1_H_

#include <avr/io.h>
#include <avr/interrupt.h>

/**** GLOBALS ****/
extern volatile uint16_t ticksPerRev;
extern volatile uint16_t timeRevEnded;
extern uint64_t speedNumerator;
extern volatile uint8_t wheelRevCompleted;	//flag for if you've already calculated the speed for this revolution
extern volatile uint64_t rawSpeed;	//the speed in mph x 100

/**** MACROS ****/
#define F_CPU 1000000UL
#define RAW_DENOMINATOR 16220160UL	//( 63360 inches/mile) * 256 prescale value, this will be multiplied by number of timer ticks to get speedDenominator
#define RAW_NUMERATOR 3600000000ULL	// 1MHz F_CPU * 3600 sec/hour, will be multiplied by wheel circumference * 100 to get speedNumerator

/**** CALLABLE FUNCTIONS ****/
void init_tcnt1();
void calculateRawSpeed();	//raw speed is mph * 100
void updateSpeedNumerator();	//to be called if the user changes the wheel circumference
void decay_raw_speed();

/**** INTERNAL FUNCTIONS ****/
uint64_t percent_decay(uint8_t percent, uint64_t number);

#endif /* TIMER1_H_ */