/*
 * terminal_FSM.h
 *
 * Created: 4/30/2026 12:19:59 PM
 * Author : Riley Hoffman, Ray Yasmin
 *
 * Notes: 0 doesn't count as a digit, hex is lowercase only for now
 *
 * Description: Takes character inputs from a terminal over USART3
 * Uses fsm and one function to parse these command inputs then
 * updates the control register global struct values
 *
 *******************************************************************/


#ifndef TERMINAL_FSM_H_
#define TERMINAL_FSM_H_

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
 
#include "ak9723aj.h"
//extern volatile input; //the actual char value received from the USART

 //INITIALIZE THE FSM
 
 //States in the FSM
 typedef enum {expect_command, expect_reg_number, expect_equals, expect_upper_hex, expect_lower_hex, expect_CR, expect_CR_page, expect_CR_updates} state;
 
 //types of inputs possible
 typedef enum {IN_C, IN_DIGIT, IN_EQUALS, IN_HEX, IN_CR, IN_P, IN_D, IN_EOL} inputType;
  
 //functions
 void commandError_fn(void); //tell user that input was incorrect
 void nullStub_fn(void);  //do nothing when moving to next state
 void storeRegisterNumber_fn(void); //stores the register number globally
 void storeUpperNibble_fn(void); //turns a hex character into the 4 msb of a byte, stores globally
 void storeLowerNibble_fn(void);   //combines lower and upper hex into one byte
 void updateControlValue_fn(void); //USes global variables to store the byte in the correct register number in the control struct
 uint8_t getNibble(char input);	//turns a char into its hex equivalent
 void change_page_fn(void);	//changes the page
 void change_updates_fn(void);	//changes the status of LCD update enable
 
 //supporting variables for the functions
 extern volatile char input; //the actual value received from the USART*/
 extern volatile uint8_t regNum; //number of chosen register
 extern volatile struct ctrl controlValues; //storing the values that will should be/already are in the control registers
 extern volatile uint8_t regVal;
 extern volatile uint8_t *controlValues_ptr;	//pointer to first control register
 extern volatile uint8_t pageFlag;	//tells you what page to display
 extern volatile uint8_t UpdateLCDFlag; //tells you whether or not to update the lcd
 /*struct meas measurementReadings;*/
 
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
 
 
void fsm(state ps, char inputChar);
inputType getInputType(char rawChar);


#endif /* TERMINAL_FSM_H_ */