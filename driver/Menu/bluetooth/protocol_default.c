#include "protocol_default.h"
#include "bluetooth_default.h"


typedef enum
{
   MSG_STATE_PROTOCOL_HEAD			= 1,
   MSG_STATE_PROTOCOL_FRAME			= 2,
   MSG_STATE_PROTOCOL_DATA			= 3,
}T_MSG_STATE;

static T_MSG_STATE protocol_recv_state = MSG_STATE_PROTOCOL_HEAD;

#define DEFAULT_PROTOCOL			(0)

static unsigned int default_address = 0xFFFFFFFF;
static unsigned char default_head[2] = {0x55, 0xAA};
static unsigned char head_index = 0, frame_index = 0, data_index = 0;
static unsigned char protocol_type = DEFAULT_PROTOCOL;
static T_D_PROTOCOL_MSG g_defaultProtocolReq;

static T_MSG_STATE protocol_head_recv(unsigned char data);
static T_MSG_STATE protocol_frame_recv(unsigned char data);
static T_MSG_STATE protocol_data_recv(unsigned char data);

static T_MSG_STATE defaul_protocol_frame_deal(PT_D_PROTOCOL_MSG pReq, unsigned char data, unsigned char index);
static T_MSG_STATE defaul_protocol_data_deal(PT_D_PROTOCOL_MSG pReq, unsigned char data, unsigned char index);
    
unsigned char default_protocol_chksum_get(PT_D_PROTOCOL_MSG pMsg)
{
	unsigned char chksum = 0;
	unsigned short i = 0, len = pMsg->len - 3;
	
	chksum += 0x55;
	chksum += 0xAA;
	chksum += CALC_U32_DATA_SUM(pMsg->address);
	chksum += CALC_U16_DATA_SUM(pMsg->len);
	chksum += pMsg->type;
	chksum += pMsg->cmd;
	
	for(i=0; i<len; i++)
	{
		chksum += pMsg->data[i]; 
	}
//	APP_PRINT_INFO1("default_protocol_chksum_get:0x%02x", chksum);
	return chksum;
}

bool default_protocol_chksum_check(PT_D_PROTOCOL_MSG pMsg)
{
    uint8_t chk = default_protocol_chksum_get(pMsg);
    
    if(pMsg->data[pMsg->len-2-1] == chk)
    {
        return true;
    }
    else
    {
        APP_PRINT_INFO2("default_protocol_chksum_check error: 0x%02x(r) != 0x%02x", pMsg->data[pMsg->len-2-1], chk);
        return false;
    }
}

void default_protocol_cmd_build(PT_D_PROTOCOL_MSG pReq, unsigned int address, unsigned char cmd, unsigned char *data, unsigned short len)
{
    pReq->head[0]  = 0x55;
    pReq->head[1]  = 0xAA;
    pReq->address  = address;
    pReq->len      = len+3;
    pReq->type     = DEFAULT_FRAME_TYPE_CMD;
    pReq->cmd      = cmd;
  
    if(len != 0)
        memcpy(&pReq->data[0], data, len);
     
    pReq->data[pReq->len-3] = default_protocol_chksum_get(pReq);
    
    return;
}

void default_protocol_resp_cmd_build(PT_D_PROTOCOL_MSG pReq, PT_D_PROTOCOL_MSG pResp, unsigned char compCode, unsigned short len, unsigned char *data)
{
    pResp->head[0]  = 0x55;
    pResp->head[1]  = 0xAA;
    pResp->address  = pReq->address;
    pResp->len      = len+4;
    pResp->type     = DEFAULT_FRAME_TYPE_RESP;
    pResp->cmd      = pReq->cmd;
    pResp->data[0]  = compCode;
  
    if(len != 0)
        memcpy(&pResp->data[1], data, len);
     
    pResp->data[pResp->len-3] = default_protocol_chksum_get(pResp);
    
    return;
}

void default_protocol_resp_build(PT_D_PROTOCOL_MSG pResp, uint8_t cmd, unsigned char compCode, unsigned char *data, unsigned short len)
{
    pResp->head[0]  = 0x55;
    pResp->head[1]  = 0xAA;
    pResp->address  = default_address;
    pResp->len      = len+4;
    pResp->type     = DEFAULT_FRAME_TYPE_RESP;
    pResp->cmd      = cmd;
    pResp->data[0]  = compCode;
  
    if(len != 0)
        memcpy(&pResp->data[1], data, len);
     
    pResp->data[pResp->len-3] = default_protocol_chksum_get(pResp);
    
    return;
}

