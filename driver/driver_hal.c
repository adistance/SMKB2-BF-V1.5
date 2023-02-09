#include "driver_hal.h"
#include "driver_led.h"
#include "driver_motor.h"
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_nvic.h"
#include <board.h>
#include <trace.h>
#include "menu_manage.h"


bool b_HAL_Check_Motor_Status = true;         //trueÎª»ô¶û¼ì²âµ½´ÅÌú£¬falseÎ´¼ì²âµ½
bool b_HAL_Check_Door_Status = true; 	      //true£º±ÕËø×´Ì¬			false£º·Ç±ÕËø×´Ì¬
volatile static bool b_HAL1_work;

bool get_b_HAL1_work_status(void)
{
	return b_HAL1_work;
}

void set_b_HAL1_work_status(bool data)
{
	b_HAL1_work = data;
}
void gpio_hal_pinmux_config(void)
{
	Pinmux_Config(PAIR_HAL_POWER, DWGPIO);
    Pinmux_Config(PAIR_HAL1, DWGPIO);
	Pinmux_Config(PAIR_HAL2, DWGPIO);
}

void gpio_hal_pad_config(void)
{
	Pad_Config(PAIR_HAL_POWER, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	Pad_Config(PAIR_HAL2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
}


void gpio_hal_enter_dlps_config(void)
{
	Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
	
  if(GPIO_ReadInputDataBit(GPIO_GetPin(PAIR_HAL2))) 
	{
		System_WakeUpPinEnable(PAIR_HAL2, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_DISABLE);
	}
	else
	{
		Pad_Config(PAIR_HAL2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
		System_WakeUpPinDisable(PAIR_HAL2);
	}
  
	if(door_open_status() == E_OPEN_NONE)
	{
    Pad_Config(PAIR_HAL_POWER, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
  }	
}

void gpio_hal_exit_dlps_config(void)
{
    Pad_Config(PAIR_HAL_POWER, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
	Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	Pad_Config(PAIR_HAL2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
//	if(door_open_status() == E_OPEN_NONE)
//	{
//		set_b_HAL1_work_status = false;
//	}
}

void driver_hal_init(void)
{ 
	Pad_Config(PAIR_HAL_POWER, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
	Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	Pad_Config(PAIR_HAL2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	
	Pinmux_Config(PAIR_HAL1, DWGPIO);
	Pinmux_Config(PAIR_HAL2, DWGPIO);
	Pinmux_Config(PAIR_HAL_POWER, DWGPIO);
	
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
 
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin        = GPIO_GetPin(PAIR_HAL1);
    GPIO_InitStruct.GPIO_Mode       = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd      = ENABLE;
    GPIO_InitStruct.GPIO_ITTrigger  = GPIO_INT_Trigger_EDGE;
    GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;
    GPIO_InitStruct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
    GPIO_InitStruct.GPIO_DebounceTime = 10;
    GPIO_Init(&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin        = GPIO_GetPin(PAIR_HAL2);
    GPIO_InitStruct.GPIO_Mode       = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd      = ENABLE;
    GPIO_InitStruct.GPIO_ITTrigger  = GPIO_INT_Trigger_EDGE;
    GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;
    GPIO_InitStruct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
    GPIO_InitStruct.GPIO_DebounceTime = 10;
    GPIO_Init(&GPIO_InitStruct);



	GPIO_InitStruct.GPIO_Pin		= GPIO_GetPin(PAIR_HAL_POWER);
	GPIO_InitStruct.GPIO_Mode		= GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_ITCmd		= DISABLE;
	//GPIO_InitStruct.GPIO_ITTrigger	= GPIO_INT_Trigger_EDGE;
	//GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;
	//GPIO_InitStruct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
	//GPIO_InitStruct.GPIO_DebounceTime = 10;
	GPIO_Init(&GPIO_InitStruct);


    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = PAIR_HAL1_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = PAIR_HAL2_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    
    /* Enable interrupt */
    GPIO_MaskINTConfig(GPIO_GetPin(PAIR_HAL1), DISABLE);
    GPIO_INTConfig(GPIO_GetPin(PAIR_HAL1), ENABLE);

	GPIO_MaskINTConfig(GPIO_GetPin(PAIR_HAL2), DISABLE);
    GPIO_INTConfig(GPIO_GetPin(PAIR_HAL2), ENABLE);

}

bool hal_get_motor_status(void)
{
	return b_HAL_Check_Motor_Status;
}

bool hal_get_door_status(void)
{
	return b_HAL_Check_Door_Status;
}

void Set_hal_door_status(bool data)
{
	b_HAL_Check_Door_Status = data;
}

bool hal_set_motor_status(unsigned int data)
{
	if(data)
	{
		b_HAL_Check_Motor_Status = true;
	}
	else
	{
		b_HAL_Check_Motor_Status = false;
	}
  return b_HAL_Check_Motor_Status;
}

void GPIO8_Handler(void)
{
    GPIO_INTConfig(GPIO_GetPin(PAIR_HAL1), DISABLE);
    GPIO_MaskINTConfig(GPIO_GetPin(PAIR_HAL1), ENABLE);
	//menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
		if (GPIO_ReadInputDataBit(GPIO_GetPin(PAIR_HAL1)))        //release
    {
    APP_PRINT_INFO0("release");
      b_HAL_Check_Motor_Status = false;
        GPIO->INTPOLARITY &= ~GPIO_GetPin(PAIR_HAL1);  //Polarity Low
    }
    else														//press
    {
    //APP_PRINT_INFO0("press");
	  if(b_HAL1_work == true)
	  	{
	  		background_msg_set_motor(BACKGROUND_MSG_MOTOR_SUBTYPE_OFF); 	//É²³µ
			b_HAL_Check_Motor_Status = true;
			Pad_Config(PAIR_HAL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
	  	}
	  else
	  	{
			b_HAL1_work = true;
	  	}
        GPIO->INTPOLARITY |= GPIO_GetPin(PAIR_HAL1);   //Polarity High
    }

    GPIO_ClearINTPendingBit(GPIO_GetPin(PAIR_HAL1));
    GPIO_MaskINTConfig(GPIO_GetPin(PAIR_HAL1), DISABLE);
    GPIO_INTConfig(GPIO_GetPin(PAIR_HAL1), ENABLE);
}



void GPIO23_Handler(void)
{
    GPIO_INTConfig(GPIO_GetPin(PAIR_HAL2), DISABLE);
    GPIO_MaskINTConfig(GPIO_GetPin(PAIR_HAL2), ENABLE);
	//menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
		if (GPIO_ReadInputDataBit(GPIO_GetPin(PAIR_HAL2)))        //release
    {
      APP_PRINT_INFO0("release");
      Set_hal_door_status(false);
        GPIO->INTPOLARITY &= ~GPIO_GetPin(PAIR_HAL2);  //Polarity Low
    }
    else														//press
    {
      APP_PRINT_INFO0("press");
      Set_hal_door_status(true);			
	  menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
        GPIO->INTPOLARITY |= GPIO_GetPin(PAIR_HAL2);   //Polarity High
    }

    GPIO_ClearINTPendingBit(GPIO_GetPin(PAIR_HAL2));
    GPIO_MaskINTConfig(GPIO_GetPin(PAIR_HAL2), DISABLE);
    GPIO_INTConfig(GPIO_GetPin(PAIR_HAL2), ENABLE);
}


//ÉèÖÃÖ¸ÎÆ²Ù×÷(subtype T_MENU_MSG_HAL_SUBTYPE)£¬ÅÐ¶Ïµç»úÍ£×ª
void background_msg_set_hal(unsigned char subtype)
{
	T_MENU_MSG menMsg = {0};
	menMsg.type = MENU_MSG_TYPE_HAL;
	menMsg.subtype = subtype;
	menu_task_msg_send(&menMsg);
}

//uint32_t hal_handle_msg(T_MENU_MSG *hal_msg)
//{
//	if(hal_msg == NULL)
//	{
//		APP_PRINT_INFO0("button_handle_msg input param is NULL");
//		return 0;
//	}
//	uint16_t deleteId = 0;
//	
//	switch(hal_msg->subtype)
//	{
//		case MENU_MSG_HAL_SUBTYPE_RESET:					
//			if(bKeyOn)
//			background_msg_set_led(BACKGROUND_MSG_LED_SUBTYPE_RESET_INIT);
//			break;
//			
//		default:
//			break;
//	}

//	return 0;
//}














