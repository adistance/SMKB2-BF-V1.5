/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      peripheral_app.h
   * @brief     This file handles BLE peripheral application routines.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

#ifndef _PERIPHERAL_APP__
#define _PERIPHERAL_APP__

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <app_msg.h>
#include <gap_le.h>
#include <profile_server.h>


/** @defgroup PERIPH_APP Peripheral Application
  * @brief Peripheral Application
  * @{
  */


/*============================================================================*
 *                              Variables
 *============================================================================*/
extern T_SERVER_ID simp_srv_id; /**< Simple ble service id*/
extern T_SERVER_ID bas_srv_id;  /**< Battery service id */
extern T_SERVER_ID g_ota_service_id;
extern T_SERVER_ID tuya_srv_id;

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg);

/**
 * @brief    All the BT Profile service callback events are handled in this function
 * @note     Then the event handling function shall be called according to the
 *           service_id.
 * @param[in] service_id  Profile service ID
 * @param[in] p_data      Pointer to callback data
 * @return   Indicates the function call is successful or not
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT app_profile_callback(T_SERVER_ID service_id, void *p_data);

/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data);


unsigned char app_gap_adv_state_get(void);
uint8_t app_bt_get_con_tag(void);  
void app_bt_set_con_tag(void);
uint8_t get_bt_real_con_state(void);
uint32_t app_get_bt_real_state(void);
uint32_t app_ble_disconnect(void);
bool ble_value_notify(uint8_t *data,uint8_t len);


/** End of PERIPH_APP
* @}
*/


#ifdef __cplusplus
}
#endif

#endif

