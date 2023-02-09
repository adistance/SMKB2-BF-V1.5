#include "menu_manage.h"
#include <os_sync.h>
#include <trace.h>
/*============================================================================*
 *                            peripheral
 *============================================================================*/
#include "driver_led.h"
#include "driver_uart.h"
#include "module.h"
#include "bluetooth_menu.h"
#include "os_msg.h"
#include "driver_motor.h"
#include "rtl876x_tim.h"
#include "os_sched.h"
#include "mlapi.h"
#include "filesys.h"
#include "password.h"
#include "gap_conn_le.h"
#include "peripheral_app.h"
#include "driver_sensor.h"
#include "gap_adv.h"
#include "driver_button.h"
#include "system_setting.h"
#include "fpc_sensor_spi.h"
#include "driver_rtc.h"
#include "rtl876x_adc.h"
#include "fingerprint.h"
#include <os_msg.h>
#include <os_task.h>
#include <os_timer.h>
#include "app_task.h"
#include "test_mode.h"
#include "driver_usbkey.h"
#include "tuya_ble_app_demo.h"
#include "tuya_ble_main.h"
#include "rtl876x_gpio.h"
#include "driver_hal.h"

#if 1
#define MENU_MANAGE_PRINT_INFO0   APP_PRINT_TRACE0
#define MENU_MANAGE_PRINT_INFO1   APP_PRINT_TRACE1
#define MENU_MANAGE_PRINT_INFO2   APP_PRINT_TRACE2
#define MENU_MANAGE_PRINT_INFO3   APP_PRINT_TRACE3
#define MENU_MANAGE_PRINT_INFO4   APP_PRINT_TRACE4
#else
#define MENU_MANAGE_PRINT_INFO0(...)
#define MENU_MANAGE_PRINT_INFO1(...)
#define MENU_MANAGE_PRINT_INFO2(...)
#define MENU_MANAGE_PRINT_INFO3(...)
#define MENU_MANAGE_PRINT_INFO4(...)
#endif

#define MENU_TIMER_FINGER_VALUE         (10)              //10msΪ��λ
#define MENU_TIMER_NFC_VALUE            (10)
#define MENU_TIMER_TOUCH_VALUE          (5)     
#define MENU_TIMER_BUTTON_VALUE         (5)
#define MENU_TIMER_VOICE_VALUE          (1)
#define MENU_TIMER_UART_VALUE           (10)

/*============================================================================*
 *                              task
 *============================================================================*/
 
#define MENU_TASK_PRIORITY                  3           //���˵���������ȼ��������ʵ���������
#define MENU_TASK_STACK_SIZE                256 * 8      //���˵�����Ķ�ջ��С�������ʵ���������
#define MAX_NUMBER_OF_MENU_MESSAGE          0x20        //���˵�������Ϣ���е���Ŀ
void *menu_task_handle;                                 //���˵�������
void *menu_queue_handle;                                //���˵�������Ϣ���о��
static void menu_task(void *p_param);                   //���˵�������

#define BACKGROUND_TASK_PRIORITY            4           //��̨�������ȼ�
#define BACKGROUND_TASK_STACK_SIZE          256 * 8      //��̨�����ջ��С
#define MAX_NUMBER_OF_BACKGROUND_MESSAGE    0x20        //��̨�������Ϣ�ش�С
void *background_task_handle;                           //��̨������
void *background_queue_handle;                          //��̨������Ϣ���о��

#define LOOP_TASK_PRIORITY            2           //��ѯ�������ȼ�
void *loop_task_handle;                           //��ѯ������

static bool s_pressFlag = false;              //ָ�ư�ѹ��

#define BT_TIMEOUT				1800  //180s
static uint16_t s_u16BtTimeout = BT_TIMEOUT;

static void background_task(void *p_param);             //��̨������

/*============================================================================*
 *                              TIMER
 *============================================================================*/

extern uint32_t background_msg_motor_handle(T_MENU_MSG *p_msg);
extern uint32_t background_msg_beep_handle(T_MENU_MSG *p_msg);
extern uint32_t fingerprint_handle_msg(T_MENU_MSG *fp_msg);
extern uint32_t button_handle_msg(T_MENU_MSG *bt_msg);
extern void app_le_set_adv_int(uint16_t interval);
/*============================================================================*
 *                              SEM
 *============================================================================*/

/*============================================================================*
 *                              Functions
 *============================================================================*/
static void menu_task_msg_handle(T_MENU_MSG *menu_msg);
static void background_task_msg_handle(T_MENU_MSG *menu_msg);

extern bool bAdminFpFlag;

