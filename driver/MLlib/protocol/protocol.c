/*
 * protocol.c
 *
 *  Created on: 2020年8月15日
 *      Author: sks
 */
#include "command.h"
#include "protocol.h"
#include "system_setting.h"
#include "command_SY.h"

#include "os_sync.h"
#include "driver_uart.h"


#if 0
#define ML_PROTOCOL_PRINTF(x)   Printf x
#else
#define ML_PROTOCOL_PRINTF(x)  
#endif

#define UTRNS_STATE_RECV_PROTOCOL                   (0)
#define UTRNS_STATE_IDLE                            (1)
#define UTRNS_STATE_RECV_HEAD                       (2)
#define UTRNS_STATE_RECV_LENGTH                     (3)
#define UTRNS_STATE_RECV_HEAD_CHKSUM                (4)
#define UTRNS_STATE_RECV_FRAME                      (5)
#define UTRNS_STATE_RECV_FRAME_ML                   (6)
#define UTRNS_STATE_RECV_FRAME_TZ                   (7)
#define UTRNS_STATE_RECV_FRAME_SY                   (8)
#define UTRNS_STATE_RECV_DATA_ML                    (9)
#define UTRNS_STATE_RECV_DATA_TZ                    (10)
#define UTRNS_STATE_RECV_DATA_SY                    (11)
#define UTRNS_STATE_RECV_FRAME_BR                   (12)
#define UTRNS_STATE_RECV_DATA_BR                    (13)

#define ML_FRAME_HEAD_LENGTH                        (8)
#define ML_CMD_REQ_HEAD_LEN                         (6)
#define SUM_OF_FRAME_HEAD                           (0x73)

uint8_t protocolFlag = ML_PROTOCOL;

extern void *command_req_sem_handle;
extern void *command_resp_sem_handle;
uint8_t g_RunningFlag = false;

/******************ML***********************/
static uint8_t s_Protocolstate = UTRNS_STATE_RECV_PROTOCOL;
static uint8_t g_MLframeHead[8] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A};
static uint8_t MLHeadCheckSum = 0;
static uint32_t recvFramelength = 0;
static uint16_t  index_head = 0, g_recvLen = 0, index_len = 0;

static ML_CMD_REQ_DATA g_MLmainCmdReq;
static ML_CMD_REQ_DATA g_stMLMainCmdReq;

volatile uint8_t g_recvFlag = 0;

/********************SY*********************/
#define SY_FRAME_HEAD_LENGTH	(2)
uint8_t g_SYframeHead[2] = {0xef,0x01};
SY_CMD_REQ_DATA g_SYmainCmdReq;
uint8_t g_cmdReqData[258] = {0};
uint8_t g_cmdRespData[260] = {0};

extern uint8_t g_QuitAtOnce;

uint8_t get_recv()
{
	return g_recvFlag;
}

void UTRNS_SendFrame_ML(P_ML_CMD_RESP_DATA pFrame)
{
    uint32_t i;
    uint16_t len;
    uint8_t data, chksum;
    
    len = pFrame->len + CMD_RESP_HEAD_LEN;
    chksum = SUM_OF_FRAME_HEAD;

    //发送头信息
    USART_SendData(&g_MLframeHead[0], sizeof(g_MLframeHead));

    //发送帧长度信息
    data = (len & 0xFF00) >> 8;
    chksum += data;
    USART_SendData(&data, 1);
    
    data = len & 0xFF;
    chksum += data;
    USART_SendData(&data, 1);

    //发送chksum
    chksum = 0 - chksum;
    USART_SendData(&chksum, 1);

    for (i = 0; i < len; i++)
    {
        data = 0;
        ML_COMMAND_TransGetRespData(pFrame, i, &data);
        USART_SendData(&data, 1);
    }
}

uint8_t UTRNS_recv_ML_data(unsigned char data)
{
    g_MLmainCmdReq.req[g_recvLen] = data;
    g_recvLen++;

    if(g_recvLen>512)
    {
        index_len = 0;
        g_recvLen = 0;
        return UTRNS_STATE_RECV_PROTOCOL; 
    }
    
    if(g_recvLen >= g_MLmainCmdReq.len)
    {
        if((CMD_TYPE_FINGERPRINT == g_MLmainCmdReq.cmd1) && (CMD_FP_REGISTER_CANCEL == g_MLmainCmdReq.cmd2))
        {
            g_QuitAtOnce = 1;
        }
        else
        {
            g_QuitAtOnce = 0;
        }
		uart_mode_set(true);
        g_recvFlag = 1;
        protocol_ReqProc();
        
        index_len = 0;
        g_recvLen = 0;
        return UTRNS_STATE_RECV_PROTOCOL;
    }

     return UTRNS_STATE_RECV_DATA_ML;
}