void default_protocol_resp_cmd_send(PT_D_PROTOCOL_MSG pResp)
{
    uint16_t send_len = pResp->len + 8;
    
    pResp->address = BIG_TO_LITTLE_U32_ENDIAN(pResp->address);
    pResp->len = BIG_TO_LITTLE_U16_ENDIAN(pResp->len);
    app_notify_v3_send((uint8_t*)pResp, send_len);
}


void protocol_recv(unsigned char data)
{
	switch(protocol_recv_state)
	{
		case MSG_STATE_PROTOCOL_HEAD:
			protocol_recv_state = protocol_head_recv(data);
			break;
			
		case MSG_STATE_PROTOCOL_FRAME:
			protocol_recv_state = protocol_frame_recv(data);
			break;
			
		case MSG_STATE_PROTOCOL_DATA:
			protocol_recv_state = protocol_data_recv(data);
			break;
			
		default:
			protocol_recv_state = MSG_STATE_PROTOCOL_HEAD;
			break;
	}
	
	return;
}

void protocol_recv_buffer( unsigned char *data, unsigned short len)
{
    unsigned short i;
    
    for(i=0; i<len; i++)
    {
        protocol_recv(data[i]);
    }
    
    return;
}

static T_MSG_STATE protocol_head_recv(unsigned char data)
{
	if(data == default_head[head_index])
	{   
		g_defaultProtocolReq.head[head_index] = data;
		head_index ++;
		if(head_index == 2)
		{
			protocol_type = DEFAULT_PROTOCOL;

			head_index = 0;
			frame_index = 0;
			data_index = 0;
			return MSG_STATE_PROTOCOL_FRAME;
		}
	}
	else
	{
		head_index = 0;
		frame_index = 0;
		data_index = 0;
	}
	return MSG_STATE_PROTOCOL_HEAD;
}

static T_MSG_STATE protocol_frame_recv(unsigned char data)
{
	T_MSG_STATE rv;
	
	switch(protocol_type)
	{
		case DEFAULT_PROTOCOL:
			rv = defaul_protocol_frame_deal(&g_defaultProtocolReq, data, frame_index);
			frame_index++;
			return rv;
		
		default:
			return MSG_STATE_PROTOCOL_HEAD;
	}
}

static T_MSG_STATE protocol_data_recv(unsigned char data)
{
    T_MSG_STATE rv;
    
	switch(protocol_type)
	{
		case DEFAULT_PROTOCOL:
			rv = defaul_protocol_data_deal(&g_defaultProtocolReq, data, data_index);
			data_index++;
			return rv;
		
		default:
			return MSG_STATE_PROTOCOL_HEAD;
	}
}

static T_MSG_STATE defaul_protocol_frame_deal(PT_D_PROTOCOL_MSG pReq, unsigned char data, unsigned char index)
{
	switch(index)
	{
		case 0:
            pReq->address = 0;
		case 1:
		case 2:
		case 3:
			pReq->address |= (data << ((3-index) * 8));
			break;
			
		case 4:
            pReq->len = 0;
		case 5:
			pReq->len |= (data << ((5-index) * 8));
			break;
			
		case 6:
			pReq->type = data;
			break;
			
		case 7:
			pReq->cmd = data;
			return MSG_STATE_PROTOCOL_DATA;
			
		default:
			return MSG_STATE_PROTOCOL_HEAD;
	}
	
	return MSG_STATE_PROTOCOL_FRAME;
}

static T_MSG_STATE defaul_protocol_data_deal(PT_D_PROTOCOL_MSG pReq, unsigned char data, unsigned char index)
{
    if(index <= (pReq->len - 3))
    {
        pReq->data[index] = data;
        if(index == (pReq->len - 3))
        {
            //recv complete
            bluetooth_default_proc(pReq);
            
            return MSG_STATE_PROTOCOL_HEAD;
        }
	}
	else
    {
        return MSG_STATE_PROTOCOL_HEAD;
    }
	return MSG_STATE_PROTOCOL_DATA;
}