//������ѵ����
void resume_task(bool bStartBLE)
{
	os_task_resume(loop_task_handle);	
	if(bStartBLE)
	{
		os_delay(50);
		//le_adv_start();
	}	
}


//������������
void suspend_task(void)
{
	os_task_suspend(loop_task_handle);
}

//���ð�ѹ��־λ
void menu_set_press_status(bool data)
{
	s_pressFlag = data;
}

void menu_reset_bt_timeout(void)
{
	s_u16BtTimeout = BT_TIMEOUT;
}

static void loop_task(void *p_param)
{
	uint8_t u8Cnt = 0;
	uint16_t u16PressCnt = 0;		//���㰴ѹʱ��
	E_DOOR_STATUS eDoorStatus;
	eAPP_for_open e_if_app_open = no_open_from_tuya;
	uint8_t u8CheckCnt = 0;
	uint8_t u8HalCnt = 0;
	uint8_t u8pressFlagCnt = 0;
	bool bVolShow = false;
	bool bDoorStatus = false;
	
    DBG_DIRECT("[DIR] [TASK] loop_task start");		
    
    while(true)
    {  	
    	app_wdg_reset();
		eDoorStatus = door_open_status();
		
#if DLPS_FLAG
        menu_sleep_event_timeout_cnt_decrease();
#endif   	
    	if(SLEEP_STATUS_READY == menu_sleep_event_state_get())                           //ȫ���¼���������
	    {
	    	//APP_PRINT_INFO0("sleep mode");
	    	s_pressFlag = false;
	
	    	//le_adv_stop();	
					
	        //׼����������
			while(Sensor_Sleep()){};
						
			menu_sleep_event_timeout_cnt_decrease();
			
			suspend_task();						
	    }	
    	else if((!s_pressFlag && ((eDoorStatus == E_OPEN_NONE) || (eDoorStatus == E_OPEN_SUC))) && MLAPI_QueryFingerPresent())
    	{  		
    		Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);

			u8Cnt = 0;
			u16PressCnt = 0;
			u8HalCnt = 0;
			if(eDoorStatus == E_OPEN_SUC)
			{
				bVolShow = false;
			}
			else 
			{
				bVolShow = true;
			}
    		s_pressFlag = true;
    		//APP_PRINT_INFO0("press");
			background_msg_set_fp(FP_MSG_SUBTYPE_MATH);
			while(MLAPI_QueryFingerPresent());
			menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
    	}
		else if(eDoorStatus == E_OPEN_SUC) //���Ź���0.5��ſ����ٴδ���
		{
			u8Cnt++;
//			bShort_Sleep_flag = 0;
			e_if_app_open = Get_App_for_open_flag();
			if(e_if_app_open == open_from_tuya)					//����tuya�Ŀ���ָ��
			{
				Set_App_for_open_flag(no_open_from_tuya);
				//bVolShow = true;
				u8HalCnt = 0;
			}
			if(u8Cnt == 5)
			{
				driver_adc_start();
			}
			else if(u8Cnt >= 15)
			{	
				u8Cnt = 0;				
				//door_open_status_reset();
				if(s_VolAvg < LOW_POWER_DATA && bVolShow)
				{
					background_msg_set_beep(50, 5);
					background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_POWER_LOW);
				}
				bVolShow = false;
				//background_msg_set_motor(BACKGROUND_MSG_MOTOR_SUBTYPE_RIGHT);
			}
			
	      //�ȴ���ָ�뿪
	      if(MLAPI_QueryFingerPresent())
	      {
	        u16PressCnt++;
	        if(u16PressCnt == 30 && bAdminFpFlag)
	        {
//	          background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_ENROLL_TIPS);
//			  background_msg_set_beep(150, 1);
	        }
	        else if(u16PressCnt == 60 && bAdminFpFlag)
	        {
	          background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_INIT_TIPS);
			  background_msg_set_beep(150, 1);
	        }
			u8HalCnt = 0;
	        menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
	      }
	      else
	      {
	    	//DBG_DIRECT("close suc");				
	        //��ѧϰ
	        MLAPI_UpdateFTR();
			
	        if(u16PressCnt >= 30 && u16PressCnt < 60 && bAdminFpFlag) //���ָ��
	        {
//	        	if(fp_admin_match(1))
//					u8HalCnt = 0;
//	         	background_msg_set_fp(FP_MSG_SUBTYPE_ENROLL);
	        }
	        else if(u16PressCnt >= 60 && bAdminFpFlag) //�ָ���������
	        {
				if(fp_admin_match(2))
	        	{	
	        		u8HalCnt = 0;
					background_msg_set_beep(100, 2);
					background_msg_set_button(MENU_MSG_BUTTON_SUBTYPE_RESET);
				}	
	        }
			
			u8pressFlagCnt ++;
			if(u8pressFlagCnt == 5)
			{
				u8pressFlagCnt = 0;
				s_pressFlag = false;	
			}
	        u16PressCnt = 0;      
	      }	
		  
		}
		
		else if(eDoorStatus == E_CLOSE_SUC)
		{
			door_open_status_reset();
//			bShort_Sleep_flag = 1;
			menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
		}
	bDoorStatus = hal_get_door_status();
	if(GPIO_ReadInputDataBit(GPIO_GetPin(PAIR_HAL2)) == 0 && (eDoorStatus == E_OPEN_SUC) && bDoorStatus)
	{
		u8HalCnt++;
		if(u8HalCnt % 10 == 0)
			menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
		
		if(u8HalCnt >= 50)
		{
			u8HalCnt = 0;
 			set_b_HAL1_work_status(false);
			background_msg_set_motor(BACKGROUND_MSG_MOTOR_SUBTYPE_RIGHT);
		}
	}
	else if(GPIO_ReadInputDataBit(GPIO_GetPin(PAIR_HAL2)))
	{
		u8HalCnt = 0;
		//set_b_HAL1_work_status(true);
		Set_hal_door_status(false);				//��ʾ������ڱ���״̬��
	}
		if(app_get_bt_real_state() == 2) //����������
		{
			menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
			s_u16BtTimeout--;
			if(s_u16BtTimeout == 0)
			{
				s_u16BtTimeout = BT_TIMEOUT;
				//APP_PRINT_INFO0("connect timeout, disconnect now!!!!");
				app_ble_disconnect();
			}

			u8CheckCnt++;
			if(u8CheckCnt == 25)
			{
				if(eDoorStatus == E_OPEN_NONE)
				{
					driver_adc_start();
				}
			}
			else if(u8CheckCnt == 33)
			{
				u8CheckCnt = 0;
				tuya_ble_get_battery();
			}
		}
		else
			s_u16BtTimeout = BT_TIMEOUT;
		
				
		os_delay(100);
    }

}




