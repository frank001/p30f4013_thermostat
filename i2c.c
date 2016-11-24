#include <p30f4013.h>
#include "config.h"
#include <i2c.h>
#include "i2c.h"


volatile unsigned int i2c_byte_count;
volatile unsigned int i2c_reg_addr;
volatile unsigned char i2c_reg_map[MAXBYTES];       //TODO: make this as small as possible
unsigned char address_flag;                     //TODO: create a flag register/struct

extern unsigned char bme280_sampledata;                    //some more to flag
extern unsigned char bme280_data[];    
extern unsigned char bme280_comp[];
extern unsigned char bme280_comp_cntr;

extern unsigned char hyt221_sampledata;                     //more flagging
extern unsigned char hyt221_data[];

unsigned char data_flag;
unsigned char rcv;



//initialization
void i2c_init(void) {               //todo: return error code
    unsigned int config1;
    unsigned int d;

    TRISFbits.TRISF2 = 1;
    TRISFbits.TRISF3 = 1;
    
    config1 = (I2C_ON & I2C_IDLE_CON & I2C_CLK_HLD 
                & I2C_IPMI_DIS & I2C_7BIT_ADD 
                & I2C_SLW_DIS & I2C_SM_DIS & 
                I2C_GCALL_DIS & I2C_STR_DIS & 
                I2C_NACK & I2C_ACK_DIS & I2C_RCV_DIS & 
                I2C_STOP_DIS & I2C_RESTART_DIS 
                & I2C_START_DIS); 
    
    OpenI2C(config1, I2CBAUD);
    //I2CBRG = I2CBAUD;
    
    IEC0bits.MI2CIE = 1 ; // Master Interrupt Enable
    I2CCONbits.RCEN = 1;    //enable Master receive interrupt
    IFS0bits.MI2CIF = 0 ; // Clr Master Interrupt Flag
    I2CADD = I2C_ADDRESS;
    
    IEC0bits.SI2CIE = 1 ; // Slave Interrupt Enable
    IFS0bits.SI2CIF = 0 ; // Clr Slave Interrupt Flag
    
    /*
    
    
    I2CCONbits.I2CSIDL = 0 ;
    I2CCONbits.SCLREL = 1 ;
    I2CCONbits.IPMIEN = 0 ;
    I2CCONbits.A10M = 0 ; // 10 bit Address not supported (use of 7 bit)
    I2CCONbits.SMEN = 0 ; // Disable SMBus Compatibility
    I2CCONbits.GCEN = 0 ; // Enable General Call Address (0x00)
    I2CCONbits.STREN = 0 ; // Enable Clock Stretch
    I2CCONbits.ACKDT = 0 ;
    
    I2CCONbits.RCEN = 1;    //enable Master receive interrupt
    //I2CCONbits. =1;
    //I2CADD = 0x27;            
    I2CADD = I2C_ADDRESS;
    
    IPC3bits.SI2CIP = 1 ; // Slave Interrupt Priority
    IPC3bits.MI2CIP = 1 ; // Master Interrupt Priority
    IEC0bits.SI2CIE = 1 ; // Slave Interrupt Enable
    IEC0bits.MI2CIE = 1 ; // Master Interrupt Enable
    IFS0bits.SI2CIF = 0 ; // Clr Slave Interrupt Flag
    IFS0bits.MI2CIF = 0 ; // Clr Master Interrupt Flag
    
    
    I2CCONbits.I2CEN = 1 ; // Enable I²C
    I2CSTATbits.I2COV=1;
     */
    
}
//I2C Master routines
unsigned char readI2C(unsigned char address, unsigned char index, unsigned char length) {       //TODO: return error code
    unsigned char data, cntr;
    //i2c_init();
    IdleI2C();
    StartI2C();
    
    IdleI2C();
    MasterWriteI2C((address<<1) | I2CWRITE);
    
    IdleI2C();
    MasterWriteI2C(index);
    IdleI2C();
    
    RestartI2C();
    DELAY_US(100);         
    IdleI2C();
    
    
    MasterWriteI2C((address<<1) | I2CREAD);
    //IdleI2C();
    
    cntr =0;
    while (cntr<length) {
        IdleI2C();
        data = MasterReadI2C();
        
        switch (bme280_sampledata) {
            case 1:
                bme280_data[cntr] = data;
                break;
            case 2:
                bme280_comp[bme280_comp_cntr++] = data;
                break;
            default:
                if (hyt221_sampledata) {
                    hyt221_data[cntr++] = data;
                } else {
                    i2c_reg_map[i2c_reg_addr++] = data;
                }
                break;
            
        }
        /*if (bme280_sampledata) {
            bme280_data[cntr] = data;
        } else {
            i2c_reg_map[i2c_reg_addr++] = data;
        }*/
        
        IdleI2C();
        //DELAY_US(100);
        if (cntr==(length-1)) NotAckI2C(); else AckI2C();
        IdleI2C();
        //DELAY_US(500);
        //DELAY_US(5);
        cntr++;
    }

    //IdleI2C();
    StopI2C();
    IdleI2C();
    //CloseI2C();
    return 0;
}

