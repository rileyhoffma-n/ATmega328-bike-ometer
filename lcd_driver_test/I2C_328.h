/*
 * I2C_328.h
 *
 * Created: 5/26/2026 4:11:47 PM
 *  Author: Riley Hoffman
 */ 


#ifndef I2C_328_H_
#define I2C_328_H_

#include <avr/io.h>
#include <stdint.h>

//******** Macros ********

// codes assuming lower three bits have been masked to 000
#define TWSR_CODE_bm 0xF8
#define TWSR_START 0x08
#define TWSR_REPSTART 0x10
#define TWSR_SLAW_ACK 0x18
#define TWSR_SLAW_NACK 0x20
#define TWSR_DATA_SENT_ACK 0x28
#define TWSR_DATA_SENT_NACK 0x30
#define TWSR_ARB_LOST 0x38



//******** Internal Functions ********
uint8_t I2C_start_write(uint8_t saddr);
uint8_t I2C_send_byte(uint8_t data);

//******** Callable Functions ********
void I2C_init(void);
void I2C_stop(void);


#endif /* I2C_328_H_ */