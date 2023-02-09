/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      app_task.c
   * @brief     Routines to create App task and handle events & messages
   * @author    jane
   * @date      2017-06-02
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <os_msg.h>
#include <os_task.h>
#include <gap.h>
#include <gap_le.h>
#include <app_task.h>
#include <app_msg.h>
#include <app_task.h>
#include <peripheral_app.h>
#include <trace.h>
#include <os_timer.h>
#include "rtl876x_tim.h"

#include "board.h"
#include "otp_config.h"
#include "driver_rtc.h"


#include "rtl876x_aon_wdg.h"
#if (AON_WDG_ENABLE == 1)

#endif

#include "os_mem.h"
#include <os_sync.h>

#include "menu_manage.h"
#include "module.h"
#include "driver_uart.h"
#include "driver_hal.h"

#include "gap_adv.h"
#include "bluetooth_menu.h"
#include "driver_led.h"
#include "filesys.h"
#include "test_mode.h"
#include "system_setting.h"
#include "driver_motor.h"
#include "driver_wdg.h"
#include "driver_button.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_app_demo.h"
#include "tuya_ble_secure.h"
#include "record.h"
#include "otp.h"

/** @defgroup  PERIPH_APP_TASK Peripheral App Task
    * @brief This file handles the implementation of application task related functions.
    *
    * Create App task and handle events & messages
    * @{
    */
/*============================================================================*
 *                              Macros
 *============================================================================*/
#define APP_TASK_PRIORITY             1         //!< Task priorities
#define APP_TASK_STACK_SIZE           1024*2    //256 * 4   //!<  Task stack size
#define MAX_NUMBER_OF_GAP_MESSAGE     0x20      //!<  GAP message queue size
#define MAX_NUMBER_OF_IO_MESSAGE      0x20      //!<  IO message queue size
#define MAX_NUMBER_OF_EVENT_MESSAGE   (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE)    //!< Event message queue size

/*============================================================================*
 *                              Variables
 *============================================================================*/
void *app_task_handle = NULL;   //!< APP Task handle
void *evt_queue_handle = NULL;  //!< Event queue handle
void *io_queue_handle = NULL;   //!< IO queue handle
void *spi_sem_handle = NULL;
void *uart_sem_handle = NULL;
/*============================================================================*
 *                              SEM
 *============================================================================*/
void *command_req_sem_handle = NULL;
void *command_resp_sem_handle = NULL;
extern bool s_u8DlpsFlag;
void *tuya_queue_handle;

/*============================================================================*
 *                              Functions
 *============================================================================*/
void app_main_task(void *p_param);
extern void driver_init(void);
extern void app_ble_init(void);

bool bWdgFlag = true;   //看门狗喂狗，true就喂狗

void app_ram_check(void)
{
    size_t dataRamFreeSize = 0, bufferRamFreeSize = 0, numRamFreeSize = 0;
    dataRamFreeSize = os_mem_peek(RAM_TYPE_DATA_ON);
    bufferRamFreeSize =  os_mem_peek(RAM_TYPE_BUFFER_ON);
    numRamFreeSize = os_mem_peek(RAM_TYPE_NUM);
    
    APP_PRINT_INFO3("dataRamFreeSize:%d, bufferRamFreeSize:%d, numRamFreeSize:%d\r\n", dataRamFreeSize, bufferRamFreeSize, numRamFreeSize);
}

/**
 * @brief  Initialize App task
 * @return void
 */
void app_task_init()
{
//    app_ram_check();
    
    os_msg_queue_create(&io_queue_handle, MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
    os_msg_queue_create(&evt_queue_handle, MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint8_t));
	os_msg_queue_create(&tuya_queue_handle, MAX_NUMBER_OF_TUYA_MESSAGE, sizeof(tuya_ble_evt_param_t));
    if(os_sem_create(&command_req_sem_handle, 0, 1) != true)
    {
         APP_PRINT_INFO0("command req sem create fail");
    }
    
    if(os_sem_create(&command_resp_sem_handle, 1, 1) != true)
    {
         APP_PRINT_INFO0("command resp sem create fail");
    }

	if(os_sem_create(&uart_sem_handle, 1, 1) != true)
    {
        APP_PRINT_INFO0("os_sem_create fail");
    }
    
    if(os_sem_create(&spi_sem_handle, 1, 1) != true)
    {
        APP_PRINT_INFO0("os spi sem create fail");
    }
	
    if(os_task_create(&app_task_handle, "app", app_main_task, 0, APP_TASK_STACK_SIZE, APP_TASK_PRIORITY) != true)
    {
        APP_PRINT_INFO0("app_task_handle create fail");
    }       
    
    app_ram_check();
}

