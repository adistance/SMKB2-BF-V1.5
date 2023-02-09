#ifndef PROTOCOL_DEFAULT_H
#define PROTOCOL_DEFAULT_H

#include "stdio.h"
#include "string.h"
#include "trace.h"

/*
���ݸ�ʽ:	|  head	 | address | len    | type   | cmd 	  | data    | chksum |
			| 2bytes | 4bytes  | 2bytes | 1bytes | 1bytes | n bytes | 1bytes |
			| 0x55AA | XXXX    | 		|        |        |         |        |
			
������			
	head ------- �̶���ͷ��	 0x55AA;
	address ---- ����ַ��	 ȱʡֵΪ0xFFFFFFFF������ͨ�����������豸��ַ���������ݰ�������Я����Ӧ��ַͨѶ
	len -------- ���ݳ��ȣ�  len = type + cmd + data + chksum
    type ------- ���ݰ�����: �����:0x01; 	Ӧ���:0x02; 	���ݰ�:0x03; 	���ݽ�����:0x04
	cmd -------- ��������:   �����/Ӧ���:��������; 	���ݰ�/���ݽ�����:���ݰ����� 
	data ------- ���ݣ�  	 �����:data; 				Ӧ���:compCode(1bytes) + data; 	���ݰ�/���ݽ�����:data
	chksum ----- У��ͣ�	 type + cmd + data ��У���
*/

/*
*type ����֡���� ���� 	
*/
#define DEFAULT_FRAME_TYPE_CMD                      		(0x01)	//�����
#define DEFAULT_FRAME_TYPE_RESP                     		(0x02)	//Ӧ���
#define DEFAULT_FRAME_TYPE_DATA                     		(0x03)	//���ݰ�
#define DEFAULT_FRAME_TYPE_DATA_END                 		(0x04)	//���ݽ�����



/*
*cmd ��������� 	
*/

//ϵͳ�ࣺ0x01->0x1F
#define DEFAULT_SYSTEM_CMD_START                    		(0x00)
#define DEFAULT_SYSTEM_CMD_OPENDOOR		            		(0x01)
#define DEFAULT_SYSTEM_CMD_END                      		(0x1F)

//ָ���ࣺ0x20->0x3F
#define DEFAULT_FINGERPRINT_CMD_START               		(0x20)
#define DEFAULT_FINGERPRINT_CMD_ENROLL		        		(0x21)
#define DEFAULT_FINGERPRINT_CMD_DELETE		        		(0x22)
#define DEFAULT_FINGERPRINT_CMD_GET_TEMPLATE_CNT    		(0x23)
#define DEFAULT_FINGERPRINT_CMD_END                 		(0x3F)

/********************************** ����������(compcode �����data���ݶεĵ�һ��byte)***************************************/
/********************************** ����֡��0x01->0x1F 	*************************************************************/
/********************************** ϵͳ�ࣺ0x81->0x9F 	*************************************************************/
/********************************** ָ���ࣺ0xA1->0xBF	*************************************************************/

/********************************** ָ�������������룺0x01->0x1F **************************************************/
#define DEFAULT_COMP_CODE_OK                        		(0x00)		//- Ĭ�ϳɹ��Ĵ�����
#define DEFAULT_COMP_CODE_LEN                       		(0x01)
#define DEFAULT_COMP_CODE_TYPE                      		(0x02)
#define DEFAULT_COMP_CODE_CMD                       		(0x03)
#define DEFAULT_COMP_CODE_CHK                       		(0x04)
#define DEFAULT_COMP_CODE_DATA                      		(0x05)
#define DEFAULT_COME_CODE_BUSY								(0x06)

/********************************** ָ�������������룺0xA1->0xBF **************************************************/
/*����������������������������������������������������������������������������������������¼������롪����������������������������������������������������������������������������������������������������������������������-*/
#define DEFAULT_COMP_CODE_FINGERPRINT_ENROLL_TIMEOUT		(0xA1)		//- ¼�볬ʱ
#define DEFAULT_COMP_CODE_FINGERPRINT_ENROLL_REPEAT			(0xA2)		//- ¼���ظ�ָ��
#define DEFAULT_COMP_CODE_FINGERPRINT_ENROLL_NO_STOCK		(0xA3)		//- ¼��ռ�����
#define DEFAULT_COMP_CODE_FINGERPRINT_ENROLL_INVALID_PRESS	(0xA4)		//- ¼��ָ�ư�ѹ��ȫ
#define DEFAULT_COMP_CODE_FINGERPRINT_ENROLL_STORE_FAIL		(0xA5)		//- ¼��ָ�Ʊ���ʧ��
/*����������������������������������������������������������������������������������������ɾ�������롪����������������������������������������������������������������������������������������������������������������������-*/
#define DEFAULT_COMP_CODE_FINGERPRINT_DELETE_PARAMETER		(0xA6)		//- ɾ�������쳣
#define DEFAULT_COMP_CODE_FINGERPRINT_DELETE_FAIL			(0xA7)		//- ɾ��ʧ��
#define DEFAULT_COMP_CODE_FINGERPRINT_DELETE_INVALID_ID		(0xA8)		//- ɾ��ID��Ч

#define DEFAULT_COMP_CODE_FINGERPRINT_HARDWARE_ERROR		(0xBF)		//- ָ�Ʋ���Ӳ���쳣

#define CALC_U16_DATA_SUM(x) \
    (((x)&0xFF) + (((x)&0xFF00)>>8))
	
#define CALC_U32_DATA_SUM(x) \
    (((x)&0xFF) + (((x)&0xFF00)>>8) + (((x)&0xFF0000)>>16) + (((x)&0xFF000000)>>24))

#define BIG_TO_LITTLE_U32_ENDIAN(x) \
    ( (((x)&0x000000ff)<< 24) | (((x)&0x0000ff00) << 8) | (((x)&0x00ff0000) >> 8) | (((x)&0xff000000) >> 24) )

#define BIG_TO_LITTLE_U16_ENDIAN(x) \
    ( (((x)&0x000000ff)<< 8) | (((x)&0x0000ff00) >> 8) )
    
typedef struct								//����ṹ��
{
	unsigned char 	head[2]; 				//Э��ͷ��0x55 0xAA��
	unsigned int 	address;				//��ַ
	unsigned short  len;					//����
	unsigned char 	type;					//����
	unsigned char	cmd;					//����
	unsigned char 	data[128];				//���� + �����
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
