#ifndef MENU_SLEEP_H
#define MENU_SLEEP_H

#include "stdio.h"
#include "stdbool.h"

#define MENU_SLEEP_EVENT_WAKEUP_BIT         (0)
#define MENU_SLEEP_EVENT_SETKEY_BIT         (1)
#define MENU_SLEEP_EVENT_CLEANKEY_BIT       (2)
#define MENU_SLEEP_EVENT_TOUCH_BIT          (3)
#define MENU_SLEEP_EVENT_NFC_BIT            (4)
#define MENU_SLEEP_EVENT_FINGER_BIT         (5)
#define MENU_SLEEP_EVENT_VOICE_BIT          (6)
#define MENU_SLEEP_EVENT_UART_BIT           (7)

#define MENU_SLEEP_EVENT_COUNT              (8)
#define MENU_SLEEP_EVENT_ADV_BIT            (MENU_SLEEP_EVENT_COUNT)            //用于处理广播事件

#define MENU_SLEEP_BIT(x)                   (1<<x)

//睡眠的状态
typedef enum
{
	SLEEP_STATUS_INIT = 2,	
	SLEEP_STATUS_READY = 1,	//开始准备进入休眠
	SLEEP_STATUS_OK = 0,	//休眠前的准备都OK了
}eSleepStatus;

//extern bool bShort_Sleep_flag = 0;
void menu_sleep_event_init(void);
void menu_sleep_event_control(unsigned char eventBit, bool isEnable);
void menu_sleep_event_timeout_cnt_decrease(void);
unsigned int menu_sleep_event_timeout_cnt_get(unsigned char eventBit);
unsigned int menu_sleep_event_state_get(void);

void menu_sleep_adv_cnt_set(unsigned int count);
unsigned int menu_sleep_adv_cnt_get(void);
void menu_sleep_adv_cnt_decrease(void);
void menu_sleep_event_reset(void);

#endif