#if (AON_WDG_ENABLE == 1)
void *xTimerPeriodWakeupDlps;
#define TIMER_WAKEUP_DLPS_PERIOD     ((AON_WDG_TIME_OUT_PERIOD_SECOND - 1) * 1000)

void vTimerPeriodWakeupDlpsCallback(void *pxTimer)
{
	if(bWdgFlag)
	{
		if(!uart_tx_flag) //串口模式下一直喂狗
			bWdgFlag = false;
		AON_WDG_Restart();
		wdg_feed();
	}   
	if(s_u8DlpsFlag)
		rtc_update_in_dlps();
    //APP_PRINT_INFO0("TimerPeriodWakeupDlps timeout!");
}

#endif

void app_wdg_reset(void)
{
	bWdgFlag = true;
}


//看门狗初始化
void app_wdg_init(void)
{
#if (ROM_WATCH_DOG_ENABLE == 1)
	driver_wdg_init();
#endif

#if (AON_WDG_ENABLE == 1)
	bool retval;

	aon_wdg_init(1, AON_WDG_TIME_OUT_PERIOD_SECOND);
	retval = os_timer_create(&xTimerPeriodWakeupDlps, "xTimerPeriodWakeupDlps",  AON_WDG_OS_TIMER_ID, \
							 TIMER_WAKEUP_DLPS_PERIOD, true, vTimerPeriodWakeupDlpsCallback);
	if (!retval)
	{
		APP_PRINT_INFO1(" xTimerPeriodWakeupDlps retval is %d", retval);
	}
	else
	{
		os_timer_start(&xTimerPeriodWakeupDlps);
		APP_PRINT_INFO0("Start xTimerPeriodWakeupDlps!");
	}

	aon_wdg_enable();
#endif

}

void app_change_log_pin(uint8_t u8PinNum)
{
 	OTP->logPin = u8PinNum;  //eg:P3_0
    extern void LOGUARTDriverInit(void);
    LOGUARTDriverInit();
}


/**
 * @brief        App task to handle events & messages
 * @param[in]    p_param    Parameters sending to the task
 * @return       void
 */

void app_main_task(void *p_param)
{
    uint8_t event;

	//app_change_log_pin(P3_0);
	
	app_wdg_init();

	driver_init(); //外设初始化由外部实现

	driver_motor_init();

	driver_hal_init();

	MLAPI_SystemInit();

	app_ble_init();

    //启动蓝牙协议层和GAP初始化流程
    gap_start_bt_stack(evt_queue_handle, io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE); 
	driver_rtc_init();	   //打开rtc

	tuya_ble_app_init();
	
    menu_sleep_event_init();

	
#if !DEBUG_FLAG
    menu_module_init();                         //指纹模组的初始化
#endif
    
	driver_uart_init();
	os_delay(500);
	resume_task(false);
    app_ram_check();
	
    while (true)
    {
		if (os_msg_recv(evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
        	
            if (event == EVENT_IO_TO_APP)
            {
                T_IO_MSG io_msg;
                if (os_msg_recv(io_queue_handle, &io_msg, 0) == true)
                {
                    app_handle_io_msg(io_msg);
                }
            }
			else if(event == EVENT_APP_CUSTOM)
			{
				tuya_ble_evt_param_t tuya_evt;
				if (os_msg_recv(tuya_queue_handle, &tuya_evt, 0) == true)
                {
                	menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
                	//APP_PRINT_INFO1("tuya_evt.hdr.event is %d", tuya_evt.hdr.event);
					tuya_ble_event_process(&tuya_evt);
				}
			}
            else
            {
                gap_handle_msg(event);
            }
        }		
		
    }
}

bool app_send_msg_to_apptask(T_IO_MSG *p_msg)
{
    uint8_t event = EVENT_IO_TO_APP;
    
    if (os_msg_send(io_queue_handle, p_msg, 0) == false)
    {
        APP_PRINT_ERROR0("send_io_msg_to_app fail");
        return false;
    }
    if (os_msg_send(evt_queue_handle, &event, 0) == false)
    {
        APP_PRINT_ERROR0("send_evt_msg_to_app fail");
        return false;
    }
    return true;
}

bool tuya_event_queue_send(tuya_ble_evt_param_t *evt, uint32_t wait_ms)
{
    T_EVENT_TYPE app_event = EVENT_APP_CUSTOM;

    if(os_msg_send(tuya_queue_handle, evt, wait_ms))
    {
        if(os_msg_send(evt_queue_handle,&app_event, wait_ms))
        {
            return true;
        }
        else
        {
            return false;
        }

    }
    else
    {
        return false;
    }    
    
}


