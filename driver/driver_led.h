#ifndef DRIVER_LED_H
#define DRIVER_LED_H

#include <rtl876x.h>

#include "menu_manage.h"

#define LED_CLOSE    (1)  //关闭led的功能，有些项目不需要

#define RED_PWM_TIMER_NUM               TIM5
#define RED_PWM_OUT_PIN_PINMUX          timer_pwm5
#define RED_NOR_OUT_PIN_PINMUX          DWGPIO

#define BLUE_PWM_TIMER_NUM              TIM6
#define BLUE_PWM_OUT_PIN_PINMUX         timer_pwm6
#define BLUE_NOR_OUT_PIN_PINMUX         DWGPIO

#define BEEP_PWM_TIMER_NUM             	TIM3
#define BEEP_PWM_OUT_PIN_PINMUX        	timer_pwm3
#define BEEP_NOR_OUT_PIN_PINMUX        	DWGPIO


#define GREEN_PWM_TIMER_NUM             TIM7
#define GREEN_PWM_OUT_PIN_PINMUX        timer_pwm7
#define GREEN_NOR_OUT_PIN_PINMUX        DWGPIO

#define LED_CTR_TIMER_NUM               TIM4
#define LED_CTR_TIMER_IRQN              TIMER4_IRQn


#define PWM_TIMER_PERIOD                (10000)         //uint:us    (1s: 1000000; 1ms:1000; 10ms:10000)
#define PWM_FREQ_HZ                     (20)            //20M
#define PWM_DUTY_COUNT_GET(x)           ((x==0) ? (0): ((((PWM_TIMER_PERIOD/100)*((x*(PWM_FREQ_HZ))))-1)))  //x:0-100
#define LED_CTR_TIMER_PERIOD            ((PWM_TIMER_PERIOD)*(PWM_FREQ_HZ/2))        //5ms

/*****************led drivre start****************/
#define LED_COLOUR_NUM              3
#define COLOUR(x,y)                 (x&(1<<y))
#define LED_PULSE_STEP              (2)
#define LED_PULSE_OFF               (100)
#define LED_PULSE_ON                (0)


typedef enum 
{
    EM_LED_CTRL_OFF = 0,                //关闭LED灯
    EM_LED_CTRL_ON = 1,                 //开启LED灯
    EM_LED_CTRL_BY_TOUCH = 2,           //当手指触碰时自动点亮LED灯(预留)
    EM_LED_CTRL_PWM = 3,                //PWM控制LED灯(呼吸灯)
    EM_LED_CTRL_BLINK = 4,              //闪烁LED灯
    EM_LED_CTRL_PWM_SY = 5,              //晟元协议 PWN控制LED灯
} EM_LED_CTRL;

typedef enum
{
    EM_LED_NONE 	= 0,			//无颜色控制
    EM_LED_BLUE    	= 1,			//蓝色
    EM_LED_GREEN 	= 2,			//绿色
    EM_LED_CYAN    	= 3,			//青色
    EM_LED_RED 		= 4,			//红色
    EM_LED_PINK    	= 5,			//粉色
    EM_LED_YELLOW 	= 6,			//黄色
	EM_LED_WHITE    = 7				//白色
} EM_LED_COLOR;

//自动控灯
typedef enum
{ 
	AUTO_LED_MATCH_OK,         
	AUTO_LED_STORAGE_OK,          
	AUTO_LED_CAPT_OK,                  
	AUTO_LED_FAIL,                              
	AUTO_LED_DIE,                            
	AUTO_LED_BACKLIGHRT, 
	AUTO_LED_CLOSE, 
}EM_LED_AUTO_CTRL;

typedef struct 
{
    EM_LED_CTRL ucLedCtrlMode;
    EM_LED_COLOR ucLedCtrColor;

    uint16_t ucLedCtrCycle;
    uint16_t ucLedCtrPulse;
    
    uint16_t ucLedCtrCount;
    uint8_t ucLedCtrDecFlag;
	bool bLedAlaway;		//循环无限次标志
    
    uint8_t ucLedPwmMaxDuty;
    uint8_t ucLedPwmMinDuty;
} ST_LED_CTRL_INFO, *PST_LED_CTRL_INFO;


typedef enum 
{
    MODE_MATCH_SUCCESS = 0,
    MODE_MATCH_FAIL,
    MODE_PRESS_SUCCESS,
    MODE_PRESS_FAIL,
    MODE_INIT_SUCCESS,
    MODE_ENROLL_SUCCESS,
    MODE_INIT_FAIL,
    MODE_ENROLL_FAIL,
    MODE_ALARM,			//低电量警告
    MODE_MOTOR_BACK ,	//电机回转
    MODE_ENROLL_STATUS,
    MODE_INIT_STATUS,
    MODE_LED_ON,
    MODE_BEEP_ON,
    MODE_LED_OFF,
    MODE_BEEP_OFF ,
    MODE_CHARGE,
    MODE_ADMIN_EMIT,
    MODE_SET_STATUS,
    MODE_ENROLL_PRESS,
    MODE_VERIFY_PRESS,
    MODE_DELETE_STATUS,
    MODE_DELETE_START,
    MODE_DELETE_SUCCESS,
    MODE_DELETE_FAIL,
    MODE_STORE_FULL,
    MODE_PWM,
    MODE_RESET,
    MODE_POWER_ON,  
} EM_LED_BEEP_CTRL_MODE;

#define LED_DELAY_TIME              (1000)          //10s
#define LED_TIMER_FREQ              (10)           //10ms
#define LED_FREQ_MS(x)              ((x)/(10))


void gpio_led_enter_dlps_config(void);
void gpio_led_exit_dlps_config(void);

void driver_led_init(void);
void ledModeWait(uint16_t delay);
void lockLedControl(uint8_t mode, uint32_t wait_time);
void background_msg_led_handle(T_MENU_MSG *p_background_msg);

void ledPwmModeSet(uint8_t max_duty, uint8_t min_duty, uint8_t freq);
void ledModeSet(EM_LED_CTRL mode, EM_LED_COLOR colour, uint32_t frequency, uint16_t count);
int ledAutoCtrl(EM_LED_AUTO_CTRL type);
void beepOpenControl(bool flag);
void beepWorkControl(uint16_t times, uint8_t count);
uint32_t background_msg_beep_handle(T_MENU_MSG *p_msg);
bool beepWorkStatusGet(void);
void background_msg_set_led(unsigned char subtype);
void background_msg_set_led(unsigned char subtype);
void background_msg_set_beep(uint16_t u16Times, uint16_t u16Count);
bool ledWorkStatusGet(void);

#endif
