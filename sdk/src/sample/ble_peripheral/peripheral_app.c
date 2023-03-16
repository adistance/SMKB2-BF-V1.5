/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      peripheral_app.c
   * @brief     This file handles BLE peripheral application routines.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <trace.h>
#include <string.h>
#include <gap.h>
#include <gap_adv.h>
#include <gap_bond_le.h>
#include <profile_server.h>
#include <gap_msg.h>
#include <simple_ble_service.h>
#include <bas.h>
#include <app_msg.h>
#include <peripheral_app.h>
#include <gap_conn_le.h>
#if F_BT_ANCS_CLIENT_SUPPORT
#include <ancs_client.h>
#include <ancs.h>
#endif

#include "ota_service.h"
#include "dfu_flash.h"

#include "driver_button.h"
#include "driver_spi.h"
#include "driver_uart.h"
#include "bluetooth_menu.h"
#include "app_crypto.h"
#include "driver_motor.h"
#include "tuya_ble_service_rtl8762d.h"
#include "tuya_ble_api.h"
#include "tuya_ble_main.h"

static bool switch_to_ota_mode_pending = false;
extern bool s_u8DlpsFlag;

/** @defgroup  PERIPH_APP Peripheral Application
    * @brief This file handles BLE peripheral application routines.
    * @{
    */
/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @addtogroup  PERIPH_SEVER_CALLBACK Profile Server Callback Event Handler
    * @brief Handle profile server callback event
    * @{
    */
T_SERVER_ID simp_srv_id; /**< Simple ble service id*/
T_SERVER_ID bas_srv_id;  /**< Battery service id */
T_SERVER_ID g_ota_service_id;

/** @} */ /* End of group PERIPH_SEVER_CALLBACK */
/** @defgroup  PERIPH_GAP_MSG GAP Message Handler
    * @brief Handle GAP Message
    * @{
    */
T_GAP_DEV_STATE gap_dev_state = {0, 0, 0, 0};                 /**< GAP device state */
T_GAP_CONN_STATE gap_conn_state = GAP_CONN_STATE_DISCONNECTED; /**< GAP connection state */
bool g_notify_v3_enable = false;
static uint8_t notify_v3_conn_id = 0xFF;

static uint8_t bt_con_tag = 0;  //这个标志是为了解决特殊情况，显示连接了，但是notify enable标志没有触发

static uint8_t m_conn_id = 0;
T_SERVER_ID tuya_srv_id;

uint32_t app_ble_disconnect(void)
{
    if(le_disconnect(m_conn_id)==GAP_CAUSE_SUCCESS)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

bool ble_value_notify(uint8_t *data,uint8_t len)
{
    if(len > PARA_VALUE_LEN_MAX)
    {
        APP_PRINT_ERROR0("ble notify value len exceed the limit");
        return false;
    }
    else
    {
        return server_send_data(m_conn_id, tuya_srv_id, KNS_KEY_VALUE_INDEX, data,len, GATT_PDU_TYPE_NOTIFICATION);
    }
}


/*============================================================================*
 *                              Functions
 *============================================================================*/
void app_handle_gap_msg(T_IO_MSG  *p_gap_msg);
/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg)
{
    uint16_t msg_type = io_msg.type;

    switch (msg_type)
    {
        case IO_MSG_TYPE_BT_STATUS:
            {
                app_handle_gap_msg(&io_msg);
            }
            break;
            
        case IO_MSG_TYPE_GPIO:
            {
                
            }
            break;
            
        case IO_MSG_TYPE_GDMA:
            {

            }
            break;
            
        case IO_MSG_TYPE_UART:
            {
                
            }
            break;
            
        case IO_MSG_TYPE_FINGERPRINT:           //本地处理信息返回到蓝牙端入口
            {
                
            }
            break;
            
    #if F_BT_ANCS_CLIENT_SUPPORT
        case IO_MSG_TYPE_ANCS:
            {
                ancs_handle_msg(&io_msg);
            }
            break;
    #endif
	case IO_MSG_TYPE_ENCRYPT_COMP:
			APP_PRINT_INFO0("cryptlib_check_result start!\n");
			//cryptlib_check_result(NULL);
			break;
        default:
            break;
    }
}

/**
 * @brief    Handle msg GAP_MSG_LE_DEV_STATE_CHANGE
 * @note     All the gap device state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] new_state  New gap device state
 * @param[in] cause GAP device state change cause
 * @return   void
 */
void app_handle_dev_state_evt(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    APP_PRINT_INFO3("app_handle_dev_state_evt: init state %d, adv state %d, cause 0x%x",
                    new_state.gap_init_state, new_state.gap_adv_state, cause);
    
    if (gap_dev_state.gap_init_state != new_state.gap_init_state)
    {
        if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY) //1
        {
            APP_PRINT_INFO0("GAP stack ready");
            /*stack ready*/
			le_adv_start();

        }
    }

    if (gap_dev_state.gap_adv_state != new_state.gap_adv_state)
    {
        if (new_state.gap_adv_state == GAP_ADV_STATE_IDLE)          //0 广播已关闭             
        {
            if (new_state.gap_adv_sub_state == GAP_ADV_TO_IDLE_CAUSE_CONN)      //连接时触发
            {
                APP_PRINT_INFO0("GAP adv stoped: because connection created");
            }
            else
            {
                APP_PRINT_INFO0("GAP adv stoped");
            }
        }
        else if (new_state.gap_adv_state == GAP_ADV_STATE_ADVERTISING)  //2 广播已开启
        {
            APP_PRINT_INFO0("GAP adv start");
        }
    }

    gap_dev_state = new_state;
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_STATE_CHANGE
 * @note     All the gap conn state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] conn_id Connection ID
 * @param[in] new_state  New gap connection state
 * @param[in] disc_cause Use this cause when new_state is GAP_CONN_STATE_DISCONNECTED
 * @return   void
 */
