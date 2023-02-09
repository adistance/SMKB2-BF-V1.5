#include "bluetooth_default.h"

#include "bluetooth_menu.h"

#include "fingerprint.h"

#include "mlapi.h"

#if 1
#define BLUETOOTH_PRINT_LOG_INFO0   APP_PRINT_TRACE0
#define BLUETOOTH_PRINT_LOG_INFO1   APP_PRINT_TRACE1
#define BLUETOOTH_PRINT_LOG_INFO2   APP_PRINT_TRACE2
#define BLUETOOTH_PRINT_LOG_INFO3   APP_PRINT_TRACE3
#define BLUETOOTH_PRINT_LOG_INFO4   APP_PRINT_TRACE4
#else
#define BLUETOOTH_PRINT_LOG_INFO0(...)
#define BLUETOOTH_PRINT_LOG_INFO1(...)
#define BLUETOOTH_PRINT_LOG_INFO2(...)
#define BLUETOOTH_PRINT_LOG_INFO3(...)
#define BLUETOOTH_PRINT_LOG_INFO4(...)
#endif




static T_D_PROTOCOL_MSG g_defaultProtocolResp;

/**********************协议归类处理********************************/
static unsigned char bluetooth_default_protocol_system_cmd_deal(T_D_PROTOCOL_MSG *pReq);
static unsigned char bluetooth_default_protocol_fingerprint_cmd_deal(T_D_PROTOCOL_MSG *pReq);

/**********************配置类处理********************************/
static unsigned char bluetooth_default_protocol_system_cmd_opendoor_handle(T_D_PROTOCOL_MSG *pReq);
    
/**********************指纹类处理********************************/
static unsigned char bluetooth_default_protocol_fingerprint_enroll_handle(T_D_PROTOCOL_MSG *pReq);
static unsigned char bluetooth_default_protocol_fingerprint_delete_handle(T_D_PROTOCOL_MSG *pReq);
static unsigned char bluetooth_default_protocol_fingerprint_get_template_cnt_handle(T_D_PROTOCOL_MSG *pReq);

extern void MLAPI_AbortCommand(void);

/*
*信息接收处理函数
*/
void bluetooth_default_proc(T_D_PROTOCOL_MSG *pReq)                 
{
    unsigned char rv;
    
//    if(pReq->len > 3)
//    {
//        APP_PRINT_INFO5("recv default data address:0x%08x, len:0x%02x, type:0x%02x, cmd:0x%02x, data:%b", pReq->address, pReq->len, pReq->type, pReq->cmd, TRACE_BINARY(pReq->len-3, pReq->data));
//    }
//    else
//    {
//        APP_PRINT_INFO5("recv default data address:0x%08x, len:0x%02x, type:0x%02x, cmd:0x%02x, chk:0x%02x", pReq->address, pReq->len, pReq->type, pReq->cmd, pReq->data[pReq->len-3]);
//    }

    if(pReq->type != DEFAULT_FRAME_TYPE_CMD)
    {
        APP_PRINT_INFO0("[req]:type error");
        default_protocol_resp_cmd_build(pReq, &g_defaultProtocolResp, DEFAULT_COMP_CODE_TYPE, 0, NULL);
        default_protocol_resp_cmd_send(&g_defaultProtocolResp);
        
        return;
    }
    
    if(!default_protocol_chksum_check(pReq))
    {
        APP_PRINT_INFO0("[req]:chk error");
        default_protocol_resp_cmd_build(pReq, &g_defaultProtocolResp, DEFAULT_COMP_CODE_CHK, 0, NULL);
        default_protocol_resp_cmd_send(&g_defaultProtocolResp);
        
        return;
    }
    
    if((pReq->cmd > DEFAULT_SYSTEM_CMD_START) && (pReq->cmd < DEFAULT_SYSTEM_CMD_END))                  //配置类
    {
        rv = bluetooth_default_protocol_system_cmd_deal(pReq);
    }
    else if((pReq->cmd > DEFAULT_FINGERPRINT_CMD_START) && (pReq->cmd < DEFAULT_FINGERPRINT_CMD_END))   //指纹类
    {
        rv = bluetooth_default_protocol_fingerprint_cmd_deal(pReq);
    }
    else
    {
        default_protocol_resp_cmd_build(pReq, &g_defaultProtocolResp, DEFAULT_COMP_CODE_CMD, 0, NULL);
        default_protocol_resp_cmd_send(&g_defaultProtocolResp);
    }
    
    if(rv == 0)
    {
        APP_PRINT_INFO0("[req]: deal success");
    }
    else
    {
        APP_PRINT_INFO0("[req]: deal fail");
        default_protocol_resp_cmd_send(&g_defaultProtocolResp);
    }
    
    return;
}

