#include <p30f4013.h>
#include "config.h"
#include "esp12e.h"
#include "uart.h"

unsigned char wfStatus;


void initESP12E(void) {
    wfStatus=0;
    resetWIFI();
    wfStatus++;
    while (wfStatus<2){
        cmdUART1("print(wifi.sta.status())\r\n");
        doUART();
        DELAY_MS(100);
    }
    cmdUART1("srv=net.createServer(net.TCP);srv:listen(80, function(conn) conn:on(\"receive\",function(client,request) print(request);wClient=client;end)end);\r\n");  //hello\"); client:close(); collectgarbage(); end) end)\n\r");
    logUART2("Running.\r\n");
}

void resetWIFI(void){
    logUART2("Reset ESP12E.\r\n");
    DELAY_MS(10);
    LATFbits.LATF0 = 0;
    DELAY_MS(10);
    LATFbits.LATF0 = 1;
    DELAY_MS(10);
}

