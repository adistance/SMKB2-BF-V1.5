/*
 * action.h
 *
 *  Created on: 2020��8��17��
 *      Author: sks
 */

#ifndef APPLICATION_ALGO_ACTION_H_
#define APPLICATION_ALGO_ACTION_H_

#include "mlapi.h"
#include "sensor.h"

#define SYS_POLICY_MERGE_CONTROL        (0x00000001)    //�ϲ�ָ������
#define SYS_POLICY_REPEAT_CHECK         (0x00000002)    //�ظ�ָ�Ƽ��
#define SYS_POLICY_SELF_LEARN           (0x00000004)    //��ѧϰ����
#define SYS_POLICY_AUTO_SLEEP           (0x00000008)    //�Զ����߹���(�Զ���Ƶ)
#define SYS_POLICY_360_IDENTIFY         (0x00000010)    //360��ʶ��
#define SYS_POLICY_UART_WAKEUP          (0x00000020)    //�����жϻ�������
#define SYS_POLICY_AUTO_CTRL_LED        (0x00000040)    //�Զ�����LED
#define SYS_POLICY_SYN_COMMAND          (0x00000080)    //ͬ������
#define SYS_TEST_SAVE_PIC				(0x00000100)	//����ע�ᡢƥ�䱣��ͼƬ
#define SYS_POLICY_TZ_MERGE_CONTROL     (0x02000000)    //ͼ��ƴ������
#define SYS_TZ_ENROLL_1C3R				(0x04000000)	//ͼ��ע�᷽ʽΪ1C3R
#define SYS_TZ_MATCH_CAPTURE_RESP		(0x08000000)	//ͼ����ͼ�ɹ��󷵻�


#define ALG_MATCH_THRESHOLD             (4600)
#define ALG_ENROLL_NUM                  (6)                         //�]�ԴΔ�
#define ALG_AUTO_LEARN_NUM              (8)                         //�ԌW���Δ�

#define SENSOR_FTR_BUFFER_TMP           (1024)
#define FTR_BUFFER_MAX          			(0x2000) //8K

typedef struct{
    uint8_t  *pFeature;
    uint32_t length;
    uint32_t progress;
    uint32_t storage_addr;
}ENROLL_PARA, *PENROLL_PARA;

extern ENROLL_PARA g_stEnrollPara;
extern uint8_t g_FtrUpdateFlag;
extern uint8_t g_FtrInfoUpdateFlag;

extern uint8_t* g_pUpdateFTRData;
extern uint16_t g_UpdateFTRIndex;
extern uint32_t g_UpdateFTRLen;
extern uint32_t g_FtrLenTmp;

extern uint16_t g_algMatchScoreThres;

extern uint8_t g_EnrollFtrBuf[SENSOR_FTR_BUFFER_MAX];

extern int32_t action_Extract(uint8_t *Imgbuff);
extern int32_t action_Enrollment(uint8_t num, uint8_t *pProgress, uint16_t storage_index);
extern int32_t action_Match(uint16_t *Fp_Index);
extern void action_AfisInitEnrollNumIndex(uint32_t enrollNum, uint16_t index);
extern int32_t action_MatchEx(uint16_t *Fp_Index);
extern int action_StoreFtr(unsigned int storage_index, unsigned short numId);
extern void action_AfisInit_FirstEnroll(uint16_t index);
void action_AfisInit(int nEnrollMaxNum);


#endif /* APPLICATION_ALGO_ACTION_H_ */
