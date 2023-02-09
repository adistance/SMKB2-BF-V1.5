/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code to realize RTC tick function.
            RTC is a 24-bit counter.
* @details
* @author   yuan
* @date     2018-05-25
* @version  v1.0
*********************************************************************************************************
*/

/* Defines ------------------------------------------------------------------*/
/** Prescaler value.
  * 12 bits prescaler for COUNTER frequency (32768/(PRESCALER+1)).
  * If use internal 32KHz, (32000/(PRESCALER+1)).
  * Must be written when RTC 24-bit counter is stopped.
  */
#define RTC_PRESCALER_VALUE     (3200-1) //f = 10Hz

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "rtl876x_nvic.h"
#include "rtl876x_rcc.h"
#include "rtl876x_rtc.h"
#include "trace.h"
#include "os_sched.h"
#include "driver_rtc.h"
#include "otp_config.h"

static st_times s_times = {0};
const uint8_t Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static volatile uint32_t tmp_time = 0;

/**
  * @brief  Initialize rtc peripheral.
  * @param   No parameter.
  * @return  void
  */
void driver_rtc_init(void)
{
    RTC_DeInit();
    RTC_SetPrescaler(RTC_PRESCALER_VALUE);

    RTC_MaskINTConfig(RTC_INT_TICK, DISABLE);
    RTC_INTConfig(RTC_INT_TICK, ENABLE);

    /* Config RTC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    RTC_NvCmd(ENABLE);

	//RTC_SystemWakeupConfig(ENABLE);
    /* Start RTC */
    RTC_ResetCounter();
    RTC_Cmd(ENABLE);
}

//连接上蓝牙小程序是设置了一下时间戳
void rtc_set_time(unsigned int timestamp)
{
	tmp_time = timestamp;
}

//低功耗更新时间，dlps模式下就不单独唤醒了
void rtc_update_in_dlps(void)
{
	tmp_time += AON_WDG_TIME_OUT_PERIOD_SECOND - 1;
}

uint32_t rtc_get_time()
{
	return tmp_time;
}

void rtc_local_time_updata(unsigned int timestamp)
{   
	uint32_t days;     
    uint16_t leap_num;   
	//APP_PRINT_INFO0("******************************************\n");
	//APP_PRINT_INFO1("rtc_local_time_updata is %d\n", timestamp);
    
    s_times.seconds = timestamp % 60;    
    timestamp /= 60;  //获取分   
    s_times.minutes = timestamp % 60;    
    timestamp += 8 * 60 ;    
    timestamp /= 60; //获取小时    
    s_times.hours = timestamp % 24;  
	
    days = timestamp / 24;    
    leap_num = (days + 365) / 1461;    
    if( ((days + 366) % 1461) == 0)     
    {        
        s_times.year = (days / 366)+ 1970 - 2000;        
        s_times.month = 12;       
        s_times.date = 31;      
    } 
    else 
    {       
        days -= leap_num;      
        s_times.year = (days / 365) + 1970 - 2000;  
        days %= 365;     
        days += 1;    
        if(((s_times.year%4) == 0 ) && (days==60))    
        {           
            s_times.month = 2;            
            s_times.date = 29;        
        }
        else    
        {  
            if(((s_times.year%4) == 0 ) && (days > 60)) 
                --days;            
            
            for(s_times.month = 0;Days[s_times.month] < days; s_times.month++)       
            {               
                days -= Days[s_times.month];   
            }        
            
            ++s_times.month;      
            s_times.date = days;      
        }  
    }  
    //APP_PRINT_INFO6("time: %d-%d-%d  %d.%d.%d\r\n", s_times.year, s_times.month, s_times.date, 
    //	s_times.hours, s_times.minutes, s_times.seconds);

}


void RTC_Handler(void)
{
	static int tick = 0;
    /* RTC tick interrupt handle */
    if (RTC_GetINTStatus(RTC_INT_TICK) == SET)
    {
        /* Notes: DBG_DIRECT function is only used for debugging demonstrations, not for application projects.*/
		tick++;
		if(tick == 10)
		{
			tick = 0;
			tmp_time++;
			//APP_PRINT_INFO1("RTC TICK IS %d", os_sys_tick_get());
			rtc_local_time_updata(tmp_time);
		}
        
        // Add application code here
        RTC_ClearTickINT();
    }
}


/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

