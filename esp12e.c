#include <p30f4013.h>
#include "config.h"
#include "esp12e.h"


unsigned char wfStatus;



void initESP12E(void) {
    wfStatus=0;
    resetWIFI();
    wfStatus++;
    while (wfStatus<2){
        DELAY_MS(500);
    }
    
}

void resetWIFI(void){
    unsigned char d=0;
    if (d) {
        DELAY_MS(20);
    }
    else {
        DELAY_MS(10);
        LATFbits.LATF0 = 0;
        DELAY_MS(10);
        LATFbits.LATF0 = 1;
        DELAY_MS(10);
    }
}

