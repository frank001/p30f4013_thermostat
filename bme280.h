/* 
 * File:   bme280.h
 * Author: frans
 *
 * Created on 21 oktober 2016, 19:05
 */

#ifndef BME280_H
#define	BME280_H

#define BME280_ADDRESS 0x77
#define BME280ID_REG 0xd0
#define BME280MODE_REG 0xf4
#define BME280HUMSAM_REG 0xf2

#define BME280_ID 0x60
#define OSS 0x04               //8x oversampling 

unsigned char bme280_init(void);
unsigned char bme280_sample(void);
long calcTemperature(void);
long unsigned int calcPressure(void);
long unsigned int calcHumidity(void);
#endif	/* BME280_H */

