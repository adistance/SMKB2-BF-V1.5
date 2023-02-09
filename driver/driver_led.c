#include "driver_led.h"
#include <trace.h>
#include <board.h>

#include <os_timer.h>

#include <gap_adv.h>

#include <rtl876x_pinmux.h>
#include "rtl876x_gpio.h"
#include "driver_delay.h"

#include "rtl876x_rcc.h"
#include "rtl876x_tim.h"
#include "rtl876x_nvic.h"
#include "system_setting.h"
#include "os_sync.h"
#include "tuya_ble_app_demo.h"

#define BEEP_INTERVAL_MS 		100			//定时器100ms间隔

static volatile ST_LED_CTRL_INFO stLedCtrlInfo;
static void LedTimerStart(void);
static void LedTimerStop(void);
static void SetLedPwmPulse(uint8_t colour, uint32_t Val);
static void LedControl(uint8_t colour,uint32_t Val);

static volatile uint32_t ledFrequencyCount = 0;
static volatile uint8_t ledControlFlag = SET;
static void *led_mutex_handle = NULL;   //互斥锁句柄

static void *pBeepTimer;		//蜂鸣器定时器句柄
static uint8_t s_u8BeepCount = 0;
static uint16_t s_u16BeepTime = 0;
static void *beep_mutex_handle = NULL;   //蜂鸣器互斥锁句柄
static bool s_bBeepWorking = false;   //蜂鸣器是否在工作标志
static bool s_bBeepInit = false;  //蜂鸣器是否初始化
static bool s_bLedInit = false;   //LED是否初始化
static uint16_t s_u16BeepData[2] = {0};	//蜂鸣器数据
static bool s_bLedWorking = false;  //led是否正在工作标志

void ledModeSet(EM_LED_CTRL mode, EM_LED_COLOR colour, uint32_t frequency, uint16_t count);

void ledLock()
{
	os_mutex_take(led_mutex_handle, 0);
}

void ledUnlock()
{
	os_mutex_give(led_mutex_handle);
}

void beepLock()
{
	os_mutex_take(beep_mutex_handle, 0);
}

void beepUnlock()
{
	os_mutex_give(beep_mutex_handle);
}


