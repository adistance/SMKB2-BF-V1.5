#ifndef DRIVER_DELAY_H
#define DRIVER_DELAY_H

#include <rtl876x.h>
#include <app_task.h>

void delay_us(unsigned int nus);
void delay_us_Ex(unsigned int nus);

void delay_ms(unsigned int nms);
void dumpBuff(unsigned char *pBuff, unsigned int len);
    
#endif

