/*
 * command.h
 *
 *  Created on: 2020年8月15日
 *      Author: sks
 */

#ifndef APPLICATION_PROTOCOL_COMMAND_H_
#define APPLICATION_PROTOCOL_COMMAND_H_

#include "mlapi.h"

#define RESP_FLAG_NULL                          (0xAA)
#define RESP_FLAG_OK                            (0x55)
#define RESP_FLAG_RUNNING                       (0x11)
#define SY_FLAG_HOST_SENT                       (0x01)
#define SY_RESP_FLAG_DATA                       (0x02)
#define SY_RESP_FLAG_OK                         (0x07)
#define SY_RESP_FLAG_DATA_END                   (0x08) 



//命令字，命令大类
#define CMD_TYPE_FINGERPRINT                    (0x01)
#define CMD_TYPE_SYSTEM                         (0x02)
#define CMD_TYPE_MAINTAINCE                     (0x03)
#define CMD_TYPE_UPDATE                         (0xFE) //(私有命令类型)

//命令字1，指纹类命令
#define CMD_FP_CAPTURE_START                    (0x01)
#define CMD_FP_CAPTURE_READ_DATA                (0x02)
#define CMD_FP_CAPTURE_AFTER_PRESENT            (0x03) //检测到手指在位后，发送此命令采图
#define CMD_FP_TEST_CAPTURE_START               (0x0F) //测试采图命令
#define CMD_FP_REGISTER_START                   (0x11)
#define CMD_FP_REGISTER_GET_RESULT              (0x12)
#define CMD_FP_STORAGE_FTR_START                (0x13)
#define CMD_FP_STORAGE_FTR_GET_RESULT           (0x14)
#define CMD_FP_REGISTER_CANCEL                  (0x15)
#define CMD_FP_STORAGE_FTR_UPDATE               (0x16)
#define CMD_FP_STORAGE_FTR_UPDATE_GET_RESULT    (0x17)
#define CMD_FP_AUTO_REGISTER                    (0x18) //自动注册命令(包含采图+提取+采图+拼接+保存)
#define CMD_FP_MATCH_START                      (0x21)
#define CMD_FP_MATCH_GET_RESULT                 (0x22)
#define CMD_FP_MATCH_SYN                        (0x23) //匹配命令(同步)
#define CMD_FP_MATCH_AFTER_WAKEUP_START         (0x2F)
#define CMD_FP_CLEAR_RECORD_START               (0x31)
#define CMD_FP_CLEAR_RECORD_GET_RESULT          (0x32)
#define CMD_FP_QUERY_FINGER_EXIST               (0x33)
#define CMD_FP_QUERY_FINGER_DISTRIBUTION        (0x34)
#define CMD_FP_QUERY_FINGER_PRESENT             (0x35) //检测手指在位
#define CMD_FP_CLEAR_RECORD_SYN                 (0x36) //清除命令(同步)
#define CMD_FP_VERIFY_START                     (0x41)
#define CMD_FP_VERIFY_GET_RESULT                (0x42)
#define CMD_FP_DOWNLOAD_FTR_INFO                (0x51)
#define CMD_FP_DOWNLOAD_FTR_DATA                (0x52)
#define CMD_FP_UPLOAD_FTR_INFO                  (0x53)
#define CMD_FP_UPLOAD_FTR_DATA                  (0x54)
#define CMD_FP_MATCH_START_UID                  (0x71)
#define CMD_FP_MATCH_GET_RESULT_UID             (0x72)
#define CMD_FP_DELAY_REBOOT                     (0xFA)//0xfd
#define CMD_FP_POWERSAVING_START                (0xFB) //系统休眠0xfc
#define CMD_FP_READ_FTR_START                   (0xFC)
#define CMD_FP_READ_FTR                         (0xFD)
#define CMD_FP_DOWNLOAD_START                   (0xFE)
#define CMD_FP_REQ_TO_RESP                      (0x01) //由请求计算响应命令字，加1