void app_handle_conn_state_evt(uint8_t conn_id, T_GAP_CONN_STATE new_state, uint16_t disc_cause)
{
    APP_PRINT_INFO4("app_handle_conn_state_evt: conn_id %d old_state %d new_state %d, disc_cause 0x%x",
                    conn_id, gap_conn_state, new_state, disc_cause);
    switch (new_state)
    {
        case GAP_CONN_STATE_DISCONNECTED:       //0
            {
                if ((disc_cause != (HCI_ERR | HCI_ERR_REMOTE_USER_TERMINATE))
                    && (disc_cause != (HCI_ERR | HCI_ERR_LOCAL_HOST_TERMINATE)))
                {
                    APP_PRINT_ERROR1("app_handle_conn_state_evt: connection lost cause 0x%x", disc_cause);
                }
                m_conn_id = 0;
                APP_PRINT_INFO1("disc_cause is %d", disc_cause);
                if (switch_to_ota_mode_pending)
                {
                    switch_to_ota_mode_pending = false;
                    dfu_switch_to_ota_mode();
                }
                else
                {
                    le_adv_start();
					tuya_ble_disconnected_handler();
           			//tuya_ota_init_disconnect();
                }
                //le_adv_stop();
            }
            break;
		case GAP_CONN_STATE_CONNECTING:
		   if(s_u8DlpsFlag)
		   {
		    Pad_Config(SENSOR_INT_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
		   }
		   break;

        case GAP_CONN_STATE_CONNECTED:          //2
            {
                uint16_t conn_interval;
                uint16_t conn_latency;
                uint16_t conn_supervision_timeout;
                uint8_t  remote_bd[6];
                T_GAP_REMOTE_ADDR_TYPE remote_bd_type;
				bt_con_tag = 1;
								
                le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
                le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_latency, conn_id);
                le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
                le_get_conn_addr(conn_id, remote_bd, &remote_bd_type);
//                APP_PRINT_INFO5("GAP_CONN_STATE_CONNECTED:remote_bd %s, remote_addr_type %d, conn_interval 0x%x, conn_latency 0x%x, conn_supervision_timeout 0x%x",
//                                TRACE_BDADDR(remote_bd), remote_bd_type,
//                                conn_interval, conn_latency, conn_supervision_timeout);

				m_conn_id = conn_id;
				tuya_ble_connected_handler();
#if 1//F_BT_LE_5_0_SET_PHY_SUPPORT            
                uint8_t tx_phy; 
                uint8_t rx_phy; 
                le_get_conn_param(GAP_PARAM_CONN_RX_PHY_TYPE, &rx_phy, conn_id); 
                le_get_conn_param(GAP_PARAM_CONN_TX_PHY_TYPE, &tx_phy, conn_id); 
//                APP_PRINT_INFO2("GAP_CONN_STATE_CONNECTED: tx_phy %d, rx_phy %d",tx_phy,rx_phy);
#endif  
//				driver_adc_start();	
            }
            break;

        default:
            break;
    }
    gap_conn_state = new_state;
}


