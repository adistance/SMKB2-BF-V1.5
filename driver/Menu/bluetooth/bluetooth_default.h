#ifndef BLUETOOTH_DEFAULT_H
#define BLUETOOTH_DEFAULT_H

#include "protocol_default.h"
#include "app_msg.h"

/*
ע��ʱ��״̬����
*/
#define FINGERPRINT_ENROLL_ADD_ADMIN    (0x00)
#define FINGERPRINT_ENROLL_ADD_USER     (0x01)

#define FINGERPRINT_ENROLL_START		(0xFA)		//¼�뿪ʼ
#define FINGERPRINT_ENROLL_CONTINUE		(0xFB)		//¼����
#define FINGERPRINT_ENROLL_END			(0xFC)		//¼�����
#define FINGERPRINT_ENROLL_CANCEL		(0xFD)		//¼��ȡ��
#define FINGERPRINT_ENROLL_RESERVE		(0xFF)		//����λ���

#define FINGERPRINT_DELETE_ALL          (0x00)
#define FINGERPRINT_DELETE_SINGLE       (0x01)      

typedef struct								//ע��ʱdata�����Ľṹ��
{
	unsigned char 	flag; 				//����    - 0x00:����Ա��0x01:��ͨ�û�
	unsigned char 	id;				    //id      - ����
	unsigned char   state;              //״̬    - 0xFA:��ʼ¼�롢0xFE:¼��ȡ��
    unsigned char   reserve;            //����
}__attribute__((packed)) T_D_ENROLL_MSG_REQ, *PT_D_ENROLL_MSG_REQ;

void bluetooth_default_proc(T_D_PROTOCOL_MSG *req_msg);
unsigned char bluetooth_default_protocol_fingerprint_respond(T_IO_MSG *io_msg);

#endif
