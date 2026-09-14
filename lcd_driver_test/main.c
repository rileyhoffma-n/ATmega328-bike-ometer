/*
 * lcd_driver_test.c
 *
 * Created: 6/9/2026 12:43:10 PM
 * Author : shelj
 */ 

#include <avr/io.h>
#define  F_CPU 1000000UL
#include <util/delay.h>
#include "odometer_FSM.h"
#include "timer1.h"
#include "timer0.h"
#include "EEPROM328.h"
#include <stdio.h>	//for sprintf()


int main(void)
{
	uint8_t saddr = 0x27;
	I2C_init();	//initialize the twi module
	LCD_init(saddr, LCD_backpack_2004, LCD_displayon_hidecursor, LCD_left2right_noshift);	//initialize LCD screen
	init_tcnt1();	//initialize the wheel speed timer
	init_tcnt0();	//initialize heartbeat timer
	buttons_init();	//initialize the buttons
	
	//WHEN YOU ARE USING THE EEPROM
	tempCircumference = wheelCircumference;	//make them the same on startup, AFTER YOU READ EEPROM STORED VALUE INTO wheelCircumference
	sei();
	
	calculateFactorsForDistance();	//initialize odometer for new wheel size
	
	rawSpeed = 0;
	
	present_state = RIDE;	//have the FSM start in the RIDE state by default
	sprintf(line1Buff, "    CURRENT RIDE    ");
	sprintf(line2Buff, "                    ");
	sprintf(line3Buff, "           MI/H     ");
	sprintf(line4Buff, "             MI     ");
	
	LCD_update_image(saddr);
	
	//flags for main loop
	//CCOMBINE THEM INTO ONE VARIABLE WITH MACROS ONCE YOU KNOW IT WORKS
	updatePageFlag = 0x00; //part of the FSM
	uint8_t updateRIDEFlag = 0x00;
	uint8_t updateLIFETIMEFlag = 0x00;
	
	//variables for the current ride
	
	uint16_t wholeDistanceRide = 0;
	uint8_t fracDistanceRide = 0;	//the hundredths of each unit of distance
	
	//EEPROM TEST
	uint8_t test_8;
	uint16_t test_16;
	uint32_t test_32;

	while(1){
		addDistance();	//add distance each loop JUST FOR TESTING ONLY
		
		if(pressedFlag){	//if a button has been pressed
				
			fsm(present_state, getInput());	//Run the FSM
			//by calling get_input(), you are evaluatin the buttons and clearin te button pressed flag
			//the fsm function will set the  update page flag if a transition occurred
			
		}
		
		if(oneSecondFlag){	//if one second has elapsed, update the time
			increase_ride_time();
			oneSecondFlag = 0x00;	//clear the one second flag			
		}
		
		if(hundredMillisecondFlag){	//if 100ms have elapsed, time to refresh the speed and/or time
			hundredMillisecondFlag = 0x00;	//clear the flag
			
			if(wheelRevCompleted){	//if a wheel revolution has been completed in the last 100ms
				
				calculateRawSpeed();	//then calculate the raw speed
				//this also clears the revolution flag
				interpret_rawSpeed();
			}
			else{	//if no revolution has been completed, apply decay if necessary
				decay_raw_speed();	//evaluates current pulse width and decays speed
				interpret_rawSpeed();
			}
			
			if(present_state == RIDE){
				sprintf(rideBuff, "%02u:%02u:%02u%02u.%02u%04u.%02u", hoursRide, minutesRide, secondsRide, wholeSpeed, fracSpeed, wholeDistanceRide, fracDistanceRide);
				//update the speed, time and distance buffer
				//then do the update ride function which only updates these values, not the other words on the screen
				updateRIDEFlag = 0xFF;
			}
			else if(present_state == LIFETIME){
				
				//TEMPORARY FOR TESTING
				uint16_t wholeHundredths = hundredthsTraveled / 100;
				uint8_t fracHundredths = hundredthsTraveled % 100;
				
				updateEEPROM_8(TEST_8_ADDRESS, 123);
				updateEEPROM_16(TEST_16_ADDRESS, 0xF0F0);
				updateEEPROM_32(TEST_32_ADDRESS, 0xFF00FF00);
				
				test_8 = readEEPROM_8(TEST_8_ADDRESS);
				test_16 = readEEPROM_16(TEST_16_ADDRESS);
				test_32 = readEEPROM_32(TEST_32_ADDRESS);
				
				uint16_t upper32 = test_32 >> 16;
				uint16_t lower32 = (uint16_t)(test_32 & 0x0000FFFF);
				
				sprintf(line1Buff, "DISTANCE TEST:");
				sprintf(line2Buff, "HUNDREDTHS: %u", fracHundredths);
				sprintf(line3Buff, "8=%u 16=%u", test_8, test_16);
				sprintf(line4Buff, "32=%u %u", upper32, lower32);
				
				//update the time buffer
				updateLIFETIMEFlag = 0xFF;
			}
		}
		
		if(updatePageFlag){
			//TEMPORARY FOR TESTING
			LCD_update_image(saddr);
			
			//update page
			updatePageFlag = 0x00;	//clear flag
		}
		if(updateRIDEFlag){
			//TEMPORARY FOR TESTING
			updateRideLCD(saddr);
			
			//update RIDE
			updateRIDEFlag = 0x00;	//clear flag
		}
		if(updateLIFETIMEFlag){
			//TEMPORARY FOR TESTING
			LCD_update_image(saddr);
			
			//update LIFETIME
			updateLIFETIMEFlag = 0x00;	//clear flag
		}
		
		
		
		
// 		if(pressedFlag){	//if a button has been pressed 
// 			
// 			buttons_eval();	//evaluate the buttons
// 			
// 			if(buttonState & 0x02){	//if button 1 was pressed
// 				button1presses++;
// 				sprintf(line1Buff, "Button 1: %d", button1presses);	//update LCD
// 			}
// 			if(buttonState & 0x04){	//if button 2 was pressed
// 				button2presses++;
// 				sprintf(line2Buff, "Button 2: %d", button2presses);	//update LCD
// 			}
// 			if(buttonState & 0x08){	//if button 3 was pressed
// 				button3presses++;
// 				sprintf(line3Buff, "Button 3: %d", button3presses);	//update LCD
// 			}
// 		
// 			LCD_update_image(saddr);	//update the LCD afterwards
// 		}
		
/* TEST FOR COUNTING SECONDS */		
// 		if(oneSecondFlag){	//if one second has elapsed
// 			SECONDSTEST++;	//increment seconds
// 			oneSecondFlag = 0x00;	//clear the flag
// 			sprintf(line2Buff, "Seconds: %d", SECONDSTEST);	//update LCD line
// 			LCD_update_image(saddr);	//update the LCD to show the new time
// 			
// 		}
		
		
/* TEST FOR UPDATING SPEED CONTINUOUSLY*/
// 		if(wheelRevCompleted){	//check if a new "revolution" has been completed
// 			rawSpeed = calculateRawSpeed();	//then calculate the raw speed
// 			
// 			wholeMPH = rawSpeed / 100;
// 			fracMPH = (rawSpeed % 100);
// 			
// 			sprintf(speedBuff, "%d.%02d", wholeMPH, fracMPH);
// 			updateSpeedLCD(saddr);
// 			
// 			}
	
	}
}

