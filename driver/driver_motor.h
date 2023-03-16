#ifndef DRIVER_MOTOR_H
#define DRIVER_MOTOR_H

#include <rtl876x.h>
#include "driver_delay.h"

typedef enum 
{
    EM_MOTOR_CTRL_OFF   = 0,                        //关闭电机
    EM_MOTOR_CTRL_ON    = 1,                        //开启电机正转
    EM_MOTOR_CTRL_BACK  = 2,                        //开启电机反转
    EM_MOTOR_CTRL_WAIT  = 3,                        //等待状态
} EM_MOTOR_CTRL_MODE;

typedef struct 
{
    EM_MOTOR_CTRL_MODE ucMotorCtrlMode;             //电机转动模式

    unsigned int ucMotorCtrlTotalTime;              //电机需要转动的时间
    unsigned int ucMotorCtrlPreTime;                //电机当前转动的时间

    unsigned char ucMotorCtrlBlockStatus;           //电机堵转检测功能的使能标志   
    unsigned char ucMotorCtrlBlockPreCount;         //电机堵转检测功能的堵转检测次数(当前次数)

    unsigned char ucMotorCtrlRecvStatus;            //电机回转功能的使能标志
} ST_MOTOR_CTRL_INFO, *PST_MOTOR_CTRL_INFO;

typedef struct
{
    unsigned char ucMotorParaDir;                           //电机转动方向
    unsigned char ucMotorParaRecvMode;                      //停止后是否回转
    unsigned char ucMotorParaBlockMode;                     //堵转模式是否开启
    unsigned char ucMotorParaBlockCount;                    //堵转次数

    unsigned int ucMotorParaRecvTimeOfOn;                   //正转的回转时间
    unsigned int ucMotorParaRecvTimeOfBack;                 //反转回转时间
    
    unsigned short ucMotorParaBlockAdcThreshold;            //堵转检测计数触发阈值
    unsigned short ucMotorParaBlockAdcSampleValue;          //堵转检测的当前采样值

    unsigned char ucMotorParaBlockSubtact;                  //堵转的波动值
    unsigned char ucMotorParaNoBlockTime;                   //堵转检测起始时间
    
}ST_MOTOR_PARAMETER_INFO, *PST_MOTOR_PARAMETER_INFO;

//开门状态
typedef enum
{
	E_OPEN_NONE,   
	E_OPEN_START,  //开始开门
	E_OPEN_SUC,		//开门完成
	E_CLOSE_START,  //开始关门(回转)
	E_CLOSE_SUC,	//关门完成
}E_DOOR_STATUS;

#define ADC_VOLTAGE_CONVERT_3000MV_10BIT                           750/256
#define ADC_VOLTAGE_CONVERT_3300MV_10BIT                           825/256
#define VOLTAGE_CONVERT_CURRENT                                    (2) 

#define VOL_CHANNEL   	4		//电池电压通道
#define MOT_CHANNEL  	5		//电机堵转通道

#define MOTOR_INTERVAL_MS 		10			//定时器10ms间隔(这里要是10的倍数，不然时间不准)
#define ADC_CONTINUOUS_SAMPLE_PERIOD        (200-1)//sampling once 20ms

#define MOTOR_BLOCK_CHECK   0
#define SAMPLE_AVG_MAX   8	    //8个采集数据来求平均值
#define LOW_POWER_DATA   4600	//4.6V
#define NOT_OPEN_POWER_DATA   4000	//3.4V 不开门电压
#define EMPTY_POWER_DATA    4000  //3.5v

#define CHARGE_POWER_DATA   6200	//充电电压基本都超过4.3V
#define FULL_POWER_DATA		6000
extern float s_VolAvg;
extern bool Motor_Rst_Flag;

#define MOTOR_PWM_LEFT_TIMER_NUM             TIM1
#define MOTOR_PWM_OUT_LEFT_PIN_PINMUX        timer_pwm1


#define MOTOR_PWM_RIGHT_TIMER_NUM             TIM2
#define MOTOR_PWM_OUT_RIGHT_PIN_PINMUX        timer_pwm2


void driver_motor_init(void);
void driver_motor_control(EM_MOTOR_CTRL_MODE mode, unsigned int mtime);
void driver_motor_time_handleCallback(void);
void driver_motor_wait(uint16_t delay);
void gpio_motor_enter_dlps_config(void);
void gpio_motor_exit_dlps_config(void);
void driver_adc_start(void);
void door_open_status_reset(void);
void set_door_open_status(E_DOOR_STATUS e_door_status);
E_DOOR_STATUS door_open_status(void);
uint8_t get_battery_data(void);
void background_msg_set_motor(unsigned char subtype);
uint8_t charge_check(void);

#endif
