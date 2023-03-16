#ifndef DRIVER_MOTOR_H
#define DRIVER_MOTOR_H

#include <rtl876x.h>
#include "driver_delay.h"

typedef enum 
{
    EM_MOTOR_CTRL_OFF   = 0,                        //�رյ��
    EM_MOTOR_CTRL_ON    = 1,                        //���������ת
    EM_MOTOR_CTRL_BACK  = 2,                        //���������ת
    EM_MOTOR_CTRL_WAIT  = 3,                        //�ȴ�״̬
} EM_MOTOR_CTRL_MODE;

typedef struct 
{
    EM_MOTOR_CTRL_MODE ucMotorCtrlMode;             //���ת��ģʽ

    unsigned int ucMotorCtrlTotalTime;              //�����Ҫת����ʱ��
    unsigned int ucMotorCtrlPreTime;                //�����ǰת����ʱ��

    unsigned char ucMotorCtrlBlockStatus;           //�����ת��⹦�ܵ�ʹ�ܱ�־   
    unsigned char ucMotorCtrlBlockPreCount;         //�����ת��⹦�ܵĶ�ת������(��ǰ����)

    unsigned char ucMotorCtrlRecvStatus;            //�����ת���ܵ�ʹ�ܱ�־
} ST_MOTOR_CTRL_INFO, *PST_MOTOR_CTRL_INFO;

typedef struct
{
    unsigned char ucMotorParaDir;                           //���ת������
    unsigned char ucMotorParaRecvMode;                      //ֹͣ���Ƿ��ת
    unsigned char ucMotorParaBlockMode;                     //��תģʽ�Ƿ���
    unsigned char ucMotorParaBlockCount;                    //��ת����

    unsigned int ucMotorParaRecvTimeOfOn;                   //��ת�Ļ�תʱ��
    unsigned int ucMotorParaRecvTimeOfBack;                 //��ת��תʱ��
    
    unsigned short ucMotorParaBlockAdcThreshold;            //��ת������������ֵ
    unsigned short ucMotorParaBlockAdcSampleValue;          //��ת���ĵ�ǰ����ֵ

    unsigned char ucMotorParaBlockSubtact;                  //��ת�Ĳ���ֵ
    unsigned char ucMotorParaNoBlockTime;                   //��ת�����ʼʱ��
    
}ST_MOTOR_PARAMETER_INFO, *PST_MOTOR_PARAMETER_INFO;

//����״̬
typedef enum
{
	E_OPEN_NONE,   
	E_OPEN_START,  //��ʼ����
	E_OPEN_SUC,		//�������
	E_CLOSE_START,  //��ʼ����(��ת)
	E_CLOSE_SUC,	//�������
}E_DOOR_STATUS;

#define ADC_VOLTAGE_CONVERT_3000MV_10BIT                           750/256
#define ADC_VOLTAGE_CONVERT_3300MV_10BIT                           825/256
#define VOLTAGE_CONVERT_CURRENT                                    (2) 

#define VOL_CHANNEL   	4		//��ص�ѹͨ��
#define MOT_CHANNEL  	5		//�����תͨ��

#define MOTOR_INTERVAL_MS 		10			//��ʱ��10ms���(����Ҫ��10�ı�������Ȼʱ�䲻׼)
#define ADC_CONTINUOUS_SAMPLE_PERIOD        (200-1)//sampling once 20ms

#define MOTOR_BLOCK_CHECK   0
#define SAMPLE_AVG_MAX   8	    //8���ɼ���������ƽ��ֵ
#define LOW_POWER_DATA   4600	//4.6V
#define NOT_OPEN_POWER_DATA   4000	//3.4V �����ŵ�ѹ
#define EMPTY_POWER_DATA    4000  //3.5v

#define CHARGE_POWER_DATA   6200	//����ѹ����������4.3V
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
