#ifndef DRIVER_HAL_H
#define DRIVER_HAL_H


#include <rtl876x.h>
#include <app_task.h>

bool get_b_HAL1_work_status(void);
void set_b_HAL1_work_status(bool data);
void gpio_hal_pinmux_config(void);
void gpio_hal_pad_config(void);
void gpio_hal_enter_dlps_config(void);
void gpio_hal_exit_dlps_config(void);
void driver_hal_init(void);
bool hal_get_motor_status(void);
bool hal_get_door_status(void);
void Set_hal_door_status(bool data);
bool hal_set_motor_status(unsigned int data);
void background_msg_set_hal(unsigned char subtype);


extern bool bFirst_WakeUp;
extern bool b_HAL1_work;



#endif
