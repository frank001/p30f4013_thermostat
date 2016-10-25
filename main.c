#include <p30f4013.h>
//#include <stdio.h>
//#include <stdlib.h>
#include "config.h"
#include "uart.h"
#include "i2c.h"
#include "bme280.h"
#include "esp12e.h"
//#include "x10.h"
//#include "eeprom.h"

int FOSC __attribute__((space(prog), address(0xF80000))) = 0xC70A ;     //FRC w/PLL 8x //64MHz?
int FWDT __attribute__((space(prog), address(0xF80002))) = 0x00 ;       //WDT OFF

//http://www.esp8266.com/viewtopic.php?f=13&t=5630

extern unsigned char rxU1Buffer[];
extern unsigned char rxU1Ptr;
extern unsigned char wfStatus;

unsigned int lightlevel = 0;

unsigned int read_light_sensor(void) {
    ADCHS = 0x08;               //select channel
    ADCON1bits.SAMP  = 1;       //start sampling
    DELAY_US(1);                //give some time for measurement
    ADCON1bits.SAMP = 0;        //start converting
    while (ADCON1bits.DONE);    //wait for the convert
    return ADCBUF0;
}


int main(void) {
    unsigned int i;
    unsigned long int j;
    
    //write_eedata(0, 0);     //eeprom test
    i=RCON;
    TRISB = 0;              //led indicators
    TRISFbits.TRISF0 = 0;   //output: WIFI reset
    TRISDbits.TRISD2 = 1;   //input: switch
    
    TRISBbits.TRISB8 = 1;   //input: light sensor
    ADPCFGbits.PCFG8 = 0;   //analog input
    ADCON1 = 0;             //clear sampling
    ADCON2 = 0;             //voltage reference
    ADCON3 = 0x05;          //Manual sample
    ADCON1bits.ADON = 1;    //turn ADC on
    
    
    initUART();
    logUART2("\r\nThermostat initializing.\r\n");
    
    bme280_init();      //temp/hum/press sensor
    initX10();          //remote control transmitter/sensor
    initESP12E();       //wireless module
    
    DELAY_MS(500);
    
    testI2C();
    lightlevel = read_light_sensor();  
    bme280_sample();
    j=0;
    while (1) {
        if (!PORTDbits.RD2 && i==0) {
            i=1;
            //logUART2("I2C test.\n\r");
            //lightlevel = read_light_sensor();
            testI2C();
        }
        if (i & PORTDbits.RD2) {
            i=0;
        }
        LATBbits.LATB0 = i;
        doUART();
        j++;
        if (j>0xfffff) {                             //TODO: implement timing
            lightlevel = read_light_sensor();     
            bme280_sample();
            j=0;
        }
        
        /*
        LATBbits.LATB0 = 0x01;
        DELAY_MS(100);
        LATBbits.LATB0 = 0x00;
        DELAY_MS(100);
        */
    }
    
    return 0;
}
