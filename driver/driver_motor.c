#include "driver_motor.h"
#include "rtl876x_pinmux.h"
#include "board.h"
#include "trace.h"
#include "bluetooth_menu.h"
#include "rtl876x_adc.h"
#include "rtl876x_rcc.h"
#include "rtl876x_nvic.h"
#include "rtl876x_gdma.h"
#include "adc_lib.h"
#include "driver_led.h"
#include "os_timer.h"
#include "os_sched.h"
#include "rtl876x_gpio.h"
#include "tuya_ble_app_demo.h"
#include "rtl876x_rcc.h"
#include "rtl876x_tim.h"
#include <peripheral_app.h>


static ST_MOTOR_CTRL_INFO st_motorCtrlInfo = {EM_MOTOR_CTRL_OFF, 0, 0, 0, 0, 0};
static ST_MOTOR_PARAMETER_INFO st_motorParameter;

static void *pMotorTimer;	

static uint16_t s_VolData[SAMPLE_AVG_MAX] = {0};		//��ص�������
static uint16_t s_MotorData[SAMPLE_AVG_MAX] = {0};		//�����ת��������

float s_VolAvg = 5000;		//��ص���ƽ��ֵ
static float s_MotAvg = 0;		//�����תƽ��ֵ
static bool s_SampleFlag = false;	//�ɼ��õı�־
//static uint8_t s_u8SampleCnt = 0;	//ÿ�ο�������ĵ�ǰ��������
static float s_fPerData = 0;   //����֮ǰ�Ķ�תֵ
bool Motor_Rst_Flag = false;

static E_DOOR_STATUS s_eDoorStatus = E_OPEN_NONE;  //����״̬
extern bool b_HAL_Check_Motor_Status;
#define PWM_DUTY_COUNT_GET3(x)           ((x==0) ? (0): ((((1)*((x*(20))))-1)))  //20K
//#define PWM_DUTY_COUNT_GET3(x)           ((x==0) ? (0): ((((5)*((x*(20))))-1)))  //4k


EM_MOTOR_CTRL_MODE motorModeStateGet(void)
{
    return st_motorCtrlInfo.ucMotorCtrlMode;
}

void driver_motor_motor_para_init(void)
{
    st_motorParameter.ucMotorParaDir = 0;                       //Ĭ�ϣ�0���෴��1
    st_motorParameter.ucMotorParaRecvMode = 0;                  //Ĭ�ϣ�0���л�ת��1
    st_motorParameter.ucMotorParaBlockMode = 1;                 //Ĭ�ϣ�0���ж�ת��⣺1
    st_motorParameter.ucMotorParaBlockCount = 0;                //Ĭ�ϣ�0����ת�ļ�����ֵ�� ucMotorParaBlockMode = 1ʱ��Ч

    st_motorParameter.ucMotorParaRecvTimeOfOn = 0;              //Ĭ�ϣ�0����ת���תʱ��ֵ�� ucMotorParaRecvMode = 1ʱ��Ч
    st_motorParameter.ucMotorParaRecvTimeOfBack = 0;            //Ĭ�ϣ�0����ת���תʱ��ֵ�� ucMotorParaRecvMode = 1ʱ��Ч

    st_motorParameter.ucMotorParaBlockAdcThreshold = 70;         //Ĭ�ϣ�0����ת�����Ч����ֵ�� ucMotorParaBlockMode = 1ʱ��Ч
    st_motorParameter.ucMotorParaBlockAdcSampleValue = 0;       //Ĭ�ϣ�0��ϵͳ����Ĭ��ֵ��     ucMotorParaBlockMode = 1ʱ��Ч

    st_motorParameter.ucMotorParaBlockSubtact = 0;              //Ĭ�ϣ�0����ת������ֵ��     ucMotorParaBlockMode = 1ʱ��Ч
    st_motorParameter.ucMotorParaNoBlockTime = 0;               //Ĭ�ϣ�0����������ʱ�䣬     ucMotorParaBlockMode = 1ʱ��Ч
}