uint8_t UTRNS_recv_ML_frame(unsigned char data)
{
    ML_COMMAND_TransSetReqData(&g_MLmainCmdReq, data, g_recvLen);
    g_recvLen++;
    if(g_recvLen >= ML_CMD_REQ_HEAD_LEN)
    {
        g_MLmainCmdReq.len = ( recvFramelength >= ML_CMD_REQ_HEAD_LEN) ? (recvFramelength - ML_CMD_REQ_HEAD_LEN) : 0;
        index_len = 0;
        g_recvLen = 0;
        return UTRNS_STATE_RECV_DATA_ML;
    }

    return UTRNS_STATE_RECV_FRAME_ML;
}

/*****************SY*************************/
uint8_t UTRNS_recv_SY_frame(unsigned char data)
{
    //SY_COMMAND_TransSetReqData(&g_SYmainCmdReq, data, g_recvLen);
    g_recvLen++;
    if (g_recvLen >= SY_CMD_REQ_HEAD_LEN)
    {
        g_recvLen = 0;
        index_len = 0;
		return UTRNS_STATE_RECV_DATA_SY;
    }

    return UTRNS_STATE_RECV_FRAME_SY;
}

uint8_t UTRNS_recv_SY_data(unsigned char data)
{
    if(g_SYmainCmdReq.len > sizeof(g_cmdReqData))
    {
        g_cmdRespData[g_recvLen] = data;
    }
    else
    {
        g_cmdReqData[g_recvLen] = data;
    }
    g_recvLen++;
    
    if(g_recvLen >= (g_SYmainCmdReq.len-1))
    {
    	if(g_recvLen > 0)
    	{
    		memcpy(g_SYmainCmdReq.req, g_cmdReqData, g_recvLen);
    	}
		g_recvFlag = 1;
        //SY_COMMAND_ReqProc(&g_SYmainCmdReq);
		g_recvFlag = 0;
        g_recvLen = 0;
        index_len = 0;
        return UTRNS_STATE_RECV_PROTOCOL;
    }

    return UTRNS_STATE_RECV_DATA_SY;
}
/*****************SY END*************************/

uint8_t UTRNS_recv_frame(unsigned char data)
{
    switch(protocolFlag)
    {
        case ML_PROTOCOL:
            return UTRNS_recv_ML_frame(data);
        
        case SY_PROTOCOL:
            return UTRNS_recv_SY_frame(data);
        
        default:
            index_len = 0;
            g_recvLen = 0;
            return UTRNS_STATE_RECV_PROTOCOL;
    }
}

uint8_t UTRNS_recv_head_chksum(unsigned char data)
{
    MLHeadCheckSum += data;
    if(0 == MLHeadCheckSum) //这里把 长度+校验+0x73=256，超过了8bit，所以等于0
    {
        g_recvLen = 0;
        return UTRNS_STATE_RECV_FRAME;
    }
    else
    {
        index_len = 0;
        return UTRNS_STATE_RECV_PROTOCOL;
    }
}

uint8_t UTRNS_recv_length(unsigned char data)
{
    MLHeadCheckSum += data;
    if(0 == index_len)
    {
        recvFramelength = (data << 8);
        index_len++;
        return UTRNS_STATE_RECV_LENGTH;
    }
    else
    {
        recvFramelength += data;
        if((recvFramelength - ML_CMD_REQ_HEAD_LEN) > 512)       //data 长度异常
        {
            return UTRNS_STATE_RECV_PROTOCOL;
        }
        return UTRNS_STATE_RECV_HEAD_CHKSUM;
    }
}