void driver_pwm_init(void)
{	
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);
    
    TIM_InitStruct.TIM_SOURCE_DIV       = TIM_CLOCK_DIVIDER_2;          //20M
    TIM_InitStruct.TIM_PWM_En           = PWM_ENABLE;
    TIM_InitStruct.TIM_Mode             = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_PWM_High_Count   = PWM_DUTY_COUNT_GET(100);           
    TIM_InitStruct.TIM_PWM_Low_Count    = PWM_DUTY_COUNT_GET(0);            
    TIM_TimeBaseInit(RED_PWM_TIMER_NUM, &TIM_InitStruct);
    
    TIM_InitStruct.TIM_SOURCE_DIV       = TIM_CLOCK_DIVIDER_2;          //20M
    TIM_InitStruct.TIM_PWM_En           = PWM_ENABLE;
    TIM_InitStruct.TIM_Mode             = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_PWM_High_Count   = PWM_DUTY_COUNT_GET(100);           
    TIM_InitStruct.TIM_PWM_Low_Count    = PWM_DUTY_COUNT_GET(0);            
    TIM_TimeBaseInit(BLUE_PWM_TIMER_NUM, &TIM_InitStruct);
		
    TIM_InitStruct.TIM_SOURCE_DIV       = TIM_CLOCK_DIVIDER_2;          //20M
    TIM_InitStruct.TIM_PWM_En           = PWM_ENABLE;
    TIM_InitStruct.TIM_Mode             = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_PWM_High_Count   = PWM_DUTY_COUNT_GET(100);           
    TIM_InitStruct.TIM_PWM_Low_Count    = PWM_DUTY_COUNT_GET(0);            
    TIM_TimeBaseInit(GREEN_PWM_TIMER_NUM, &TIM_InitStruct);
    
    TIM_InitStruct.TIM_SOURCE_DIV       = TIM_CLOCK_DIVIDER_2;          //20M
    TIM_InitStruct.TIM_PWM_En           = PWM_DISABLE;
    TIM_InitStruct.TIM_Mode             = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_Period           = LED_CTR_TIMER_PERIOD;        
    TIM_TimeBaseInit(LED_CTR_TIMER_NUM, &TIM_InitStruct);
  
    /*  Enable TIMER IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = LED_CTR_TIMER_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    TIM_ClearINT(LED_CTR_TIMER_NUM);
    TIM_INTConfig(LED_CTR_TIMER_NUM, ENABLE);
    
    TIM_Cmd(RED_PWM_TIMER_NUM, ENABLE);
    TIM_Cmd(BLUE_PWM_TIMER_NUM, ENABLE);
    TIM_Cmd(GREEN_PWM_TIMER_NUM, ENABLE);
}

void beep_timer_timeout_callback()
{
	static uint32_t u32Timecnt = 0;
	static bool bDelay = true;   //间隔标志
	
	u32Timecnt += 50; //100ms一次

	//超出1.5秒 自动退出蜂鸣器
	if(u32Timecnt >= 1500)
	{
		u32Timecnt = 0; 
		beepOpenControl(false);
		beepUnlock();
	}

	if(bDelay)
	{
		bDelay = false;
		TIM_Cmd(BEEP_PWM_TIMER_NUM, ENABLE);
	}

	if(u32Timecnt == s_u16BeepTime + BEEP_INTERVAL_MS/2)
	{
		u32Timecnt = 0; 
		if(s_u8BeepCount == 1)
		{
			//关闭定时器
			beepOpenControl(false);
			beepUnlock();
		}
		else
		{
			s_u8BeepCount--;
			TIM_Cmd(BEEP_PWM_TIMER_NUM, DISABLE);
			bDelay = true;
		}
	}
	
	menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true); 
}

void driver_beep_init(void)
{
	//2k ((1000/100)*(50*PWM_FREQ_HZ))-1
	//uint32_t beepCnt = ((500/100)*(50*PWM_FREQ_HZ))-1; //4K 
	uint32_t beepCnt = ((74)*(5*PWM_FREQ_HZ))-1; //2.7K 
	
	Pad_Config(BEEP_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH); 

	TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);
	TIM_InitStruct.TIM_SOURCE_DIV       = TIM_CLOCK_DIVIDER_1;          //40M
    TIM_InitStruct.TIM_PWM_En           = PWM_ENABLE;
    TIM_InitStruct.TIM_Mode             = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_PWM_High_Count   = beepCnt;           
    TIM_InitStruct.TIM_PWM_Low_Count    = beepCnt;            
    TIM_TimeBaseInit(BEEP_PWM_TIMER_NUM, &TIM_InitStruct);
	
	Pinmux_Config(BEEP_PIN, BEEP_PWM_OUT_PIN_PINMUX); 
}

void driver_led_init(void)
{	
	bool ret = false;

	s_bLedInit = true;
	
    Pad_Config(LED_RED_PIN,   PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(LED_BLUE_PIN,  PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(LED_GREEN_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH); 

    driver_pwm_init();
    /* PWM mode */
    Pinmux_Config(LED_RED_PIN,   RED_PWM_OUT_PIN_PINMUX);        
    Pinmux_Config(LED_BLUE_PIN,  BLUE_PWM_OUT_PIN_PINMUX);        
    Pinmux_Config(LED_GREEN_PIN, GREEN_PWM_OUT_PIN_PINMUX);  

	driver_beep_init();
	s_bBeepInit = true;
	
	if(led_mutex_handle == NULL)
	{
		if (os_mutex_create(&led_mutex_handle) != true)
		{
			APP_PRINT_INFO0("bt_men os_mutex_create fail");
		}
	}

	if(beep_mutex_handle == NULL)
	{
		if (os_mutex_create(&beep_mutex_handle) != true)
		{
			APP_PRINT_INFO0("beep_mutex_handle os_mutex_create fail");
		}
	}

	ret = os_timer_create(&pBeepTimer, "pBeepTimer",  BEEP_OS_TIMER_ID, \
    	                        BEEP_INTERVAL_MS, true, beep_timer_timeout_callback);
    if (!ret)
    {
        APP_PRINT_INFO1("pBeepTimer retval is %d", ret);
    }
    else
    {
        APP_PRINT_INFO0("create pBeepTimer suc!");
		//os_timer_start(&pBeepTimer);
    }

    APP_PRINT_INFO0("driver led init");
}

