#include "driver_delay.h"
#include "os_sched.h"
#include "trace.h"

#include "mlapi.h"
#include "menu_sleep.h"
void delay_us(unsigned int nus)
{   
    uint32_t i;
    uint32_t j;

    for (j = 0; j < nus; j++)
    {
        for (i = 0; i < 28; i++)
        {
            __ASM volatile ("nop");
        }
    }
}

void delay_us_Ex(unsigned int nus)
{   
    uint32_t i;
    uint32_t j;

    for (j = 0; j < nus; j++)
    {
        for (i = 0; i < 10; i++)
        {
            __ASM volatile ("nop");
        }
    }
}

void delay_ms(unsigned int nms)
{
//	menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
    os_delay(nms);
}

void dumpBuff(unsigned char *pBuff, unsigned int len)
{
    unsigned int i;
    
    uint32_t tmp = len/64;
    uint32_t less = len%64;
    
    for(i = 0; i < tmp; )
    {
        APP_PRINT_INFO1("%b", TRACE_BINARY(64, &pBuff[i]));

        os_delay(10); 
        i += 64;
    }
    if(less)
        APP_PRINT_INFO1("%b", TRACE_BINARY(less, &pBuff[i]));
}

#if (TEST_FPS_SPEED == 1)
static unsigned int tick_start;
static unsigned char flag;
static unsigned int tick[3];

void timerStart(char step)
{
    if(step == 'C')
    {
        flag = 0;
    }
    else if(step == 'E')
    {
        flag = 1;
    }
    else if(step == 'M')
    {
        flag = 2;
    }
    
    tick_start = os_sys_tick_get();
}

uint32_t timerEnd(void)
{
    tick[flag] = os_sys_tick_get() - tick_start;
    if(flag == 2)
    {
        APP_PRINT_TRACE4("C:%-3d E:%-3d M:%-3d SUM:%d", tick[0], tick[1], tick[2], tick[0]+tick[1]+tick[2]);
    }
      
    return tick[flag];
}
#endif
