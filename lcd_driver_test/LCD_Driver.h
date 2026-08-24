/*
 * LCD_Driver.h
 *
 * Created: 5/11/2026
 *  Author: Riley Hoffman
 *
 *
 *
 */ 


#ifndef LCD_DRIVER_H_
#define LCD_DRIVER_H_


#include <avr/io.h>
#include "I2C_328.h"
#define F_CPU 1000000UL
#include <util/delay.h>

//**** GLOBALS ****
extern uint8_t currentDDRAMaddress;	//allows the software to keep track of the next character to be written without having to talk to the LCD each time
extern char line1Buff[21];	//holds the LCD image
extern char line2Buff[21];
extern char line3Buff[21];
extern char line4Buff[21];
//extern char speedBuff[16];	//room for speed to start on spot 5, + /0
extern char rideBuff[21];	//room for speed, time, and distance numbers only + \0

//******** Macros ********
#define LCD_regsel_bp 0
#define LCD_rw_bp 1
#define LCD_data_enable_bp 2
#define LCD_BT_bp 3


#define LCD_select_instructionReg_bm 0x00 << LCD_regsel_bp
#define LCD_select_dataReg_bm 0x01 << LCD_regsel_bp
#define LCD_write_bm 0x00 << LCD_rw_bp
#define LCD_read_bm 0x01 << LCD_rw_bp
#define LCD_data_enable 0x01 << LCD_data_enable_bp
#define LCD_backlight_bm 0x01 << LCD_BT_bp

//****  Assuming the characters are NOT SHIFTED ****//
#define LCD_line1_start 0x00
#define LCD_line3_start 0x14
#define LCD_line2_start 0x40
#define LCD_line4_start 0x54
#define LCD_line1_end 0x13
#define LCD_line3_end 0x27
#define LCD_line2_end 0x53
#define LCD_line4_end 0x67
//*****************************************************

#define LCD_clear_display 0x01
#define LCD_return_home 0x02
#define LCD_left2right_noshift 0x06
#define LCD_displayon_hidecursor 0x0C
#define LCD_cursor_left 0x10
#define LCD_cursor_right 0x14
#define LCD_backpack_2004 0x28
#define LCD_set_DDRAM_addr 0x80	//OR with the desired 7-bit address

#define CLEAR_LINE "                    "
#define RIDE_BUFF_TIME 0	//the index of rideBuff where time starts
#define RIDE_BUFF_SPEED 8	//the index of rideBuff where speed starts
#define RIDE_BUFF_DISTANCE 13 //the index of rideBuff where the distance starts

//******** Internal Functions ********
//these functions do start start or end an I2C communication themselves
uint8_t send_4bit_nibble(uint8_t nibble);
uint8_t send_byte(uint8_t byte, uint8_t commonNibble);
uint8_t send_character(uint8_t c);
uint8_t set_DDRAM_location(uint8_t desiredAddress);
uint8_t LCD_send_line(uint8_t lineNum);
uint8_t send_instruction(uint8_t instruction);


//******** Callable Functions ********
uint8_t LCD_init(uint8_t saddr, uint8_t numLinesFont, uint8_t onCursorBlink, uint8_t entryModeSet);
uint8_t LCD_update_image(uint8_t saddr);
uint8_t updateSpeedLCD(uint8_t saddr);
uint8_t updateRideLCD(uint8_t saddr);

#endif /* LCD_DRIVER_H_ */