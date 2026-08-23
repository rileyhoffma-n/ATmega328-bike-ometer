/*
 * I2C_328.c
 *
 * Created: 6/2/2026 3:06:19 PM
 *  Author: Riley Hoffman
 */ 

#include "I2C_328.h"

void I2C_init(void){
	TWBR = 0x00;	//lowest TBR value for highest I2C speed
	TWSR &= ~0x03;	//clear TWPS1,0 to get lowest prescale value for highest speed
	//TWCR = 0x04;	//TWEN is set to enable TWI hardware, not using interrupts here TWEN WILL BE SET BY SENDING START ANYWAY
}

uint8_t I2C_start_write(uint8_t saddr){ //starts an I2C write frame, returns 0xFF if successful, 0x00 if not
	uint8_t status;
	uint16_t timeout = 10000;	//with 1 MHz clock this will be over 10 ms
	
	//send start condition
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);	//enable TWI and send start condition
	while(!(TWCR & (1 << TWINT))){	//wait with timeout until TWINT set
		if(timeout == 0){
			return 0x00;
		}
		timeout--;
	}	
	
	status = TWSR & TWSR_CODE_bm;	//get the status code
	if((status != TWSR_START) && (status != TWSR_REPSTART)){	//if the start or repeated start condition was never sent
		return 0x00;
	}
	
	//address slave
	TWDR = (saddr << 1);	//load SLA+W into data reg
	TWCR = (1 << TWINT) | (1 << TWEN);	//clear TWINT to allow the SLA+W to send
	timeout = 10000;	//reset the timeout counter
	while(!(TWCR & (1 << TWINT))){	//wait with timeout until TWINT set
		if(timeout == 0){
			TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);	//send STOP since timeout occurred
			return 0x00;
		}
		timeout--;
	}
	
	status = TWSR & TWSR_CODE_bm;	//get the status code
	if(status == TWSR_SLAW_ACK){	//if the slave acknowledges that it has been addressed
		return 0xFF;
	}
	else{	//if not
		TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);	//send STOP since the addressing failed
		return 0x00;
	}
}

uint8_t I2C_send_byte(uint8_t data){	//assumes the frame has been started already, returns 0xFF if data is acknowledged, 0x00 if not
	uint8_t status;
	uint16_t timeout = 10000;	//with 1 MHz clock this will be over 10 ms
	
	TWDR = data;	//load the data reg
	TWCR = (1 << TWINT) | (1 << TWEN);	//clear TWINT to allow the data to send
	
	while(!(TWCR & (1 << TWINT))){	//wait with timeout until TWINT set
		if(timeout == 0){
			TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);	//send STOP since timeout occurred
			return 0x00;
		}
		timeout--;
	}
	
	status = TWSR & TWSR_CODE_bm;	//get the status
	if((status) == TWSR_DATA_SENT_ACK){
		return 0xFF;
	}
	else{	//if not
		TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);	//send STOP since the data send failed
		return 0x00;
	}
}

void I2C_stop(void){
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}