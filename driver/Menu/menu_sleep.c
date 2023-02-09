#include "menu_sleep.h"

#include <os_timer.h>
#include "trace.h"

static unsigned int menu_sleep_event_flag = SLEEP_STATUS_INIT;
static unsigned int menu_sleep_event_timeout[MENU_SLEEP_EVENT_COUNT+1] = {0};
//bool bShort_Sleep_flag = 0;

#define SLEEP_TIMES      1000  //12s

//时间单位：10ms
#define TIMEOUT_CNT_MS(x)           (x/10)

/*
*函数说明:  对休眠事件的状态值进行复位
*函数名称:  void menu_sleep_event_init(void)
*形参:      无
*返回码:    无
*/
void menu_sleep_event_init(void)
{
    unsigned char i;
	
    menu_sleep_event_flag = SLEEP_STATUS_INIT;
    for(i=0; i<MENU_SLEEP_EVENT_COUNT; i++)
    {
        menu_sleep_event_timeout[i] = 0;
    }

	menu_sleep_event_timeout[0] = TIMEOUT_CNT_MS(SLEEP_TIMES);
}

/*
	重新设置mcu睡眠时间，让mcu快速进入低功耗
*/
void menu_sleep_event_reset()
{
	unsigned char i;
	
	for(i=0; i<MENU_SLEEP_EVENT_COUNT; i++)
    {
        menu_sleep_event_timeout[i] = 5;
    }
}

/*
*函数说明:  用于事件的状态进行设置
*函数名称:  void menu_sleep_event_control(unsigned char eventBit, bool isEnable)
*形参:      对应的事件位
*返回码:    无
*/
void menu_sleep_event_control(unsigned char eventBit, bool isEnable)
{
    
    if(isEnable)
    {
    	menu_sleep_event_flag = SLEEP_STATUS_INIT;
        //menu_sleep_event_flag |= MENU_SLEEP_BIT(eventBit);          //事件置位
        
        switch(eventBit)                                            //按照需要设置时间的休眠时间
        {
            case MENU_SLEEP_EVENT_WAKEUP_BIT:
//				if(bShort_Sleep_flag)
//					{
//						if(app_get_bt_real_state() == 0)
//							menu_sleep_event_timeout[eventBit] = TIMEOUT_CNT_MS(200);
//						else
							menu_sleep_event_timeout[eventBit] = TIMEOUT_CNT_MS(SLEEP_TIMES);
//					}
                break;
            
            case MENU_SLEEP_EVENT_SETKEY_BIT:
                menu_sleep_event_timeout[eventBit] = TIMEOUT_CNT_MS(SLEEP_TIMES);
                break;
            
            case MENU_SLEEP_EVENT_CLEANKEY_BIT:
                menu_sleep_event_timeout[eventBit] = TIMEOUT_CNT_MS(SLEEP_TIMES);
                break;
            
            case MENU_SLEEP_EVENT_TOUCH_BIT:
                menu_sleep_event_timeout[eventBit] = TIMEOUT_CNT_MS(SLEEP_TIMES);
                break;
            
            case MENU_SLEEP_EVENT_NFC_BIT:
                menu_sleep_event_timeout[eventBit] = TIMEOUT_CNT_MS(SLEEP_TIMES);
                break;
            
            case MENU_SLEEP_EVENT_FINGER_BIT:
                menu_sleep_event_timeout[eventBit] = TIMEOUT_CNT_MS(SLEEP_TIMES);
                break;
            
            case MENU_SLEEP_EVENT_VOICE_BIT:
                menu_sleep_event_timeout[eventBit] = TIMEOUT_CNT_MS(SLEEP_TIMES);
                break;
            
            case MENU_SLEEP_EVENT_UART_BIT:
                menu_sleep_event_timeout[eventBit] = TIMEOUT_CNT_MS(SLEEP_TIMES);
                break;
        }
        
    }
    else
    {
        menu_sleep_event_flag &= ~(MENU_SLEEP_BIT(eventBit));         //对应位清0及时间清0
        menu_sleep_event_timeout[eventBit] = 0;
    }
    
    return;
}

/*
*函数说明:  用于对事件的超时时间进行递减操作，系统可用一个定时器去调用该函数
*函数名称:  void menu_sleep_event_timeout_cnt_decrease(void)
*形参:      无
*返回码:    无
*/
void menu_sleep_event_timeout_cnt_decrease(void)
{
    unsigned char i;
	unsigned char flag = 0;
	
    for(i=0; i<MENU_SLEEP_EVENT_COUNT; i++)
    {
        if(menu_sleep_event_timeout[i] > 1)
        {
        	flag = 1;
            menu_sleep_event_timeout[i]--;
			if(menu_sleep_event_timeout[i] == 1)
				menu_sleep_event_flag = SLEEP_STATUS_READY;		
        }
    }

	if(flag == 0) //但事件时间都超时
	{
		
		menu_sleep_event_flag = SLEEP_STATUS_OK;
	}
    
    return;
}

/*
*函数说明:  用于查询对应事件的状态
*函数名称:  unsigned int menu_sleep_event_timeout_cnt_get(unsigned char eventBit)
*形参:      对应的事件位
*返回码:    0：表示该事件未发生
*           1：表示该事件超时时间已经到了
*           大于1：表示该事件超时时间未到
*/
unsigned int menu_sleep_event_timeout_cnt_get(unsigned char eventBit)
{
    return menu_sleep_event_timeout[eventBit];
}

/*
*函数说明:  用于查询所有事件的休眠状态
*函数名称:  unsigned int menu_sleep_event_state_get(void)
*形参:      无
*返回码:    0：所有事件都允许进入了休眠状态
            非0：存在个别事件不允许进入休眠状态，可通过位操作查阅哪一个事件
*/
unsigned int menu_sleep_event_state_get(void)
{
    return menu_sleep_event_flag;
}

void menu_sleep_adv_cnt_set(unsigned int count)
{
    menu_sleep_event_timeout[MENU_SLEEP_EVENT_ADV_BIT] = count;
}

unsigned int menu_sleep_adv_cnt_get(void)
{
    return menu_sleep_event_timeout[MENU_SLEEP_EVENT_ADV_BIT];
}

void menu_sleep_adv_cnt_decrease(void)
{
    if(menu_sleep_event_timeout[MENU_SLEEP_EVENT_ADV_BIT] > 1)
    {
        menu_sleep_event_timeout[MENU_SLEEP_EVENT_ADV_BIT]--;
    }
}


