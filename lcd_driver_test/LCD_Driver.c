/*
 * LCD_Driver.c
 *
 * Created: 5/11/2026
 * Author : Riley Hoffman
 * 
 * Description: 
 * 
 * Notes:
 *
 */ 

#include "LCD_Driver.h"

//**** GLOBALS ****
uint8_t currentDDRAMaddress;	//allows the software to keep track of the next character to be written without having to talk to the LCD each time
char line1Buff[21];	//holds the LCD image
char line2Buff[21];
char line3Buff[21];
char line4Buff[21];
//char speedBuff[16];	//room for speed to start on spot 5, + /0
char rideBuff[21];	//room for speed, time, and distance numbers only + \0

//**** FUNCTIONS ****
uint8_t send_4bit_nibble(uint8_t nibble){	//FORMAT: D7, D6, D5, D4, BT, E, R/W, RS, assumes I2C frame is already started
	//This function should pulse the enable bit while sending the 4 data bits, and keeping BT, R/W, and RS the same throughout 
	
	nibble = nibble | LCD_data_enable;	//ensure that the Enable bit is set at first
	
	//send the data with E=1
	if(!I2C_send_byte(nibble)){
		return 0x00;	//return 0 if send failed
	}
	
	_delay_us(1);	//the enable high pulse must be at least 450 ns
	
	//send the data with E=0
	nibble = nibble & ~LCD_data_enable;	//toggle the E bit to 0
	if(!I2C_send_byte(nibble)){
		return 0x00;	//return 0 if send failed
	}
	
	return 0xFF; //success
}

uint8_t send_byte(uint8_t byte, uint8_t commonNibble){
		//the data you want to send to the backpack must be in two nibbles
		uint8_t upperNibble, lowerNibble;
		upperNibble = (byte & 0xF0) | commonNibble;	//get upper nibble, keep it in 4 most significant bit positions, append common nibble
		lowerNibble = ((byte << 4) & 0xF0) | commonNibble;	//get the lower nibble, shift it to the most significant 4 bit positions as well, append common nibble
		
		//send the upper nibble of the instruction first
		if(!send_4bit_nibble(upperNibble)){
			return 0x00;	//return 0 if send failed
		}
		
		//send the lower nibble
		if(!send_4bit_nibble(lowerNibble)){
			return 0x00;	//return 0 if send failed
		}
		
		return 0xFF;	//success
}

uint8_t send_instruction(uint8_t instruction){	//sends an 8 bit instruction to an instruction register, I2C frame must already be started, and stopped afterwards
	uint8_t commonNibble = LCD_data_enable | LCD_select_instructionReg_bm | LCD_write_bm | LCD_backlight_bm;	//this will be the least significant 4 bits of each 4 bit send 
	
	if(!send_byte(instruction, commonNibble)){	//send the data byte and control R/W, RS, and E
		return 0x00;	//return 0 if failed
	}
	
	//delay 45 microseconds to be safe ... The return home instruction is the only one which needs a longer delay
	if(instruction == LCD_return_home){
		_delay_us(1600);
	}
	else{
	_delay_us(45);
	}
	
	return 0xFF;	//success
}

uint8_t send_character(uint8_t c){	/*sends an ascii character to the LCD, frame must be started and stopped outside this function,
									 make sure you set the DDRAM address first so that you write the character to DDRAM, not CGRAM*/

	uint8_t commonNibble = LCD_data_enable | LCD_select_dataReg_bm | LCD_write_bm | LCD_backlight_bm;	//this will be the least significant 4 bits of each 4 bit send
	
	if(!send_byte(c, commonNibble)){	//send the data byte and control R/W, RS, and E
		return 0x00;	//return 0 if failed
	}
	
	//delay 50 microseconds to be safe ... at least need 37 + 4 us
	_delay_us(50);
	
	return 0xFF;	//success
}

uint8_t set_DDRAM_location(uint8_t desiredAddress){	//takes a 7 bit address
	if(!send_instruction(LCD_set_DDRAM_addr | desiredAddress)){
		return 0x00;
	}
	
	currentDDRAMaddress == desiredAddress;
	
	return 0xFF;	//success
}

uint8_t LCD_init(uint8_t saddr, uint8_t numLinesFont, uint8_t onCursorBlink, uint8_t entryModeSet){	//usually would be saddr, LCD_backpack_2004, LCD_displayon_hidecursor, LCD_left2right_noshift
	_delay_ms(20);	//delay at least 15 ms after power supply exceeds 4.5v
	
	if(!I2C_start_write(saddr)){	//start the frame and address the LCD backpack
		return 0x00;	//if the addressing failed at any point, return 0
	}
	
	if(!send_4bit_nibble(0x30)){
		return 0x00;
	}
	_delay_ms(5);	
	
	if(!send_4bit_nibble(0x30)){
		return 0x00;
	}
	_delay_us(150);
	
	if(!send_4bit_nibble(0x30)){
		return 0x00;
	}
	_delay_us(150);
	
	if(!send_4bit_nibble(0x20)){
		return 0x00;
	}
	_delay_us(150);			//not shown in data sheet but just to be safe
	
	//set number of lines and dot font type
	send_instruction(numLinesFont);
	
	//display on/off, cursor on/off, cursor blink
	send_instruction(onCursorBlink);
	
	//clear display
	send_instruction(LCD_clear_display);
	_delay_ms(2);	//longer delay needed for clearing the display
	
	//entry mode set
	send_instruction(entryModeSet);
	
	I2C_stop();
	return 0xFF;	//if this is reached, then the communication was successful
}

