#include "config.h"
#include "http.h"
#include "uart.h"
//#include "bme280.h"
#include "i2c.h"
#include <stdlib.h>
#include <string.h>

#define _FTOA_TOO_LARGE -2
#define _FTOA_TOO_SMALL -1

char *header="wClient:send(\"<HTML><body style='background-color:black;color:yellow;'>";
char *footer="</body></html>\")";
char *endconn="wClient:close();\r\ncollectgarbage();\r\n";

extern long bme280_temperature;
extern long unsigned int bme280_pressure;
extern long unsigned int bme280_humidity;

typedef union {
    long L;
    float F;
} LF_t;

char *ftoa(float f) {
    long mantissa, int_part, frac_part;
    short exp2;
    LF_t x;
    char *p;
    static char outbuf[15];

    //*status = 0;
    if (f == 0.0)   {
        outbuf[0] = '0';
        outbuf[1] = '.';
        outbuf[2] = '0';
        outbuf[3] = 0;
        return outbuf;
    }
    x.F = f;

    exp2 = (unsigned char)(x.L >> 23) - 127;
    mantissa = (x.L & 0xFFFFFF) | 0x800000;
    frac_part = 0;
    int_part = 0;

    if (exp2 >= 31) {
        //*status = _FTOA_TOO_LARGE;
        return 0;
    }
    else if (exp2 < -23) {
        //*status = _FTOA_TOO_SMALL;
        return 0;
    }   
    else if (exp2 >= 23)
        int_part = mantissa << (exp2 - 23);
    else if (exp2 >= 0) {
        int_part = mantissa >> (23 - exp2);
        frac_part = (mantissa << (exp2 + 1)) & 0xFFFFFF;
    }
    else /* if (exp2 < 0) */
    frac_part = (mantissa & 0xFFFFFF) >> -(exp2 + 1);

    p = outbuf;

    if (x.L < 0)
    *p++ = '-';

    if (int_part == 0)
    *p++ = '0';
    else    {
        ltoa(p, int_part, 10);
        while (*p)  p++;
    }
    *p++ = '.';

    if (frac_part == 0)
        *p++ = '0';
    else {
        char m, max;

        max = sizeof (outbuf) - (p - outbuf) - 1;
        if (max > 7)    max = 7;
        /* print BCD */
        for (m = 0; m < max; m++) {
            /* frac_part *= 10; */
            frac_part = (frac_part << 3) + (frac_part << 1);

            *p++ = (frac_part >> 24) + '0';
            frac_part &= 0xFFFFFF;
        }
        /* delete ending zeroes */
        for (--p; p[0] == '0' && p[-1] != '.'; --p);
        ++p;
    }
    *p = 0;
    return outbuf;
}



void handleHTTP(void) {
    unsigned char i;
    char *body;
    
    
    float t,h,p;
    char *temperature; //*humidity, *pressure;
    
    
    t=((float)bme280_temperature)/100;
    h=((float)bme280_humidity)/1024;
    p=((float)bme280_pressure)/256;
    
    //sprintf(&temperature,"%f",t);
    //temperature=ftoa(t);
    temperature = ftoa(t);
    
    //body="Temperature: ";
    //strcat(body, temperature);
    //body=temperature;

    cmdUART1(header);
    cmdUART1("Temperature: ");
    cmdUART1(ftoa(t));
    cmdUART1("°C.<br/>");
    
    cmdUART1("Humidity: ");
    cmdUART1(ftoa(h));
    cmdUART1("%.<br/>");
    
    cmdUART1("Pressure: ");
    cmdUART1(ftoa(p));
    cmdUART1(" Pa.<br/>");
    
    
    //cmdUART1(body);
    cmdUART1(footer);
    cmdUART1(endconn); //"
    testI2C();
}
