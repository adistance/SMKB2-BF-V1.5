#ifndef DRIVER_BUTTON_H
#define DRIVER_BUTTON_H

#include <rtl876x.h>
#include <app_task.h>

typedef enum
{
    KEYBOARD_PAIR_PRESS,
    KEYBOARD_PAIR_RELEASE
} KEYBOARD_BUTTON_MSGTYPE;

void gpio_button_pinmux_config(void);
void gpio_button_pad_config(void);
void gpio_button_enter_dlps_config(void);
void gpio_button_exit_dlps_config(void);

void driver_button_init(void);
bool button_get_press_status(void);
void background_msg_set_button(unsigned char subtype);

#endif
