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
 uint32_t tempCircumference = 0;	//stores the value of the temporary circumference while it's being changed
 uint16_t placeToChange = 1;	//stores the place that is being changed * 10

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
	 {BUTTON_NEXT,	UNITS,				disp_units},
	 {BUTTON_UP,	LIFETIME,			error_func},
	 {BUTTON_DOWN,  LIFETIME,			error_func},
	 {EOL,			LIFETIME,			error_func}
 };
 
 const transition units_transitions[] = //subtable for the UNITS state
 {
	 //INPUT		NEXT STATE			TASK
	 {BUTTON_NEXT,	WHEEL,				disp_wheel},
	 {BUTTON_UP,	UNITS,				toggle_units},
	 {BUTTON_DOWN,  UNITS,				toggle_units},
	 {EOL,			UNITS,				error_func}
 };
 
 const transition wheel_transitions[] = //subtable for the WHEEL state
 {
	  //INPUT		NEXT STATE			TASK
	  {BUTTON_NEXT,	RIDE,				disp_ride},
	  {BUTTON_UP,	TENTHS_CHANGE,		change_tenths},
	  {BUTTON_DOWN, TENTHS_CHANGE,		change_tenths},
	  {EOL,			WHEEL,				error_func}  
 };
 
 const transition change_tenths_transitions[] = //subtable for the CHANGE_TENTHS state
 {
	 //INPUT		NEXT STATE			TASK
	 {BUTTON_NEXT,	ONES_CHANGE,		change_ones},
	 {BUTTON_UP,	TENTHS_CHANGE,		add_unit},
	 {BUTTON_DOWN,  TENTHS_CHANGE,		subtract_unit},
	 {EOL,			TENTHS_CHANGE,		error_func}
 };
 
 const transition change_ones_transitions[] = //subtable for the CHANGE_ONES state
 {
	 //INPUT		NEXT STATE			TASK
	 {BUTTON_NEXT,	TENS_CHANGE,		change_tens},
	 {BUTTON_UP,	ONES_CHANGE,		add_unit},
	 {BUTTON_DOWN,  ONES_CHANGE,		subtract_unit},
	 {EOL,			ONES_CHANGE,		error_func}
 };
 
 const transition change_tens_transitions[] = //subtable for the CHANGE_TENS state
 {
	 //INPUT		 NEXT STATE			TASK
	 {BUTTON_NEXT,	HUNDREDS_CHANGE,	change_hundreds},
	 {BUTTON_UP,	TENS_CHANGE,		add_unit},
	 {BUTTON_DOWN,  TENS_CHANGE,		subtract_unit},
	 {EOL,			TENS_CHANGE,		error_func}
 };
 
 const transition change_hundreds_transitions[] = //subtable for the CHANGE_HUNDREDS state
 {
	 //INPUT		NEXT STATE			TASK
	 {BUTTON_NEXT,	WHEEL,					disp_wheel},
	 {BUTTON_UP,	HUNDREDS_CHANGE,		add_unit},
	 {BUTTON_DOWN,  HUNDREDS_CHANGE,		subtract_unit},
	 {EOL,			HUNDREDS_CHANGE,		error_func}
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
	 units_transitions,
	 wheel_transitions,
	 change_tenths_transitions,
	 change_ones_transitions,
	 change_tens_transitions,
	 change_hundreds_transitions
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
	  
  void disp_wheel(void){
	  change_circumference(tempCircumference);	//when you enter this state, update the wheelCircumference variable to be the same as the temp circumference variable (in case the user changed it)
	  
	  sprintf(line1Buff, "WHEEL DIAMETER:");
	  disp_circ();	//display circumference depending on the units
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
  
  void change_hundreds(void){
	  placeToChange = 1000;	//100 * 10
	  
	  sprintf(line1Buff, "CHANGE HUNDREDS");
	  disp_circ();
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
	  
	  //UPON LEAVING THIS STATE TO RETURN TO DISPLAYING THE WHEEL, THE ACTUAL CIRCUMFERENCE VARIABLE WILL BE UPDATED TO THE SAME AS THE TEMP VARIABLE, SOLIDIFYING THE USER'S CHANGE THAT THEY JUST MADE
  }
	  
  void change_tens(void){
	  placeToChange = 100;	//10 * 10
	  
	  sprintf(line1Buff, "CHANGE TENS");
	  disp_circ();
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void change_ones(void){
	  placeToChange = 10; //1 * 10;
	  
	  sprintf(line1Buff, "CHANGE ONES");
	  disp_circ();
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
	  
  void change_tenths(void){
	  tempCircumference = wheelCircumference;	//set them equal only for this state, as tempCircumference and wheelCircumference will differ in te followin states
	  placeToChange = 1;	//0.1 * 10
	  
	  sprintf(line1Buff, "CHANGE TENTHS");
	  disp_circ();
	  sprintf(line3Buff, CLEAR_LINE);
	  sprintf(line4Buff, CLEAR_LINE);
	  
	  updatePageFlag = 0xFF;
  }
 
	  
  void add_unit(void){
	  if(can_change(UP)){	//if you can add the amount you want
		  
		  if(!unitsFlag){	//if imperial
			tempCircumference = tempCircumference + ((uint32_t)(placeToChange * 100UL));	//add that amount
			//have to manually promote placetochange to uint32 temporarily, spent a lot of time on this bug (if not adding 100s causes overflow)
		  }
		  else{	//if metric
			 tempCircumference = tempCircumference +  (39370UL / (1000 / placeToChange));
		  }  
		  sprintf(line4Buff, "added");
	  }
	  else{
		sprintf(line4Buff, "too high");
	  }
	  
	  disp_circ();
	  updatePageFlag = 0xFF;
  }
	  
  void subtract_unit(void){
	  if(can_change(DOWN)){	//if you can subtract the amount you want
		  
		  if(!unitsFlag){	//if imperial
			  tempCircumference = tempCircumference - ((uint32_t)(placeToChange * 100UL));	//subtract that amount
			  //have to manually promote placetochange to uint32 temporarily, spent a lot of time on this bug (if not adding 100s causes overflow)
		  }
		  else{	//if metric
			  tempCircumference = tempCircumference -  (39370UL / (1000 / placeToChange));
		  }
		  sprintf(line4Buff, "subtracted");
	  }
	  else{
		sprintf(line4Buff, "too low");
	  }
	  
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
  
    void disp_circ(void){
	    
	    //displays the value of temp circumference
	    
	    uint16_t wholeWheelCirc;	//because in centimeter mode, you can exceed 255
	    uint8_t fracWheelCirc;
	    
	    if(!unitsFlag){	//if imperial
		    //determine the whole and decimal part of the circumference
		    wholeWheelCirc = tempCircumference / 1000;
		    fracWheelCirc = (tempCircumference % 1000) / 100;
		    
		    //load it into the buffer
		    sprintf(line2Buff, "%03u.%u INCHES", wholeWheelCirc, fracWheelCirc );
	    }
	    else{	//if in metric mode
		    uint32_t metricCirc = (tempCircumference * 254) / 100;
		    
		    wholeWheelCirc = metricCirc / 1000;
		    fracWheelCirc = (metricCirc % 1000) / 100;
		    
		    //load circumference it into the buffer
		    sprintf(line2Buff, "%03u.%u CM", wholeWheelCirc, fracWheelCirc );
	    }
		
		//FOR DEBUGGING
		uint16_t lowerVar = tempCircumference & 0x0000FFFF;
		uint16_t upperVar = ((tempCircumference >> 16) & 0x0000FFFF);
		sprintf(line3Buff, "var: %04x %04x", upperVar, lowerVar);
    }
    
 uint8_t can_change(uint8_t up){	//checks if you can add or subtract from the current decimal place in the wheel circumference
	 //max inches = 190.0 or 190,000 
	 //max cm = 482.0 or 482,000
	 
	 //min inches = 20 or 20,000 (arbitrary)
	 //min cm around 7.9 or 7,900
	 
	 // 1 in = 2.54 cm
	 // 1 cm = 0.3937 in
	 
	 /* first, see if adding will put you above the max, or subtracting will put you below the min */
	 uint32_t amountChanging;	//the amount that will be added or subtracted from the variable
	 
	 if(!unitsFlag){	//if imperial
		amountChanging = ((uint32_t)100 * placeToChange);	//promote to uint32_t to avoid the overflow bug
	 }
	 else{	//if metric
		 amountChanging = 39370UL / (1000 / placeToChange);
	 }
	 
	 if(up){	//if you are adding
		 if((MAX_CIRC - amountChanging) < tempCircumference){	//if adding puts you over 190,000
			 return(0x00);	//indicate that you CAN'T change
		 }
	 }
	 //otherwise, you're subtracting
	 else if((MIN_CIRC + amountChanging) > tempCircumference){	//if subtracting puts you below 20,000
		 return(0x00);	//indicate that you CAN'T change
	 }
	 
	 
	 /* if you made it here you wont violate the size limit of the whole variable, so see the if the digit itself will stay between 0-9*/
	 uint8_t digit;
	 
	 if(unitsFlag){	//if metric
		 uint32_t metricCirc = (tempCircumference * 254) / 100;	//convert first
		 digit = ((metricCirc / 100) / placeToChange) % 10;	//where place to change is 10x, and circumference is 1000x
	 }
	 else{	//if imperial
		 digit = ((tempCircumference / 100) / placeToChange) % 10;	//where place to change is 10x, and circumference is 1000x
	 }
	 
	 if( (up && (digit >= 9)) || (!up && (digit <= 0)) ){	//if you try to go up from 9, or down from 0
		 return(0x00);	//indicate that you CAN'T change
	 }

	 return(0xFF);	//indicate that you CAN change if you made it here
 }