void menu_task_init(void)
{
    if(!os_msg_queue_create(&menu_queue_handle, MAX_NUMBER_OF_MENU_MESSAGE, sizeof(T_MENU_MSG)))
    {
        DBG_DIRECT("[ERROR] [MENU_MANAGE]:menu queue create fail");
    }
    
    if(!os_task_create(&menu_task_handle, "menu", menu_task, 0, MENU_TASK_STACK_SIZE, MENU_TASK_PRIORITY))
    {
        DBG_DIRECT("[ERROR] [MENU_MANAGE]:menu task create fail");
    }
    
    if(!os_msg_queue_create(&background_queue_handle, MAX_NUMBER_OF_BACKGROUND_MESSAGE, sizeof(T_MENU_MSG)))
    {
        DBG_DIRECT("[ERROR] [MENU_MANAGE]:background queue create fail");
    }
    
    if(!os_task_create(&background_task_handle, "background", background_task, 0, BACKGROUND_TASK_STACK_SIZE, BACKGROUND_TASK_PRIORITY))
    {
        DBG_DIRECT("[ERROR] [MENU_MANAGE]:background task create fail");
    }

	if(!os_task_create(&loop_task_handle, "loop", loop_task, 0, BACKGROUND_TASK_STACK_SIZE, LOOP_TASK_PRIORITY))
    {
        DBG_DIRECT("[ERROR] [MENU_MANAGE]:loop_task create fail");
    }
	os_task_suspend(loop_task_handle);
}


static void menu_task(void *p_param)
{
    T_MENU_MSG menu_msg;
        
    DBG_DIRECT("[DIR] [TASK] menu task start");
    
    while(true)
    {
    	if (os_msg_recv(menu_queue_handle, &menu_msg, 0xFFFFFFFF) == true)
        {
            menu_task_msg_handle(&menu_msg);
        }
		
    }
}

static void background_task(void *p_param)
{
    T_MENU_MSG background_msg;

    DBG_DIRECT("[DIR] [TASK] background task start");
    
    while(true)
    {
        if (os_msg_recv(background_queue_handle, &background_msg, 0xFFFFFFFF) == true)
        {
            background_task_msg_handle(&background_msg);
        }
    }

}