/**
  * @brief  Initialize ADC peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_adc_init(void)
{
	
	ADC_DeInit(ADC);
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    ADC_InitTypeDef ADC_InitStruct;
    ADC_StructInit(&ADC_InitStruct);
    /* Configure the ADC sampling schedule, a schedule represents an ADC channel data,
       up to 16, i.e. schIndex[0] ~ schIndex[15] */
    //for(i = 0; i < 16; i++)
    ADC_InitStruct.ADC_SchIndex[0]  = EXT_SINGLE_ENDED(VOL_CHANNEL);
	ADC_InitStruct.ADC_SchIndex[1]  = EXT_SINGLE_ENDED(MOT_CHANNEL);
	//ADC_InitStruct.ADC_SchIndex[1]  = EXT_SINGLE_ENDED(5);
    /* Set the bitmap corresponding to schedule, 16 bits, LSB,
       schIndex[0-15] corresponding to 16 bits of bitmap bit0-bit15.
       For example, if config schIndex[0] and schIndex [1], then bitmap is 0000 0000 0011 (that is, 0x0003);
       if config schIndex [0] and schIndex [2], then bitmap is 0000 0000 0101 (that is, 0x0005).
    */
    ADC_InitStruct.ADC_Bitmap       = 0x3;
    
    /* Configuration of ADC continuous sampling cycle,
       can be configured to (1-256) clock cycles.
       That is, N = T*10000000 - 1 ,max = 255.
    */
    ADC_InitStruct.ADC_SampleTime   = 255;
    /* Configure the interrupt of ADC_INT_FIFO_TH threshold value. */
    ADC_InitStruct.ADC_FifoThdLevel = 16;

    ADC_Init(ADC, &ADC_InitStruct);

    ADC_BypassCmd(VOL_CHANNEL, DISABLE);
	ADC_BypassCmd(MOT_CHANNEL, DISABLE);
    APP_PRINT_INFO0("[ADC] ADC sample mode is divide mode !");

	//ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);
    ADC_INTConfig(ADC, ADC_INT_FIFO_THD, ENABLE);
	//ADC_INTConfig(ADC, ADC_INT_FIFO_RD_ERR, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    /* When ADC is enabled, sampling will be done quickly and interruption will occur.
       After initialization, ADC can be enabled when sampling is needed.*/
    ADC_Cmd(ADC, ADC_CONTINUOUS_MODE, ENABLE);
}

void driver_adc_start(void)
{
	Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
	os_delay(100);
	ADC_Cmd(ADC, ADC_CONTINUOUS_MODE, ENABLE);
}


void global_data_adc_init(void)
{
    /* Initialize adc k value! */
    bool adc_k_status = false;
    adc_k_status = ADC_CalibrationInit();
    if (false == adc_k_status)
    {
        APP_PRINT_ERROR0("[io_adc] global_data_adc_init: ADC_CalibrationInit fail!");
    }
	else
		APP_PRINT_INFO0("[io_adc] global_data_adc_init success");
}