void gpio_led_enter_dlps_config(void)
{
    Pad_Config(LED_RED_PIN,   PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(LED_BLUE_PIN,  PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

	Pad_Config(LED_GREEN_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

	//Pad_ControlSelectValue(UART_TX_PIN, PAD_SW_MODE);
    //Pad_ControlSelectValue(UART_RX_PIN, PAD_SW_MODE);
	
	Pad_Config(BEEP_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
	
	s_bBeepInit = false;
	s_bLedInit = false;
}

void gpio_led_exit_dlps_config(void)
{
    Pad_Config(LED_RED_PIN,   PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(LED_BLUE_PIN,  PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(LED_GREEN_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_HIGH);

}

//获取蜂鸣器工作状态
bool beepWorkStatusGet(void)
{
	return s_bBeepWorking;
}

//获取LED工作状态
bool ledWorkStatusGet(void)
{
	return s_bLedWorking;
}

void beepOpenControl(bool flag)
{
	if(flag == true)
	{		
		
		os_timer_start(&pBeepTimer);
		TIM_Cmd(BEEP_PWM_TIMER_NUM, ENABLE);
		s_bBeepWorking = true;
	}		
	else
	{
		TIM_Cmd(BEEP_PWM_TIMER_NUM, DISABLE);
		os_timer_stop(&pBeepTimer);
		s_bBeepWorking = false;
	}
		
}

void beepWorkControl(uint16_t times, uint8_t count)
{
	beepLock();
	s_u8BeepCount = count;
	s_u16BeepTime = times;
	beepOpenControl(true);

}

static void LedTimerStart(void)
{
    ledFrequencyCount = 0;
    //ledControlFlag = SET;

	s_bLedWorking = true;
    TIM_Cmd(LED_CTR_TIMER_NUM, ENABLE);
}


static void LedTimerStop(void)
{
 
    //LedControl(EM_LED_WHITE, LED_PULSE_OFF);

	s_bLedWorking = false;
    TIM_Cmd(LED_CTR_TIMER_NUM, DISABLE);
}


static void SetLedPwmPulse(uint8_t colour, uint32_t duty)
{
    switch(colour)
    {
        case EM_LED_RED:
            TIM_PWMChangeFreqAndDuty(RED_PWM_TIMER_NUM, PWM_DUTY_COUNT_GET(duty), PWM_DUTY_COUNT_GET((100-duty)));
            break;
        
        case EM_LED_BLUE:
            TIM_PWMChangeFreqAndDuty(BLUE_PWM_TIMER_NUM, PWM_DUTY_COUNT_GET(duty), PWM_DUTY_COUNT_GET((100-duty)));
            break;

        case EM_LED_GREEN:
            TIM_PWMChangeFreqAndDuty(GREEN_PWM_TIMER_NUM, PWM_DUTY_COUNT_GET(duty), PWM_DUTY_COUNT_GET((100-duty)));
            break;
        
        default:
            break;
    }
}

static void LedControl(uint8_t colour,uint32_t Val)
{
    int i;
    for(i=0; i<LED_COLOUR_NUM; i++)
    {
        if(COLOUR(colour,i))
        {
            SetLedPwmPulse((1<<i), Val); 
        }
    }
    return;
}

uint8_t ledModeGet(void)
{
    return stLedCtrlInfo.ucLedCtrlMode;
}

//参数frequency 假设想100ms闪烁一次，需要填写20
void ledModeSet(EM_LED_CTRL mode, EM_LED_COLOR colour, uint32_t frequency, uint16_t count)
{
//    APP_PRINT_INFO2("ledModeSet mode:%d colour:%d", mode, colour);

	//ledLock();
	LedControl(EM_LED_WHITE, LED_PULSE_OFF);
    LedTimerStop();		

    stLedCtrlInfo.ucLedCtrlMode = mode;
    stLedCtrlInfo.ucLedCtrColor = colour;

    switch(stLedCtrlInfo.ucLedCtrlMode)
    {
        case EM_LED_CTRL_ON:
			s_bLedWorking = true;
            LedControl(stLedCtrlInfo.ucLedCtrColor, LED_PULSE_ON);
            break;

        case EM_LED_CTRL_OFF:
            LedControl(stLedCtrlInfo.ucLedCtrColor, LED_PULSE_OFF);
            break;

        case EM_LED_CTRL_BLINK:    
            stLedCtrlInfo.ucLedCtrCycle = frequency;
            stLedCtrlInfo.ucLedCtrCount = count;
            stLedCtrlInfo.ucLedCtrPulse = LED_PULSE_ON;
			ledControlFlag = SET;
            if(stLedCtrlInfo.ucLedCtrCount == 0)
            {
                stLedCtrlInfo.ucLedCtrDecFlag = RESET;
                stLedCtrlInfo.ucLedCtrCount = 1;
				
            }
            else
            {
                stLedCtrlInfo.ucLedCtrDecFlag = SET;
            }
            LedTimerStart();
            break;
            
        case EM_LED_CTRL_PWM:
			ledPwmModeSet(100, 0, 100); 
            stLedCtrlInfo.ucLedCtrCount = count;            //次数
        
            if(stLedCtrlInfo.ucLedCtrCount == 0)    //当这里次数为0就会一直呼吸
            {
                stLedCtrlInfo.ucLedCtrDecFlag = RESET;
                stLedCtrlInfo.ucLedCtrCount = 1;
            }
            else
            {
                stLedCtrlInfo.ucLedCtrDecFlag = SET;
            }
            
            //APP_PRINT_INFO5("led control Mode:%d, Color:%d, Cycle:%d, MaxDuty:%d, MinDuty:%d", 
            //                 stLedCtrlInfo.ucLedCtrlMode, stLedCtrlInfo.ucLedCtrColor,
            //                 stLedCtrlInfo.ucLedCtrCycle, stLedCtrlInfo.ucLedPwmMaxDuty,
            //                 stLedCtrlInfo.ucLedPwmMinDuty);
            LedTimerStart();
            break;
		case EM_LED_CTRL_PWM_SY:
			ledPwmModeSet(100, 0, 100);  
            stLedCtrlInfo.ucLedCtrCount = count;  //次数
        
            if(stLedCtrlInfo.ucLedCtrCount == 0)  //当次数为0的时候默认由亮到灭的变化
            {
            	ledControlFlag = SET;            //set代表着ucLedCtrCount++
				stLedCtrlInfo.ucLedCtrPulse = 0;
				stLedCtrlInfo.ucLedCtrCount = 1;
            }
            else								//当次数为1就是由灭到亮，或者大于1就是多次呼吸的效果
            {
            	ledControlFlag = RESET;			//reset代表着ucLedCtrCount--
            	stLedCtrlInfo.ucLedCtrPulse = 100;
            }
            
           // APP_PRINT_INFO6("EM_LED_CTRL_PWM_SY control Mode:%d, Color:%d, Cycle:%d, MaxDuty:%d, MinDuty:%d, ucLedCtrPulse:%d", 
           //                  stLedCtrlInfo.ucLedCtrlMode, stLedCtrlInfo.ucLedCtrColor,
            //                 stLedCtrlInfo.ucLedCtrCycle, stLedCtrlInfo.ucLedPwmMaxDuty,
            //                 stLedCtrlInfo.ucLedPwmMinDuty, stLedCtrlInfo.ucLedCtrPulse);
            LedTimerStart();
			break;
			
        default:
            APP_PRINT_INFO0("ledModeSet func switch default");
            break;
    }
	//ledUnlock();
}

void ledPwmModeSet(uint8_t max_duty, uint8_t min_duty, uint8_t freq)
{
    stLedCtrlInfo.ucLedCtrPulse = LED_PULSE_ON;
    
    stLedCtrlInfo.ucLedPwmMaxDuty = max_duty;       //0-100
    stLedCtrlInfo.ucLedPwmMinDuty = min_duty;       //0-100
    stLedCtrlInfo.ucLedCtrCycle = 200/freq;         //每秒变化率1~100,如100% -> 10ms占空比加一 -> ucLedCtrCycle = 200/100(irq: 5ms)
}

void ledModeWait(uint16_t delay)
{
    while(delay--)
    {
        if(EM_LED_CTRL_OFF == ledModeGet())
        {
            break;
        }
        delay_ms(10);
    }
}

//设置蜂鸣器
void background_msg_set_beep(uint16_t u16Times, uint16_t u16Count)
{
	T_MENU_MSG menMsg = {0};
	menMsg.type = BACKGROUND_MSG_TYPE_BEEP;
	s_u16BeepData[0] = u16Times;	 //蜂鸣器时间ms
	s_u16BeepData[1] = u16Count;	//蜂鸣器次数
	menMsg.u.buf = (void *)s_u16BeepData;
	menMsg.len = 2;
	background_task_msg_send(&menMsg);

}

//设置led状态(subtype T_BACKGROUND_MSG_LED_SUBTYPE)
void background_msg_set_led(unsigned char subtype)
{
	T_MENU_MSG menMsg = {0};
	menMsg.type = BACKGROUND_MSG_TYPE_LED;
	menMsg.subtype = subtype;
	background_task_msg_send(&menMsg);
}

void background_msg_led_handle(T_MENU_MSG *p_background_msg)
{
	if(!s_bLedInit)
	{
		driver_led_init();
	}
	
    switch(p_background_msg->subtype)
    {
        case BACKGROUND_MSG_LED_SUBTYPE_MATCH_SUCCESS:	//匹配成功
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_GREEN, 50, 3); 
            break;
        
        case BACKGROUND_MSG_LED_SUBTYPE_MATCH_FAIL:		//匹配失败
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_RED, 80, 3); 
            break;
        
        case BACKGROUND_MSG_LED_SUBTYPE_ENROLL_STATE:	//注册中
			ledModeSet(EM_LED_CTRL_ON, EM_LED_BLUE, 100, 0xFFFF); 
            break;
        
        case BACKGROUND_MSG_LED_SUBTYPE_ENROLL_SUCCESS:	//注册成功
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_GREEN, 200, 1); 
			while(stLedCtrlInfo.ucLedCtrlMode != EM_LED_CTRL_OFF){};
            break;
        
        case BACKGROUND_MSG_LED_SUBTYPE_ENROLL_FAIL:	//注册失败
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_RED, 50, 1); 
            break;
       		
		case BACKGROUND_MSG_LED_SUBTYPE_RESET_INIT:	//回复出厂设置
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_PINK, 20, 3); 
            break;   
		
		case BACKGROUND_MSG_LED_SUBTYPE_POWER_LOW:	//低电量警告
			//ledModeSet(EM_LED_CTRL_BLINK, EM_LED_GREEN, 30, 1); 
			//while(stLedCtrlInfo.ucLedCtrlMode != EM_LED_CTRL_OFF);
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_RED, 20, 15); 
			while(stLedCtrlInfo.ucLedCtrlMode != EM_LED_CTRL_OFF);
			tuya_ble_alarm_report(E_LOW_POWER);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_MOTOR_BACK: 	//电机回转
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_GREEN, 30, 1); 
			break;
		
		case BACKGROUND_MSG_LED_SUBTYPE_PRESS_SUC:	//按压成功一次
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_BLUE, 10, 1);
			while(stLedCtrlInfo.ucLedCtrlMode != EM_LED_CTRL_OFF){};
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_CLEAN_MODE_SUC: //
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_PINK, 50, 1);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_PRESS_ALARM: 	//按压警告
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_RED, 50, 0xFFFF);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_UART_MODE:
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_BLUE, 100, 1);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_ENROLL_TIMEOUT:
			ledModeSet(EM_LED_CTRL_OFF, EM_LED_BLUE, 100, 1);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_CHARGE:  //充电中
			ledModeSet(EM_LED_CTRL_PWM, EM_LED_BLUE, 100, 0);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_CHARGE_FULL: //充满电
			ledModeSet(EM_LED_CTRL_ON, EM_LED_GREEN, 0, 0);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_CHARGE_STOP:	//停止充电
			ledModeSet(EM_LED_CTRL_OFF, EM_LED_WHITE, 100, 0);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_BLE_CONNECT_TIPS:		//蓝牙连接提示
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_BLUE, 30, 3);
			while(stLedCtrlInfo.ucLedCtrlMode != EM_LED_CTRL_OFF);
			break;
		
		case BACKGROUND_MSG_LED_SUBTYPE_BLE_DISCONNECT_TIPS:	//蓝牙断开提示
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_WHITE, 30, 1);
			while(stLedCtrlInfo.ucLedCtrlMode != EM_LED_CTRL_OFF);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_GREEN_ON:
			ledModeSet(EM_LED_CTRL_ON, EM_LED_GREEN, 0, 0);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_RED_ON:
			ledModeSet(EM_LED_CTRL_ON, EM_LED_RED, 0, 0);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_BLUE_ON:
			ledModeSet(EM_LED_CTRL_ON, EM_LED_BLUE, 0, 0);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_STOP_ALL:
			ledModeSet(EM_LED_CTRL_OFF, EM_LED_WHITE, 0, 0);
			break;
		
		case BACKGROUND_MSG_LED_SUBTYPE_INIT_TIPS:
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_RED, 50, 3);
			while(stLedCtrlInfo.ucLedCtrlMode != EM_LED_CTRL_OFF);
			ledModeSet(EM_LED_CTRL_ON, EM_LED_RED, 0, 0);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_ENROLL_TIPS:
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_BLUE, 50, 3);
			while(stLedCtrlInfo.ucLedCtrlMode != EM_LED_CTRL_OFF);
			ledModeSet(EM_LED_CTRL_ON, EM_LED_BLUE, 0, 0);
			break;
			
		case BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN:
			ledModeSet(EM_LED_CTRL_ON, EM_LED_PINK, 50, 1);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN_FAIL:
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_RED, 40, 1); 
			while(stLedCtrlInfo.ucLedCtrlMode != EM_LED_CTRL_OFF);
            break;

		case BACKGROUND_MSG_LED_SUBTYPE_REPEAT:
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_GREEN, 50, 2);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_PRESS_FAIL:
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_RED, 40, 1); 
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_EXIT_ENROLL:
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_BLUE, 40, 3);
			break;

		case BACKGROUND_MSG_LED_SUBTYPE_CHECK_ADMIN_SUC:
			ledModeSet(EM_LED_CTRL_BLINK, EM_LED_PINK, 30, 1);
			while(stLedCtrlInfo.ucLedCtrlMode != EM_LED_CTRL_OFF){};
			break;
    }
}


