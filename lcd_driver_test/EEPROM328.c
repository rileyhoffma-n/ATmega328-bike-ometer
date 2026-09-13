/*
 * EEPROM328.c
 *
 * Created: 9/10/2026 3:03:25 PM
 *  Author: shelj
 */ 

#include "EEPROM328.h"

void updateEEPROM_8(uint16_t address, uint8_t data){	//places data into address of EEPROM if it is different than what's already there
	uint8_t existingData = readEEPROM_8(address);	//read the data that is currently in the address you want to update
	if(existingData != data){	//if the value you want to put into EEPROM is different, then write it in
		writeEEPROM(address, data);
	}
	
	//otherwise, the values are the same, so there is no point of writing
}

void updateEEPROM_16(uint16_t address, uint16_t data){	//places low byte of uint16_t data into address, and high byte into address + 1
	uint8_t lowByte = data & 0x00FF;	//isolate the low byte of the data
	uint8_t highByte = (data >> 8) & 0x00FF;		//shift the high byte into the position of the low byte to isolate it into a uint8_t
	
	updateEEPROM_8(address, lowByte);	//place the low byte into address
	updateEEPROM_8((address + 1), highByte);	//place the high byte into address + 1
}

void updateEEPROM_32(uint16_t address, uint32_t data){	//places the low byte at address, the 2nd lsb at address + 1, etc
	uint16_t lowHalf = data & 0x0000FFFF;	//isolate the lowest 16 bits
	uint16_t highHalf = (data >> 16) & 0x0000FFFF;	//shift the high half into position and isolate it
	
	updateEEPROM_16(address, lowHalf);
	updateEEPROM_16((address + 2), highHalf);
}

uint8_t readEEPROM_8(uint16_t address){	//returns the byte stored at address in EEPROM
	while(EECR & (1<<EEPE)){};	//infinite wait until any previous write operation is complete
		
	EEAR = address;	//place 16 bit address into the 2 registers before starting the read
	EECR |= (1<<EERE);	//place read enable bit into control register, starting the read operation
	//the CPU is halted for four cycles, then the data is immediately available in EEDR
	return EEDR;	//return the value in the data register
}

uint16_t readEEPROM_16(uint16_t address){	//reads two consecutive bytes from EEPROM starting at address
	uint16_t result = (uint16_t)readEEPROM_8(address) & 0x00FF;	//read the low byte into the result, clear the upper byte
	result = result | ( ((uint16_t)readEEPROM_8(address + 1) << 8) & 0xFF00 );	//read high byte, shift into position, mask it, combine it with low byte
	return(result);
}

uint32_t readEEPROM_32(uint16_t address){	//reads four consecutive bytes from EEPROM starting at address
	uint32_t result = (uint32_t)readEEPROM_16(address) & 0x0000FFFF;
	result = result | ( ((uint32_t)readEEPROM_16(address + 2) << 16) & 0xFFFF0000 );
	return(result);
}

void writeEEPROM(uint16_t address, uint8_t data){	//writes data into address of EEPROM regardless of what is there
	while(EECR & (1<<EEPE)){};	//infinite wait until any previous write operation is complete
		
	// Set up address and data register before writing
	EEAR = address;
	EEDR = data;
	
	EECR |= (1<<EEMPE);	//write a one to EEMPE to enable a write to be triggered when EEPE is 0
	//you have 4 clock cycles after EEMPE = 1 to put a one in EEPE, so do so immediately
	EECR |= (1<<EEPE);	//start the EEPROM write by putting a one in EEPE
	
	//EEPE will automatically after the write.  It typically takes 3.3 ms
}