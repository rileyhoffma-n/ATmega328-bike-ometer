/*
 * odometer_FSM.h
 *
 * Created: 8/2/2026 3:34:04 PM
 *  Author: shelj
 */ 


#ifndef ODOMETER_FSM_H_
#define ODOMETER_FSM_H_

#include <avr/io.h>
#include <stdio.h>	//for sprintf()
#include "buttons.h"	//so that the button presses can be used as inputs to the FSM
#include "LCD_Driver.h"	//so that the FSM can update the LCD image and display
#include "timer1.h"		//so that the FSM can change the wheel circumference and units for calculation
 
 //States in the FSM
 typedef enum {RIDE, LIFETIME, WHEEL, TENS_CHANGE, ONES_CHANGE, TENTHS_CHANGE, UNITS} state;
 
 //types of inputs possible
 typedef enum {BUTTON_UP, BUTTON_DOWN, BUTTON_NEXT, EOL} inputType;
  
 //function stubs
 void error_func(void);
 void disp_lifetime(void);
 void disp_wheel(void);
 void change_tens(void);
 void change_ones(void);
 void change_tenths(void);
 void add_unit(void);
 void subtract_unit(void);
 void disp_units(void);
 void toggle_units(void);
 void disp_ride(void);
 
 //supporting functions
 inputType getInput(void);		//tells the FSM what the input is based on the buttons
 
 //supporting variables for the functions
 #define LCD_SADDR 0x27
 extern volatile uint8_t updatePageFlag;
 
 //global variable to store the present state of the FSM
 extern volatile state present_state;
 
 //pointer to a function that takes no parameters, and returns void
 typedef void (* task_fn_ptr) ();
 
 /* each "transition" represents one row of the respective
 state transition table, has input, next state, and output.  The table itself
 is the current state*/
 typedef struct{
  inputType input_val;
  state next_state;
  task_fn_ptr tf_ptr;
 } transition;
 
/**** the actual FSM function that should be called from main after an input occurs ****/
void fsm(state ps, inputType input);	


#endif /* ODOMETER_FSM_H_ */