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




int main(void) {
    unsigned int i;
    
    //write_eedata(0, 0);     //eeprom test
    i=RCON;
    TRISB = 0;              //led indicators
    TRISFbits.TRISF0 = 0;   //output: WIFI reset
    TRISDbits.TRISD2 = 1;   //input: switch
    
    initUART();
    logUART2("\r\nThermostat initializing.\r\n");
    
    bme280_init();      //temp/hum/press sensor
    //initX10();          //remote control transmitter/sensor
    initESP12E();       //wireless module
    
    DELAY_MS(500);
    
    //testI2C();
        
    while (1) {
        if (!PORTDbits.RD2 && i==0) {
            i=1;
            //logUART2("I2C test.\n\r");
            //testI2C();
        }
        if (i & PORTDbits.RD2) {
            i=0;
        }
        LATBbits.LATB0 = i;
        doUART();
        
        
        /*
        LATBbits.LATB0 = 0x01;
        DELAY_MS(100);
        LATBbits.LATB0 = 0x00;
        DELAY_MS(100);
        */
    }
    
    return 0;
}
