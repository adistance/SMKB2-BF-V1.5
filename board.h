/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      board.h
* @brief     header file of Keypad demo.
* @details
* @author    tifnan_ge
* @date      2015-06-26
* @version   v0.1
* *********************************************************************************************************
*/


#ifndef _BOARD_H_
#define _BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rtl876x_pinmux.h"

#define DLPS_FLAG				1    //低功耗标志
#define DEBUG_FLAG				0   //调试标志，打后能看到打印，但是sensor不能用，因为共用了引脚
#define DLPS_BLE				0   //低功耗蓝牙广播
#define BLE_DMZ					0    //使用大拇指蓝牙小程序，默认使用顶固小程序
#define BLE_ML					1	//使用中性蓝牙小程序，默认使用顶固小程序

#define BH308					1  //SY11

/***************OS Timer ID******************/
#define LOCK_CTR_OS_TIMER_ID    2
#define AON_WDG_OS_TIMER_ID     3
#define MOTOR_OS_TIMER_ID 		4 

#define BEEP_OS_TIMER_ID		6

/***************Button Config****************/
#define PAIR_HAL1         P1_0         //PAIR_BUTTON
#define PAIR_HAL1_IRQ     GPIO8_IRQn   //PAIR_BUTTON_IRQ


#define PAIR_HAL2         P2_7         
#define PAIR_HAL2_IRQ     GPIO23_IRQn   

#define PAIR_HAL_POWER   H_0

#define PAIR_BUTTON         P1_0         
#define PAIR_BUTTON_IRQ     GPIO8_IRQn
/***************LED Config*******************/
#define LED_RED_PIN         H_1
#define LED_BLUE_PIN        P3_1
#define LED_GREEN_PIN       P3_0

/**************Flash and Sensor SPI Config***/
#define SPI0_PORT           SPI0

//#define SPI_F_CS_PIN        P2_7
#define SPI_S_CS_PIN        P4_0

#define SPI_MOSI_PIN        P4_1
#define SPI_MISO_PIN        P4_2
#define SPI_CLK_PIN         P4_3

#if DEBUG_FLAG
#define SENSOR_RST_PIN      P0_1 //P0_1 //P0_3
#else
#define SENSOR_RST_PIN      P0_3 //P0_1 //P0_3
#endif
#define SENSOR_INT_PIN      P0_2
#define SENSOR_INT_IRQ      GPIO2_IRQn

/***************UART Config*******************/
#define UART_TX_PIN         P3_0
#define UART_RX_PIN         P3_1

/***************MOTOR Config*******************/
#define MOTOR_LEFT         	P1_1
#define MOTOR_RIGHT         P0_1
#define BAT_PIN 			P2_4	 //电池检测	
#define MOTOR_PIN 			P2_5	//堵转引脚
#define BAT_EN_HAL1_POW			P2_6		//电池检测需要先把这个引脚拉高
//电量满的标志引脚，充电时低电平，充满电高电平，没充电一直是高电平
#define BAT_CHG_PIN			H_2		//低功耗时设置成输入上拉
#define BAT_CHARGE          P2_6	//充电判断引脚
/***************ADC Config*******************/
#define ADC_SAMPLE_PIN 		P1_0
#define ADC_SAMPLE_PIN2 	P2_6

/***************BEEP Config*******************/
#define BEEP_PIN       H_2

/* if use user define dlps enter/dlps exit callback function */
#define USE_USER_DEFINE_DLPS_EXIT_CB      1
#define USE_USER_DEFINE_DLPS_ENTER_CB     1

/* if use any peripherals below, #define it 1 */
#define USE_I2C0_DLPS        0
#define USE_I2C1_DLPS        0

#if (ROM_WATCH_DOG_ENABLE == 1)
#define USE_TIM_DLPS         1
#else
#define USE_TIM_DLPS         1
#endif

#define USE_QDECODER_DLPS    0
#define USE_IR_DLPS          0
#define USE_RTC_DLPS         0
#define USE_UART_DLPS        1
#define USE_ADC_DLPS         0
#define USE_SPI0_DLPS        1
#define USE_SPI1_DLPS        0
#define USE_SPI2W_DLPS       0
#define USE_KEYSCAN_DLPS     0
#define USE_DMIC_DLPS        0
#define USE_GPIO_DLPS        1
#define USE_PWM0_DLPS        0
#define USE_PWM1_DLPS        0
#define USE_PWM2_DLPS        0
#define USE_PWM3_DLPS        0


/* do not modify USE_IO_DRIVER_DLPS macro */
#define USE_IO_DRIVER_DLPS   (USE_I2C0_DLPS | USE_I2C1_DLPS | USE_TIM_DLPS | USE_QDECODER_DLPS\
                              | USE_IR_DLPS | USE_RTC_DLPS | USE_UART_DLPS | USE_SPI0_DLPS\
                              | USE_SPI1_DLPS | USE_SPI2W_DLPS | USE_KEYSCAN_DLPS | USE_DMIC_DLPS\
                              | USE_GPIO_DLPS | USE_USER_DEFINE_DLPS_EXIT_CB\
                              | USE_RTC_DLPS | USE_PWM0_DLPS | USE_PWM1_DLPS | USE_PWM2_DLPS\
                              | USE_PWM3_DLPS | USE_USER_DEFINE_DLPS_ENTER_CB)
#define DLPS_EN               1

/*============================================================================*
 *                              OTA Config
 *============================================================================*/
#define SUPPORT_NORMAL_OTA                          1
#define ENABLE_AUTO_BANK_SWITCH                     0 //for qc test
#define ENABLE_BANK_SWITCH_COPY_APP_DATA            1
#define ENABLE_SLAVE_REQUEST_UPDATE_COON_PARA       0

extern uint8_t g_ota_mode;
#define DFU_TEMP_BUFFER_SIZE                        2048
#define DFU_BUFFER_CHECK_ENABLE                     0x1//(g_ota_mode & 0x1)


#ifdef __cplusplus
}
#endif

#endif  /* _BOARD_H_ */