unsigned char writeI2C(unsigned char address, unsigned char index, unsigned char data) {        //TODO: return error code
    //i2c_init();
    IdleI2C();
    StartI2C();
    
    IdleI2C();
    MasterWriteI2C((address<<1) | I2CWRITE);
    
    IdleI2C();
    MasterWriteI2C(index);
    IdleI2C();
    
    MasterWriteI2C(data);
    IdleI2C();
    
    StopI2C();
    IdleI2C();
    //CloseI2C();
    
    
    return 0;
}



//I2C Master ISR
void __attribute__((interrupt, no_auto_psv)) _MI2CInterrupt(void) {
    unsigned char data;
    unsigned char i;
    data=I2CRCV;
    i2c_reg_map[i2c_reg_addr++] = data;
    i2c_reg_addr %= sizeof(i2c_reg_map);            //limit address to size of register map
    i=0;
    //_RCEN = 0;           //disable Master receive interrupt
    IFS0bits.MI2CIF = 0;    //clear the interrupt flag
    
}

//I2C Slave ISR routine
void __attribute__((interrupt, no_auto_psv)) _SI2CInterrupt(void) {
    unsigned char data;
    unsigned char tmp;
    unsigned char i;
    
    IEC0bits.SI2CIE = 0; //Disable interrupts
    //LATBbits.LATB2 = 1;
	if((I2CSTATbits.R_W == 0)&&(I2CSTATbits.D_A == 0)) {	//address matched, writing register address to slave
		tmp = I2CRCV;                               //dummy read to remove stored address value
		address_flag = 1;                           //next byte will be address
		data_flag = 0;
	}
	else if((I2CSTATbits.R_W == 0)&&(I2CSTATbits.D_A == 1)) {	//Check for data	
		if(address_flag) {
			address_flag = 0;	
			data_flag = 1;                          //next byte is data
            i2c_reg_addr = I2CRCV;                  //store address
			I2CCONbits.SCLREL = 1;                  //release SCL line
		} else 
            if(data_flag) {
            rcv = I2CRCV;
            //ReadClient(rcv);                     //pass data to websocket
            i2c_reg_map[i2c_reg_addr++] = rcv;   //store data into register, auto-increment
			I2CCONbits.SCLREL = 1;               //release SCL line
            
		}
	}
	else if((I2CSTATbits.R_W == 1)&&(I2CSTATbits.D_A == 0)) { //Been told to put data on bus, must already have desired register address
		tmp = I2CRCV;                               //ditch stored address
		I2CTRN = i2c_reg_map[i2c_reg_addr++];       //read data from register & send data to I2C transmit buffer, setting D/A in the process
		I2CCONbits.SCLREL = 1;                      //release SCL line
	} else 
        if((I2CSTATbits.R_W == 1)&&(I2CSTATbits.D_A == 1)) { //D/A hasn't been reset, master must want more data
		I2CTRN = i2c_reg_map[i2c_reg_addr++];       //read data from register & send it to I2C master device
		I2CCONbits.SCLREL = 1;                      //release SCL line
	}
	
    i2c_reg_addr %= sizeof(i2c_reg_map);            //limit address to size of register map
    
    _SI2CIF = 0;                                    //clear I2C1 Slave interrupt flag
    //LATBbits.LATB2 = 0;
	IEC0bits.SI2CIE = 1;                            //Enable interrupts
 
} 

//DEBUGGING
void testI2C() {
    //unsigned char d,i;
    //d=bme280_sample();
     
}