static void menu_task_msg_handle(T_MENU_MSG *p_menu_msg)
{
	menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
	
    switch(p_menu_msg->type)
    {
        case MENU_MSG_TYPE_MAIN:                /*���˵�����*/
            break;
        
        case MENU_MSG_TYPE_FINGERPRINT:         /*ָ������*/
			fingerprint_handle_msg(p_menu_msg);
            break;
            
        case MENU_MSG_TYPE_NFC:                 /*NFC����*/
            break;
        
        case MENU_MSG_TYPE_TOUCH:               /*��������*/
            break;
        
        case MENU_MSG_TYPE_BUTTON:              /*����ť����*/
			button_handle_msg(p_menu_msg);
            break;
        			
        case MENU_MSG_TYPE_TASK:
            io_handle_uart_msg(p_menu_msg);
            break;
        
        default:
            MENU_MANAGE_PRINT_INFO1("[ERROR] menu_msg->type:%d is default", p_menu_msg->type);
            break;       
    }

}

/*********************************************
**fingerprint->fingerprint_finger_menu_msg_send                 //ָ�Ʋ���������Ϣ�����
**bluetooth_menu->bluebooth_menu_send_msg_to_menu_task          //��������������Ϣ�����
**********************************************/
bool menu_task_msg_send(T_MENU_MSG *p_menu_msg)                 
{
//    MENU_MANAGE_PRINT_INFO2("[MENU] menu message send type:0x%02x, subtype:0x%02x", p_menu_msg->type, p_menu_msg->subtype);
    
    if (os_msg_send(menu_queue_handle, p_menu_msg, 0) == false)
    {
        DBG_DIRECT("[MENU] menu_task_msg_send fail!!!");
        return false;
    }
    
    return true;
}

bool menu_task_msg_send_front(T_MENU_MSG *p_menu_msg)                 
{
//    MENU_MANAGE_PRINT_INFO2("[MENU] menu message send type:0x%02x, subtype:0x%02x", p_menu_msg->type, p_menu_msg->subtype);
    
    if (os_msg_send_to_front(menu_queue_handle, p_menu_msg, 0) == false)
    {
        DBG_DIRECT("[MENU] menu_task_msg_send fail!!!");
        return false;
    }
    
    return true;
}


static void background_task_msg_handle(T_MENU_MSG *p_background_msg)
{
	menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
	
    switch(p_background_msg->type)
    {
        case BACKGROUND_MSG_TYPE_VOICE:                 /*��������*/
            
            break;
        
        case BACKGROUND_MSG_TYPE_LED:                   /*�ƹ�����*/
            background_msg_led_handle(p_background_msg);
            break;

        case BACKGROUND_MSG_TYPE_MOTOR:                 /*�������*/			
            background_msg_motor_handle(p_background_msg);
            break;
        
        case BACKGROUND_MSG_TYPE_UART:   				/*�������ݴ���*/	
            io_uart_background_task_deal(p_background_msg);
            break;

		case BACKGROUND_MSG_TYPE_BEEP:				   /*������*/	
			background_msg_beep_handle(p_background_msg);
			break;

		case BACKGROUND_MSG_TYPE_BT:    //����С����
			//bluetooth_handle_msg(p_background_msg);
			break;

		//case BACKGROUND_MSG_TYPE_HAL:				   /*��������*/	
		//	background_msg_motor_handle(p_background_msg);
		//	break;

		case BACKGROUND_WAKEUP:		//����
			resume_task(true);
			break;

		case BACKGROUND_SUSPEND:	//��������
			suspend_task();
			break;
			
		case BACKGROUND_TEST:
			break;

        default:
            MENU_MANAGE_PRINT_INFO1("[ERROR] background_msg->type:%d is default", p_background_msg->type);
            break;
    }
	
}

//���͵����е�ͷ��
bool background_task_msg_send_front(T_MENU_MSG *p_menu_msg)                 
{
//    MENU_MANAGE_PRINT_INFO2("[MENU] menu message send type:0x%02x, subtype:0x%02x", p_menu_msg->type, p_menu_msg->subtype);
    
    if (os_msg_send_to_front(background_queue_handle, p_menu_msg, 0) == false)
    {
        DBG_DIRECT("[MENU] background_task_msg_send_front fail!!!");
        return false;
    }
    
    return true;
}

//���͵����е�β��
bool background_task_msg_send(T_MENU_MSG *p_background_msg)
{
//    MENU_MANAGE_PRINT_INFO2("[MENU] background message send type:0x%02x, subtype:0x%02x", p_background_msg->type, p_background_msg->subtype);
    
    if (os_msg_send(background_queue_handle, p_background_msg, 0) == false)
    {
        DBG_DIRECT("[MENU] background_task_msg_send fail!!!");
        return false;
    }
    
    return true;
}