uint8_t UTRNS_recv_head(unsigned char data)
{
    if(data == g_MLframeHead[index_len])
    {
        index_head++;
        index_len++;
        if(index_head == ML_FRAME_HEAD_LENGTH)
        {
            index_head = 0;
            index_len = 0;
            g_recvLen = 0;
            MLHeadCheckSum = SUM_OF_FRAME_HEAD;

            protocolFlag = ML_PROTOCOL;
            return UTRNS_STATE_RECV_LENGTH;
        }
        else
        {
            return UTRNS_STATE_RECV_HEAD;
        }
    }
    else if (data == g_SYframeHead[index_len] )
    {
        index_len++;
        if (SY_FRAME_HEAD_LENGTH == index_len)
        {
            index_len = 0;
            g_recvLen = 0;
            
            protocolFlag = SY_PROTOCOL;
            return UTRNS_STATE_RECV_FRAME;
        }  
        else
        {
            return UTRNS_STATE_RECV_HEAD;
        }
    }
    else
    {
        index_head = 0;
        index_len = 0;
        g_recvLen = 0;
        return UTRNS_STATE_RECV_PROTOCOL;
    }
    
    
}

uint8_t UTRNS_recv_protocol(unsigned char data)
{
    if (data == g_MLframeHead[index_len])
    {
        index_head = 0;
        return UTRNS_recv_head(data);
    }
    else if(data == g_SYframeHead[index_len])
    {
        index_head = 0;
        return UTRNS_recv_head(data);
    }
    else
    {
        index_len = 0;
        return UTRNS_STATE_RECV_PROTOCOL;
    }
}

void USART_RecvData(uint8_t data)
{
    switch (s_Protocolstate)
    {
        case UTRNS_STATE_RECV_PROTOCOL:
            s_Protocolstate = UTRNS_recv_protocol(data);
            break;

        case UTRNS_STATE_RECV_HEAD:
            s_Protocolstate = UTRNS_recv_head(data);
            break;

        case UTRNS_STATE_RECV_LENGTH:
            s_Protocolstate = UTRNS_recv_length(data);
            break;

        case UTRNS_STATE_RECV_HEAD_CHKSUM:
            s_Protocolstate = UTRNS_recv_head_chksum(data);
            break;

        case UTRNS_STATE_RECV_FRAME:
            s_Protocolstate = UTRNS_recv_frame(data);
            break;

        case UTRNS_STATE_RECV_FRAME_ML:
            s_Protocolstate = UTRNS_recv_ML_frame(data);
            break;

        case UTRNS_STATE_RECV_DATA_ML:
            s_Protocolstate = UTRNS_recv_ML_data(data);
            break;
        
        case UTRNS_STATE_RECV_FRAME_SY:
            s_Protocolstate = UTRNS_recv_SY_frame(data);
            break; 

        case UTRNS_STATE_RECV_DATA_SY: 
            s_Protocolstate = UTRNS_recv_SY_data(data);
            break;
        

        default:
            g_recvLen = 0;
            s_Protocolstate = UTRNS_STATE_RECV_PROTOCOL;
            break;
    }
}

void protocol_ReqProc(void)
{
    if(g_recvFlag == 0)
    {
        return;
    }
    
    ML_PROTOCOL_PRINTF(("ml protocol %x %x\r\n",g_MLmainCmdReq.cmd1,g_MLmainCmdReq.cmd2));
    ML_COMMAND_ReqProc(&g_MLmainCmdReq);
    
    g_recvFlag = 0;
}

void protocol_command_cpy(P_ML_CMD_REQ_DATA pReq)
{
    int i;
    
    g_stMLMainCmdReq.pwd = pReq->pwd;
    g_stMLMainCmdReq.cmd1 = pReq->cmd1;
    g_stMLMainCmdReq.cmd2 = pReq->cmd2;
    g_stMLMainCmdReq.len = pReq->len;
    for(i=0; i<pReq->len; i++)
    {
        g_stMLMainCmdReq.req[i] = pReq->req[i];
    }
    return;
}

/*****************************************************************************
 函 数 名  : COMMAND_Task
 功能描述  : 请求处理任务
 输入参数  : void * p_arg  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
void ML_COMMAND_Task(void)
{
    if(os_sem_take(command_req_sem_handle, 0) == false)
    {
        return;
    }

    g_RunningFlag = true;
    
    switch(protocolFlag)
    {
        case ML_PROTOCOL:
            ML_COMMAND_ReqDeal(&g_stMLMainCmdReq);
            break;
        
        case SY_PROTOCOL:
            //SY_COMMAND_Task();
            break;
        
        default:
            break;
    }
    
    os_sem_give(command_resp_sem_handle);
    g_RunningFlag = false;
}

