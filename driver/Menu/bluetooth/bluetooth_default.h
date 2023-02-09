#ifndef BLUETOOTH_DEFAULT_H
#define BLUETOOTH_DEFAULT_H

#include "protocol_default.h"
#include "app_msg.h"

/*
注册时的状态类型
*/
#define FINGERPRINT_ENROLL_ADD_ADMIN    (0x00)
#define FINGERPRINT_ENROLL_ADD_USER     (0x01)

#define FINGERPRINT_ENROLL_START		(0xFA)		//录入开始
#define FINGERPRINT_ENROLL_CONTINUE		(0xFB)		//录入中
#define FINGERPRINT_ENROLL_END			(0xFC)		//录入结束
#define FINGERPRINT_ENROLL_CANCEL		(0xFD)		//录入取消
#define FINGERPRINT_ENROLL_RESERVE		(0xFF)		//保留位填充

#define FINGERPRINT_DELETE_ALL          (0x00)
#define FINGERPRINT_DELETE_SINGLE       (0x01)      

typedef struct								//注册时data参数的结构体
{
	unsigned char 	flag; 				//类型    - 0x00:管理员、0x01:普通用户
	unsigned char 	id;				    //id      - 保留
	unsigned char   state;              //状态    - 0xFA:开始录入、0xFE:录入取消
    unsigned char   reserve;            //保留
}__attribute__((packed)) T_D_ENROLL_MSG_REQ, *PT_D_ENROLL_MSG_REQ;

void bluetooth_default_proc(T_D_PROTOCOL_MSG *req_msg);
unsigned char bluetooth_default_protocol_fingerprint_respond(T_IO_MSG *io_msg);

#endif
