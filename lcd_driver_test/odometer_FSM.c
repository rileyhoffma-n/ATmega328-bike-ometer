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
 volatile uint16_t placeToChange = 0;	//stores the value of the place to be changed * 100, so 10000 for 100s, 1000 for 10s, etc

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
	 {BUTTON_UP,	HUNDREDS_CHANGE,	change_hundreds},
	 {BUTTON_DOWN,  HUNDREDS_CHANGE,	change_hundreds},
	 {EOL,			WHEEL,				error_func}
 };
 
 const transition change_hundreds_transitions[] = //subtable for the CHANGE_TENS state
 {
	 //INPUT		NEXT STATE			TASK
	 {BUTTON_NEXT,	TENS_CHANGE,		change_tens},
	 {BUTTON_UP,	HUNDREDS_CHANGE,	add_unit},
	 {BUTTON_DOWN,  HUNDREDS_CHANGE,	subtract_unit},
	 {EOL,			HUNDREDS_CHANGE,	error_func}
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
 const transition * ps_transitions_ptr[8] =
 {
	 ride_transitions,
	 lifetime_transitions,
	 wheel_transitions,
	 change_hundreds_transitions,
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
	  sprintf(line1Buff, "   LIFETIME STATS   ");	//header
	  sprintf(line2Buff, "    day:hr:mn:sc    ");	//time
	  
	  if(!unitsFlag){	//if imperial
		  sprintf(line3Buff, "    xxxxxx.xx MI    ");	//distance
	  }
	  else{	//if metric
		  sprintf(line3Buff, "    xxxxxx.xx KM    ");	//distance
	  }
	  
	  sprintf(line4Buff, CLEAR_LINE);	//don't know what to put here yet
	  
	  
	  updatePageFlag = 0xFF;
  }
	  
  void disp_circ(void){
	  
	  uint8_t wholeWheelCirc;
	  uint8_t fracWheelCirc;
	  
	  if(!unitsFlag){	//if imperial
		  //determine the whole and decimal part of the circumference
		  wholeWheelCirc = wheelCircumference / 100;
		  fracWheelCirc = (wheelCircumference /10) % 10;
		  
		  //load it into the buffer
		  sprintf(line2Buff, "%03d.%d INCHES", wholeWheelCirc, fracWheelCirc );
	  }
	  else{	//if in metric mode
		  uint32_t metricCirc = (wheelCircumference * 254) / 100;
		  
		  wholeWheelCirc = metricCirc / 100;
		  fracWheelCirc = (metricCirc / 10) % 10;
		  
		  //load it into the buffer
		  sprintf(line2Buff, "%03d.%d CM", wholeWheelCirc, fracWheelCirc );
	  }
  }	  
  
  void disp_wheel(void){
	  sprintf(line1Buff, "WHEEL DIAMETER:");
	  disp_circ();	//display circumference depending on the units
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
  
  void change_hundreds(void){
	  placeToChange = 100 * 100;
	  
	  sprintf(line1Buff, "CHANGE HUNDREDS");
	  disp_circ();
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void change_tens(void){
	  placeToChange = 10 * 100;
	  
	  sprintf(line1Buff, "CHANGE TENS");
	  disp_circ();
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void change_ones(void){
	  placeToChange = 1 * 100;
	  
	  sprintf(line1Buff, "CHANGE ONES");
	  disp_circ();
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void change_tenths(void){
	  placeToChange = 10;	//0.1 * 100
	  
	  sprintf(line1Buff, "CHANGE TENTHS");
	  disp_circ();
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
  
  uint8_t circ_boundary_check(uint16_t place, uint8_t upOrDown){	//checks if the digit that you want to change is already at 0 or 9, because then you can't go down or up respectively
	  //returns 0xFF if you can add or subtract, 0x00 if you are at the boundary
	  //upOrDown = 0xFF, up, upOrDown = 0x00, down
	  //place should really be place * 100 when its passed in
	  uint32_t tempCirc = wheelCircumference;
	  
	  if(unitsFlag){	//if metric
		  tempCirc = (wheelCircumference * 254) / 100;	//convert to cm
	  }
	  //otherwise, tempCirc is the normal metric wheelCircuference
	  
	  uint8_t digit = (tempCirc / place) % 10;	//find the digit in the place you want to change
	  
	  if( ((upOrDown) && (digit == 9)) || ((!upOrDown) && (digit == 0)) ){	//if you try to go up from 9, or down from 0
		  return 0x00;	//return 0 to show that you should do nothing
	  }
	  
	  return 0xFF;	//return this value to show that you can change the digit
  }
	  
  void add_unit(void){
	  
	  if(circ_boundary_check((placeToChange), 0xFF)){	//see if you can add from this place
		  
		  //then add a hundred	
		  if(!unitsFlag){	//if imperial
			  wheelCircumference += placeToChange;	//add place * 100  
		  }
		  else{	//if metric
			  wheelCircumference += (39 * placeToChange) / 100;	//add 0.39 * 100 * place
			  //ROOM FOR ERROR SINCE ITS REALLY 0.3937...
		  }
	  }
	  
	  //otherwise, do nothing
	  
	  //and of course, remember to update the circumference on the display so the user can see that they changed it
	  disp_circ();
	  updatePageFlag = 0xFF;
  }
	  
  void subtract_unit(void){
	  
	  if(circ_boundary_check((placeToChange), 0x00)){	//see if you can subtract a hundred from this place
		  
		  //then add a hundred
		  if(!unitsFlag){	//if imperial
			  wheelCircumference -= placeToChange;	//subtract place * 100
		  }
		  else{	//if metric
			  wheelCircumference -= (39 * placeToChange) / 100;;	//subract 0.39 * 100 * place
			  //ROOM FOR ERROR SINCE ITS REALLY 0.3937...
		  }
	  }
	  
	  //otherwise, do nothing
	  
	  //and of course, remember to update the circumference on the display so the user can see that they changed it
	  disp_circ();
	  updatePageFlag = 0xFF;
  }
    
  void disp_units(void){
	  sprintf(line1Buff, "UNITS:");
	  
	  if(!unitsFlag){	//if flag is clear, imperial
		sprintf(line2Buff, "IMPERIAL");
	  }
	  else{		//if flag is set, metric
		  sprintf(line2Buff, "METRIC");
	  }
	  
	  sprintf(line3Buff, CLEAR_LINE);	//other two lines clear
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
	  
	  //this function is also executed in the FSM right after the user updates the wheel size, so be sure to update it in memory
  }
	  
  void toggle_units(void){
	  sprintf(line1Buff, "UNITS:");
	  
	  if(!unitsFlag){	//if you're in imperial, switch to metric
		  sprintf(line2Buff, "METRIC");
		  unitsFlag = 0xFF;	//and set the flag
	  }
	  else{
		  sprintf(line2Buff, "IMPERIAL");	//switch to imperial
		  unitsFlag = 0x00;	//and clear the flag
	  }
	  
	  sprintf(line3Buff, CLEAR_LINE);	//other two lines should be blank
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void disp_ride(void){
	  sprintf(line1Buff, "    CURRENT RIDE    ");	//header
	  sprintf(line2Buff, "      xx:xx:xx      ");	//time
	  
	  if(!unitsFlag){	//if units flag is 0x00, imperial
		  sprintf(line3Buff, "     xx.xx MI/H     ");	//speed
		  sprintf(line4Buff, "     xxxx.xx MI     ");	//distance
	  }
	  else{	//otherwise, metric mode
		  sprintf(line3Buff, "     xx.xx KM/H     ");	//speed
		  sprintf(line4Buff, "     xxxx.xx KM     ");	//distance
	  }
	  
	  updatePageFlag = 0xFF;
  }
  