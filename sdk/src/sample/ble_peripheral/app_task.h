/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      app_task.h
   * @brief     Routines to create App task and handle events & messages
   * @author    jane
   * @date      2017-06-02
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */
#ifndef _APP_TASK_H_
#define _APP_TASK_H_
#include "app_msg.h"
#include "tuya_ble_type.h"


/** @defgroup PERIPH_APP_TASK Peripheral App Task
  * @brief Peripheral App Task
  * @{
  */


/**
 * @brief  Initialize App task
 * @return void
 */
void app_task_init(void);
bool app_send_msg_to_apptask(T_IO_MSG *p_msg);
bool app_send_msg_to_locktask(T_IO_MSG *p_msg);
bool tuya_event_queue_send(tuya_ble_evt_param_t *evt, uint32_t wait_ms);
void app_wdg_reset(void);

/** End of PERIPH_APP_TASK
* @}
*/


#endif