//命令字2，系统类命令
#define CMD_SYS_SET_PASSWORD                    (0x01)
#define CMD_SYS_REBOOT                          (0x02)
#define CMD_SYS_GET_TEMPLATE_CNT                (0x03)
#define CMD_SYS_SET_INTERRUPT                   (0x05)
#define CMD_SYS_READ_AND_CLEAR_INTERRUPT        (0x06)
#define CMD_SYS_SET_TIMEOUT                     (0x07)
#define CMD_SYS_SET_SENSOR_GAIN                 (0x08)
#define CMD_SYS_GET_SENSOR_GAIN                 (0x09)
#define CMD_SYS_SET_MATCH_THRESHOLD             (0x0A)
#define CMD_SYS_GET_MATCH_THRESHOLD             (0x0B)
#define CMD_SYS_POWER_SAVING                    (0x0C)
#define CMD_SYS_SET_ENROLL_MAX_NUM              (0x0D)
#define CMD_SYS_GET_ENROLL_NUM                  (0x0E)
#define CMD_SYS_SET_LED_CTRL_INFO               (0x0F)
#define CMD_SYS_GET_POLICY                      (0xFB)
#define CMD_SYS_SET_POLICY                      (0xFC)
#define CMD_SYS_SET_COVERAGE_RATE               (0xFD)
#define CMD_SYS_SET_QUALITY_LEVEL               (0xFE)
#define CMD_SYS_SET_BT_OPEN						(0x10)  //打开蓝牙命令

//命令字3，维护类命令
#define CMD_MT_READ_ID                          (0x01)
#define CMD_MT_WRITE_ID                         (0x02)
#define CMD_MT_HEARTBEAT                        (0x03)
#define CMD_MT_SET_BAUDRATE                     (0x04)
#define CMD_MT_SET_PROTO_PWD                    (0x05)
#define CMD_MT_GET_SW_VERSION                   (0x06)
#define CMD_MT_MOTOR_TEST                       (0x07)	//电机测试
#define CMD_MT_READ_SN                          (0x08)
#define CMD_MOTOR_SETTING                       (0x50)
#define ML_MAIN_SET_055_BATTERY_SETTING         (0x51)
#define ML_MAIN_BUTTON                          (0x52)
#define ML_MAIN_POWER_OFF                       (0x53)
#define CMD_MT_CHG_TEST                         (0x56)
#define CMD_MT_BAT_ADC_TEST                     (0x57)  //获取电池电量
#define CMD_TUYA_UUID_SET						(0x60)
#define CMD_TUYA_UUID_GET						(0x61)
#define CMD_TUYA_KEY_SET						(0x62)
#define CMD_TUYA_KEY_GET						(0x63)
#define CMD_TUYA_MAC_SET						(0x64)
#define CMD_TUYA_MAC_GET						(0x65)

#define CMD_MT_READ_BOARD_SERIES_NUM            (0xFA)
#define CMD_MT_WRITE_BOARD_SERIES_NUM           (0xFB)

//命令字4，在线升级类命令(私有命令)
#define CMD_UPDATE_START_DOWNLOAD               (0x01)  //启动升级
#define CMD_UPDATA_TRANSFER_FILE                (0x02)
#define CMD_UPDATA_TRANS_COMPLETE               (0x03)
#define CMD_UPDATA_SYS_PARA         		    (0x0f)
#define CMD_UPDATE_EXCHANGE                     (0xf1)  //启动交换升级

#define CMD_RESP_HEAD_LEN                       (10)

void ML_COMMAND_TransSetReqData(P_ML_CMD_REQ_DATA pReq, uint8_t data, uint32_t index);
uint8_t ML_COMMAND_TransGetRespData(P_ML_CMD_RESP_DATA pResp, uint32_t index, uint8_t *pdata);
uint32_t ML_COMMAND_ReqProc(P_ML_CMD_REQ_DATA pReq);
void ML_COMMAND_ReqDeal(P_ML_CMD_REQ_DATA pReq);
uint32_t ML_COMMAND_Post(P_ML_CMD_REQ_DATA pReq);
uint32_t ML_COMMAND_IsRespInvalid(P_ML_CMD_REQ_DATA pReq);

#endif /* APPLICATION_PROTOCOL_COMMAND_H_ */
