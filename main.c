#include <p30f4013.h>
#include "config.h"
#include "timers.h"
#include "i2c.h"
#include "bme280.h"
#include "hyt221.h"
#include "esp12e.h"

//#include "x10.h"
//#include "eeprom.h"

int FOSC __attribute__((space(prog), address(0xF80000))) = 0xC70A ;     //FRC w/PLL 8x //64MHz
int FWDT __attribute__((space(prog), address(0xF80002))) = 0x00 ;       //WDT OFF


//http://www.esp8266.com/viewtopic.php?f=13&t=5630

extern volatile unsigned char ms100Tick;
extern volatile unsigned char sec1Tick;
extern volatile unsigned char sec10Tick;


extern long bme280_temperature;
extern long unsigned int bme280_humidity;
extern long unsigned int bme280_pressure;

extern unsigned long hyt221_temperature;
extern unsigned long hyt221_humidity;

extern unsigned int i2c_reg_addr;
extern unsigned char i2c_reg_map[];

unsigned int lightlevel = 0;

unsigned char bme280_sample_taken=0;
unsigned char hyt221_sample_taken=0;
unsigned char envParam_written=0;
unsigned char envParam_read=0;

void init() {
    TRISB = 0;              //set port B to output

    TRISBbits.TRISB0 = 0;   //heartbeat led 
    TRISBbits.TRISB1 = 1;   //ESP8266 reset button
    TRISBbits.TRISB2 = 1;   //ESP8266 request I2C bus
    
    TRISBbits.TRISB3 = 0;   //ESP8266 interrupt 
    LATBbits.LATB3 = 0;
    //TRISFbits.TRISF0 = 0;   //output: WIFI reset
    //TRISDbits.TRISD2 = 1;   //input: switch
    

    TRISBbits.TRISB8 = 1;   //input: light sensor
    ADPCFG = 0xffff;
    //ADPCFGbits.PCFG2 = 1;
    ADPCFGbits.PCFG8 = 0;   //analog input
    ADCON1 = 0x04;             //ASAM bit=1 
    ADCON2 = 0;             //voltage reference
    ADCON3 = 0x05;          //Manual sample
    ADCON1bits.ADON = 1;    //turn ADC on
    
}

unsigned int read_light_sensor(void) {
    ADCHS = 0x08;               //select channel
    //ADCON1bits.SAMP  = 1;       //start sampling
    DELAY_US(1);                //give some time for measurement
    ADCON1bits.SAMP = 0;        //start converting
    while (ADCON1bits.DONE);    //wait for the convert
    return ADCBUF0;
}

void writeEnvParam(long unsigned int param){
    unsigned char i;
    for (i=0;i<4;i++) {
        i2c_reg_map[i2c_reg_addr++] = (param>>(i*8)) & 0xff ;
    }
}

void triggerESP12E() {
    LATBbits.LATB3 = 1;
    Nop();
    LATBbits.LATB3 = 0;
}


void takeSample(unsigned char (*psample)(void), unsigned char *psample_taken, volatile unsigned char tick) {
    if (tick && !*psample_taken) {  
        psample();
        *psample_taken=1;
    }
    if (*psample_taken && !tick) *psample_taken=0;
    
}

void setESP12E_Data() {
    i2c_reg_addr=0;
    
    //first register is command for esp12e, 0 - just read the envParams, 1 - do something else
    i2c_reg_map[i2c_reg_addr++]=0x00;
    
    //BME280 
    writeEnvParam(bme280_temperature);
    writeEnvParam(bme280_humidity);
    writeEnvParam(bme280_pressure);

    //HYT221
    writeEnvParam(hyt221_temperature);
    writeEnvParam(hyt221_humidity);
    
    
}

void test() {
    unsigned char i;
    i2c_reg_addr=0;
    i2c_reg_map[i2c_reg_addr++]=0x01;
    for (i=0;i<32;i++) i2c_reg_map[i2c_reg_addr++]=0;
    
}


int main(void) {
    unsigned int i;
    unsigned long int j;
    unsigned char i2c_release;
    
    
    //write_eedata(0, 0);     //eeprom test
    //i=RCON;
    init();             //self initialize
    timers_init();      //setup timers
    
    i2c_init();         //setup I2C
    bme280_init();      //temp/hum/press sensor
    hyt221_init();      //temp/hum sensor

    //takeSample(&bme280_sample, &bme280_sample_taken, 1);
    //takeSample(&hyt221_sample, &hyt221_sample_taken, 1);
    
    //setESP12E_Data();
    //envParam_written=1;
    //triggerESP12E();
    
    ms100Tick=0;
    sec1Tick=0;
    
    while (1) {

        i2c_release = PORTBbits.RB2;

        if (i2c_release) { 
            takeSample(&bme280_sample, &bme280_sample_taken, !ms100Tick);    //1 sample / second
            takeSample(&hyt221_sample, &hyt221_sample_taken, !sec1Tick);     //1 sample / 10 seconds
            if (!sec1Tick && !envParam_written) { 
                setESP12E_Data();
                envParam_written=1;
                triggerESP12E();
            } else if (envParam_written && !sec1Tick) { 
                envParam_written=0; 
                test(); 
                triggerESP12E();
            }
        }
        
    }

    return 0;
}
