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
#include <stdio.h>	//for sprintf()


int main(void)
{
	uint8_t saddr = 0x27;
	I2C_init();	//initialize the twi module
	LCD_init(saddr, LCD_backpack_2004, LCD_displayon_hidecursor, LCD_left2right_noshift);	//initialize LCD screen
	init_tcnt1();	//initialize the wheel speed timer
	init_tcnt0();	//initialize heartbeat timer
	buttons_init();	//initialize the buttons
	sei();
	
	updateSpeedNumerator();
	rawSpeed = 0;
	uint8_t wholeMPH = 0;
	uint8_t fracMPH = 0;
	
	sprintf(line1Buff, "MPH: 00.00");
	sprintf(line2Buff, "assume 27.5in tire");
	sprintf(line3Buff, "updated for last");
	sprintf(line4Buff, "detected revolution");
	
	LCD_update_image(saddr);
	
	present_state = RIDE;	//have the FSM start in the RIDE state by default
	
	//flags for main loop
	//CCOMBINE THEM INTO ONE VARIABLE WITH MACROS ONCE YOU KNOW IT WORKS
	updatePageFlag = 0x00; //part of the FSM
	uint8_t updateRIDEFlag = 0x00;
	uint8_t updateLIFETIMEFlag = 0x00;
	
	//variable for heartbeat seconds counted
	uint16_t secondsCounted = 0x00;	//starts at 0, will be incremented on each heartbeat

	while(1){
		if(pressedFlag){	//if a button has been pressed
				
			fsm(present_state, getInput());	//Run the FSM
			//by calling get_input(), you are evaluatin the buttons and clearin te button pressed flag
			//the fsm function will set the  update page flag if a transition occurred
			
		}
		
		if(oneSecondFlag){	//if one second has elapsed, update the time
			secondsCounted++;	//update time variable
			oneSecondFlag = 0x00;	//clear the one second flag			
		}
		
		if(hundredMillisecondFlag){	//if 100ms have elapsed, time to refresh the speed and/or time
			hundredMillisecondFlag = 0x00;	//clear the flag
			
			if(wheelRevCompleted){	//if a wheel revolution has been completed in the last 100ms
				
				calculateRawSpeed();	//then calculate the raw speed
				//this also clears the revolution flag
				wholeMPH = rawSpeed / 100;
				fracMPH = (rawSpeed % 100);
			}
			else{	//if no revolution has been completed, apply decay if necessary
				decay_raw_speed();	//evaluates current pulse width and decays speed
				wholeMPH = rawSpeed / 100;
				fracMPH = (rawSpeed % 100);
			}
			
			if(present_state == RIDE){
				
				//TEMPORARY FOR TESTING
				sprintf(line1Buff, "RIDE STATS:");
				sprintf(line2Buff, "DISTANCE: ");
				sprintf(line3Buff, "TIME: %d", secondsCounted);
				sprintf(line4Buff, "%d.%02d MPH", wholeMPH, fracMPH);
				
				//update the speed and time buffer
				updateRIDEFlag = 0xFF;
			}
			else if(present_state == LIFETIME){
				
				//TEMPORARY FOR TESTING
				sprintf(line1Buff, "LIFETIME STATS:");
				sprintf(line2Buff, "DISTANCE: ");
				sprintf(line3Buff, "TIME: %d", secondsCounted);
				sprintf(line4Buff, "TOP SPEED: ");
				
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
			LCD_update_image(saddr);
			
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