static unsigned char bluetooth_default_protocol_system_cmd_deal(T_D_PROTOCOL_MSG *pReq)
{
    unsigned char rv = 0;
    
    switch(pReq->cmd)
    {
        case DEFAULT_SYSTEM_CMD_OPENDOOR:
            rv = bluetooth_default_protocol_system_cmd_opendoor_handle(pReq);
            break;
        
        default:
            default_protocol_resp_cmd_build(pReq, &g_defaultProtocolResp, DEFAULT_COMP_CODE_CMD, 0, NULL);
            return 1;
    }
    
    return rv;
}

static unsigned char bluetooth_default_protocol_fingerprint_cmd_deal(T_D_PROTOCOL_MSG *pReq)
{
    unsigned char rv = 0;
    switch(pReq->cmd)
    {
        case DEFAULT_FINGERPRINT_CMD_ENROLL:
            rv = bluetooth_default_protocol_fingerprint_enroll_handle(pReq);
            break;
        
        case DEFAULT_FINGERPRINT_CMD_DELETE:
            rv = bluetooth_default_protocol_fingerprint_delete_handle(pReq);
            break;
        
        case DEFAULT_FINGERPRINT_CMD_GET_TEMPLATE_CNT:
            rv = bluetooth_default_protocol_fingerprint_get_template_cnt_handle(pReq);
            break;
        
        default:
            default_protocol_resp_cmd_build(pReq, &g_defaultProtocolResp, DEFAULT_COMP_CODE_CMD, 0, NULL);
            return 1;
    }
    
    return rv;
}

/****************************************************************/
/**********************系统类处理********************************/
/****************************************************************/
static unsigned char bluetooth_default_protocol_system_cmd_opendoor_handle(T_D_PROTOCOL_MSG *pReq)
{
    unsigned char rv = 0;
    
    BLUETOOTH_PRINT_LOG_INFO0("[BLUETOOTH]:bluetooth_default_protocol_system_cmd_opendoor_handle");
    
    return rv;
}


/****************************************************************/
/**********************指纹类处理********************************/
/****************************************************************/
static unsigned char bluetooth_default_protocol_fingerprint_enroll_handle(T_D_PROTOCOL_MSG *pReq)
{
    unsigned char rv = 0;
    unsigned char step = 1;
    
    PT_D_ENROLL_MSG_REQ pReq_enroll_msg = (PT_D_ENROLL_MSG_REQ)pReq->data;
    
    BLUETOOTH_PRINT_LOG_INFO0("[BLUETOOTH]:bluetooth_default_protocol_fingerprint_enroll_handle");
    
    switch(pReq_enroll_msg->state)
    {
        case FINGERPRINT_ENROLL_START:
            if(RESET == fingerprint_finger_press_state_get() && (0 == menu_sleep_event_timeout_cnt_get(MENU_SLEEP_EVENT_UART_BIT)))               //与单机版逻辑进行互斥
            {
                fingerprint_finger_press_state_set(SET);
                bluebooth_menu_send_msg_to_menu_task(MENU_MSG_TYPE_FINGERPRINT, MENU_MSG_FINGERPRINT_SUBTYPE_ENROLL, 1, &step);
            }
            else
            {
                default_protocol_resp_cmd_build(pReq, &g_defaultProtocolResp, DEFAULT_COME_CODE_BUSY, 0, NULL);
                return 1;
            }
            break;
        
        case FINGERPRINT_ENROLL_CANCEL:
            MLAPI_AbortCommand();
            break;
        
        default:
            default_protocol_resp_cmd_build(pReq, &g_defaultProtocolResp, DEFAULT_COMP_CODE_DATA, 0, NULL);
            return 1;
    }

    return rv;
}

