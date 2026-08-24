/*
 * odometer_FSM.c
 *
 * Created: 8/2/2026 3:33:37 PM
 *  Author: shelj
 */ 

#include "odometer_FSM.h"

 //declare the present state variable
 volatile state present_state = RIDE;	//have the FSM start in the RIDE state by default
 volatile uint8_t updatePageFlag = 0x00;	//this flag is set upon state transition

 //subtables for each state
 
 const transition ride_transitions[] = //subtable for the RIDE state
 {
	 //INPUT		NEXT STATE			TASK
	 {BUTTON_NEXT,	LIFETIME,			disp_lifetime},
	 {BUTTON_UP,	RIDE,				error_func},
	 {BUTTON_DOWN,  RIDE,				error_func},
	 {EOL,			RIDE,				error_func}
 };
 
 const transition lifetime_transitions[] = //subtable for the LIFETIME state
 {
	 //INPUT		NEXT STATE			TASK
	 {BUTTON_NEXT,	WHEEL,				disp_wheel},
	 {BUTTON_UP,	LIFETIME,			error_func},
	 {BUTTON_DOWN,  LIFETIME,			error_func},
	 {EOL,			LIFETIME,			error_func}
 };
 
 const transition wheel_transitions[] = //subtable for the WHEEL state
 {
	 //INPUT		NEXT STATE			TASK
	 {BUTTON_NEXT,	UNITS,				disp_units},
	 {BUTTON_UP,	TENS_CHANGE,		change_tens},
	 {BUTTON_DOWN,  TENS_CHANGE,		change_tens},
	 {EOL,			WHEEL,				error_func}
 };
 
 const transition change_tens_transitions[] = //subtable for the CHANGE_TENS state
 {
	 //INPUT		NEXT STATE			TASK
	 {BUTTON_NEXT,	ONES_CHANGE,		change_ones},
	 {BUTTON_UP,	TENS_CHANGE,		add_unit},
	 {BUTTON_DOWN,  TENS_CHANGE,		subtract_unit},
	 {EOL,			TENS_CHANGE,		error_func}
 };
 
 const transition change_ones_transitions[] = //subtable for the CHANGE_ONES state
 {
	 //INPUT		 NEXT STATE			TASK
	 {BUTTON_NEXT,	TENTHS_CHANGE,		change_tenths},
	 {BUTTON_UP,	ONES_CHANGE,		add_unit},
	 {BUTTON_DOWN,  ONES_CHANGE,		subtract_unit},
	 {EOL,			ONES_CHANGE,		error_func}
 };
 
 const transition change_tenths_transitions[] = //subtable for the CHANGE_TENTHS state
 {
	 //INPUT		NEXT STATE			TASK
	 {BUTTON_NEXT,	UNITS,				disp_units},
	 {BUTTON_UP,	TENTHS_CHANGE,		add_unit},
	 {BUTTON_DOWN,  TENTHS_CHANGE,		subtract_unit},
	 {EOL,			TENTHS_CHANGE,		error_func}
 };
 
 const transition units_transitions[] = //subtable for the UNITS state
 {
	 //INPUT		NEXT STATE			TASK
	 {BUTTON_NEXT,	RIDE,				disp_ride},
	 {BUTTON_UP,	UNITS,				toggle_units},
	 {BUTTON_DOWN,  UNITS,				toggle_units},
	 {EOL,			UNITS,				error_func}
 };
 
 // the outer array is an array of pointers to an array of transition
 // structures for the previous state
 // basically an array of state tables, where each table is an array
 // of transitions, where each transition is a row of the state table
 //for example, ps_transitions_ptr[expect_CR][0].next_state refers to
 // the next state of the first row of the expect_CR_transitions table
 const transition * ps_transitions_ptr[7] =
 {
	 ride_transitions,
	 lifetime_transitions,
	 wheel_transitions,
	 change_tens_transitions,
	 change_ones_transitions,
	 change_tenths_transitions,
	 units_transitions
 };
 
  void fsm(state ps, inputType input){ //takes the present state and the input in order to get next state and task
	  
	  int i; //must be declared outside of the for loop
	  
	  //the if statement increments i until the correct transition in the table
	  //of the present sate has been reached
	  for(i = 0; (ps_transitions_ptr[ps][i].input_val != input)
	  && (ps_transitions_ptr[ps][i].input_val != EOL); i++);
	  
	  //now i has the index of the correct transition for the given input and ps
	  
	  //invoke the function pointed to by the task fcn_ptr for the transition
	  ps_transitions_ptr[ps][i].tf_ptr();
	  
	  //make the present state equal to the next state of the correct transition
	  present_state = ps_transitions_ptr[ps][i].next_state;
	  
  }
  
  inputType getInput(void){		//tells the FSM what the input is based on the buttons
	  buttons_eval();	//evaluate the buttons after they settle
	  
	  if(buttonState & 0x02){	//if button 1 was pressed (NEXT)
		 return(BUTTON_NEXT);
	  }
	  else if(buttonState & 0x04){	//if button 2 was pressed (UP)
		  return(BUTTON_UP);
	  }
	  else if(buttonState & 0x08){	//if button 3 was pressed (DOWN)
		  return(BUTTON_DOWN);
	  }
	  else{	//if multiple buttons at once or none
		  return(EOL);
	  }
  }
  
  void error_func(void){
	  // NOTHING SHOULD HAPPEN FOR NOW
  }
  
  void disp_lifetime(void){
	  sprintf(line1Buff, "    CURRENT RIDE    ");
	  sprintf(line2Buff, "                    ");
	  sprintf(line3Buff, "           MI/H     ");
	  sprintf(line4Buff, "             MI     ");
	  
	  
	  updatePageFlag = 0xFF;
  }
	  
  void disp_wheel(void){
	  sprintf(line1Buff, "WHEEL DIAMETER:");
	  sprintf(line2Buff, "00.0 inches");
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void change_tens(void){
	  sprintf(line1Buff, "CHANGE TENS MODE");
	  sprintf(line2Buff, CLEAR_LINE);
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void change_ones(void){
	  sprintf(line1Buff, "CHANGE ONES MODE");
	  sprintf(line2Buff, CLEAR_LINE);
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void change_tenths(void){
	  sprintf(line1Buff, "CHANGE TENTHS MODE");
	  sprintf(line2Buff, CLEAR_LINE);
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void add_unit(void){
	  sprintf(line2Buff, "ADDED UNIT     ");
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void subtract_unit(void){
	  sprintf(line2Buff, "SUBTRACTED UNIT     ");
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void disp_units(void){
	  sprintf(line1Buff, "UNITS:");
	  sprintf(line2Buff, "IMPERIAL");
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void toggle_units(void){
	  sprintf(line1Buff, "UNITS:");
	  sprintf(line2Buff, "TOGGLE");
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void disp_ride(void){
	  sprintf(line1Buff, "RIDE STATS:");
	  sprintf(line2Buff, "DISTANCE: ");
	  sprintf(line3Buff, "TIME: ");
	  sprintf(line4Buff, "SPEED: ");
	  
	  updatePageFlag = 0xFF;
  }