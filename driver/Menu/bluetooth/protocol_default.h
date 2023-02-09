#ifndef PROTOCOL_DEFAULT_H
#define PROTOCOL_DEFAULT_H

#include "stdio.h"
#include "string.h"
#include "trace.h"

/*
数据格式:	|  head	 | address | len    | type   | cmd 	  | data    | chksum |
			| 2bytes | 4bytes  | 2bytes | 1bytes | 1bytes | n bytes | 1bytes |
			| 0x55AA | XXXX    | 		|        |        |         |        |
			
描述：			
	head ------- 固定包头：	 0x55AA;
	address ---- 设别地址：	 缺省值为0xFFFFFFFF，主控通过命令生成设备地址后，所有数据包都必须携带对应地址通讯
	len -------- 数据长度：  len = type + cmd + data + chksum
    type ------- 数据包类型: 命令包:0x01; 	应答包:0x02; 	数据包:0x03; 	数据结束包:0x04
	cmd -------- 操作命令:   命令包/应答包:操作命令; 	数据包/数据结束包:数据包类型 
	data ------- 数据：  	 命令包:data; 				应答包:compCode(1bytes) + data; 	数据包/数据结束包:data
	chksum ----- 校验和：	 type + cmd + data 的校验和
*/

/*
*type 数据帧类型 定义 	
*/
#define DEFAULT_FRAME_TYPE_CMD                      		(0x01)	//命令包
#define DEFAULT_FRAME_TYPE_RESP                     		(0x02)	//应答包
#define DEFAULT_FRAME_TYPE_DATA                     		(0x03)	//数据包
#define DEFAULT_FRAME_TYPE_DATA_END                 		(0x04)	//数据结束包



/*
*cmd 操作命令定义 	
*/

//系统类：0x01->0x1F
#define DEFAULT_SYSTEM_CMD_START                    		(0x00)
#define DEFAULT_SYSTEM_CMD_OPENDOOR		            		(0x01)
#define DEFAULT_SYSTEM_CMD_END                      		(0x1F)

//指纹类：0x20->0x3F
#define DEFAULT_FINGERPRINT_CMD_START               		(0x20)
#define DEFAULT_FINGERPRINT_CMD_ENROLL		        		(0x21)
#define DEFAULT_FINGERPRINT_CMD_DELETE		        		(0x22)
#define DEFAULT_FINGERPRINT_CMD_GET_TEMPLATE_CNT    		(0x23)
#define DEFAULT_FINGERPRINT_CMD_END                 		(0x3F)

/********************************** 错误码描述(compcode 存放在data数据段的第一个byte)***************************************/
/********************************** 数据帧：0x01->0x1F 	*************************************************************/
/********************************** 系统类：0x81->0x9F 	*************************************************************/
/********************************** 指纹类：0xA1->0xBF	*************************************************************/

/********************************** 指纹类操作类错误码：0x01->0x1F **************************************************/
#define DEFAULT_COMP_CODE_OK                        		(0x00)		//- 默认成功的错误码
#define DEFAULT_COMP_CODE_LEN                       		(0x01)
#define DEFAULT_COMP_CODE_TYPE                      		(0x02)
#define DEFAULT_COMP_CODE_CMD                       		(0x03)
#define DEFAULT_COMP_CODE_CHK                       		(0x04)
#define DEFAULT_COMP_CODE_DATA                      		(0x05)
#define DEFAULT_COME_CODE_BUSY								(0x06)

/********************************** 指纹类操作类错误码：0xA1->0xBF **************************************************/
/*————————————————————————————————————————————录入错误码————————————————————————————————————————————————————————————-*/
#define DEFAULT_COMP_CODE_FINGERPRINT_ENROLL_TIMEOUT		(0xA1)		//- 录入超时
#define DEFAULT_COMP_CODE_FINGERPRINT_ENROLL_REPEAT			(0xA2)		//- 录入重复指纹
#define DEFAULT_COMP_CODE_FINGERPRINT_ENROLL_NO_STOCK		(0xA3)		//- 录入空间已满
#define DEFAULT_COMP_CODE_FINGERPRINT_ENROLL_INVALID_PRESS	(0xA4)		//- 录入指纹按压不全
#define DEFAULT_COMP_CODE_FINGERPRINT_ENROLL_STORE_FAIL		(0xA5)		//- 录入指纹保存失败
/*————————————————————————————————————————————删除错误码————————————————————————————————————————————————————————————-*/
#define DEFAULT_COMP_CODE_FINGERPRINT_DELETE_PARAMETER		(0xA6)		//- 删除参数异常
#define DEFAULT_COMP_CODE_FINGERPRINT_DELETE_FAIL			(0xA7)		//- 删除失败
#define DEFAULT_COMP_CODE_FINGERPRINT_DELETE_INVALID_ID		(0xA8)		//- 删除ID无效

#define DEFAULT_COMP_CODE_FINGERPRINT_HARDWARE_ERROR		(0xBF)		//- 指纹操作硬件异常

#define CALC_U16_DATA_SUM(x) \
    (((x)&0xFF) + (((x)&0xFF00)>>8))
	
#define CALC_U32_DATA_SUM(x) \
    (((x)&0xFF) + (((x)&0xFF00)>>8) + (((x)&0xFF0000)>>16) + (((x)&0xFF000000)>>24))

#define BIG_TO_LITTLE_U32_ENDIAN(x) \
    ( (((x)&0x000000ff)<< 24) | (((x)&0x0000ff00) << 8) | (((x)&0x00ff0000) >> 8) | (((x)&0xff000000) >> 24) )

#define BIG_TO_LITTLE_U16_ENDIAN(x) \
    ( (((x)&0x000000ff)<< 8) | (((x)&0x0000ff00) >> 8) )
    
typedef struct								//命令结构体
{
	unsigned char 	head[2]; 				//协议头（0x55 0xAA）
	unsigned int 	address;				//地址
	unsigned short  len;					//长度
	unsigned char 	type;					//类型
	unsigned char	cmd;					//命令
	unsigned char 	data[128];				//数据 + 检验和
}__attribute__((packed)) T_D_PROTOCOL_MSG, *PT_D_PROTOCOL_MSG;

extern void app_notify_v3_send(uint8_t *sendbuf, uint8_t len);

void protocol_recv(unsigned char data);
void protocol_recv_buffer( unsigned char *data, unsigned short len);
void default_protocol_resp_cmd_send(PT_D_PROTOCOL_MSG pResp);

bool default_protocol_chksum_check(PT_D_PROTOCOL_MSG pMsg);
void default_protocol_cmd_build(PT_D_PROTOCOL_MSG pReq, unsigned int address, unsigned char cmd, unsigned char *data, unsigned short len);
void default_protocol_resp_cmd_build(PT_D_PROTOCOL_MSG pReq, PT_D_PROTOCOL_MSG pResp, unsigned char compCode, unsigned short len, unsigned char *data);
void default_protocol_resp_build(PT_D_PROTOCOL_MSG pResp, uint8_t cmd, unsigned char compCode, unsigned char *data, unsigned short len);


#endif