void gpio_motor_enter_dlps_config(void)
{	
	Pad_Config(MOTOR_LEFT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
	Pad_Config(MOTOR_RIGHT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);

	Pad_Config(MOTOR_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	Pad_Config(BAT_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	//Pad_Config(BAT_CHG_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	//Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
}

void gpio_motor_exit_dlps_config(void)
{
	//GPIO�ܽų�ʼ��   
	Pad_Config(MOTOR_LEFT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_LOW);
	Pad_Config(MOTOR_RIGHT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_LOW);  

	Pad_Config(BAT_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
	Pad_Config(MOTOR_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
	
//	Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
}

//������ų�ʼ��
void gpio_charge_check_init()
{
	Pad_Config(BAT_CHG_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
	Pinmux_Config(BAT_CHG_PIN, DWGPIO);
	
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin        = GPIO_GetPin(BAT_CHG_PIN);
    GPIO_InitStruct.GPIO_Mode       = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd      = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
}

void driver_motor_left_change_pwm(uint8_t duty)
{
	Pad_Config(MOTOR_LEFT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH); 
	TIM_PWMChangeFreqAndDuty(MOTOR_PWM_LEFT_TIMER_NUM, PWM_DUTY_COUNT_GET3(duty), PWM_DUTY_COUNT_GET3((100-duty)));
}

void driver_motor_right_change_pwm(uint8_t duty)
{
	Pad_Config(MOTOR_RIGHT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH); 
	TIM_PWMChangeFreqAndDuty(MOTOR_PWM_RIGHT_TIMER_NUM, PWM_DUTY_COUNT_GET3(duty), PWM_DUTY_COUNT_GET3((100-duty)));
}

void driver_motor_pwm_init(void)
{	

	Pad_Config(MOTOR_LEFT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_LOW); 
	Pad_Config(MOTOR_RIGHT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_LOW); 

	Pinmux_Config(MOTOR_LEFT,   MOTOR_PWM_OUT_LEFT_PIN_PINMUX); 
	Pinmux_Config(MOTOR_RIGHT,   MOTOR_PWM_OUT_RIGHT_PIN_PINMUX); 
	
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);

	
	TIM_InitStruct.TIM_SOURCE_DIV		= TIM_CLOCK_DIVIDER_40;			//20M
	TIM_InitStruct.TIM_PWM_En			= PWM_ENABLE;
	TIM_InitStruct.TIM_Mode 			= TIM_Mode_UserDefine;
	TIM_InitStruct.TIM_PWM_High_Count	= PWM_DUTY_COUNT_GET3(0);			
	TIM_InitStruct.TIM_PWM_Low_Count	= PWM_DUTY_COUNT_GET3(100); 		   
	TIM_TimeBaseInit(MOTOR_PWM_LEFT_TIMER_NUM, &TIM_InitStruct);
	
    TIM_InitStruct.TIM_SOURCE_DIV       = TIM_CLOCK_DIVIDER_40;          //20M
    TIM_InitStruct.TIM_PWM_En           = PWM_ENABLE;
    TIM_InitStruct.TIM_Mode             = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_PWM_High_Count   = PWM_DUTY_COUNT_GET3(0);           
    TIM_InitStruct.TIM_PWM_Low_Count    = PWM_DUTY_COUNT_GET3(100);            
    TIM_TimeBaseInit(MOTOR_PWM_RIGHT_TIMER_NUM, &TIM_InitStruct);

    TIM_Cmd(MOTOR_PWM_LEFT_TIMER_NUM, ENABLE);
    TIM_Cmd(MOTOR_PWM_RIGHT_TIMER_NUM, ENABLE);
}


void driver_motor_init(void)
{
	bool ret = 0;
	
    //������ʼ��
    driver_motor_motor_para_init();         
    
    //GPIO�ܽų�ʼ��   
	Pad_Config(MOTOR_LEFT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
	Pad_Config(MOTOR_RIGHT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);  

	Pad_Config(BAT_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
	Pad_Config(MOTOR_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
	
//	Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
		
	//Pad_Config(BAT_CHARGE, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
	//gpio_charge_check_init();
	driver_motor_pwm_init();
	
    //��ʱ����ʼ��
    ret = os_timer_create(&pMotorTimer, "pMotorTimer",  MOTOR_OS_TIMER_ID, \
                             MOTOR_INTERVAL_MS, true, driver_motor_time_handleCallback);
    if (!ret)
    {
        APP_PRINT_INFO1("pMotorTimer retval is %d", ret);
    }
    else
    {
        APP_PRINT_INFO0("create pMotorTimer suc!");
		//os_timer_start(&pMotorTimer);
    }
	global_data_adc_init();
	driver_adc_init();	
	
    //adc��ת����ʼ��
}

/*********************************************************************/
/***************************��оƬ�ܽſ������************************/
/*********************************************************************/
static void driver_motor_timer(bool isEnable)               //��ʱ��ʹ�ܻ����
{
    if(isEnable)                        
    {                  	    		
        //ʹ�ܶ�ʱ��
        os_timer_start(&pMotorTimer);

    }
    else
    {         		
        //���ܶ�ʱ��
        os_timer_stop(&pMotorTimer);

		if(s_eDoorStatus == E_OPEN_START)
		{
			s_eDoorStatus = E_OPEN_SUC;		
			Motor_Rst_Flag = true;
			tuya_ble_report_door_status(true);
		}
		else if(s_eDoorStatus == E_OPEN_SUC)
		{
			
			Motor_Rst_Flag = true;
		}
		else if(s_eDoorStatus == E_CLOSE_START)
		{
			s_eDoorStatus = E_CLOSE_SUC;
			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_MOTOR_BACK);
			tuya_ble_report_door_status(false);	
		}
    }
}


E_DOOR_STATUS door_open_status(void)
{
	return s_eDoorStatus;
}

void set_door_open_status(E_DOOR_STATUS e_door_status)
{
	 s_eDoorStatus = e_door_status;
}

void door_open_status_reset(void)
{
	s_eDoorStatus = E_OPEN_NONE;
	Pad_Config(MOTOR_LEFT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
	Pad_Config(MOTOR_RIGHT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW); 
}


static void driver_motor_on(void)                           //��ת
{
//	uint8_t battery_data;
//	battery_data = get_battery_data();
	//APP_PRINT_INFO0("driver_motor_on");
    if(st_motorParameter.ucMotorParaDir == 0)               //��������           
    {                                                      
        //�ܽſ���
        Pad_Config(MOTOR_LEFT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
		Pad_Config(MOTOR_RIGHT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
//		driver_motor_left_change_pwm(100);
//		driver_motor_right_change_pwm(0);
    }
    else                                                   
    {                                                       
        //�ܽſ���
        Pad_Config(MOTOR_LEFT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
		//Pad_Config(MOTOR_LEFT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
		Pad_Config(MOTOR_RIGHT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    }
    st_motorParameter.ucMotorParaBlockCount  = 0;
	s_MotAvg = 0;
	s_fPerData = 0;
    driver_motor_timer(true);
    
    return;
}


static void driver_motor_back(void)                         //��ת
{
	uint8_t battery_data;
	battery_data = get_battery_data();
	//APP_PRINT_INFO0("driver_motor_back");

    if(st_motorParameter.ucMotorParaDir == 0)               //��������
    {                                                      
        //�ܽſ���
        //Pad_Config(MOTOR_LEFT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
		//Pad_Config(MOTOR_RIGHT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_HIGH);
		driver_motor_left_change_pwm(0);
		if(battery_data > 50)
			driver_motor_right_change_pwm(55);
		else if (30 < battery_data < 50)
			driver_motor_right_change_pwm(65);
		else  if (battery_data < 30)
			driver_motor_right_change_pwm(75);
		
    }
    else                                                   
    {                                                       
        //�ܽſ���
        Pad_Config(MOTOR_LEFT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
		Pad_Config(MOTOR_RIGHT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
    }
	st_motorParameter.ucMotorParaBlockCount  = 0;
	s_MotAvg = 0;
	s_fPerData = 0;
    driver_motor_timer(true);

    return;
}


static void driver_motor_off(void)                          //ֹͣת��
{            
	//APP_PRINT_INFO0("driver_motor_off");
    //�ܽſ���        
    //�ص����ʱ�����������𵽿���ֹͣ�����ã�Ȼ���������𵽽�������ʱ�Ĺ���
    Pad_Config(MOTOR_LEFT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
	Pad_Config(MOTOR_RIGHT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH); 
//	driver_motor_left_change_pwm(100);
//	driver_motor_right_change_pwm(100);

	os_delay(100);
	st_motorParameter.ucMotorParaBlockCount = 0;
	//s_eDoorStatus = E_OPEN_NONE;
    driver_motor_timer(false);
	driver_motor_left_change_pwm(0);
	driver_motor_right_change_pwm(0);
	
//	Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
//	Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);

	
    return;
}


/***********************************************************/
/**********************�����߼��ӿ�*************************/
/***********************************************************/
//��ת���
void driver_motor_block_check(void)
{
		
	if(s_SampleFlag)
	{
		s_SampleFlag = false;		
//		if(1 == door_open_status())
//		{
//			APP_PRINT_INFO1("open_start_s_MotAvg is %d", (uint32_t)s_MotAvg);
//		}
//
//		if(3 == door_open_status())
//		{
//			APP_PRINT_INFO1("close_start_s_MotAvg is %d", (uint32_t)s_MotAvg);
//		}
//		APP_PRINT_INFO1("123123s_MotAvg is %d", (uint32_t)s_MotAvg);

		if( ((s_MotAvg>s_fPerData) ? (s_MotAvg-s_fPerData) : (s_fPerData-s_MotAvg)) > 1 )
		{
			st_motorParameter.ucMotorParaBlockCount = 0;
		}			
		else 
		{
			if((E_CLOSE_START == door_open_status()) && (s_MotAvg >= 70))    
			{
				st_motorParameter.ucMotorParaBlockCount++;				
				if(st_motorParameter.ucMotorParaBlockCount >= 2)
				{
					//ledModeSet(EM_LED_CTRL_BLINK, EM_LED_GREEN, 30 , 1);
					st_motorParameter.ucMotorParaBlockCount = 0;
					//driver_motor_off();					
					driver_motor_left_change_pwm(100);
					driver_motor_right_change_pwm(100); 							//��ɲ�����ٷ���Ϣ��ȥ����rtos��motor�������ܽ�״̬����Ϊ0
//					background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_MOTOR_BACK);

					background_msg_set_motor(BACKGROUND_MSG_MOTOR_SUBTYPE_OFF);
//					APP_PRINT_INFO1("123123st_motorCtrlInfo.ucMotorCtrlPreTime is %d", (uint32_t)st_motorCtrlInfo.ucMotorCtrlPreTime);
				}
			}
				
		}
		s_fPerData = s_MotAvg;
//		Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
		ADC_Cmd(ADC, ADC_CONTINUOUS_MODE, ENABLE);
	}
	
}



void driver_motor_control(EM_MOTOR_CTRL_MODE mode, unsigned int mtime)
{
	//APP_PRINT_INFO2("mode is %d, time is %d", mode, mtime);
    st_motorCtrlInfo.ucMotorCtrlMode = mode;

    st_motorCtrlInfo.ucMotorCtrlTotalTime = mtime;
    st_motorCtrlInfo.ucMotorCtrlPreTime = 0;

    st_motorCtrlInfo.ucMotorCtrlBlockStatus = RESET;        //��ն�ת��⹦�ܱ�־
    st_motorCtrlInfo.ucMotorCtrlBlockPreCount = 0;

    st_motorCtrlInfo.ucMotorCtrlRecvStatus = RESET;         //��ջ�ת����
    st_motorParameter.ucMotorParaBlockAdcSampleValue = 0;   //��ת����ֵ��0

    switch(st_motorCtrlInfo.ucMotorCtrlMode)
    {
        case EM_MOTOR_CTRL_OFF:
            driver_motor_off();
            break;

        case EM_MOTOR_CTRL_ON:
            driver_motor_on();
            //driver_motor_back();
            break;

        case EM_MOTOR_CTRL_BACK:
            driver_motor_back();
            break;

        case EM_MOTOR_CTRL_WAIT:
            driver_motor_timer(true);
            break;

        default:
            break;
    }

//    MOTOR_PRINT(("motor mode:%d, control time:%d, start time:%d, motorBlockCheckStatus:%d, motorRecvCheckStatus:%d\r\n", st_motorCtrlInfo.ucMotorCtrlMode, st_motorCtrlInfo.ucMotorCtrlTotalTime, st_motorParameter.ucMotorParaNoBlockTime, st_motorCtrlInfo.ucMotorCtrlBlockStatus, st_motorCtrlInfo.ucMotorCtrlRecvStatus));
}

void driver_motor_time_handleCallback(void)     //10ms
{
	st_motorCtrlInfo.ucMotorCtrlPreTime += MOTOR_INTERVAL_MS;
	
    if(st_motorCtrlInfo.ucMotorCtrlPreTime >= st_motorCtrlInfo.ucMotorCtrlTotalTime)
    {       
		background_msg_set_motor(BACKGROUND_MSG_MOTOR_SUBTYPE_OFF);
    }
    else if(st_motorParameter.ucMotorParaBlockMode == 1 && st_motorCtrlInfo.ucMotorCtrlPreTime >= 300)//��ת��⿪��
	{
		driver_motor_block_check();
	}

	menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true); 
			
}


void driver_motor_wait(uint16_t delay)
{
    while(delay--)
    {
        if(st_motorCtrlInfo.ucMotorCtrlMode == EM_MOTOR_CTRL_OFF)
        {
            break;
        }
        delay_ms(10);
    }
}

//���ļ��
uint8_t charge_check(void)
{
	static uint8_t s_u8CheckCnt = 0;
	static bool s_bChargeFlag = false;
	static uint8_t s_u8UsbOutCnt = 0;
	static uint8_t s_u8FullCnt = 0;
	
	if(s_eDoorStatus != E_OPEN_NONE)
	{
		s_u8CheckCnt = 0;
		return 0;
	}

	s_u8CheckCnt++;
	if(s_u8CheckCnt < 5) //500ms���һ���Ƿ��ڳ��
	{
		if(s_u8CheckCnt == 3)
			driver_adc_start();
		return 0;
	}
	s_u8CheckCnt = 0;
	
	if(GPIO_ReadInputDataBit(GPIO_GetPin(BAT_CHG_PIN)) == 0) //���ڳ��
	{		
		menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true); 
		if(!ledWorkStatusGet())
			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_CHARGE);
		s_bChargeFlag = true;		
	}
	else 
	{
		if(s_bChargeFlag)
		{	
			
			menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true); 

			APP_PRINT_INFO1("charge s_VolAvg is %d", (uint32_t)s_VolAvg);
			if(s_VolAvg > CHARGE_POWER_DATA)
			{
				s_u8FullCnt++;
				if(s_u8FullCnt >= 3)
				{
					APP_PRINT_INFO0("charge full");
					//���������̵�
					background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_CHARGE_FULL);
					s_u8FullCnt = 0;
				}				
				s_u8UsbOutCnt = 0;
			}
			else
			{
				s_u8FullCnt = 0;
				s_u8UsbOutCnt++;
				if(s_u8UsbOutCnt >= 3)
				{
					s_u8UsbOutCnt = 0;
					s_bChargeFlag = false;
					APP_PRINT_INFO0("usb out ");	
					background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_CHARGE_STOP);	
				}					
			}
			
		}
		
	}
	

	return 0;
}



//��ȡ��صİٷֱȣ�6.0V��Ӧ100, 4.0V��Ӧ0��4.6V��ʱ����е͵�ѹ������
uint8_t get_battery_data(void)
{
	uint8_t u8Vol = 0;
	
	if(u32Voltage >= FULL_POWER_DATA) 
		u8Vol = 100;
	else if(u32Voltage <= EMPTY_POWER_DATA)
		u8Vol = 0;
	else
		u8Vol = (uint8_t)((u32Voltage - EMPTY_POWER_DATA) / 20);
//		u8Vol = (uint8_t)(((uint32_t)u32Voltage - EMPTY_POWER_DATA) / 20);

	if(u8Vol > 100)
		u8Vol = 100;
//	APP_PRINT_INFO1("[io_adc]  voltage = %dmV",(uint32_t)s_VolAvg);
	return u8Vol;
}



//ð������
void bubble_Sort(void) 
{
	uint8_t i = 0, j = 0;
	uint16_t temp = 0;
	
    for(i = 0; i < SAMPLE_AVG_MAX - 1; i++) 
	{
        for(j = 0; j < SAMPLE_AVG_MAX - 1 - i; j++) 
		{
            if(s_VolData[j] > s_VolData[j+1]) {       // ����Ԫ�������Ա�
                temp = s_VolData[j+1];       // Ԫ�ؽ���
                s_VolData[j+1] = s_VolData[j];
                s_VolData[j] = temp;
            }
        }
    }

	for(i = 0; i < SAMPLE_AVG_MAX - 1; i++) 
	{
        for(j = 0; j < SAMPLE_AVG_MAX - 1 - i; j++) 
		{
            if(s_MotorData[j] > s_MotorData[j+1]) {       // ����Ԫ�������Ա�
                temp = s_MotorData[j+1];       // Ԫ�ؽ���
                s_MotorData[j+1] = s_MotorData[j];
                s_MotorData[j] = temp;
            }
        }
    }
	
    return ;
}


/**
  * @brief  Calculate adc sample voltage.
  * @param  No parameter.
  * @return void
  */
void io_adc_voltage_calculate(void)
{
	uint8_t i = 0;
    ADC_ErrorStatus errorStatus = NO_ERROR;
	uint32_t tmpVolData = 0; 
	uint32_t tmpMotData = 0; 
//	T_MENU_MSG menMsg = {0};

	//APP_PRINT_INFO2("len is %d, data is %b", tmpLen, TRACE_BINARY(32, p));
	//�����ȥ��ͷ��β����ֵ
	bubble_Sort();
    for (i = 1; i < SAMPLE_AVG_MAX-1; i++)
    {
        tmpVolData += s_VolData[i];
		tmpMotData += s_MotorData[i];
    }
	tmpVolData /= (SAMPLE_AVG_MAX - 2);
	tmpMotData /= (SAMPLE_AVG_MAX - 2);
	
    s_VolAvg = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)tmpVolData, &errorStatus);

	s_VolAvg = s_VolAvg * 4 + 70;
	
//    APP_PRINT_INFO5("[io_adc] vol rawdata = %d, voltage = %dmV , error_status = %d, battery = %d, cal = %d",
//                    tmpVolData, (uint32_t)s_VolAvg, errorStatus, get_battery_data(), tmpVolData*4-600);

	s_MotAvg = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)tmpMotData, &errorStatus);
	
    //APP_PRINT_INFO3("[io_adc] motor rawdata = %d, voltage = %dmV , error_status = %d",
     //               tmpMotData, (uint32_t)s_MotAvg, errorStatus);

	//if((s_eDoorStatus == E_OPEN_SUC) && (s_VolAvg < LOW_POWER_DATA))
	//{
		//APP_PRINT_INFO0("s_AlarmFlag happen");
	//	background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_POWER_LOW);
	//}

	
	s_SampleFlag = true;
	
    memset(s_VolData, 0, sizeof(s_VolData));
   	memset(s_MotorData, 0, sizeof(s_MotorData));

//	Pad_Config(BAT_EN_HAL1_POW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
}

void ADC_Handler(void)
{
    uint8_t dataLen = 0;
    uint16_t sampleData[32] = {0};
	uint8_t i = 0, j = 0; 

    if (ADC_GetINTStatus(ADC, ADC_INT_FIFO_THD) == SET)
    {
        ADC_Cmd(ADC, ADC_CONTINUOUS_MODE, DISABLE);

        dataLen = ADC_GetFIFODataLen(ADC);
        /* ADC continuous sampling mode, read data from FIFO.
           In multi-channel sampling,
           the data order in FIFO corresponds to the channel order set by schedule index.
        */
        ADC_ReadFIFOData(ADC, sampleData, dataLen);

        ADC_ClearFIFO(ADC);
        ADC_ClearINTPendingBit(ADC, ADC_INT_FIFO_THD);
	
        for (i = 0; i < 16; i++)
        {
        	if(i % 2 == 0) //ͨ��һ�ǵ�ص��������ݣ�ͨ�����ǵ����ת���ŵ�����
        	{
        		s_VolData[j] = sampleData[i];
        	}           	
			else  
			{
				s_MotorData[j] = sampleData[i];
				j++;
			}
        }

		io_adc_voltage_calculate();
		
    }

}

//���õ��(subtype T_BACKGROUND_MSG_MOTOR_SUBTYPE)
void background_msg_set_motor(unsigned char subtype)
{
	T_MENU_MSG menMsg = {0};
	menMsg.type = BACKGROUND_MSG_TYPE_MOTOR;
	menMsg.subtype = subtype;
	background_task_msg_send(&menMsg);
}

uint32_t background_msg_motor_handle(T_MENU_MSG *p_msg)
{
	if(p_msg == NULL)
	{		
		return 0;
	}

	uint32_t workTimes = 2000;
	
	switch(p_msg->subtype)
	{
		case BACKGROUND_MSG_MOTOR_SUBTYPE_LEFT:
			if(s_eDoorStatus == E_OPEN_NONE || s_eDoorStatus == E_OPEN_SUC)
				s_eDoorStatus = E_OPEN_START;
			Motor_Rst_Flag = true;
			driver_motor_control(EM_MOTOR_CTRL_ON, workTimes);
			break;

		case BACKGROUND_MSG_MOTOR_SUBTYPE_RIGHT:
			if(s_eDoorStatus == E_OPEN_SUC)
			{
				s_eDoorStatus = E_CLOSE_START;
			}
			workTimes = 500;
			driver_motor_control(EM_MOTOR_CTRL_BACK, workTimes);
			break;

		case BACKGROUND_MSG_MOTOR_SUBTYPE_OFF:
			workTimes = 100;
			driver_motor_control(EM_MOTOR_CTRL_OFF, workTimes);
			break;

		case BACKGROUND_MSG_MOTOR_SUBTYPE_RST:
			workTimes = 100;
			driver_motor_control(EM_MOTOR_CTRL_BACK, workTimes);
			break;
			
		default:
			break;
	}

	return 0;
}


