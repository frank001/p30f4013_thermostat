#include <p30F4013.h>

#include "x10.h"

//https://bitbucket.org/fuzzillogic/433mhzforarduino/wiki/Home

//http://ww1.microchip.com/downloads/en/AppNotes/91045a.pdf

//http://ww1.microchip.com/downloads/en/AppNotes/00831b.pdf



void __attribute__((__interrupt__, no_auto_psv)) _IC1Interrupt(void) {      //input capture interrupt
    unsigned char d;
    d=PORTDbits.RD8;
    IFS0bits.IC1IF = 0; 
}

void __attribute__((__interrupt__, no_auto_psv)) _T3Interrupt(void) {      //timer 3 interrupt
    unsigned char d;
    d=0;
    IFS0bits.T3IF = 0; 
}




void initX10(void) {
    
    //*
    TRISDbits.TRISD8 = 1;   //set pin 23/IC1/RD8 as input
    IC1CONbits.ICM0 = 1;    //trigger on rising and falling edge
    
    IFS0bits.IC1IF = 0;     //reset input capture interrupt flag
    IEC0bits.IC1IE = 1;     //enable input capture interrupt
    //IC1CONbits.ICTMR = 0;   //timer 3 resource
    //*/
    
    T3CON = 0;              //clear timer 3 configuration
    PR3 = 0xffff;           //timer 3 free running
    
    //T1CONbits.TCS = 1;      //timer 3 clock source internal
    IPC1bits.T3IP = 1;      //set priority of timer 3 interrupt
    IFS0bits.T3IF = 0;      //reset timer 3 interrupt
    IEC0bits.T3IE = 1;      //enable timer 3 interrupt
    
    T3CONbits.TON = 1;       //timer 3 enabled
}

