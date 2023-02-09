#include "driver_wdg.h"

#include "rtl876x_tim.h"
#include "rtl876x_rcc.h"
#include "rtl876x_nvic.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define TIMER_NUM               TIM6
#define TIMER_IRQN              TIMER6_IRQn
#define WDG_Timer_Handler       Timer6_Handler
#define TIMER_PERIOD            (2*1000*1000*40-1)  //2s for 40M clock

/*============================================================================*
 *                              Local Functions
 *============================================================================*/
/******************************************************************
 * @brief  Initialize watch dog timer.
 * @param  none
 * @return none
 * @retval void
 */
void watch_dog_timer_driver_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);

    TIM_InitStruct.TIM_PWM_En = PWM_DISABLE;
    TIM_InitStruct.TIM_Period = TIMER_PERIOD ;
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_TimeBaseInit(TIMER_NUM, &TIM_InitStruct);

    /*  Enable TIMER IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIMER_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_ClearINT(TIMER_NUM);
    TIM_INTConfig(TIMER_NUM, ENABLE);
    TIM_Cmd(TIMER_NUM, ENABLE);
}

/*============================================================================*
 *                              Global Functions
 *============================================================================*/
/******************************************************************
 * @brief  enable watch dog timer.
 * @param  none
 * @return none
 * @retval void
 */
void reset_watch_dog_timer_enable(void)
{
    watch_dog_timer_driver_init();
}







































/**
  * @brief  Initialize WDG.
  * @param  No parameter.
  * @return void
  */
void driver_wdg_init(void)
{
	WDG_Disable();
    WDG_ClockEnable();
    /* WDG timing time = ((77+1)/32000)*( 2^(11+1) - 1) , about 10S
     * Reset mode following:
     *      INTERRUPT_CPU: interrupt CPU
     *      RESET_ALL_EXCEPT_AON: reset all except aon
     *      RESET_CORE_DOMAIN: reset core domain
     *      RESET_ALL: reset all
     */
    WDG_Config(78, 11, RESET_ALL);
    WDG_Enable();
}

/**
  * @brief  Feeding dog.
  * @param  No parameter.
  * @return void
  */
void wdg_feed(void)
{
    WDG_Restart();
}



