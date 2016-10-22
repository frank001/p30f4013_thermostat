#include <p30F4013.h>
#include <libpic30.h>
#include "eeprom.h"


void write_eedata(unsigned char nodeIdx, unsigned char subIndex) {  //, unsigned char * data
    _prog_addressT EE_addr;
    int temp_word=0x0102;
    unsigned char i;
    
    EE_addr = 0x7FF000 + nodeIdx * 0x20 + subIndex * 0x08;
    //disableInterrupts();
    for (i=0;i<8;i+=2) {
        //temp_word = (data[i+1]<<8) | data[i];
        _erase_eedata(EE_addr+i,_EE_WORD);
        _wait_eedata();
        _write_eedata_word(EE_addr+i, temp_word);
        _wait_eedata();
    }
    //enableInterrupts();
   
    
}
/*
 
void read_eedata(unsigned char nodeIdx, unsigned char subIndex, struct Module * module) {
    union {
        float fvalue;
        unsigned int bArray[2];
    } u[2];
    unsigned char i, j;
    int temp_word[1];
    float f;
    _prog_addressT EE_addr;
    
    j=0;
    EE_addr = 0x7FF000 + nodeIdx * 0x20 + subIndex * 0x08;
    for(i=0 ; i<4 ; i+=2) {
        _memcpy_p2d16(temp_word, EE_addr+(i*0x02), _EE_WORD);
        Nop();
        u[j].bArray[0] = temp_word[0];
        _memcpy_p2d16(temp_word, EE_addr+(i*0x02)+0x02, _EE_WORD);
        Nop();
        u[j].bArray[1] = temp_word[0];
        j++;
    }
    if (!(NaN(u[0].fvalue) && NaN(u[1].fvalue)))  {
        module->calibration[subIndex].a_factor = u[0].fvalue;
        module->calibration[subIndex].b_factor = u[1].fvalue;
    }
}

 */







