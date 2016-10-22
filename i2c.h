/* 
 * File:   i2c.h
 * Author: frans
 *
 * Created on 9 oktober 2016, 12:09
 */

#ifndef I2CDSPIC_H
#define	I2CDSPIC_H

#define I2C_ADDRESS 0x10
#define I2CWRITE 0x00
#define I2CREAD 0x01



void __attribute__((interrupt, no_auto_psv)) _SI2CInterrupt(void);
void __attribute__((interrupt, no_auto_psv)) _MI2CInterrupt(void);

void testI2C(void);

unsigned char readI2C(unsigned char, unsigned char, unsigned char);
unsigned char writeI2C(unsigned char, unsigned char, unsigned char);

void i2c_init(void);
void i2c_wait(void);
void i2c_start(void);
void i2c_restart(void);
void i2c_stop(void);
void i2c_write(unsigned char);
void i2c_address(unsigned char, unsigned char);
void i2c_read(unsigned char);



#endif	/* I2C_H */