/**
 * @brief    Handle msg GAP_MSG_LE_AUTHEN_STATE_CHANGE
 * @note     All the gap authentication state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] conn_id Connection ID
 * @param[in] new_state  New authentication state
 * @param[in] cause Use this cause when new_state is GAP_AUTHEN_STATE_COMPLETE
 * @return   void
 */
void app_handle_authen_state_evt(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
    APP_PRINT_INFO2("app_handle_authen_state_evt:conn_id %d, cause 0x%x", conn_id, cause);

    switch (new_state)
    {
    case GAP_AUTHEN_STATE_STARTED:
        {
            APP_PRINT_INFO0("app_handle_authen_state_evt: GAP_AUTHEN_STATE_STARTED");
        }
        break;

    case GAP_AUTHEN_STATE_COMPLETE:
        {
            if (cause == GAP_SUCCESS)
            {
#if F_BT_ANCS_CLIENT_SUPPORT
                ancs_start_discovery(conn_id);
#endif
                APP_PRINT_INFO0("app_handle_authen_state_evt: GAP_AUTHEN_STATE_COMPLETE pair success");

            }
            else
            {
                APP_PRINT_INFO0("app_handle_authen_state_evt: GAP_AUTHEN_STATE_COMPLETE pair failed");
            }
        }
        break;

    default:
        {
            APP_PRINT_ERROR1("app_handle_authen_state_evt: unknown newstate %d", new_state);
        }
        break;
    }
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_MTU_INFO
 * @note     This msg is used to inform APP that exchange mtu procedure is completed.
 * @param[in] conn_id Connection ID
 * @param[in] mtu_size  New mtu size
 * @return   void
 */
void app_handle_conn_mtu_info_evt(uint8_t conn_id, uint16_t mtu_size)
{
    APP_PRINT_INFO2("app_handle_conn_mtu_info_evt: conn_id %d, mtu_size %d", conn_id, mtu_size);
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_PARAM_UPDATE
 * @note     All the connection parameter update change  events are pre-handled in this function.
 * @param[in] conn_id Connection ID
 * @param[in] status  New update state
 * @param[in] cause Use this cause when status is GAP_CONN_PARAM_UPDATE_STATUS_FAIL
 * @return   void
 */
void app_handle_conn_param_update_evt(uint8_t conn_id, uint8_t status, uint16_t cause)
{
    switch (status)
    {
    case GAP_CONN_PARAM_UPDATE_STATUS_SUCCESS:
        {
            uint16_t conn_interval;
            uint16_t conn_slave_latency;
            uint16_t conn_supervision_timeout;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_slave_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            APP_PRINT_INFO3("app_handle_conn_param_update_evt update success:conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x",
                            conn_interval, conn_slave_latency, conn_supervision_timeout);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_FAIL:
        {
            APP_PRINT_ERROR1("app_handle_conn_param_update_evt update failed: cause 0x%x", cause);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_PENDING:
        {
            APP_PRINT_INFO0("app_handle_conn_param_update_evt update pending.");
        }
        break;

    default:
        break;
    }
}

/**
 * @brief    All the BT GAP MSG are pre-handled in this function.
 * @note     Then the event handling function shall be called according to the
 *           subtype of T_IO_MSG
 * @param[in] p_gap_msg Pointer to GAP msg
 * @return   void
 */
void app_handle_gap_msg(T_IO_MSG *p_gap_msg)
{
    T_LE_GAP_MSG gap_msg;
    uint8_t conn_id;
    memcpy(&gap_msg, &p_gap_msg->u.param, sizeof(p_gap_msg->u.param));

//    APP_PRINT_TRACE1("app_handle_gap_msg: subtype %d", p_gap_msg->subtype);
    switch (p_gap_msg->subtype)
    {
    case GAP_MSG_LE_DEV_STATE_CHANGE:       //1 广播状态更新
        {
            app_handle_dev_state_evt(gap_msg.msg_data.gap_dev_state_change.new_state,
                                     gap_msg.msg_data.gap_dev_state_change.cause);
        }
        break;

    case GAP_MSG_LE_CONN_STATE_CHANGE:      //2 连接状态更新
        {
            app_handle_conn_state_evt(gap_msg.msg_data.gap_conn_state_change.conn_id,
                                      (T_GAP_CONN_STATE)gap_msg.msg_data.gap_conn_state_change.new_state,
                                      gap_msg.msg_data.gap_conn_state_change.disc_cause);
        }
        break;

    case GAP_MSG_LE_CONN_MTU_INFO:          //4 该消息用于通知exchange MTU (Maximum Transmission Unit) procedure已完成
        {                                   //目的是更新client和server之间交互数据包的最大长度，即更新ATT_MTU。
            app_handle_conn_mtu_info_evt(gap_msg.msg_data.gap_conn_mtu_info.conn_id,
                                         gap_msg.msg_data.gap_conn_mtu_info.mtu_size);
        }
        break;

    case GAP_MSG_LE_CONN_PARAM_UPDATE:  //3 该消息用于通知connection参数更新状态，
        {
            app_handle_conn_param_update_evt(gap_msg.msg_data.gap_conn_param_update.conn_id,
                                             gap_msg.msg_data.gap_conn_param_update.status,
                                             gap_msg.msg_data.gap_conn_param_update.cause);
        }
        break;

    case GAP_MSG_LE_AUTHEN_STATE_CHANGE:
        {
            app_handle_authen_state_evt(gap_msg.msg_data.gap_authen_state.conn_id,
                                        gap_msg.msg_data.gap_authen_state.new_state,
                                        gap_msg.msg_data.gap_authen_state.status);
        }
        break;

    case GAP_MSG_LE_BOND_JUST_WORK:
        {
            conn_id = gap_msg.msg_data.gap_bond_just_work_conf.conn_id;
            le_bond_just_work_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
            APP_PRINT_INFO0("GAP_MSG_LE_BOND_JUST_WORK");
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_DISPLAY:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_passkey_display.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_PASSKEY_DISPLAY:passkey %d", display_value);
            le_bond_passkey_display_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_USER_CONFIRMATION:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_user_conf.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_USER_CONFIRMATION: passkey %d", display_value);
            le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_INPUT:
        {
            uint32_t passkey = 888888;
            conn_id = gap_msg.msg_data.gap_bond_passkey_input.conn_id;
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d", conn_id);
            le_bond_passkey_input_confirm(conn_id, passkey, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_OOB_INPUT:
        {
            uint8_t oob_data[GAP_OOB_LEN] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            conn_id = gap_msg.msg_data.gap_bond_oob_input.conn_id;
            APP_PRINT_INFO0("GAP_MSG_LE_BOND_OOB_INPUT");
            le_bond_set_param(GAP_PARAM_BOND_OOB_DATA, GAP_OOB_LEN, oob_data);
            le_bond_oob_input_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    default:
        APP_PRINT_ERROR1("app_handle_gap_msg: unknown subtype %d", p_gap_msg->subtype);
        break;
    }
}
/** @} */ /* End of group PERIPH_GAP_MSG */

/** @defgroup  PERIPH_GAP_CALLBACK GAP Callback Event Handler
    * @brief Handle GAP callback event
    * @{
    */
/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data) //当上行信息发送到GAP层后，执行的回调
{
    T_APP_RESULT result = APP_RESULT_SUCCESS;
    T_LE_CB_DATA *p_data = (T_LE_CB_DATA *)p_cb_data;
    
//    APP_PRINT_TRACE1("app_gap_callback: cb_type 0x%x", cb_type);
    
    switch (cb_type)
    {
        case GAP_MSG_LE_DATA_LEN_CHANGE_INFO:   //0x14
//            APP_PRINT_INFO3("GAP_MSG_LE_DATA_LEN_CHANGE_INFO: conn_id %d, tx octets 0x%x, max_tx_time 0x%x",
//                            p_data->p_le_data_len_change_info->conn_id,
//                            p_data->p_le_data_len_change_info->max_tx_octets,
//                            p_data->p_le_data_len_change_info->max_tx_time);
            break;

        case GAP_MSG_LE_MODIFY_WHITE_LIST:  //0x01
            APP_PRINT_INFO2("GAP_MSG_LE_MODIFY_WHITE_LIST: operation %d, cause 0x%x",
                            p_data->p_le_modify_white_list_rsp->operation,
                            p_data->p_le_modify_white_list_rsp->cause);
            break;
        
        case GAP_MSG_LE_CREATE_CONN_IND:    //0x16
            result = APP_RESULT_ACCEPT;
//            result = APP_RESULT_REJECT;
            break;
        
        case GAP_MSG_LE_PHY_UPDATE_INFO:    //0x17  该消息表示Controller已经切换正在使用的发射机PHY或接收机PHY
#if 1//F_BT_LE_5_0_SET_PHY_SUPPORT 
//            APP_PRINT_INFO4("GAP_MSG_LE_PHY_UPDATE_INFO:conn_id %d, cause 0x%x, rx_phy %d, tx_phy %d", 
//                            p_data->p_le_phy_update_info->conn_id, 
//                            p_data->p_le_phy_update_info->cause, 
//                            p_data->p_le_phy_update_info->rx_phy, 
//                            p_data->p_le_phy_update_info->tx_phy); 
#endif
            break;
        
        case GAP_MSG_LE_REMOTE_FEATS_INFO:  //0x19 在connection建立成功后，controller会主动读取对端设备的feature。
#if 1//F_BT_LE_5_0_SET_PHY_SUPPORT          //APP可以检查对端设备是否支持LE2M PHY或LE Coded PHY
        { 
            uint8_t remote_feats[8];
//            APP_PRINT_INFO3("GAP_MSG_LE_REMOTE_FEATS_INFO: conn id %d, cause 0x%x, remote_feats %b", 
//                                            p_data->p_le_remote_feats_info->conn_id, 
//                                            p_data->p_le_remote_feats_info->cause, 
//                                            TRACE_BINARY(8,p_data->p_le_remote_feats_info->remote_feats)); 
            if(p_data->p_le_remote_feats_info->cause==GAP_SUCCESS) 
            { 
                memcpy(remote_feats,p_data->p_le_remote_feats_info->remote_feats,8); 
                if(remote_feats[LE_SUPPORT_FEATURES_MASK_ARRAY_INDEX1] &LE_SUPPORT_FEATURES_LE_2M_MASK_BIT) 
                { 
//                    APP_PRINT_INFO0("GAP_MSG_LE_REMOTE_FEATS_INFO: support 2M"); 
                }
                if(remote_feats[LE_SUPPORT_FEATURES_MASK_ARRAY_INDEX1] &LE_SUPPORT_FEATURES_LE_CODED_PHY_MASK_BIT) 
                { 
//                    APP_PRINT_INFO0("GAP_MSG_LE_REMOTE_FEATS_INFO: support CODED"); 
                }
            }
        }
#endif
        break;
        
        default:
            APP_PRINT_ERROR1("app_gap_callback: unhandled cb_type 0x%x", cb_type);
            break;
    }
    return result;
}
/** @} */ /* End of group PERIPH_GAP_CALLBACK */

/** @defgroup  PERIPH_SEVER_CALLBACK Profile Server Callback Event Handler
    * @brief Handle profile server callback event
    * @{
    */
/**
    * @brief    All the BT Profile service callback events are handled in this function
    * @note     Then the event handling function shall be called according to the
    *           service_id
    * @param    service_id  Profile service ID
    * @param    p_data      Pointer to callback data
    * @return   T_APP_RESULT, which indicates the function call is successful or not
    * @retval   APP_RESULT_SUCCESS  Function run successfully
    * @retval   others              Function run failed, and return number indicates the reason
    */

void app_notify_v3_send(uint8_t *sendbuf, uint8_t len)
{
    APP_PRINT_INFO1("app_notify_v3_send: %b", TRACE_BINARY(len, sendbuf));
    
    simp_ble_service_send_v3_notify(notify_v3_conn_id, simp_srv_id, sendbuf, len);
}
void crypto_init(sBLECmdHdr *stIn, cyp_ctrl_type_t ctrl)
{
	stIn->magic_num = 0xFE;
	stIn->flow_ctrl = ctrl;
	stIn->flow_cnt = 0x0100;
    stIn->flow_mic = 0xCDAB;
    stIn->flow_port = 0x00;
}

uint8_t app_bt_get_con_tag(void)
{
	return bt_con_tag;
}

void app_bt_set_con_tag(void)
{
	bt_con_tag = 0;
}

T_APP_RESULT app_profile_callback(T_SERVER_ID service_id, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    if (service_id == SERVICE_PROFILE_GENERAL_ID)       //profile server layer使用的service id
    {
        T_SERVER_APP_CB_DATA *p_param = (T_SERVER_APP_CB_DATA *)p_data;
        switch (p_param->eventId)   
        {
            case PROFILE_EVT_SRV_REG_COMPLETE:  // 在GAP启动流程中完成service注册流程。
                APP_PRINT_INFO1("PROFILE_EVT_SRV_REG_COMPLETE: result %d",
                                p_param->event_data.service_reg_result);
                break;

            case PROFILE_EVT_SEND_DATA_COMPLETE:    //profile server layer使用该消息向APP通知发送notification/indication的结果。
                APP_PRINT_INFO5("PROFILE_EVT_SEND_DATA_COMPLETE: conn_id %d, cause 0x%x, service_id %d, attrib_idx 0x%x, credits %d",
                                p_param->event_data.send_data_result.conn_id,
                                p_param->event_data.send_data_result.cause,
                                p_param->event_data.send_data_result.service_id,
                                p_param->event_data.send_data_result.attrib_idx,
                                p_param->event_data.send_data_result.credits);
                if (p_param->event_data.send_data_result.cause == GAP_SUCCESS)
                {
                    APP_PRINT_INFO0("PROFILE_EVT_SEND_DATA_COMPLETE success");
                }
                else
                {
                    APP_PRINT_ERROR0("PROFILE_EVT_SEND_DATA_COMPLETE failed");
                }
                break;

            default:
                break;
        }
    }
    else  if (service_id == simp_srv_id)
    {
        TSIMP_CALLBACK_DATA *p_simp_cb_data = (TSIMP_CALLBACK_DATA *)p_data;
        switch (p_simp_cb_data->msg_type)
        {
            case SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION:
                {
                    switch (p_simp_cb_data->msg_data.notification_indification_index)
                    {
                        case SIMP_NOTIFY_INDICATE_V3_ENABLE:
                            {
                                APP_PRINT_INFO0("SIMP_NOTIFY_INDICATE_V3_ENABLE");
                                notify_v3_conn_id = p_simp_cb_data->conn_id; 
                                g_notify_v3_enable = true;
								bt_con_tag = 0;
								
								sBLECmdHdr temp = {0};	
								crypto_init(&temp, CYP_AUTH_REQ);
								
								uint8_t data[100] = {0};
								uint8_t data_len = 0;
								uint32_t lock_random = 0x12345678;
								//app_crypto_auth_req((uint8_t *)&lock_random, sizeof(lock_random), NULL);
								temp.data_length = sizeof(lock_random);
								memcpy(data, &temp, sizeof(temp));
								memcpy(data + sizeof(temp), &lock_random, sizeof(lock_random));
								data_len = sizeof(temp) + sizeof(lock_random);
								
								simp_ble_service_send_v3_notify(0, simp_srv_id, data, data_len);
                            }
                            break;

                        case SIMP_NOTIFY_INDICATE_V3_DISABLE:
                            {
                                g_notify_v3_enable = false;
                                APP_PRINT_INFO0("SIMP_NOTIFY_INDICATE_V3_DISABLE");
                            }
                            break;
                        case SIMP_NOTIFY_INDICATE_V4_ENABLE:
                            {
                                APP_PRINT_INFO0("SIMP_NOTIFY_INDICATE_V4_ENABLE");
                            }
                            break;
                        case SIMP_NOTIFY_INDICATE_V4_DISABLE:
                            {
                                APP_PRINT_INFO0("SIMP_NOTIFY_INDICATE_V4_DISABLE");
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;

            case SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE:     //读
                {
                    if (p_simp_cb_data->msg_data.read_value_index == SIMP_READ_V1)
                    {
                        uint8_t value[2] = {0x01, 0x02};
                        APP_PRINT_INFO0("SIMP_READ_V1");
                        simp_ble_service_set_parameter(SIMPLE_BLE_SERVICE_PARAM_V1_READ_CHAR_VAL, 2, &value);
                    }
                }
                break;
            case SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE:    //写
                {
                    switch (p_simp_cb_data->msg_data.write.opcode)
                    {
                        case SIMP_WRITE_V2:
                            {
                                APP_PRINT_INFO3("SIMP_WRITE_V2: write type %d, len %d, data:%b", p_simp_cb_data->msg_data.write.write_type,
                                                p_simp_cb_data->msg_data.write.len, TRACE_BINARY(p_simp_cb_data->msg_data.write.len, p_simp_cb_data->msg_data.write.p_value));
                                //bluebooth_send_msg_to_menu_task(BT_MSG_RECV_DECODE,p_simp_cb_data->msg_data.write.p_value, p_simp_cb_data->msg_data.write.len);
 //                               protocol_recv_buffer(p_simp_cb_data->msg_data.write.p_value, p_simp_cb_data->msg_data.write.len);
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;

            default:
                break;
        }
    }
    else if (service_id == g_ota_service_id)
    {

        T_OTA_CALLBACK_DATA *pOTACallbackData = (T_OTA_CALLBACK_DATA *)p_data;
        switch (pOTACallbackData->msg_type)
        {
        case SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE:
            {
                if (OTA_WRITE_CHAR_VAL == pOTACallbackData->msg_data.write.opcode &&
                    OTA_VALUE_ENTER == pOTACallbackData->msg_data.write.u.value)
                {
                    /*battery level is above 60 percent*/
                    APP_PRINT_INFO0("Preparing switch into OTA mode\n");
                    /*prepare to enter OTA mode, before switch action, we should disconnect first.*/
                    switch_to_ota_mode_pending = true;
                    le_disconnect(0);
                }
            }
            break;
        default:
            break;
        }
    }
	else if(service_id == tuya_srv_id)
    {
        T_KNS_CALLBACK_DATA *p_kns_cb_data = (T_KNS_CALLBACK_DATA *)p_data;
        switch (p_kns_cb_data->msg_type)
        {
        case SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION:
            {
                switch (p_kns_cb_data->msg_data.notification_indification_index)
                {
                case KNS_NOTIFY_ENABLE:
                    {
                        APP_PRINT_INFO0("KNS_NOTIFY_ENABLE");
                    }
                    break;

                case KNS_NOTIFY_DISABLE:
                    {
                        APP_PRINT_INFO0("KNS_NOTIFY_DISABLE");
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE:
            {
                APP_PRINT_INFO1("gatt received data len = %d", p_kns_cb_data->msg_data.write_value.data_len);
             //   gTimeParaValue = p_kns_cb_data->msg_data.write_value;
                /*
                event.hdr.event_id = TUYA_BLE_DATA_REV_EVT;
                event.ble_rev_event.len = p_kns_cb_data->msg_data.write_value.data_len;
                memcpy(event.ble_rev_event.data,p_kns_cb_data->msg_data.write_value.data,p_kns_cb_data->msg_data.write_value.data_len);
                if(tuya_event_send(&event)!=0)
                {
                    APP_PRINT_ERROR1("tuya_event_send ble data error,data len = %d ", p_kns_cb_data->msg_data.write_value.data_len);   
                }
                */
                
                tuya_ble_gatt_receive_data(p_kns_cb_data->msg_data.write_value.data,p_kns_cb_data->msg_data.write_value.data_len);

				// ble_value_notify(p_kns_cb_data->msg_data.write_value.data,p_kns_cb_data->msg_data.write_value.data_len);
            }
            break;

        default:
            break;
        }
    }
#if 0    
    else if (service_id == bas_srv_id)
    {
        T_BAS_CALLBACK_DATA *p_bas_cb_data = (T_BAS_CALLBACK_DATA *)p_data;
        switch (p_bas_cb_data->msg_type)
        {
            case SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION:
                {
                    switch (p_bas_cb_data->msg_data.notification_indification_index)
                    {
                    case BAS_NOTIFY_BATTERY_LEVEL_ENABLE:
                        {
                            APP_PRINT_INFO0("BAS_NOTIFY_BATTERY_LEVEL_ENABLE");
                        }
                        break;

                    case BAS_NOTIFY_BATTERY_LEVEL_DISABLE:
                        {
                            APP_PRINT_INFO0("BAS_NOTIFY_BATTERY_LEVEL_DISABLE");
                        }
                        break;
                    default:
                        break;
                    }
                }
                break;

            case SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE:
                {
                    if (p_bas_cb_data->msg_data.read_value_index == BAS_READ_BATTERY_LEVEL)
                    {
                        uint8_t battery_level = 90;
                        APP_PRINT_INFO1("BAS_READ_BATTERY_LEVEL: battery_level %d", battery_level);
                        bas_set_parameter(BAS_PARAM_BATTERY_LEVEL, 1, &battery_level);
                    }
                }
                break;

            default:
                break;
        }
    }
#endif
    return app_result;
}



unsigned char app_gap_adv_state_get(void)
{
    if(gap_dev_state.gap_adv_state == GAP_ADV_STATE_IDLE)
    {
        return 0;
    }
    else if(gap_dev_state.gap_adv_state == GAP_ADV_STATE_ADVERTISING)
    {
        return 1;
    }
    else
    {
        return 2;
    }
}

//获取蓝牙连接情况
uint32_t app_get_bt_real_state(void)
{
	return gap_conn_state;
}


/** @} */ /* End of group PERIPH_SEVER_CALLBACK */
/** @} */ /* End of group PERIPH_APP */






