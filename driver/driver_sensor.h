#ifndef DRIVER_SENSOR_H
#define DRIVER_SENSOR_H

#include <rtl876x.h>
#include <app_task.h>

typedef enum
{
    SENSOR_COMP_CODE_OK             = 0,
    SENSOR_COMP_CODE_QUIT           = 1,
    SENSOR_COMP_CODE_MALLOC         = 2,
    SENSOR_COMP_CODE_CAPTURE        = 3,
    SENSOR_COMP_CODE_SIZE           = 4,
    SENSOR_COMP_CODE_UNQUALIFIED    = 5,
    SENSOR_COMP_CODE_HARDWARE       = 6,
    SENSOR_COMP_CODE_DEFAULT        = 7,
    SENSOR_COMP_CODE_NO_FINGER      = 8,
    SENSOR_COMP_CODE_PRESS_FINGER   = 9,
    SENSOR_COMP_CODE_SLEEP_ERROR    = 10,
    SENSOR_COMP_CODE_ID_ERROR       = 11,
    SENSOR_COMP_CODE_RELEASE_ERROR  = 12,
    SENSOR_COMP_CODE_CALIBRATION_ERROR = 13, 
    SENSOR_COMP_CODE_IRQ_ERROR      = 14,
}sensor_comp_code;

typedef enum
{
    SENSOR_FINGER_NO_PRESENT        = 0,
    SENSOR_FINGER_PRESENT           = 1,
}sensor_finger_status;

#define SENSOR_IMG_BUFFER_SIZE          			    (0x3000)//(80*64+32)      //80*64 = 11264
extern  uint8_t g_ImgBuf[SENSOR_IMG_BUFFER_SIZE];  
#define SENSOR_IMAGE_BUFFER                             (g_ImgBuf+8)

void gpio_sensor_pinmux_config(void);
void gpio_sensor_pad_config(void);
void gpio_sensor_enter_dlps_config(void);
void gpio_sensor_exit_dlps_config(void);
void driver_sensor_gpio_init(void);

void Sensor_reset_control(bool state);

sensor_comp_code Sensor_Init(void);
sensor_finger_status Sensor_FingerCheck(void);
sensor_comp_code Sensor_WaitAndCapture(unsigned int wait_time);
sensor_comp_code Sensor_Sleep(void);
    
void Sensor_GetSpiImgInfo(uint8_t ** pImg, uint32_t *pLen);
void Sensor_GetAdcGain(uint8_t *pShift, uint8_t *pGain, uint8_t *pPxlCtrl);
void Sensor_set_quit(void);

#endif