uint8_t LCD_send_line(uint8_t lineNum){	//updates one line on the display, I2C must already be in progress
	char* buffptr;	//pointer to the desired line buffer
	
	//set the starting point of the cursor and correct buffer to be sent
	if(lineNum == 1){
		if(!set_DDRAM_location(LCD_line1_start)){
			return 0x00;
		}
		buffptr = line1Buff;
	}
	else if(lineNum == 2){
		if(!set_DDRAM_location(LCD_line2_start)){
			return 0x00;
		}
		buffptr = line2Buff;
	}
	else if(lineNum == 3){
		if(!set_DDRAM_location(LCD_line3_start)){
			return 0x00;
		}
		buffptr = line3Buff;
	}
	else if(lineNum == 4){
		if(!set_DDRAM_location(LCD_line4_start)){
			return 0x00;
		}
		buffptr = line4Buff;
	}
	else{
		return 0x00;	//return failure if invalid line number is given
	}
	
	//send char by char until null terminator reached while updating address variable
	uint8_t i;
	for(i = 0; buffptr[i] != '\0'; i++){
		if(!send_character(buffptr[i])){
			return 0x00;
		}
		currentDDRAMaddress++;
	}
	
	//In order to "clear" the rest of the line, send spaces until end of line is reached
	while(i < 20){
		if(!send_character(' ')){
			return 0x00;
		}
		i++;
	}
	
	return 0xFF;	//successful
}

uint8_t LCD_update_image(uint8_t saddr){
	if(!I2C_start_write(saddr)){	//start the frame and address the LCD backpack
		return 0x00;	//if the addressing failed at any point, return 0
	}
	
	//send the four lines in order 
	for(uint8_t i = 1; i < 5; i++){
		if(!LCD_send_line(i)){
			return 0x00;
		}
	}
	
	I2C_stop();
	
	return 0xFF; //success
}

// uint8_t updateSpeedLCD(uint8_t saddr){	//only changes the characters showing the speed on the lcd to save time and cpu
// 	//assuming the display has line 1 reading "MPH: " then the speed, so the numbers start at the 5th spot on line one
// 	
// 	if(!I2C_start_write(saddr)){	//start the frame and address the LCD backpack
// 		return 0x00;	//if the addressing failed at any point, return 0
// 	}
// 	
// 	if(!set_DDRAM_location(LCD_line1_start + 5)){	//set the cursor to the 5th spot on line 1
// 		return 0x00;
// 	}
// 	
// 	//send char by char until null terminator reached while updating address variable
// 	uint8_t i;
// 	for(i = 0; speedBuff[i] != '\0'; i++){
// 		if(!send_character(speedBuff[i])){
// 			return 0x00;
// 		}
// 		currentDDRAMaddress++;
// 	}
// 	
// 	//In order to "clear" the rest of the line, send a few spaces
// 	for(uint8_t j = 0; j < 3; j++){
// 		if(!send_character(' ')){
// 			return 0x00;
// 		}
// 	}
// 	
// 	I2C_stop();	//stop the communication
// 	
// 	return 0xFF; //success
// }

uint8_t updateRideLCD(uint8_t saddr){	//only changes the numbers (not the constant letters) to save time and cpu
	/*assuming the display is
	----CURRENT RIDE----
	------xx:xx:xx------
	-----xx.xx MI/H-----
	-----xxxx.xx MI-----*/
	
	if(!I2C_start_write(saddr)){	//start the frame and address the LCD backpack
		return 0x00;	//if the addressing failed at any point, return 0
	}
	
	if(!set_DDRAM_location(LCD_line2_start + 6)){	//set the cursor to the 6th spot on line 2 for the time
		return 0x00;
	}
	//send the time char by char until the speed is reached
	uint8_t i = 0;
	for(i; i < RIDE_BUFF_SPEED; i++){
		if(!send_character(rideBuff[i])){
			return 0x00;
		}
		currentDDRAMaddress++;
	}
	
	if(!set_DDRAM_location(LCD_line3_start + 5)){	//set the cursor to the 5th spot on line 3 for the speed
		return 0x00;
	}
	//send the speed char by char until the distance is reached
	for(i; i < RIDE_BUFF_DISTANCE; i++){
		if(!send_character(rideBuff[i])){
			return 0x00;
		}
		currentDDRAMaddress++;
	}
	
	if(!set_DDRAM_location(LCD_line4_start + 5)){	//set the cursor to the 5th spot on line 4 for the distance
		return 0x00;
	}
	//send the distance char by char until the \0 is reached
	for(i; rideBuff[i] != '\0'; i++){
		if(!send_character(rideBuff[i])){
			return 0x00;
		}
		currentDDRAMaddress++;
	}
	
	I2C_stop();	//stop the communication
	
	return 0xFF; //success
}