//蜂鸣器处理接口
uint32_t background_msg_beep_handle(T_MENU_MSG *p_msg)
{
	if(p_msg == NULL || (p_msg->len != 2))
	{
		APP_PRINT_INFO0("input param error");
		return 0;
	}

	if(!s_bBeepInit)
	{
		s_bBeepInit = true;
		driver_beep_init();
	}		

	uint16_t *p = (uint16_t *)p_msg->u.buf;

	uint16_t times = p[0];		//响的时间
	uint8_t count = p[1];		//响的次数
	//APP_PRINT_INFO2("times is %d, count is %d", times, count);
	
	beepWorkControl(times, count);

	return 0;
}


int ledAutoCtrl(EM_LED_AUTO_CTRL type)
{
		
	if((SYSSET_GetSystemPolicy() & 0x40) == 0)
	{
		return 0;
	}		
	
	ledPwmModeSet(100, 0, 100);
	EM_LED_CTRL mode = EM_LED_CTRL_OFF;
	char count  = 0;
	
	//APP_PRINT_INFO1("ledAutoCtrl type:%d", type);
#ifndef SY_PROTOCOL_FALG //晟元协议标志 在keil5 c/c++ option选项里面定义了
	mode = EM_LED_CTRL_PWM;
	if(type == AUTO_LED_BACKLIGHRT)		
		count = 0;
	else
		count = 1;
#else
	mode = EM_LED_CTRL_PWM_SY;
	count = 2;
#endif

	switch (type)
    {
        case AUTO_LED_MATCH_OK:   
        case AUTO_LED_STORAGE_OK:
			//ledModeSet(EM_LED_CTRL_BLINK, EM_LED_GREEN, LED_FREQ_MS(800), 1);
			ledModeSet(mode, EM_LED_GREEN, 0, count);
        break;

        case AUTO_LED_CAPT_OK:
			ledModeSet(mode, EM_LED_GREEN, 0, count);
        break;

        case AUTO_LED_FAIL:
			ledModeSet(mode, EM_LED_RED, 0, count);
        break;

        case AUTO_LED_DIE:
			ledModeSet(mode, EM_LED_RED, 0, count);
        break;
        
        case AUTO_LED_BACKLIGHRT:
			ledModeSet(mode, EM_LED_CYAN, 0, count);
            break;
				
		case AUTO_LED_CLOSE:
			LedControl(EM_LED_WHITE, LED_PULSE_OFF);
			stLedCtrlInfo.ucLedCtrlMode = EM_LED_CTRL_OFF;
			break;
		
        default:
        	break;
    }

    return 0;
}

