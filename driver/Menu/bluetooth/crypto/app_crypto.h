/**
 ****************************************************************************************
 *
 * @file app_crypto.h
 *
 * @brief Application crypto implementation
 *
 ****************************************************************************************
 */

#ifndef APP_CRYPTO_H_
#define APP_CRYPTO_H_

#include "stdio.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "app_msg.h"

#define MAX_MESSAGE_LEN 256 + 8
typedef __packed struct _BLECmdHdr {
  uint8_t magic_num;  //[0] 0xFE
  uint8_t flow_ctrl;  //[1]	010b	Data Up, 011b Data Down
  uint16_t flow_cnt;  //[2,3]
  uint16_t flow_mic;  //[4,5]
  uint8_t flow_port;	//cmd type
  uint8_t data_length;
} sBLECmdHdr __attribute__((aligned(1)));

typedef enum {
  CYP_OK = 0,
  CYP_BUSY,     // Aes is busy in last request
  CYP_BAD_LEN,  // Buf length error
  CYP_BAD_MIC,  // Mic/signature error
  CYP_NOAUTH,   // Not authed
  CYP_ERROR,    // Other error
} cyp_status_t;

typedef enum
{
	CYP_AUTH_REQ = 0,    //认证请求
	CYP_AUTH_RESP = 1,	//认证回复
	CYP_DATA_UP = 2,	//mcu->小程序(数据流向)
	CYP_DATA_DOWN = 3,  //小程序->mcu 
}cyp_ctrl_type_t;

// crypto function
typedef void (*crypto_cb_func_t)(cyp_status_t status, uint8_t *buf, uint8_t len);
typedef bool (*msg_callback)(T_IO_MSG * p_msg);
// Init crypto
// rootkey:16 bytes rootkey
// pw:	admin password
// len: length of admin password
cyp_status_t app_crypto_init(uint16_t io_type, uint16_t io_subtype, uint8_t *rootkey, uint8_t *pw, uint8_t len, msg_callback p_msg_callback);
cyp_status_t app_crypto_set_pw(uint8_t *pw, uint8_t len);

// Auth request, set devnce.
// buf: 4 bytes random value
// len: buf length
// cb:  null
cyp_status_t app_crypto_auth_req(uint8_t *buf, uint8_t len, crypto_cb_func_t cb);
// Handle responce of auth got from app
// buf: message bufer
// len: buf length
// cb: status-return status, buf: ts , len: buffer length
cyp_status_t app_crypto_auth_resp(uint8_t *buf, uint8_t len, crypto_cb_func_t cb);

// Decode/encode the frame
// buf: message bufer
// len: buf length
// cb: status-return status, buf: message , len: buffer length
cyp_status_t app_crypto_msg_decode(uint8_t *buf, uint8_t len, crypto_cb_func_t cb);
cyp_status_t app_crypto_msg_encode(uint8_t *buf, uint8_t len, crypto_cb_func_t cb);

// Verify the signature of payload
// buf: payload without fhdr(frame header)
// len: payload length
// cb: status-return status
cyp_status_t app_crypto_pw_verify(uint8_t *buf, uint8_t len, crypto_cb_func_t cb);

// Get psk
// cb: status-return status,buf: psk , len: psk length
cyp_status_t app_crypto_get_psk(crypto_cb_func_t cb);

void cryptlib_check_result(const uint8_t *buf);
// extern function
typedef void (*aes_cb_func_t)(uint8_t status, const uint8_t *buf);
extern void app_start_encrypt_req(uint8_t key[16], uint8_t plain_data[16] /*,aes_cb_func_t cb*/);

#endif  // APP_CRYPTO_H_
