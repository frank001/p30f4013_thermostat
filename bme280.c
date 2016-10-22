#include <p30f4013.h>
#include "config.h"
#include "i2c.h"
#include "bme280.h"

extern unsigned int i2c_reg_addr;
extern unsigned char i2c_reg_map[];

unsigned char bme280_id = 0;
unsigned char bme280_mode = 0;
unsigned char bme280_sampledata = 0;
unsigned char bme280_data[8];
unsigned char bme280_comp[32];
unsigned char bme280_comp_cntr;

long t_fine;

long bme280_temperature;
long unsigned int bme280_pressure;
long unsigned int bme280_humidity;


unsigned char bme280_init(void) {
    unsigned int mode=(OSS<<5) | (OSS<<2) | 0x03;    //temp & hum sampling 1 and normal mode
    //unsigned char error=0xff;

    if (readI2C(BME280_ADDRESS, BME280ID_REG, 1)) return 1;         //read error
    bme280_id=i2c_reg_map[i2c_reg_addr-1];
    //validate device
    if (bme280_id!=BME280_ID) {
        bme280_id=0;
        return 3;                             //wrong device
    }
    //set humidity oversampling
    if (writeI2C(BME280_ADDRESS, BME280HUMSAM_REG, OSS)) return 2;     //write error     
    //set normal mode, temp & pressure oversampling
    if (writeI2C(BME280_ADDRESS, BME280MODE_REG, mode)) return 2;     //write error
    //get device mode
    if (readI2C(BME280_ADDRESS, BME280MODE_REG, 1)) return 1;         //read error
    bme280_mode=i2c_reg_map[i2c_reg_addr-1];
    //validate mode
    if (bme280_mode!=mode) return 3;                        //invalid mode 
    
    bme280_comp_cntr = 0;
    bme280_sampledata = 2;
    readI2C(BME280_ADDRESS, 0x88, 24);                   //read compensation values for temperature and pressure
    readI2C(BME280_ADDRESS, 0xA1, 1);                   //read compensation value for humidity
    readI2C(BME280_ADDRESS, 0xE1, 7);                   //read compensation value for humidity
    bme280_sampledata = 0;
    
    DELAY_MS(20);                                           //give some time to start measurements
    return 0;
}

unsigned char bme280_sample(void) {                     //TODO: return error code
    if (!bme280_id) return 1;                            //not initialized    
    bme280_sampledata = 1;
    readI2C(BME280_ADDRESS, 0xF7, 8);                   //read temp/hum/pressure
    bme280_sampledata = 0;
    calcTemperature();
    calcPressure();
    calcHumidity();
    return 0;
}

//t_fine;
//bme208_temperature;
long calcTemperature() {
    long var1, var2, adcT;
    unsigned short digT1 = bme280_comp[1]<<8 | bme280_comp[0];
    short digT2 = bme280_comp[3]<<8 | bme280_comp[2];
    short digT3 = bme280_comp[5]<<8 | bme280_comp[4];
    
    adcT = (unsigned long)bme280_data[3]<<12 | (unsigned long)bme280_data[4]<<4 | (unsigned long)bme280_data[5]>>4;
    var1  = ((((adcT>>3) - ((long)digT1<<1))) * ((long)digT2)) >> 11;
    var2  = (((((adcT>>4) -((long)digT1)) * ((adcT>>4) -((long)digT1))) >> 12) * ((long)digT3)) >> 14;
    
    t_fine = var1 + var2;
    bme280_temperature = (t_fine*5+128)>>8;
    return bme280_temperature;
}

long unsigned int calcPressure() {
    long adcP;
    unsigned short digP1;
    short digP2, digP3, digP4, digP5, digP6, digP7, digP8, digP9;
    long long int var1, var2, p;
    
    digP1 = bme280_comp[7]<<8 | bme280_comp[6];
    digP2 = bme280_comp[9]<<8 | bme280_comp[8];
    digP3 = bme280_comp[11]<<8 | bme280_comp[10];
    digP4 = bme280_comp[13]<<8 | bme280_comp[12];
    digP5 = bme280_comp[15]<<8 | bme280_comp[14];
    digP6 = bme280_comp[17]<<8 | bme280_comp[16];
    digP7 = bme280_comp[19]<<8 | bme280_comp[18];
    digP8 = bme280_comp[21]<<8 | bme280_comp[20];
    digP9 = bme280_comp[23]<<8 | bme280_comp[22];
    
    
    adcP = (unsigned long)bme280_data[0]<<12 | (unsigned long)bme280_data[1]<<4 | (unsigned long)bme280_data[2]>>4;
    
    
    var1 = ((long long int)t_fine) - 128000;
    var2 = var1 * var1 * (long long int)digP6;
    var2 = var2 + ((var1*(long long int)digP5)<<17);
    var2 = var2 + (((long long int)digP4)<<35);
    var1 = ((var1 * var1 * (long long int)digP3)>>8) + ((var1 * (long long int)digP2)<<12);
    var1 = (((((long long int)1)<<47)+var1))*((long long int)digP1)>>33;
    if (var1 == 0){
        return 0; 
    // avoid exception caused by division by zero
    }
    p = 1048576 - adcP;
    p = (((p<<31)-var2)*3125)/var1;
    var1 = (((long long int)digP9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((long long int)digP8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((long long int)digP7)<<4);
    bme280_pressure = (long unsigned int) p;
    return bme280_pressure;

    
}


long unsigned int calcHumidity(void) {
    long int adcH;
    unsigned char digH1, digH3;
    short digH2, digH4, digH5, digH6;
    
    digH1 = bme280_comp[24];
    digH2 = bme280_comp[26]<<8 | bme280_comp[25];
    digH3 = bme280_comp[27];
    digH4 = bme280_comp[28]<<4 | (bme280_comp[29]&0x07);
    digH5 = bme280_comp[30]<<8 | (bme280_comp[29]>>4);  
    
    digH5=0;
            
    //digH4 = bme280_comp[29]<<8 | bme280_comp[28];
    //digH5 = bme280_comp[31]<<8 | bme280_comp[30];
    digH6 = bme280_comp[31];
    
    
    adcH = (unsigned long)bme280_data[6]<<8 | (unsigned long)bme280_data[7];
    
    long int v_x1_u32r;
    v_x1_u32r = (t_fine - ((long int)76800));
    v_x1_u32r = (((((adcH << 14) - (((long int)digH4) << 20) - (((long int)digH5) * v_x1_u32r)) + ((long int)16384)) >> 15) 
            * (((((((v_x1_u32r * ((long int)digH6)) >> 10) * (((v_x1_u32r * ((long int)digH3)) >> 11) + ((long int)32768))) >> 10) 
            + ((long int)2097152)) * ((long int)digH2) + 8192) >> 14));
    
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >>7) * ((long int)digH1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r); 
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    bme280_humidity = (long unsigned int)(v_x1_u32r>>12);
    return bme280_humidity;        
    
    
}
