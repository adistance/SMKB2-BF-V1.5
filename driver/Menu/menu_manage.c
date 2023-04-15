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

#define MENU_TIMER_FINGER_VALUE         (10)              //10ms为单位
#define MENU_TIMER_NFC_VALUE            (10)
#define MENU_TIMER_TOUCH_VALUE          (5)     
#define MENU_TIMER_BUTTON_VALUE         (5)
#define MENU_TIMER_VOICE_VALUE          (1)
#define MENU_TIMER_UART_VALUE           (10)

/*============================================================================*
 *                              task
 *============================================================================*/
 
#define MENU_TASK_PRIORITY                  3           //主菜单任务的优先级，需根据实际情况配置
#define MENU_TASK_STACK_SIZE                256 * 8      //主菜单任务的堆栈大小，需根据实际情况配置
#define MAX_NUMBER_OF_MENU_MESSAGE          0x20        //主菜单任务消息队列的数目
void *menu_task_handle;                                 //主菜单任务句柄
void *menu_queue_handle;                                //主菜单任务消息队列句柄
static void menu_task(void *p_param);                   //主菜单任务函数

#define BACKGROUND_TASK_PRIORITY            4           //后台任务优先级
#define BACKGROUND_TASK_STACK_SIZE          256 * 8      //后台任务堆栈大小
#define MAX_NUMBER_OF_BACKGROUND_MESSAGE    0x20        //后台任务的消息池大小
void *background_task_handle;                           //后台任务句柄
void *background_queue_handle;                          //后台任务消息队列句柄

#define LOOP_TASK_PRIORITY            2           //轮询任务优先级
void *loop_task_handle;                           //轮询任务句柄

static bool s_pressFlag = false;              //指纹按压下

#define BT_TIMEOUT				1800  //180s
static uint16_t s_u16BtTimeout = BT_TIMEOUT;
unsigned int u32Voltage = 5200; 					 //设置电池电压初始值


static void background_task(void *p_param);             //后台任务函数

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

//开启轮训任务
void resume_task(bool bStartBLE)
{
	os_task_resume(loop_task_handle);	
	if(bStartBLE)
	{
		os_delay(50);
		Sensor_Init();
		//le_adv_start();
	}	
}


//挂起轮序任务
void suspend_task(void)
{
	os_task_suspend(loop_task_handle);
}