void Timer4_Handler(void)   //5ms
{
    TIM_ClearINT(LED_CTR_TIMER_NUM);	
    
    switch(stLedCtrlInfo.ucLedCtrlMode)
    {            
        case EM_LED_CTRL_BLINK:
            if( !(ledFrequencyCount % stLedCtrlInfo.ucLedCtrCycle))
            {
                if(ledControlFlag == SET)
                {
                    LedControl(stLedCtrlInfo.ucLedCtrColor, LED_PULSE_ON);  
                    
                    ledControlFlag = RESET;                                      
                }
                else
                {
                    LedControl(stLedCtrlInfo.ucLedCtrColor, LED_PULSE_OFF);
                    
                    ledControlFlag = SET;
                    if(stLedCtrlInfo.ucLedCtrDecFlag == SET)
                    {
                        stLedCtrlInfo.ucLedCtrCount--;
                    }
                    
                }

                if(0 == stLedCtrlInfo.ucLedCtrCount)
                {          
                	stLedCtrlInfo.ucLedCtrlMode = EM_LED_CTRL_OFF;
                }
            }
            break;
            
        case EM_LED_CTRL_PWM:
            if( !(ledFrequencyCount % stLedCtrlInfo.ucLedCtrCycle))
            {
                if(ledControlFlag == SET)
                {   
                    stLedCtrlInfo.ucLedCtrPulse++;              //默认一开始由亮->灭
                    
                    if(stLedCtrlInfo.ucLedCtrPulse == stLedCtrlInfo.ucLedPwmMaxDuty)
                    {
                        ledControlFlag = RESET;
                    }
                }
                else
                {
                    stLedCtrlInfo.ucLedCtrPulse--;
                    if(stLedCtrlInfo.ucLedCtrPulse == stLedCtrlInfo.ucLedPwmMinDuty)
                    {                    	
                        ledControlFlag = SET;
                        if(stLedCtrlInfo.ucLedCtrDecFlag == SET)
                        {
                            stLedCtrlInfo.ucLedCtrCount--;
                        }
                    }    
                }
								
                LedControl(stLedCtrlInfo.ucLedCtrColor, stLedCtrlInfo.ucLedCtrPulse);
                
                if(0 == stLedCtrlInfo.ucLedCtrCount)
                {
                    stLedCtrlInfo.ucLedCtrlMode = EM_LED_CTRL_OFF;
                }
            }
            break;
			
		case EM_LED_CTRL_PWM_SY:  //5ms 改变一次PWM值			
            if( !(ledFrequencyCount % stLedCtrlInfo.ucLedCtrCycle))
            {
                if(ledControlFlag == SET)
                {   
                    stLedCtrlInfo.ucLedCtrPulse++;              //由亮->灭
                    
                    if(stLedCtrlInfo.ucLedCtrPulse == stLedCtrlInfo.ucLedPwmMaxDuty)
                    {
                        ledControlFlag = RESET;
						stLedCtrlInfo.ucLedCtrCount--;
                    }
                }
                else
                {
                    stLedCtrlInfo.ucLedCtrPulse--;				//由灭->亮
                    if(stLedCtrlInfo.ucLedCtrPulse == stLedCtrlInfo.ucLedPwmMinDuty)
                    {						
                        ledControlFlag = SET;
                        stLedCtrlInfo.ucLedCtrCount--;
						
                    }    
                }
                
                LedControl(stLedCtrlInfo.ucLedCtrColor, stLedCtrlInfo.ucLedCtrPulse);
                
                if(0 == stLedCtrlInfo.ucLedCtrCount)
                {
                    stLedCtrlInfo.ucLedCtrlMode = EM_LED_CTRL_OFF;
                }
            }
            break;
        case EM_LED_CTRL_OFF:			
			LedTimerStop();			  
            break;
        
        default:
            APP_PRINT_INFO0("driver_led_timeout func switch default");
            break;
    }	

    ledFrequencyCount++;
}












