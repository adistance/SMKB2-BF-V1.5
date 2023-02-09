#include "menu_sleep.h"

#include <os_timer.h>
#include "trace.h"

static unsigned int menu_sleep_event_flag = SLEEP_STATUS_INIT;
static unsigned int menu_sleep_event_timeout[MENU_SLEEP_EVENT_COUNT+1] = {0};
//bool bShort_Sleep_flag = 0;

#define SLEEP_TIMES      1000  //12s

//ʱ�䵥λ��10ms
#define TIMEOUT_CNT_MS(x)           (x/10)

/*
*����˵��:  �������¼���״ֵ̬���и�λ
*��������:  void menu_sleep_event_init(void)
*�β�:      ��
*������:    ��
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
	��������mcu˯��ʱ�䣬��mcu���ٽ���͹���
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
*����˵��:  �����¼���״̬��������
*��������:  void menu_sleep_event_control(unsigned char eventBit, bool isEnable)
*�β�:      ��Ӧ���¼�λ
*������:    ��
*/
void menu_sleep_event_control(unsigned char eventBit, bool isEnable)
{
    
    if(isEnable)
    {
    	menu_sleep_event_flag = SLEEP_STATUS_INIT;
        //menu_sleep_event_flag |= MENU_SLEEP_BIT(eventBit);          //�¼���λ
        
        switch(eventBit)                                            //������Ҫ����ʱ�������ʱ��
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
        menu_sleep_event_flag &= ~(MENU_SLEEP_BIT(eventBit));         //��Ӧλ��0��ʱ����0
        menu_sleep_event_timeout[eventBit] = 0;
    }
    
    return;
}

/*
*����˵��:  ���ڶ��¼��ĳ�ʱʱ����еݼ�������ϵͳ����һ����ʱ��ȥ���øú���
*��������:  void menu_sleep_event_timeout_cnt_decrease(void)
*�β�:      ��
*������:    ��
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

	if(flag == 0) //���¼�ʱ�䶼��ʱ
	{
		
		menu_sleep_event_flag = SLEEP_STATUS_OK;
	}
    
    return;
}

/*
*����˵��:  ���ڲ�ѯ��Ӧ�¼���״̬
*��������:  unsigned int menu_sleep_event_timeout_cnt_get(unsigned char eventBit)
*�β�:      ��Ӧ���¼�λ
*������:    0����ʾ���¼�δ����
*           1����ʾ���¼���ʱʱ���Ѿ�����
*           ����1����ʾ���¼���ʱʱ��δ��
*/
unsigned int menu_sleep_event_timeout_cnt_get(unsigned char eventBit)
{
    return menu_sleep_event_timeout[eventBit];
}

/*
*����˵��:  ���ڲ�ѯ�����¼�������״̬
*��������:  unsigned int menu_sleep_event_state_get(void)
*�β�:      ��
*������:    0�������¼����������������״̬
            ��0�����ڸ����¼��������������״̬����ͨ��λ����������һ���¼�
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


