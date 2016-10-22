#include <p30f4013.h>
#include "config.h"
#include "uart.h"
#include "http.h"

//lua tcp server
//srv=net.createServer(net.TCP)
//srv:listen(80, function(conn) conn:on("receive",function(client,request) print(request); client:send("hello"); client:close(); collectgarbage(); end) end)


unsigned char HTTPGET[]="GET / HTTP/";
unsigned char PANIC[]="PANIC: ";
unsigned char httpRequest=0;
unsigned char waitAnswer =0;
unsigned char cmdLength =0;
extern unsigned char wfStatus;

unsigned char rxU1Buffer[MAXBYTES];
unsigned char rxU1Ptr = 0;
unsigned char readU1Ptr = 0;
unsigned char rcvLine[MAXBYTES];
unsigned char rcvPtr = 0;

unsigned char rxU2Buffer[MAXBYTES];
unsigned char rxU2Ptr = 0;
unsigned char readU2Ptr = 0;

unsigned char txU1Buffer[MAXBYTES];
unsigned char txU1Ptr = 0;
unsigned char writeU1Ptr = 0;

unsigned char txU2Buffer[MAXBYTES];
unsigned char txU2Ptr = 0;
unsigned char writeU2Ptr = 0;

unsigned char cntrCRLF=0;

void initUART(void) {       
    //initialize UART1
    U1MODEbits.ALTIO = 1;   //Use alternate IO pins for UART1
    _TRISC13 = 0;            //UART1 TX
    _TRISC14 = 1;            //UART1 RX
    U1BRG = UBRG1_VALUE;
    IEC0bits.U1RXIE=1;
    U1STA&=0xfffc;
    U1MODEbits.UARTEN=1;
    U1STAbits.UTXEN=1;
    
    //initialize UART2
    _TRISF5 = 0;            //UART2 TX
    _TRISF4 = 1;            //UART2 RX
    U2BRG = UBRG2_VALUE;
    IEC1bits.U2RXIE=1;
    U2STA&=0xfffc;
    U2MODEbits.UARTEN=1;
    U2STAbits.UTXEN=1;
 }

void doUART() {
    //Connect UARTS for console screen usage
    unsigned char rcv;
    while (writeU1Ptr!=rxU1Ptr) {
        rcv=rxU1Buffer[writeU1Ptr++];
        writeU1Ptr%=MAXBYTES;
        switch (wfStatus) { //skip the garbage when the WIFI resets
            case 0: break;
            case 2: 
                U2TXREG=rcv;
                while (!U2STAbits.TRMT) ;
            default:
                readUART1(rcv);
                break;
        }
    }
    
    //rcv=rxU1Buffer[writeU1Ptr++];
    //U2TXREG=rcv;
    //while (!U2STAbits.TRMT) ;
    
    readUART1(rcv);
    while (writeU2Ptr!=rxU2Ptr) {
        //write to txU1Buffer
        txU1Buffer[readU1Ptr++]=rxU2Buffer[writeU2Ptr++];
        readU1Ptr%=MAXBYTES;
        writeU2Ptr%=MAXBYTES;
    }
    
    //write log messages to UART2 (console)
    while (txU2Ptr!=readU2Ptr) {
        U2TXREG=txU2Buffer[txU2Ptr++];
        txU2Ptr%=MAXBYTES;
        while (!U2STAbits.TRMT) ;
    }
    //handle commands for UART1 (WIFI module)
    while (txU1Ptr!=readU1Ptr) {
        U1TXREG=txU1Buffer[txU1Ptr++];
        txU1Ptr%=MAXBYTES;
        while (!U1STAbits.TRMT);
        waitAnswer=1;
    }
}

void logUART2(char* txt) {
    unsigned char i=0;
    while (txt[i]!=0) {
        txU2Buffer[readU2Ptr++]=txt[i++];
    }
}

void cmdUART1(char* txt) {
    unsigned char i=0;
    while (txt[i]!=0) {
        txU1Buffer[readU1Ptr++]=txt[i++];
        readU1Ptr%=MAXBYTES;
    }
}

void readUART1(unsigned char rx) {
    unsigned char i=0;
    unsigned char cntr=0;
    
    if (rx==0x0d || rx ==0x0a) { 
        cntrCRLF++;
        if (cntrCRLF==2) {
            if (waitAnswer) {
                waitAnswer=0;
            } else {
                if (wfStatus==1 && rcvLine[0]=='5') wfStatus++;     //we are connected
                //Lets see what we've got
                for (i=0;i<rcvPtr;i++) {                            //detect panic from WIFI module
                    if (rcvLine[i]==PANIC[cntr]) cntr++; 
                    if (cntr && rcvLine[i]!=PANIC[cntr-1]) cntr=0;
                    if (cntr==sizeof(PANIC)-1) {                    //something went wrong with the WIFI
                        initESP12E();
                        break;
                    }
                }
               if (!httpRequest) {                                  //detect HTTP request
                    for (i=0;i<rcvPtr;i++) {
                        if (rcvLine[i]==HTTPGET[cntr]) cntr++; 
                        if (cntr && rcvLine[i]!=HTTPGET[cntr-1]) cntr=0;
                        if (cntr==sizeof(HTTPGET)-1) httpRequest=1;
                        if (httpRequest) break;
                    }
                }
                
            }
            rcvPtr=0;
        }
        
        if (httpRequest && cntrCRLF==4) {
            handleHTTP();
            httpRequest=0;
            cntrCRLF=0;
            rcvPtr=0;
        };       
    } else { 
        rcvLine[rcvPtr++]=rx;
        if (cntrCRLF>0) cntrCRLF=0;
    }
}

void __attribute__((__interrupt__, __no_auto_psv__)) _U1RXInterrupt(void) {
    unsigned char data = U1RXREG;
    rxU1Buffer[rxU1Ptr++] = data;
    rxU1Ptr%=MAXBYTES;
    //readUART1(data);
    IFS0bits.U1RXIF = 0;        //clear interrupt flag
}


void __attribute__((__interrupt__, __no_auto_psv__)) _U2RXInterrupt(void) {
    unsigned char data = U2RXREG;
    rxU2Buffer[rxU2Ptr++] = data;
    rxU2Ptr%=MAXBYTES;
    IFS1bits.U2RXIF = 0;        //clear interrupt flag
}


/*

void __attribute__((__interrupt__, __no_auto_psv__)) _U1TXInterrupt(void) {
    //TODO: disable TX Interrupts
    //U1TXREG = txU1Buffer[txU1Ptr++];
    //txU1Ptr%=MAXBYTES;
    //while (!U1STAbits.TRMT) ;
    //if (txU1Ptr==writeU1Ptr) 
    IEC0bits.U1TXIE=0;     //finished writing, disable TX interrupt
}

void __attribute__((__interrupt__, __no_auto_psv__)) _U2TXInterrupt(void) {
    //TODO: disable TX Interrupts
    //U2TXREG = txBuffer[txPtr++];
    //txPtr%=MAXBYTES;
    //while (!U2STAbits.TRMT) ;
    //if (txPtr==writePtr) 
    IEC1bits.U2TXIE=0;     //finished writing, disable TX interrupt
}
*/