static unsigned char bluetooth_default_protocol_fingerprint_delete_handle(T_D_PROTOCOL_MSG *pReq)
{
    unsigned char rv = 0;
    unsigned char type = pReq->data[0];
    T_MENU_MSG_FINGERPRINT_DELETE fingerprint_msg;

    BLUETOOTH_PRINT_LOG_INFO2("[BLUETOOTH]:bluetooth_default_protocol_fingerprint_delete_handle type:%d, id:%d", type, pReq->data[1]);
    
    if(RESET == fingerprint_finger_press_state_get() && (0 == menu_sleep_event_timeout_cnt_get(MENU_SLEEP_EVENT_UART_BIT)))
    {
        fingerprint_finger_press_state_set(SET);
    }
    else
    {
        default_protocol_resp_cmd_build(pReq, &g_defaultProtocolResp, DEFAULT_COME_CODE_BUSY, 0, NULL);
        return 1;
    }
    
    switch(type)
    {
        case FINGERPRINT_DELETE_ALL:
            fingerprint_msg.deleteMode = ERASE_ALL_FINGER;
            fingerprint_msg.deleteIdx = 0xFF;
            fingerprint_msg.deleteNum = 0;
            bluebooth_menu_send_msg_to_menu_task(MENU_MSG_TYPE_FINGERPRINT, MENU_MSG_FINGERPRINT_SUBTYPE_DELETE, sizeof(T_MENU_MSG_FINGERPRINT_DELETE), (unsigned char*)(&fingerprint_msg));
            break;
        
        case FINGERPRINT_DELETE_SINGLE:
            fingerprint_msg.deleteMode = ERASE_SINGLE_FINGER;
            fingerprint_msg.deleteIdx = pReq->data[1];
            fingerprint_msg.deleteNum = 1;
            bluebooth_menu_send_msg_to_menu_task(MENU_MSG_TYPE_FINGERPRINT, MENU_MSG_FINGERPRINT_SUBTYPE_DELETE, sizeof(T_MENU_MSG_FINGERPRINT_DELETE), (unsigned char*)(&fingerprint_msg));
            break;
        
        default:
            default_protocol_resp_cmd_build(pReq, &g_defaultProtocolResp, DEFAULT_COMP_CODE_DATA, 0, NULL);
            return 1;
    }
    
    return rv;
}

static unsigned char bluetooth_default_protocol_fingerprint_get_template_cnt_handle(T_D_PROTOCOL_MSG *pReq)
{
    unsigned char rv = 0;
    unsigned short num = 0;
    BLUETOOTH_PRINT_LOG_INFO0("[BLUETOOTH]:bluetooth_default_protocol_fingerprint_get_template_cnt_handle");
    
    MLAPI_GetFTRNum(&num);
    num = BIG_TO_LITTLE_U16_ENDIAN(num);
    
    default_protocol_resp_cmd_build(pReq, &g_defaultProtocolResp, DEFAULT_COMP_CODE_OK, 2, (unsigned char*)&num);
    default_protocol_resp_cmd_send(&g_defaultProtocolResp);
    return rv;
}

unsigned char bluetooth_default_protocol_fingerprint_respond(T_IO_MSG *io_msg)
{
    unsigned char *p_buf = io_msg->u.buf;
    
    switch(io_msg->subtype)
    {
        case IO_MSG_FINGERPRINT_SUBTYPE_ENROLL:
            default_protocol_resp_build(&g_defaultProtocolResp, DEFAULT_FINGERPRINT_CMD_ENROLL, p_buf[1], &p_buf[2], p_buf[0]-1);
            break;
        
        case IO_MSG_FINGERPRINT_SUBTYPE_DELETE:
            default_protocol_resp_build(&g_defaultProtocolResp, DEFAULT_FINGERPRINT_CMD_DELETE, p_buf[1], &p_buf[2], p_buf[0]-1);
            break;
        
        default:
            return 1;
    
    }
    
    default_protocol_resp_cmd_send(&g_defaultProtocolResp);
    
    return 0;
}





































#if 0

static void bluetooth_default_test_cmd_build(void)
{
    T_D_PROTOCOL_MSG defaultProtocolTestReq;
    uint32_t address = 0x01234567;
    uint8_t *data = NULL;
    uint8_t i = 0;
    
    for(i=1; i<10; i++)
    {
        default_protocol_cmd_build(&defaultProtocolTestReq, address, i, data, 0);
        default_protocol_resp_cmd_send(&defaultProtocolTestReq);
    }
    delay_ms(100);
    for(i=0x21; i<0x29; i++)
    {
        default_protocol_cmd_build(&defaultProtocolTestReq, address, i, data, 0);
        default_protocol_resp_cmd_send(&defaultProtocolTestReq);
    }
}
#endif
