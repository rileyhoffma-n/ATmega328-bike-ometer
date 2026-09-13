/*
 * EEPROM328.h
 *
 * Created: 9/10/2026 3:03:11 PM
 *  Author: shelj
 */ 


#ifndef EEPROM328_H_
#define EEPROM328_H_

#include <avr/io.h>

/*THE EEPROM HAS AT LEAST 100,000 WRITE/ERASE CYCLES*/
//be sure to keep track of the addresses

/**** GLOBALS ****/


/**** MACROS ****/
//PLACE THE ADDRESSES OF VARIABLES HERE SO THAT YOU DON'T MIX THEM UP
#define CIRCUMFERENCE_EEPROM_ADDRESS 0	//it's a uint32, so it uses 0, 1, 2, 3
#define DISTANCE_EEPROM_ADDRESS 4		//it's a uint32 so 4, 5, 6, 7
#define SECONDS_EEPROM_ADDRESS 8		//it's a uint8, so just 8
#define MINUTES_EEPROM_ADDRESS 9		//it's a uint8, so just 9
#define HOURS_EEPROM_ADDRESS 10			//it's a uint8, so just 10
#define DAYS_EEPROM_ADDRESS 11			//it's a uint8, so just 11

#define TEST_8_ADDRESS	100
#define TEST_16_ADDRESS	101
#define TEST_32_ADDRESS	103

/**** CALLABLE FUNCTIONS ****/
void updateEEPROM_8(uint16_t address, uint8_t data);	//places uint8_t data into address of EEPROM if it is different than what's already there
void updateEEPROM_16(uint16_t address, uint16_t data);	//places low byte of uint16_t data into address, and high byte into address + 1
void updateEEPROM_32(uint16_t address, uint32_t data);	//places the low byte at address, the 2nd lsb at address + 1, etc
uint8_t readEEPROM_8(uint16_t address);	//returns the byte stored at address in EEPROM
uint16_t readEEPROM_16(uint16_t address);	//reads two consecutive bytes from EEPROM starting at address
uint32_t readEEPROM_32(uint16_t address);	//reads four consecutive bytes from EEPROM starting at address

/**** INTERNAL FUNCTIONS ****/
void writeEEPROM(uint16_t address, uint8_t data);	//writes data into address of EEPROM regardless of what is there

#endif /* EEPROM328_H_ */