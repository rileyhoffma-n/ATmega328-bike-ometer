/*
 * terminal_FSM.c
 *
 * Created: 4/30/2026 10:51:31 PM
 *  Author: Riley Hoffman
 */ 

#include <avr/io.h>
#include "terminal_FSM.h"


 //subtables for each state
 
 const transition expect_command_transitions[] = //subtable for expect_command state
 {
	 //INPUT		NEXT STATE			TASK
	 {IN_C,			expect_reg_number,	nullStub_fn},
	 {IN_P,			expect_CR_page,		nullStub_fn},
	 {IN_D,         expect_CR_updates,  nullStub_fn}, 
	 {IN_EOL,		expect_command,		commandError_fn}
 };
 
 const transition expect_reg_number_transitions[] = //subtable for expect_reg_number state
 {
	 //INPUT		NEXT STATE			TASK
	 {IN_DIGIT,		expect_equals,		storeRegisterNumber_fn},
	 {IN_EOL,		expect_command,		commandError_fn}
 };
 
 const transition expect_equals_transitions[] = //subtable for expect_equals state
 {
	 //INPUT		NEXT STATE			TASK
	 {IN_EQUALS,	expect_upper_hex,	nullStub_fn},
	 {IN_EOL,		expect_command,		commandError_fn}
 };
 
 const transition expect_upper_hex_transitions[] = //subtable for expect_upper_hex state
 {
	 //INPUT		NEXT STATE			TASK
	 {IN_HEX,		expect_lower_hex,	storeUpperNibble_fn},
	 {IN_DIGIT,		expect_lower_hex,	storeUpperNibble_fn},
	 {IN_EOL,		expect_command,		commandError_fn}
 };
 
 const transition expect_lower_hex_transitions[] = //subtable for expect_lower_hex state
 {
	 //INPUT		 NEXT STATE			TASK
	 {IN_HEX,		 expect_CR,			storeLowerNibble_fn},
	 {IN_DIGIT,		 expect_CR,			storeLowerNibble_fn},
	 {IN_EOL,		 expect_command,	commandError_fn}
 };
 
 const transition expect_CR_transitions[] = //subtable for expect_CR state
 {
	 //INPUT		NEXT STATE			TASK
	 {IN_CR,		expect_command,		updateControlValue_fn},
	 {IN_EOL,		expect_command,		commandError_fn}
 };
 
 const transition expect_CR_page_transitions[] = //subtable for expect_CR_page state
 {
	 //INPUT		NEXT STATE			TASK
	 {IN_CR,		expect_command,		change_page_fn},
	 {IN_EOL,		expect_command,		commandError_fn}
 };
 
  const transition expect_CR_updates_transitions[] = //subtable for expect_CR_updates state
  {
	  //INPUT		NEXT STATE			TASK
	  {IN_CR,		expect_command,		change_updates_fn},
	  {IN_EOL,		expect_command,		commandError_fn}
  };
 
 // the outer array is an array of pointers to an array of transition
 // structures for the previous state
 // basically an array of state tables, where each table is an array
 // of transitions, where each transition is a row of the state table
 //for example, ps_transitions_ptr[expect_CR][0].next_state refers to
 // the next state of the first row of the expect_CR_transitions table
 const transition * ps_transitions_ptr[8] =
 {
	 expect_command_transitions,
	 expect_reg_number_transitions,
	 expect_equals_transitions,
	 expect_upper_hex_transitions,
	 expect_lower_hex_transitions,
	 expect_CR_transitions,
	 expect_CR_page_transitions,
	 expect_CR_updates_transitions
 };
 
 void fsm(state ps, char inputChar){ //takes the present state and the input in order to get next state and task
	 inputType key = getInputType(inputChar);	//determine what type of input it is
	 
	 int i; //must be declared outside of the if statement
	 
	 //the if statement increments i until the correct transition in the table
	 //of the present sate has been reached
	 for(i = 0; (ps_transitions_ptr[ps][i].input_val != key)
	 && (ps_transitions_ptr[ps][i].input_val != IN_EOL); i++);
	 
	 //now i has the index of the correct transition for the given input and ps
	 
	 //invoke the function pointed to by the task fcn_ptr for the transition
	 ps_transitions_ptr[ps][i].tf_ptr();
	 
	 //make the present state equal to the next state of the correct transition
	 present_state = ps_transitions_ptr[ps][i].next_state;
	 
 }
 
  void commandError_fn(void){ //tell user that input was incorrect
	  for(uint8_t i = 0; i < 1; i++){} //dummy loop
  }
  
  void nullStub_fn(void){  //do nothing when moving to next state
	  for(uint8_t i = 0; i < 1; i++){} //dummy loop
  }
  
  void storeRegisterNumber_fn(void){ //stores the register number globally
	  //turn the ascii value into an index, 0 to 8 from inputs 1-9.  this is because th CNTL registers in the struct are 0 indexed
	  //so CNTL1 is really contValues[0] if you access it via a pointer
	  regNum = ((uint8_t)input) - '0' - 1;	
  }
  
  void storeUpperNibble_fn(void){ //turns a hex character into the 4 msb of a byte, stores globally
	  //volatile uint8_t nibble = getNibble(input);
	  regVal = ((getNibble(input) << 4));	//modify upper 4 bits of regVal
  }
  
  void storeLowerNibble_fn(void){   //combines lower and upper hex into one byte
	  //volatile uint8_t nibble = getNibble(input);
	  regVal = (getNibble(input) | regVal);	//modify lower 4 bits of regVal
  }
  
  void updateControlValue_fn(void){ //Uses global variables to store the byte in the correct register number in the control struct
	  controlValues_ptr[regNum] = regVal;	//put the desired value into the desired register
  }
  
  void change_page_fn(void){
	  pageFlag = ~pageFlag;	//invert page flag
  }
  
  void change_updates_fn(void){
	  UpdateLCDFlag = ~UpdateLCDFlag;	//invert the updateLCD flag
  }

  //function that takes the raw char input, and evaluates what type of input it is for the FSM
  //uses the global variable char input which should be updated by the USART ISR
  inputType getInputType(char rawChar){
	  if(rawChar == '='){
		  return(IN_EQUALS);
	  }
	  else if(rawChar == '\r'){
		  return(IN_CR);
	  }
	  else if(rawChar == 'C'){ //if its uppercase C
		  return(IN_C);
	  }
	  else if(rawChar == 'P'){
		  return(IN_P);
	  }
	  else if(rawChar == 'D'){
		  return(IN_D);
	  }
	  else if((rawChar > '0') && (rawChar < ':')){ //if it's 1-9
		  return(IN_DIGIT);
	  }
	  else if( ((rawChar > '`') && (rawChar < 'g')) || (rawChar == '0') ) { //if the input is a valid lowercase hex letter or 0
		  return(IN_HEX);
	  }
	  else{
		  return(IN_EOL);
	  }
	  
  }
  
  uint8_t getNibble(char input){	//turns a char into its hex equivalent
	  if((input >= '0') && (input <= '9')){	//if its a digit
		  return(((uint8_t)input) - '0');
	  }
	  else{	//otherwise its a LOWERCASE hex character
		  return(((uint8_t)input) - 87);	//a = 10, f = 15
	  }
 }