//设置按压标志位
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
	uint16_t u16PressCnt = 0;		//计算按压时间
	E_DOOR_STATUS eDoorStatus;
	eAPP_for_open e_if_app_open = no_open_from_tuya;
	uint8_t u8CheckCnt = 0;
	uint8_t u8HalCnt = 0;
	uint8_t u8pressFlagCnt = 0;
	bool bVolShow = true;
	bool bDoorStatus = false;
	unsigned char Last_Motor_Status[1];
    DBG_DIRECT("[DIR] [TASK] loop_task start");		

	
	if(GPIO_ReadInputDataBit(GPIO_GetPin(PAIR_HAL1)) == 1)			//上电时候读一次解锁霍尔的电平，判断是否需要让电机旋转两圈
	{
		set_b_HAL1_work_status(false);
	}
	else
	{
		set_b_HAL1_work_status(true);
	}
		
    while(1)
    {  	
    	app_wdg_reset();
		eDoorStatus = door_open_status();
		
#if DLPS_FLAG
        menu_sleep_event_timeout_cnt_decrease();
#endif   	
		if(get_bFirst_WakeUp_status())											//唤醒时读一次电池电压
		{
			set_bFirst_WakeUp_status(false);
			driver_adc_start();
			os_delay(20);
			u32Voltage = s_VolAvg;									//保存唤醒时读取的电池电压,其余所有与电池电压有关的操作都用这个数据
//			APP_PRINT_INFO1("[io_adc]  bFirst_WakeUp  s_VolAvg = %dmV",(uint32_t)s_VolAvg);
			flashReadBuffer(Last_Motor_Status, 0x86D000, 4, 0);		//唤醒时读取之前存储的电机位置，用于判断电机是否需要转两圈
			if(Last_Motor_Status[0] == false)
			{
				set_b_HAL1_work_status(false);
			}
		}


    	if(SLEEP_STATUS_READY == menu_sleep_event_state_get())                           //全部事件都处理完
	    {
	    	Last_Motor_Status[0] = get_b_HAL1_work_status();	//进入低功耗前将电机位置存储在flash，防止低功耗时间过长，系统丢失数据
	    	flashWriteBuffer(Last_Motor_Status, 0x86D000,4, 0);
	    	//APP_PRINT_INFO0("sleep mode");
	    	s_pressFlag = false;
			
	    	//le_adv_stop();	
	    	
			Sensor_Init();
			
	        //准备进入休眠
			while(Sensor_Sleep()){};
			
			check_sensor_int();	
			
			menu_sleep_event_timeout_cnt_decrease();
			
			suspend_task();						
	    }	
    	else if(((!s_pressFlag) && ((eDoorStatus == E_OPEN_NONE) || (eDoorStatus == E_OPEN_SUC))) && MLAPI_QueryFingerPresent())
    	{  		
//    		driver_hal_init();

    		s_pressFlag = true;
//			Motor_Rst_Flag = true;
//			Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);//解锁霍尔只在有需要时打开
//			Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
			u8Cnt = 0;
			u16PressCnt = 0;
			u8HalCnt = 0;
			u8pressFlagCnt = 0;
		
    		
    		//APP_PRINT_INFO0("press");
			background_msg_set_fp(FP_MSG_SUBTYPE_MATH);
			while(MLAPI_QueryFingerPresent());
			menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
    	}
		else if(eDoorStatus == E_OPEN_SUC) //开门过了0.5后才可以再次触摸
		{
			u8Cnt++;
//			bShort_Sleep_flag = 0;
			e_if_app_open = Get_App_for_open_flag();

			if(Motor_Rst_Flag)
			{
				Hal_Set_IntConfig_Off();
				Motor_Rst_Flag = false;
			}	
			
			if(e_if_app_open == open_from_tuya)					//来自tuya的开锁指令
			{
				Set_App_for_open_flag(no_open_from_tuya);
				u8HalCnt = 0;
				u8pressFlagCnt = 0;
			}
			
			if(u8Cnt == 15)
			{	
				u8Cnt = 0;				
				
//				APP_PRINT_INFO1("[io_adc]  u8Cnt == 15   u32Voltage = %dmV",u32Voltage);
//				APP_PRINT_INFO1("[io_adc]  u8Cnt == 15   s_VolAvg = %dmV",(uint32_t)s_VolAvg);

				if(u32Voltage < LOW_POWER_DATA && bVolShow)
				{
					
//					APP_PRINT_INFO1("[io_adc]  voltage < LOW_POWER_DATA	 u32Voltage = %dmV",u32Voltage);
//					APP_PRINT_INFO1("[io_adc]  voltage < LOW_POWER_DATA   s_VolAvg = %dmV",(uint32_t)s_VolAvg);
					background_msg_set_beep(50, 5);
					background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_POWER_LOW);
				}
				bVolShow = false;
			}
			
	      //等待手指离开
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
	        //自学习
	        MLAPI_UpdateFTR();
			
	        if(u16PressCnt >= 30 && u16PressCnt < 60 && bAdminFpFlag) //添加指纹
	        {
//	        	if(fp_admin_match(1))
//					u8HalCnt = 0;
//	         	background_msg_set_fp(FP_MSG_SUBTYPE_ENROLL);
	        }
	        else if(u16PressCnt >= 60 && bAdminFpFlag) //恢复出厂设置
	        {
				if(fp_admin_match(2))
	        	{	
	        		u8HalCnt = 0;
					background_msg_set_beep(100, 2);
					background_msg_set_button(MENU_MSG_BUTTON_SUBTYPE_RESET);
				}	
	        }
			
			
			
			u8pressFlagCnt ++;
//			if(u8pressFlagCnt == 5)
//			{
//							
//			}
			
			if(u8pressFlagCnt == 8)
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
			set_b_HAL1_work_status(false);
			
			menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
		}
	bDoorStatus = hal_get_door_status();
	if(GPIO_ReadInputDataBit(GPIO_GetPin(PAIR_HAL2)) == 0 && (eDoorStatus == E_OPEN_SUC) && bDoorStatus)
	{
		u8HalCnt++;
		if(u8HalCnt % 10 == 0)
			menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);		
		if(u8HalCnt == 50)
		{
			bVolShow = true;
			u8HalCnt = 0;			
			background_msg_set_motor(BACKGROUND_MSG_MOTOR_SUBTYPE_RIGHT);
		}
	}
	else if(GPIO_ReadInputDataBit(GPIO_GetPin(PAIR_HAL2)))
	{
		u8HalCnt = 0;
		Set_hal_door_status(false);				//表示电机不在闭锁状态了
	}
	
	if(app_get_bt_real_state() == 2) //蓝牙连接着
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
		
		if(u8CheckCnt == 33)
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
        case MENU_MSG_TYPE_MAIN:                /*主菜单类型*/
            break;
        
        case MENU_MSG_TYPE_FINGERPRINT:         /*指纹类型*/
			fingerprint_handle_msg(p_menu_msg);
            break;
            
        case MENU_MSG_TYPE_NFC:                 /*NFC类型*/
            break;
        
        case MENU_MSG_TYPE_TOUCH:               /*触控类型*/
            break;
        
        case MENU_MSG_TYPE_BUTTON:              /*物理按钮类型*/
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
**fingerprint->fingerprint_finger_menu_msg_send                 //指纹操作发送信息的入口
**bluetooth_menu->bluebooth_menu_send_msg_to_menu_task          //蓝牙操作发送信息的入口
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
        case BACKGROUND_MSG_TYPE_VOICE:                 /*语音类型*/
            
            break;
        
        case BACKGROUND_MSG_TYPE_LED:                   /*灯光类型*/
            background_msg_led_handle(p_background_msg);
            break;

        case BACKGROUND_MSG_TYPE_MOTOR:                 /*电机类型*/			
            background_msg_motor_handle(p_background_msg);
            break;
        
        case BACKGROUND_MSG_TYPE_UART:   				/*串口数据处理*/	
            io_uart_background_task_deal(p_background_msg);
            break;

		case BACKGROUND_MSG_TYPE_BEEP:				   /*蜂鸣器*/	
			background_msg_beep_handle(p_background_msg);
			break;

		case BACKGROUND_MSG_TYPE_BT:    //蓝牙小程序
			//bluetooth_handle_msg(p_background_msg);
			break;

		//case BACKGROUND_MSG_TYPE_HAL:				   /*霍尔开关*/	
		//	background_msg_motor_handle(p_background_msg);
		//	break;

		case BACKGROUND_WAKEUP:		//唤醒
			resume_task(true);
			break;

		case BACKGROUND_SUSPEND:	//挂起任务
			suspend_task();
			break;
			
		case BACKGROUND_TEST:
			break;

        default:
            MENU_MANAGE_PRINT_INFO1("[ERROR] background_msg->type:%d is default", p_background_msg->type);
            break;
    }
	
}

//发送到队列的头部
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

//发送到队列的尾部
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

