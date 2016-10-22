/* 
 * File:   config.h
 * Author: frans
 *
 * Created on 6 oktober 2016, 16:12
 */

#ifndef CONFIG_H
#define	CONFIG_H
//default baud rate of ESP12E: 76800

#define MAXBYTES 256

#define FCY 16000000L  //define your instruction frequency, FCY = FOSC/4
#define CYCLES_PER_MS ((unsigned long)(FCY * 0.001))        //instruction cycles per millisecond
#define CYCLES_PER_US ((unsigned long)(FCY * 0.000001))   //instruction cycles per microsecond
#define DELAY_MS(ms)  __delay32(CYCLES_PER_MS * ((unsigned long) ms));   //__delay32 is provided by the compiler, delay some # of milliseconds
#define DELAY_US(us)  __delay32(CYCLES_PER_US * ((unsigned long) us));    //delay some number of microseconds

//UART Baud rates
#define UART1_BAUD 115200
#define UBRG1_VALUE (FCY/UART1_BAUD)/16 - 1

#define UART2_BAUD 115200
#define UBRG2_VALUE (FCY/UART2_BAUD)/16 - 1

//I2C Baud rate
#define FSCK 400000             //400kHz
#define I2C_BAUD FCY/FSCK



#endif	/* CONFIG_H */

