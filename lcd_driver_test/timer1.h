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
extern volatile uint8_t wheelRevCompleted;	//flag for if you've already calculated the speed for this revolution
extern volatile uint32_t rawSpeed;	//the speed in mph x 100000
extern volatile uint8_t unitsFlag;		//start in imperial units (0x00) by default
extern volatile uint32_t wheelCircumference;	//the circumference of the wheel, will be stored x1000 and in inches, max will be 193 inches, this is to ensure that circ * factor does not exceed 32 bits
extern uint8_t wholeSpeed;
extern uint8_t fracSpeed;

extern volatile uint8_t revPerHundredthMile;	//is 1 + the number of FULL ROTATIONS of the wheel needed to travel 100th of a mile
extern volatile uint32_t leftoverPerHundredthMile;	//(the fraction of a rotation EXTRA for every 1 + revPerHundredthMile completed) * 100,000
extern volatile uint32_t leftoverCounter;	//tracks extra rotations * 100,000
extern volatile uint32_t leftoverTarget;	//the value that the leftover counter has to reach for an extra 100th mile to be added to the odometer

extern volatile uint8_t revCounter;		//keeps track of the completed revolutions, will be serviced often enough such that it remains below 255
extern volatile uint32_t hundredthsTraveled;	//the amount of hundredths of a mile that you've gone for this ride

/**** MACROS ****/
#define F_CPU 1000000UL
#define MPH_to_KMPH 161	//conversion factor for getting 100 mph to kmph
#define SPEED_FACTOR 22195UL	//32 bit unsigned, it is 100x the factor 221.95 MPH * in^-1 * ticks
//so factor * wheel circumference / ticks = proportional to speed

#define DISTANCE_FACTOR 63360000000ULL	//uint64, is inches per 100th mile times 10^8

/**** CALLABLE FUNCTIONS ****/
void init_tcnt1();
void calculateRawSpeed();	//raw speed is mph * 100
void decay_raw_speed();
void interpret_rawSpeed();	//updates the fracSpeed and wholeSpeed variables
void addDistance();		//updates the amount of 100th miles you've gone

/**** INTERNAL FUNCTIONS ****/
uint64_t percent_decay(uint8_t percent, uint32_t number);
void calculateFactorsForDistance();	//takes circumference and updates the variables needed for distance counting, resets the extra upon wheel circumference change for simplicity

#endif /* TIMER1_H_ */