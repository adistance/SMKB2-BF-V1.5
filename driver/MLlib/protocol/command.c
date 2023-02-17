/*
 * command.c
 *
 *  Created on: 2020��8��15��
 *      Author: sks
 */

#ifndef APPLICATION_PROTOCOL_COMMAND_C_
#define APPLICATION_PROTOCOL_COMMAND_C_

#include "action.h"
#include "filesys.h"
#include "command.h"
#include "protocol.h"
#include "driver_sensor.h"
#include "driver_led.h"
#include "trace.h"
#include "version.h"

#include "system_setting.h"
#include "rtl876x_wdg.h"

#include "os_sync.h"
#include "test_mode.h"
#include "menu_manage.h"
#include "driver_sensor.h"
#include "driver_motor.h"
#include "menu_manage.h"
#include "fpsapierrors.h"
#include "filesys.h"
#include "tuya_ble_main.h"
#include "test_mode.h"
#include "os_sched.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_port_rtl8762.h"

extern void *command_req_sem_handle;
extern void *command_resp_sem_handle;

extern void request_upgrade_cmd(uint8_t cmd_type, uint8_t *data, uint8_t dataLen);

#if 0
#define ML_COMMAND_PRINT_INFO0   APP_PRINT_TRACE0
#define ML_COMMAND_PRINT_INFO1   APP_PRINT_TRACE1
#define ML_COMMAND_PRINT_INFO2   APP_PRINT_TRACE2
#define ML_COMMAND_PRINT_INFO3   APP_PRINT_TRACE3
#define ML_COMMAND_PRINT_INFO4   APP_PRINT_TRACE4
#else
#define ML_COMMAND_PRINT_INFO0(...) 
#define ML_COMMAND_PRINT_INFO1(...)	
#define ML_COMMAND_PRINT_INFO2(...)	
#define ML_COMMAND_PRINT_INFO3(...)	
#define ML_COMMAND_PRINT_INFO4(...)	
#endif

#define CALC_U32_DATA_SUM(x) \
    (((x)&0xFF) + (((x)&0xFF00)>>8) + (((x)&0xFF0000)>>16) + (((x)&0xFF000000)>>24))




#define CMD_REQ_HEAD_LEN                        (6)

static P_ML_CMD_RESP_DATA g_pstMLCmdResp;

static uint8_t g_StartChksum;
static uint8_t g_GetResultChksum;
static ML_CMD_RESP_DATA g_stStartResp;
static ML_CMD_RESP_DATA g_stGetResultResp;

static uint8_t g_cmdRespData[260] = {0};

#define IMG_FRAME_LEN 128
static uint32_t g_imgFrameLen = IMG_FRAME_LEN;

extern uint8_t g_RunningFlag;  //�������б�־                                    

uint32_t g_DownloadFTRLen = 0;
uint16_t g_DownloadFTRIndex = 0;
uint32_t g_UploadFTRLen = 0;
uint16_t g_UploadFTRIndex = 0;
static uint32_t unRecievedLen = 0;
static uint8_t g_imgReadBuf[132];


/*****************************************************************************
 �� �� ��  : COMMAND_CalcChksum ��������  : ���㷵����Ϣ��У���
 �������  : PCMD_RESP_DATA pResp
 �������  : ��
�� �� ֵ  :
���ú���  :
��������  :

�޸���ʷ      :
 1.��      �� : 2015��12��28��
   ��      �� : jack
   �޸�����   : �����ɺ���
*****************************************************************************/
void ML_COMMAND_CalcChksum(P_ML_CMD_RESP_DATA pResp)
{
    uint8_t chksum = 0;
    uint32_t i;
    uint32_t len = pResp->len; /*����chksum*/

    chksum += CALC_U32_DATA_SUM(pResp->pwd);
    chksum += pResp->ack1;
    chksum += pResp->ack2;
    chksum += CALC_U32_DATA_SUM(pResp->comp_code);

    len = pResp->len - 1;
    for(i = 0; i < len; i++)
    {
        chksum += pResp->resp[i];
    }

    pResp->resp[len] = 0 - chksum;
}

/*****************************************************************************
 �� �� ��  : COMMAND_IsChksumOk ��������  : �����У���Ƿ���ȷ
 �������  : PCMD_REQ_DATA pReq
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��28��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_IsChksumOk(P_ML_CMD_REQ_DATA pReq)
{
    uint8_t chksum = 0;
    uint32_t i;
    uint32_t len = pReq->len; /*����chksum*/

    chksum += CALC_U32_DATA_SUM(pReq->pwd);
    chksum += pReq->cmd1;
    chksum += pReq->cmd2;

    for(i = 0; i < len; i++)
    {
        chksum += pReq->req[i];
    }

    /*0x00��ʾУ����ȷ������ʧ��*/
    return chksum;
}


/*****************************************************************************
 �� �� ��  : COMMAND_TransSetReqData ��������  : ����I2C���ݱ��浽Э��ṹ��
 �������  : PCMD_REQ_DATA pReq
             uint8_t data
             uint32_t index
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��26��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
void ML_COMMAND_TransSetReqData(P_ML_CMD_REQ_DATA pReq, uint8_t data, uint32_t index)
{
    uint32_t max_index;

    max_index = sizeof(pReq->req) + CMD_REQ_HEAD_LEN;
    if(index >= max_index)
    {
        ML_COMMAND_PRINT_INFO1("ML_COMMAND_TransSetReqData(1): index = %d.\r\n",index);
        return;
    }

    switch(index)
    {
        case 0:
        pReq->pwd = 0;
        case 1:
        case 2:
        case 3:
        pReq->pwd |= (data << ((3 - index) << 3));
        break;

        case 4:
        pReq->cmd1 = data;
        break;

        case 5:
        pReq->cmd2 = data;
        break;

        default:
        if(index >= max_index)
        {
            ML_COMMAND_PRINT_INFO1("ML_COMMAND_TransSetReqData(2): index = %d.\r\n",index);
        }
        pReq->req[index - CMD_REQ_HEAD_LEN] = data;
        break;
    }
}

/*****************************************************************************
 �� �� ��  : COMMAND_TransGetRespData
 ��������  : ��Э��ṹ���ȡ�������
 �������  : PCMD_RESP_DATA pResp
             uint32_t index
             uint8_t *pdata
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��26��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t ML_COMMAND_TransGetRespData(P_ML_CMD_RESP_DATA pResp, uint32_t index, uint8_t *pdata)
{
    if((P_ML_CMD_RESP_DATA)0 == pResp)
    {
        return 1;
    }

    if(index >= pResp->len + CMD_RESP_HEAD_LEN)
    {
        return 1;
    }

    switch(index)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        *pdata = (pResp->pwd >> ((3 - index) << 3)) & 0xFF; //�ȷ��͸�λ
        break;

        case 4:
        *pdata = pResp->ack1;
        break;

        case 5:
        *pdata = pResp->ack2;
        break;

        case 6:
        case 7:
        case 8:
        case 9:
        *pdata = (pResp->comp_code >> ((9 - index) << 3)) & 0xFF; //�ȷ��͸�λ
        break;

        default:
        *pdata = pResp->resp[index - CMD_RESP_HEAD_LEN];
        break;
    }

    //��ȡ�����һ���ֽڣ������Ӧ����
    if(index == pResp->len + CMD_RESP_HEAD_LEN - 1)
    {
        //ML_COMMAND_SetRespInvalid(pResp->ack1, pResp->ack2);
    }
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_RespStartCmd ��������  : ��Start�������Ӧ
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
             uint8_t comp_code
             uint8_t *pdata
             uint32_t len
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��24��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_RespStartCmd(P_ML_CMD_REQ_DATA pReq, P_ML_CMD_RESP_DATA *ppResp, uint32_t comp_code, uint8_t *pdata, uint32_t len)
{
    if (pdata == NULL)
    {
        pdata = &g_StartChksum;
    }

    g_stStartResp.ack1 = pReq->cmd1;
	g_stStartResp.pwd = g_sysSetting.pwd;
    g_stStartResp.ack2 = pReq->cmd2;
    g_stStartResp.comp_code = comp_code;
    g_stStartResp.resp = pdata;
    g_stStartResp.len = len + 1; //��һ��У����
    g_stStartResp.flag = RESP_FLAG_OK;

    ML_COMMAND_CalcChksum(&g_stStartResp);

    *ppResp = &g_stStartResp;

	//if(pReq->cmd1 == 0x1 && pReq->cmd2 == 0x35)
	//{
		
	//}

    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_RespGetCmd ��������  : ��GetResult�������Ӧ
 �������  : PCMD_REQ_DATA pReq
             uint32_t comp_code
             uint8_t *pdata
             uint32_t len
             uint8_t flag
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��24��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_RespGetCmd(P_ML_CMD_REQ_DATA pReq, uint32_t comp_code, uint8_t *pdata, uint32_t len, uint8_t flag)
{
    if (pdata == NULL)
    {
        pdata = &g_GetResultChksum;
    }

    g_stGetResultResp.ack1 = pReq->cmd1;
	g_stGetResultResp.pwd = g_sysSetting.pwd;
    g_stGetResultResp.ack2 = pReq->cmd2 + CMD_FP_REQ_TO_RESP;
    g_stGetResultResp.comp_code = comp_code;
    g_stGetResultResp.resp = pdata;
    g_stGetResultResp.len = len + 1; //1��ʾchksum
    g_stGetResultResp.flag = flag;

    ML_COMMAND_CalcChksum(&g_stGetResultResp);

    return 0;
}

/************************************************
 *  ML_COMMAND_CmdTypeFpProc()������
 ***********************************************/
static unsigned int ML_COMMAND_RespGetCmdCapture(P_ML_CMD_REQ_DATA pReq)
{
    int ret = 0;
    unsigned int comp_code = 0;
    uint8_t *pbuf, *pRead;
    uint32_t len = 0;
    unsigned short frame = ((pReq->req[0] * 0x100) + pReq->req[1]);

    g_cmdRespData[0] = pReq->req[0];
    g_cmdRespData[1] = pReq->req[1];

    if (0 == frame)
    {
        ret = Sensor_WaitAndCapture(10000);
        switch(ret)
        {
            case SENSOR_COMP_CODE_OK:
                comp_code = COMP_CODE_OK;
                break;

            case SENSOR_COMP_CODE_IRQ_ERROR:
            case SENSOR_COMP_CODE_ID_ERROR:
            case SENSOR_COMP_CODE_RELEASE_ERROR:
            case SENSOR_COMP_CODE_HARDWARE:
            case SENSOR_COMP_CODE_MALLOC:
            case SENSOR_COMP_CODE_CAPTURE:
                comp_code = COMP_CODE_HARDWARE_ERROR;
                break;

            case SENSOR_COMP_CODE_NO_FINGER:
                comp_code = COMP_CODE_NO_FINGER_DECTECT;
                break;

            case SENSOR_COMP_CODE_UNQUALIFIED:
            case SENSOR_COMP_CODE_CALIBRATION_ERROR:    
                comp_code = COMP_CODE_UNQUALIFIED_IMAGE_ERROR;
                break;

            case SENSOR_COMP_CODE_SIZE:
                comp_code = COMP_CODE_ADJUST_GAIN_ERROR;
                break;

            default:
                comp_code = COMP_CODE_OTHER_ERROR;
                break;
        }

    }

    Sensor_GetSpiImgInfo(&pbuf, &len);
    pRead = pbuf + frame * g_imgFrameLen;
    memcpy(&g_cmdRespData[2], pRead, g_imgFrameLen);
    len = g_imgFrameLen + 2;
        
    if(comp_code == COMP_CODE_OK)
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, comp_code, g_cmdRespData, len);
    }
    else
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, comp_code, NULL, 0);
    }
    
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_RegisterFingerStart ��������  : ע��ָ������
 �������  : PCMD_REQ_DATA pReq
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��24��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_RegisterFingerStart(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t comp_code;
    uint8_t progress = 0;
    uint16_t index;
    //g_cmdReqData[0]
  
    comp_code = MLAPI_Enrollment(pReq->req[0], &index, &progress, 2000);

    g_cmdRespData[0] = ((index & 0xFF00) >> 8);     //RegCnt:h
    g_cmdRespData[1] = (index & 0xFF);              //RegCnt:l
    g_cmdRespData[2] = progress;
    ML_COMMAND_RespGetCmd(pReq, comp_code, g_cmdRespData, 3, RESP_FLAG_OK);

	if(comp_code == COMP_CODE_OK)
	{
		//�ȴ���ָ�뿪
		while(MLAPI_QueryFingerPresent()){}
	}
	
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_RegisterStorageFtr  ��������  : ����FTR
 �������  : PCMD_REQ_DATA pReq
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��10��25��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_RegisterStorageFtr(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t comp_code = COMP_CODE_OK;
    uint32_t storage_index = 0xFFFF;

    g_cmdRespData[0] = pReq->req[0];
    g_cmdRespData[1] = pReq->req[1];
    if(STORE_MAX_FTR <= fileSys_getStoreFtrNum())
    {
        comp_code = COMP_CODE_STORAGE_IS_FULL;
        goto out;
    }

    storage_index = (pReq->req[0] << 8)  | (pReq->req[1]);
    comp_code = MLAPI_StorageFtr(storage_index, 0);

out:
    ML_COMMAND_RespGetCmd(pReq, comp_code, g_cmdRespData, 2, RESP_FLAG_OK);
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_MatchFingerStart  ��������  : ָ��ƥ������
 �������  : PCMD_REQ_DATA pReq
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��24��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_MatchFingerStart(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t comp_code;
    uint16_t Fp_Index = 0xFFFF, Result = 0, Score = 0;
	
    comp_code = MLAPI_Match(&Result, &Score, &Fp_Index, 2000);
    //���ƥ�����ͷ���
    g_cmdRespData[0] = 0x00;                   //Result:h
    g_cmdRespData[1] = (Result&0x00FF);
    g_cmdRespData[2] = (Score & 0xFF00) >> 8;    //Score:h
    g_cmdRespData[3] = (Score & 0x00FF);         //Score:l
    g_cmdRespData[4] = (Fp_Index & 0xFF00) >> 8;    //index:h
    g_cmdRespData[5] = (Fp_Index & 0x00FF);        //index:l

    ML_COMMAND_RespGetCmd(pReq, comp_code, g_cmdRespData, 6, RESP_FLAG_OK);

    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_MatchFingerStart_UID
 ��������  : ָ��ƥ������
 �������  : PCMD_REQ_DATA pReq  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��12��24��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_MatchFingerStart_UID(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t comp_code;
    uint16_t Fp_Index = 0xFFFF, Result = 0, Score = 0;

    memset(g_cmdRespData, 0, 14);

	comp_code = MLAPI_Match(&Result, &Score, &Fp_Index, 2000);

    if(comp_code == COMP_CODE_OK)
    {
        //���ƥ�����ͷ���
        g_cmdRespData[0] = 0x00;                        //Result:h
        g_cmdRespData[1] = Result;                      //Result:l
        g_cmdRespData[2] = (Score & 0xFF00) >> 8;       //Score:h
        g_cmdRespData[3] = (Score & 0x00FF);            //Score:l
        g_cmdRespData[4] = (Fp_Index & 0xFF00) >> 8;    //index:h
        g_cmdRespData[5] = (Fp_Index & 0x00FF);         //index:l

        memcpy(&g_cmdRespData[6], g_sysSetting.board_serial_number, 8);
    }
  	
    ML_COMMAND_RespGetCmd(pReq, comp_code, g_cmdRespData, 14, RESP_FLAG_OK);

    return 0;
}


/*****************************************************************************
 �� �� ��  : COMMAND_UpdateFTR
 ��������  : ����ָ������
 �������  : PCMD_REQ_DATA pReq
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��3��3��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_UpdateFTR(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t come_code = 0;
    come_code = MLAPI_UpdateFTR();
    ML_COMMAND_RespGetCmd(pReq, come_code, NULL, 0, RESP_FLAG_OK);
    return 0;
}


/*****************************************************************************
 �� �� ��  : COMMAND_QueryFingerDistribution
 ��������  : ��ѯָ�ƴ洢״̬
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��29��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_QueryFingerDistribution(P_ML_CMD_REQ_DATA pReq)
{
    uint16_t usTotalNum;

    memset(g_cmdRespData, 0, 66);
    usTotalNum = STORE_MAX_FTR;
    fileSys_getIdDistribute( &g_cmdRespData[2]);

    g_cmdRespData[0] = (uint8_t) (usTotalNum >> 8);
    g_cmdRespData[1] = (uint8_t) (usTotalNum);

    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 66);

    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_QueryFingerPresent ��������  : ��ѯ��ָ��λ
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��29��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_QueryFingerPresent(P_ML_CMD_REQ_DATA pReq)
{
    g_cmdRespData[0] = MLAPI_QueryFingerPresent();
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 1);

    return 0;
}

/************************************************
 ************************************************
 ************************************************
 *  ML_COMMAND_CmdTypeSysProc()������
 ************************************************
 ************************************************
 ***********************************************/

/*****************************************************************************
 �� �� ��  : COMMAND_GetTemplateCnt ��������  : ��ȡע���ָ����
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��30��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_GetTemplateCnt(P_ML_CMD_REQ_DATA pReq)
{
    uint16_t NumTemp;
    NumTemp = (uint16_t)fileSys_getStoreFtrNum();
    g_cmdRespData[0] = (NumTemp & 0xff00) >> 8;
    g_cmdRespData[1] = NumTemp & 0xff;

    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 2);
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_GetSensorGain ��������  : ��ȡ����������
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��1��26��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_GetSensorGain(P_ML_CMD_REQ_DATA pReq)
{
    uint8_t shift, gain, pxl;

    Sensor_GetAdcGain(&shift, &gain, &pxl);

    g_cmdRespData[0] = shift;
    g_cmdRespData[1] = gain;
    g_cmdRespData[2] = pxl;

    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 3);
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_GetEnrollMaxNum ��������  : ��ȡע��ָ��ʱ��ƴ�Ӵ���
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��04��03��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_GetEnrollMaxNum(P_ML_CMD_REQ_DATA pReq)
{
    g_cmdRespData[0] = g_sysSetting.alg_enroll_num;

    if(6 < g_cmdRespData[0])
    {
         ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_SYS_SOFT_ERROR, g_cmdRespData, 1);
         return 0;
    }

    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 1);

    return 0;
}

uint32_t ML_COMMAND_SetEnrollMaxNum(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t unEnrollNum = 0;

    ML_COMMAND_PRINT_INFO1("Set MaxNum %d\r\n", pReq->req[0]);

    unEnrollNum = pReq->req[0];

    if ((unEnrollNum >= 1) && (unEnrollNum <= 8))
    {
        SYSSET_SetEnrollNum(unEnrollNum);
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
    }
    else
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_CMD_DATA_ERROR, NULL, 0);
    }

    return 0;
}
/*****************************************************************************
 �� �� ��  : COMMAND_GetSystemPolicy  ��������  : ��ȡϵͳ����
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��4��12��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_GetSystemPolicy(P_ML_CMD_REQ_DATA pReq)
{
    g_cmdRespData[0] = (uint8_t)((g_sysSetting.sys_policy & 0xff000000) >> 24);
    g_cmdRespData[1] = (uint8_t)((g_sysSetting.sys_policy & 0x00ff0000) >> 16);
    g_cmdRespData[2] = (uint8_t)((g_sysSetting.sys_policy & 0x0000ff00) >> 8);
    g_cmdRespData[3] = (uint8_t)(g_sysSetting.sys_policy & 0x000000ff);

    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 4);

    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_SetSystemPolicy
 ��������  : ����ϵͳ����
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��4��5��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_SetSystemPolicy(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t unPolicy = 0;

    if (pReq->len != 5)
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp,COMP_CODE_CMD_DATA_LEN_ERROR, NULL, 0);
        return 1;
    }

    unPolicy = (pReq->req[0] << 24)
           | (pReq->req[1] << 16)
           | (pReq->req[2] << 8)
           | (pReq->req[3]);

    ML_COMMAND_PRINT_INFO1("Set Policy 0x%x.\r\n", unPolicy);

    if (0 == SYSSET_SetSystemPolicy(unPolicy))
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
    }
    else
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_CMD_DATA_ERROR, NULL, 0);
    }
    
    return 0;
}

uint32_t ML_COMMAND_LedControl(P_ML_CMD_REQ_DATA pReq)
{
    if (pReq->len != 6)
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp,COMP_CODE_CMD_DATA_LEN_ERROR, NULL, 0);
        return 1;
    }

    if ( (pReq->req[0] > EM_LED_CTRL_BLINK) || (pReq->req[1] > EM_LED_WHITE) || 
		 (pReq->req[2] > 100) || (pReq->req[3] > 100) )
	{
		ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_CMD_DATA_ERROR, NULL, 0);
        return 1;
	}
    
    EM_LED_COLOR ledColor = EM_LED_NONE;
	unsigned int freq = LED_FREQ_MS(200);
	unsigned short count = pReq->req[4];
	         
    switch((EM_LED_COLOR)pReq->req[1])
    {
        case 0:
            ledColor = EM_LED_NONE;
            break;
            
        case 1:
            ledColor = EM_LED_GREEN;
            break;
            
        case 2:
            ledColor = EM_LED_RED;
            break;
            
        case 3:
            ledColor = EM_LED_YELLOW;
            break;
            
        case 4:
            ledColor = EM_LED_BLUE;
            break;

        case 5:
            ledColor = EM_LED_PINK;
            break;

        case 6: 
            ledColor = EM_LED_CYAN;
            break;

        case 7: 
            ledColor = EM_LED_WHITE;
            break;
    }
	
    if (EM_LED_CTRL_PWM == pReq->req[0])
    {
    	ledPwmModeSet(pReq->req[2], pReq->req[3], pReq->req[4]);
        //ledPwmModeSet(100, 0, 100);
		ledModeSet((EM_LED_CTRL)pReq->req[0], ledColor, freq, 0);
    }
	else
	{
		//��˸���õ�freq��count���������õ�count������ģʽ�ò���freq��count
		ledModeSet((EM_LED_CTRL)pReq->req[0], ledColor, freq, count);
	}
    
    
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);

    return 0;
}


uint32_t ML_COMMAND_PowerSaving(P_ML_CMD_REQ_DATA pReq)
{
  		
    if (pReq->len != 2)
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_CMD_DATA_LEN_ERROR, NULL, 0);
        return 1;
    }
    
    //�������Ͳ��Ϸ�
    if ((SLEEP_NORMAL_MODE != pReq->req[0]) && (SLEEP_DEEP_MODE != pReq->req[0]))
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_CMD_DATA_ERROR, NULL, 0);
        return 1;
    }

    MLAPI_AbortCommand();     //ǿ���˳���ͼ����

    //�����ָ��λ���򱾴���ͨ��������ʧ��
    if ((SLEEP_NORMAL_MODE == pReq->req[0]) && (1 == MLAPI_QueryFingerPresent()))
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_FINGER_PRESENT, NULL, 0);
        APP_PRINT_TRACE0("[cmd] sensor sleep finger is press");
        return 1;
    }

    if(COMP_CODE_OK != MLAPI_PowerSaving(SLEEP_NORMAL_MODE))
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OTHER_ERROR, NULL, 0);
        APP_PRINT_TRACE0("[cmd] sensor sleep fail");
        return 1;
    }
    APP_PRINT_TRACE0("[cmd] sensor success");

	menu_sleep_event_reset(); 
	resume_task(false);
	
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
	
    
    return 0;
}


/************************************************
 *  ML_COMMAND_CmdTypeMtProc()������
 ***********************************************/

/*****************************************************************************
 �� �� ��  : COMMAND_ReadBoardId ��������  : ��ȡ����ID
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��30��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_ReadBoardId(P_ML_CMD_REQ_DATA pReq)
{
    memcpy(g_cmdRespData, g_sysSetting.board_id, 16);
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 16);

    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_WriteBoardId
 ��������  : ����дborad id
 �������  : PCMD_REQ_DATA pReq      
             PCMD_RESP_DATA *ppResp  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��5��13��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_WriteBoardId(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t len;

    if(pReq->len != 17)
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp,COMP_CODE_CMD_DATA_LEN_ERROR, NULL, 0);
        return 1;
    }

    for (len = 0; len < 16; len++)
    {
        if(pReq->req[len] == 0)
        {
            break;
        }
    }
    
    SYSSET_SetBoardId(&pReq->req[0], len);
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_ReadSWVersion
 ��������  : ��ȡ����汾��
 �������  : PCMD_REQ_DATA pReq      
             PCMD_RESP_DATA *ppResp  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��12��30��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_ReadSWVersion(P_ML_CMD_REQ_DATA pReq)
{
    sprintf((char*)g_cmdRespData, "%02d%02d", VERSION_MAJOR, VERSION_BUILDNUM%32);
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 4);
    
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_ReadBoardSN ��������  : ��ȡSN
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��11��8��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_ReadBoardSN(P_ML_CMD_REQ_DATA pReq)
{
    memcpy(g_cmdRespData, &g_sysSetting.board_id[16], 16);
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 16);

    return 0;
}

/*****************************************************************************
 �� �� ��  : ML_COMMAND_FingerIDIsExist 
 ��������  : ��ѯָ��ID�Ƿ����
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :

*****************************************************************************/
uint32_t ML_COMMAND_FingerIDIsExist(P_ML_CMD_REQ_DATA pReq)
{
	uint16_t id = 0;

	id = (pReq->req[0] << 8) | pReq->req[1];
	
	ML_COMMAND_PRINT_INFO1("check ID is %d", id);
	
	if(fileSys_IdToIndex(id) < 0)
	{
		g_cmdRespData[0] = 0;  //������
	}
	else 
	{
		g_cmdRespData[0] = 1; //����
	}
	g_cmdRespData[1] = pReq->req[0];
	g_cmdRespData[2] = pReq->req[1];
	
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 3);

    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_ClearRecord
 ��������  : ���ָ��
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��6��17��
    ��    ��   : li dong
    �޸�����   : �����ɺ���
  2.��    ��   : 2019��7��2��
    ��    ��   : phager
    �޸�����   : ��ͬ���첽д��ͳһ�ӿڣ���ӿ�ɾ������
    
*****************************************************************************/
uint32_t ML_COMMAND_ClearRecord(P_ML_CMD_REQ_DATA pReq)
{
    uint8_t  flag = 0;
	uint8_t syn = 0;
    uint16_t index = 0;
    uint16_t usFPNum = 0;
    uint16_t firstId, lastId;
    uint16_t ausFPIdx[STORE_MAX_FTR] = {0};
    uint16_t ii = 0;
    int ret = 0;
    
    g_FtrUpdateFlag = false;
    g_FtrInfoUpdateFlag = false;
    
	if( CMD_FP_CLEAR_RECORD_SYN == pReq->cmd2 )
	{
		syn = 1;
	}

    flag = pReq->req[0];

    switch (flag)
    {
        case ERASE_SINGLE_FINGER:
            index = (pReq->req[1] << 8) + pReq->req[2];
            ret = fileSys_deleteOneFtr(index);
        break;

        case ERASE_ALL_FINGER:
            ret = fileSys_deleteAllFtr();
        break;
        
        case ERASE_BATCH_FINGER:
            usFPNum = (pReq->req[1] << 8) + pReq->req[2];

            if(usFPNum >= STORE_MAX_FTR)
            {
                usFPNum = STORE_MAX_FTR;
            }
            
            for (ii = 0; ii < usFPNum; ii++)
            {
                ausFPIdx[ii] = (pReq->req[ii*2+3] << 8) + pReq->req[ii*2+4];
            }
            ret = fileSys_deleteBatchFtr(ausFPIdx, usFPNum);
        break;
        
        case ERASE_BLOCK_FINGER:
            firstId = ((pReq->req[1] * 0x100) + pReq->req[2]);
            lastId = ((pReq->req[3] * 0x100) + pReq->req[4]);
            usFPNum = lastId - firstId + 1;

            if(usFPNum >= STORE_MAX_FTR)
            {
                usFPNum = STORE_MAX_FTR;
            }
            
            for(ii = 0; ii < usFPNum; ii++)
            {
                ausFPIdx[ii] = firstId + ii;
            }
            ret = fileSys_deleteBatchFtr(ausFPIdx , usFPNum);
        break;
        
        default:
            ML_COMMAND_RespGetCmd(pReq, COMP_CODE_CMD_DATA_ERROR, NULL, 0, RESP_FLAG_OK);
        return 1;
    }

    if (syn)
    {
        switch (ret)
        {
            case 0:
                ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
            break;

            case FILE_SYSTEM_PARAMETER_ERROR:
                ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_INVALID_FINGERPRINT_ID, NULL, 0);
            break;

            default:
                ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OTHER_ERROR, NULL, 0);
            break;
        }
    }
    else
    {
        switch (ret)
        {
            case 0:
                ML_COMMAND_RespGetCmd(pReq, COMP_CODE_OK, NULL, 0, RESP_FLAG_OK);
            break;
            
            case FILE_SYSTEM_PARAMETER_ERROR:
                ML_COMMAND_RespGetCmd(pReq, COMP_CODE_INVALID_FINGERPRINT_ID, NULL, 0, RESP_FLAG_OK);
            break;
            
            default:
                ML_COMMAND_RespGetCmd(pReq, COMP_CODE_HARDWARE_ERROR, NULL, 0, RESP_FLAG_OK);
            break;
        }
    } 
    return 0;
}


/*****************************************************************************
 �� �� ��  : COMMAND_MatchFingerSyn
 ��������  : ָ��ƥ������(ͬ��)
 �������  : PCMD_REQ_DATA pReq  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��6��16��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_MatchFingerSyn(P_ML_CMD_REQ_DATA pReq)
{
    int32_t ret = 0;
    uint32_t comp_code = 0;
    uint16_t Fp_Index = 0;

    g_cmdRespData[0] = 0x00;  //Result:h
    g_cmdRespData[1] = 0x00;  //Result:l
    g_cmdRespData[2] = 0x00;  //Score:h
    g_cmdRespData[3] = 0x00;  //Score:l
    g_cmdRespData[4] = 0x00;  //index:h
    g_cmdRespData[5] = 0x00;  //index:l

    ret = Sensor_WaitAndCapture(5000);
    switch(ret)
	{
        case SENSOR_COMP_CODE_OK:
            comp_code = COMP_CODE_OK;
            break;

        case SENSOR_COMP_CODE_IRQ_ERROR:
        case SENSOR_COMP_CODE_ID_ERROR:
        case SENSOR_COMP_CODE_RELEASE_ERROR:
        case SENSOR_COMP_CODE_CALIBRATION_ERROR:
        case SENSOR_COMP_CODE_HARDWARE:
        case SENSOR_COMP_CODE_MALLOC:
        case SENSOR_COMP_CODE_CAPTURE:
            comp_code = COMP_CODE_HARDWARE_ERROR;
            break;

        case SENSOR_COMP_CODE_NO_FINGER:
            comp_code = COMP_CODE_NO_FINGER_DECTECT;
            break;

        case SENSOR_COMP_CODE_UNQUALIFIED:
            comp_code = COMP_CODE_UNQUALIFIED_IMAGE_ERROR;
            break;

        case SENSOR_COMP_CODE_SIZE:
            comp_code = COMP_CODE_ADJUST_GAIN_ERROR;
            break;
        
        case SENSOR_COMP_CODE_QUIT:
            comp_code = COMP_CODE_FORCE_QUIT;
            break;

        default:
            comp_code = COMP_CODE_OTHER_ERROR;
            break;
    }

    if(comp_code != COMP_CODE_OK)
    {
        //��ͼʧ�ܣ����ش���
        goto out;
    }

    ret = action_Extract(SENSOR_IMAGE_BUFFER);
    if (ret != 0)
    {
        comp_code = COMP_CODE_FINGER_EXTRACT_ERROR;
        goto out;
    }
   
    ret = action_Match(&Fp_Index);
    if (ret < 0)
    {
        comp_code = COMP_CODE_FINGER_MATCH_ERROR;
        goto out;
    }
    else
    {   
        //MLAPI_UpdateFTR();

        //���ƥ�����ͷ���
        g_cmdRespData[0] = 0x00;                                    //Result:h
        g_cmdRespData[1] = (ret >= g_sysSetting.fp_score) ? 1 : 0;  //Result:l
        g_cmdRespData[2] = (ret & 0xFF00) >> 8;                     //Score:h
        g_cmdRespData[3] = (ret & 0x00FF);                          //Score:l
        g_cmdRespData[4] = (Fp_Index & 0xFF00) >> 8;                //index:h
        g_cmdRespData[5] = (Fp_Index & 0x00FF);                     //index:l

        comp_code = COMP_CODE_OK;
    }

out:    
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, comp_code, g_cmdRespData, 6);

    return 0;
}



/*****************************************************************************
 �� �� ��  : COMMAND_AutoRegisterFinger
 ��������  : �Զ�ע������(������ͼ+��ȡ+��ͼ+ƴ��+����)
 �������  : PCMD_REQ_DATA pReq  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��6��17��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_AutoRegisterFinger(P_ML_CMD_REQ_DATA pReq)
{
    int32_t ret;
    uint32_t comp_code = COMP_CODE_OK;
    uint8_t progress = 0;
    uint16_t storage_index = 0xFFFF;
    //uint16_t Repeat_Fp_Index = 0xFFFF;
    uint32_t unCaptureTimes = 0;	//ע�����
    uint32_t unEnrollTimes = 0;
    uint32_t ii = 0;
	uint8_t check_finger_flag;  //�����ָ�Ƿ��뿪

	g_cmdRespData[0] = 0;
	g_cmdRespData[1] = 0;
	g_cmdRespData[2] = 0;
	g_cmdRespData[3] = 0;

	if(pReq->len != 5)
	{
		comp_code = COMP_CODE_CMD_DATA_LEN_ERROR;
        goto out;
	}

    //ע�����
    unCaptureTimes = pReq->req[1];
    if ((unCaptureTimes < 0x01) || (unCaptureTimes > 0x06))
    {
        comp_code = COMP_CODE_CMD_DATA_ERROR;
        goto out;
    }

	//g_sysSetting.alg_enroll_num = unCaptureTimes;

    //ָ��ID
    storage_index = (pReq->req[2]<<8) + pReq->req[3];
	if((storage_index > STORE_MAX_FTR) && (0xFFFF != storage_index))
	{
		ML_COMMAND_PRINT_INFO1("index:0x%x error.\r\n", storage_index);
        comp_code = COMP_CODE_CMD_DATA_ERROR;
        goto out;
	}
    else if (0xFFFF == storage_index)
    {
		storage_index = fileSys_getUnuseSmallestIndex();
    }

	if(0 == fileSys_checkIDExist(storage_index))
    {
		if(fileSys_getStoreFtrNum() >= STORE_MAX_FTR)
        {
            comp_code = COMP_CODE_STORAGE_IS_FULL;
            goto out;
        }
	}

	g_cmdRespData[1] = ((storage_index & 0xff00) >> 8);
    g_cmdRespData[2] = (storage_index & 0xff);

    /**************************************��ʼ��**************************************/
    //ÿ��ע��ʱ���³�ʼ���㷨
    action_AfisInitEnrollNumIndex(unCaptureTimes, storage_index);
    /*********************************************************************************/

	check_finger_flag = pReq->req[0];
    for (ii = 1; ii <= unCaptureTimes; ii++)
    {
        /*** ��ͼ ***/
		g_cmdRespData[0] = unEnrollTimes + 1;

        ret = Sensor_WaitAndCapture(10000);

        switch (ret)
	    {
	        case SENSOR_COMP_CODE_OK:
	            comp_code = COMP_CODE_OK;
	            break;

	        case SENSOR_COMP_CODE_IRQ_ERROR:
	        case SENSOR_COMP_CODE_ID_ERROR:
	        case SENSOR_COMP_CODE_RELEASE_ERROR:
	        case SENSOR_COMP_CODE_CALIBRATION_ERROR:
	        case SENSOR_COMP_CODE_HARDWARE:
	        case SENSOR_COMP_CODE_MALLOC:
	        case SENSOR_COMP_CODE_CAPTURE:
	            comp_code = COMP_CODE_HARDWARE_ERROR;
	            break;

	        case SENSOR_COMP_CODE_NO_FINGER:
	            comp_code = COMP_CODE_NO_FINGER_DECTECT;
	            break;

	        case SENSOR_COMP_CODE_UNQUALIFIED:
	            comp_code = COMP_CODE_UNQUALIFIED_IMAGE_ERROR;
	            break;

	        case SENSOR_COMP_CODE_SIZE:
	            comp_code = COMP_CODE_ADJUST_GAIN_ERROR;
	            break;
	        
	        case SENSOR_COMP_CODE_QUIT:
	            comp_code = COMP_CODE_FORCE_QUIT;
	            break;

	        default:
	            comp_code = COMP_CODE_OTHER_ERROR;
	            break;
	    }
        if (COMP_CODE_OK != ret)
        {
        	//ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, comp_code, g_cmdRespData, 4);
			//UTRNS_SendFrame_ML(g_pstMLCmdResp);
            ii--;
			goto UartResp;
        }

        /************** ������ȡ **************/
        ret = action_Extract(SENSOR_IMAGE_BUFFER);
        if (ret != 0)
        {
        	comp_code = COMP_CODE_FINGER_EXTRACT_ERROR;
           // ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, comp_code, g_cmdRespData, 4);
			//UTRNS_SendFrame_ML(g_pstMLCmdResp);
            ii--;
			goto UartResp;
        }
	   
        /******************************** ����ƴ�� **********************************/
        ret = action_Enrollment(unEnrollTimes+1, &progress, storage_index);
        switch (ret)
        {
            case PR_OK:
            case PR_ENROLL_OK:
            case PR_ENROLL_COMPLETE:
            case PR_ENROLL_ALMOST_NOMOVE:
                unEnrollTimes++;
                comp_code = COMP_CODE_OK;
                break;

            case PR_ENROLL_LARGE_MOVE:
                comp_code = COMP_CODE_CAPTURE_LARGE_MOVE;
                break;

            case PR_ENROLL_FAIL:
            case PR_ENROLL_BUFFER_OVERFLOW:
            case PR_ENROLL_EXCEEDED:
                comp_code = COMP_CODE_SYS_SOFT_ERROR;
                break;

            case PR_ENROLL_LOW_IMAGE_QUALITY:
                comp_code = COMP_CODE_UNQUALIFIED_IMAGE_ERROR;
                break;

            case PR_ENROLL_LOW_COVERAGE:
                comp_code = COMP_CODE_IMAGE_LOW_COVERAGE_ERROR;
                break;

            default:
                comp_code = COMP_CODE_OTHER_ERROR;
                break;
        }

		if(COMP_CODE_OK != comp_code)
		{
			ii--;
		}
		
		g_cmdRespData[3] = progress;
		
UartResp:
		ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, comp_code, g_cmdRespData, 4);
		UTRNS_SendFrame_ML(g_pstMLCmdResp);			

		if(progress == 100)
		{
			break;
		}
		else if(1 == check_finger_flag)
		{
			while(MLAPI_QueryFingerPresent()){}		
		}
    }
    /*********************************************************************************/

    comp_code = MLAPI_StorageFtr(storage_index, 0);

	g_cmdRespData[0] = 0xff;
	
out:
	g_cmdRespData[1] = ((storage_index & 0xff00) >> 8);
	g_cmdRespData[2] = (storage_index & 0xff);
	g_cmdRespData[3] = progress;

    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, comp_code, g_cmdRespData, 4);

    return 0;
}


/*****************************************************************************
 �� �� ��  :  ML_COMMAND_ReadSerialNumber
 ��������  : ��ȡ�������к�
 �������  : PCMD_REQ_DATA pReq      
             PCMD_RESP_DATA *ppResp  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��12��30��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/

uint32_t ML_COMMAND_ReadSerialNumber(P_ML_CMD_REQ_DATA pReq)
{
    memcpy(g_cmdRespData, g_sysSetting.board_serial_number, 24);
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 24);

    return 0;
}

uint32_t ML_COMMAND_WriteSerialNumber(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t len;
    
    len = pReq->len;
    
    if(len != 25)
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_CMD_DATA_LEN_ERROR, NULL, 0);
        return 1;
    }

    SYSSET_SetBoardSerialNumber(&pReq->req[0], len);
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
    
    return 0;
}


/*****************************************************************************
 �� �� ��  : COMMAND_GetEnrollMaxNum
 ��������  : ����ϵͳ����
 �������  : PCMD_REQ_DATA pReq      
             PCMD_RESP_DATA *ppResp  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��11��13��
    ��    ��   : lance
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_UpdateSystemPara(P_ML_CMD_REQ_DATA pReq)
{
//    SYSSET_UpdateSystemPara(&pReq->req[0]);

    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);

    return 0;
}

/*****************************************************************************
 �� �� ��  : ML_COMMAND_Reboot 
 ��������  : ����ģ��
 �������  : none
 �������  : ��
 �� �� ֵ  :

*****************************************************************************/
void ML_COMMAND_Reboot(void)
{
	WDG_SystemReset(RESET_ALL, SW_RESET_APP_START);
}

/*****************************************************************************
 �� �� ��  : COMMAND_DownloadFTRInfo
 ��������  : �·�������FTR��Ϣ
 �������  : PCMD_REQ_DATA pReq  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
    1.��    ��   : 2017��5��13��
    ��    ��   : li dong
    �޸�����   : �����ɺ���
        
    2.��    ��   : 2019��6��21��
    ��    ��   : phager
    �޸�����   : ����FPM08Xϵ��ģ���FTR����
    

*****************************************************************************/
uint32_t ML_COMMAND_DownloadFTRInfo(P_ML_CMD_REQ_DATA pReq)
{
	uint32_t unCompCode = COMP_CODE_OK;
    uint16_t index = 0;

	index = fileSys_getUnuseSmallestIndex();
    if(index >= STORE_MAX_FTR)
    {
        unCompCode = COMP_CODE_STORAGE_IS_FULL;	
        goto out;
    }

    //��1��2�ֽ�Ϊָ������������
    g_DownloadFTRIndex = pReq->req[0]*0x100 + pReq->req[1];
    //��3��4�ֽ�Ϊָ����������
    g_DownloadFTRLen = pReq->req[2]*0x100 + pReq->req[3];

    if (g_DownloadFTRLen <= FTR_BUFFER_MAX)
    {
        unCompCode = COMP_CODE_OK;
    }
    else
    {
        unCompCode = COMP_CODE_DATA_BUFFER_OVERFLOW;
    }

out:
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, unCompCode, NULL, 0);
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_DownloadFTRData
 ��������  : ����FTR����
 �������  : PCMD_REQ_DATA pReq  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
    1.��    ��   : 2017��5��13��
    ��    ��   : li dong
    �޸�����   : �����ɺ���
    
    2.��    ��   : 2019��6��21��
    ��    ��   : phager
    �޸�����   : ����FPM08Xϵ��ģ���FTR����

*****************************************************************************/
uint32_t ML_COMMAND_DownloadFTRData(P_ML_CMD_REQ_DATA pReq)
{
    uint16_t usFrameNum = 0;
    uint32_t unDataLen = 0;
    uint32_t unSrcCRC = 0;
    uint32_t unCRC = 0;
    int nRet = 0;
    uint16_t id = 0;
    uint32_t FTRLen = 0;

    usFrameNum = pReq->req[0]*0x100 + pReq->req[1];
    unDataLen = pReq->len - 3; //2�ֽ�֡��+1�ֽ�У���

    if (0 == usFrameNum)
    {
        memset(&g_EnrollFtrBuf[0], 0xFF, FTR_BUFFER_MAX);
        unRecievedLen = 0;
    }

    if (unRecievedLen+unDataLen <= FTR_BUFFER_MAX)
    {
        //��������
        memcpy(&g_EnrollFtrBuf[unRecievedLen], &pReq->req[2], unDataLen);
        unRecievedLen += unDataLen;

        //������ϣ���FTR����д��FLASH
        if (g_DownloadFTRLen <= unRecievedLen)
        {
            //��FTR���ݽ���У��
            unSrcCRC = (g_EnrollFtrBuf[g_DownloadFTRLen-1] << 24)
                          | (g_EnrollFtrBuf[g_DownloadFTRLen-2] << 16)
                          | (g_EnrollFtrBuf[g_DownloadFTRLen-3] << 8)
                          | (g_EnrollFtrBuf[g_DownloadFTRLen-4]);

            unCRC = CRC32_calc(g_EnrollFtrBuf, g_DownloadFTRLen-4);
           
            if (unCRC == unSrcCRC)
            {
                id = g_DownloadFTRIndex;
                FTRLen = g_DownloadFTRLen - 4;
                
                nRet = fileSys_storeFtrAndUpdateFtrHead(id , 1, g_EnrollFtrBuf, FTRLen);    //spend time ~= 400ms
                
                switch (nRet)   
                {
                    case 0:
                        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
                    break;

                    case FILE_SYSTEM_PARAMETER_ERROR:
                        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_DATA_BUFFER_OVERFLOW, NULL, 0);
                    break;

                    default:
                        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_STORAGE_WRITE_ERROR, NULL, 0);
                    break;
                }
            }
            else
            {
                ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_STORAGE_WRITE_ERROR, NULL, 0);
            }
        
            unRecievedLen = 0;
        }
        else
        {
            //δ������ɣ���������
            ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
        }
    }
    else
    {
        unRecievedLen = 0;
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_DATA_BUFFER_OVERFLOW, NULL, 0);
    }
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_UploadFTRInfo
 ��������  : ��ȡ���ϴ�FTR��Ϣ
 �������  : PCMD_REQ_DATA pReq  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 

 �޸���ʷ    :
    1.��    ��   : 2017��5��13��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

    2.��    ��   : 2019��6��21��
    ��    ��   : phager
    �޸�����   : ����FPM08Xϵ��ģ���FTR����

*****************************************************************************/
uint32_t ML_COMMAND_UploadFTRInfo(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t unCRC = 0;
    uint32_t unCompCode = COMP_CODE_OK;
    int unIndex = 0;
    int ret = 0;
    uint16_t id = 0;  

    g_cmdRespData[0] = 0;
    g_cmdRespData[1] = 0;
    
    id = pReq->req[0]*0x100 + pReq->req[1];

    //��ָ��IDΪ0xFFFFʱ��ֱ�ӽ������е�FTR�����ϴ�
    if (0xFFFF == id)
    {
        g_UploadFTRLen = g_stEnrollPara.length;

        unCRC = CRC32_calc(g_EnrollFtrBuf , g_UploadFTRLen);
        g_EnrollFtrBuf[g_UploadFTRLen] = (unCRC >> 0) & 0xFF;
        g_EnrollFtrBuf[g_UploadFTRLen+1] = (unCRC >> 8) & 0xFF;
        g_EnrollFtrBuf[g_UploadFTRLen+2] = (unCRC >> 16) & 0xFF;
        g_EnrollFtrBuf[g_UploadFTRLen+3] = (unCRC >> 24) & 0xFF;
    }
    else
    {
        unIndex = fileSys_IdToIndex(id);    
        
        if(0 > unIndex)                     //�ж�FP Index�Ƿ�Ϸ�
        {
            unCompCode = COMP_CODE_INVALID_FINGERPRINT_ID;
            goto out;
        }
        
        g_UploadFTRIndex = (uint16_t)unIndex;    
        
        ret = fileSys_readFtr(g_UploadFTRIndex, g_EnrollFtrBuf, &g_UploadFTRLen);

        if(FILE_SYSTEM_OK != ret)
        {
            unCompCode = COMP_CODE_STORAGE_READ_ERROR;
            goto out;
        }

        unCRC = CRC32_calc(g_EnrollFtrBuf , g_UploadFTRLen);
        g_EnrollFtrBuf[g_UploadFTRLen] = (unCRC >> 0) & 0xFF;
        g_EnrollFtrBuf[g_UploadFTRLen+1] = (unCRC >> 8) & 0xFF;
        g_EnrollFtrBuf[g_UploadFTRLen+2] = (unCRC >> 16) & 0xFF;
        g_EnrollFtrBuf[g_UploadFTRLen+3] = (unCRC >> 24) & 0xFF;
    }

    if ((g_UploadFTRLen + 4) > FTR_BUFFER_MAX)
    {
        unCompCode = COMP_CODE_DATA_BUFFER_OVERFLOW;
        goto out;
    }

    g_UploadFTRLen += 4;
		
    g_cmdRespData[0] = (g_UploadFTRLen & 0xff00) >> 8;
    g_cmdRespData[1] = g_UploadFTRLen & 0x00ff;

out:
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, unCompCode, g_cmdRespData, 2);
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_UploadFTRData
 ��������  : �ϴ�FTR����
 �������  : PCMD_REQ_DATA pReq  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ    :
    1.��    ��   : 2017��5��13��
    ��    ��   : li dong
    �޸�����   : �����ɺ���
    
    2.��    ��   : 2019��6��21��
    ��    ��   : phager
    �޸�����   : ����FPM08Xϵ��ģ���FTR����   

*****************************************************************************/
uint32_t ML_COMMAND_UploadFTRData(P_ML_CMD_REQ_DATA pReq)
{
    uint8_t *pbuf = g_EnrollFtrBuf;
    uint8_t *pRead;
    uint32_t len;
    uint16_t frame;
	
    g_imgReadBuf[0] = pReq->req[0];
    g_imgReadBuf[1] = pReq->req[1];

    frame = (g_imgReadBuf[0] << 8) + g_imgReadBuf[1];
    ML_COMMAND_PRINT_INFO1("frame is %d", frame);
    if (frame * IMG_FRAME_LEN >= FTR_BUFFER_MAX)
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_DATA_BUFFER_OVERFLOW, NULL, 0);
        return 0;
    }
    
    pRead = pbuf + frame * IMG_FRAME_LEN;
    memcpy(&g_imgReadBuf[2], pRead, IMG_FRAME_LEN);
    
    len = IMG_FRAME_LEN + 2;

    g_stGetResultResp.ack1 = pReq->cmd1;
    g_stGetResultResp.ack2 = pReq->cmd2;
    g_stGetResultResp.comp_code = COMP_CODE_OK;
    g_stGetResultResp.resp = g_imgReadBuf;
    g_stGetResultResp.len = len + 1; //1��ʾchksum
    g_stGetResultResp.flag = RESP_FLAG_OK;

    ML_COMMAND_CalcChksum(&g_stGetResultResp);
	memcpy(g_pstMLCmdResp, &g_stGetResultResp, sizeof(g_stGetResultResp));

    return 0;
}

uint32_t COMMAND_BAT_ADC_Test(P_ML_CMD_REQ_DATA pReq)
{
	uint16_t NumTemp;

	driver_adc_start();
	
	os_delay(50);
	NumTemp = (uint16_t)s_VolAvg / 10;
	//QT�����������3.6V��360������Ϊ�쳣������ʱ��ʹ��3.3V���磬�����������ֻ�ܼ�⵽2.8-2.9V
	//������0.3V(30)ѹ����ʵ�ʲ�����Ų�0.4V��������0.4V

	//    NumTemp = 6600000/1024*NumTemp/40000;

	g_cmdRespData[0] = (NumTemp & 0xff00) >> 8;
	g_cmdRespData[1] = NumTemp & 0xff;

	ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 2);
	return 0;
}


uint32_t ML_COMMAND_SetUUID(P_ML_CMD_REQ_DATA pReq)
{
	if(pReq == NULL)
	{
		ML_COMMAND_PRINT_INFO0("ML_COMMAND_SetUUID input param error");
		return 1;
	}

	memset(tuya_ble_current_para.auth_settings.device_id, 0 , sizeof(tuya_ble_current_para.auth_settings.device_id));
	memcpy(tuya_ble_current_para.auth_settings.device_id, pReq->req, sizeof(tuya_ble_current_para.auth_settings.device_id));

	ML_COMMAND_PRINT_INFO1("set uuid is %b", TRACE_BINARY(sizeof(tuya_ble_current_para.auth_settings.device_id), \
						tuya_ble_current_para.auth_settings.device_id));
	ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
	tuya_ble_storage_save_auth_settings();
	
	return 0;
}

uint32_t ML_COMMAND_GetUUID(P_ML_CMD_REQ_DATA pReq)
{
	if(pReq == NULL)
	{
		ML_COMMAND_PRINT_INFO0("ML_COMMAND_GetUUID input param error");
		ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_PARAMETER_ERROR, NULL, 0);
		return 1;
	}

	memcpy(g_cmdRespData, tuya_ble_current_para.auth_settings.device_id, sizeof(tuya_ble_current_para.auth_settings.device_id));
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, sizeof(tuya_ble_current_para.auth_settings.device_id));
	
	return 0;
}

uint32_t ML_COMMAND_SetKey(P_ML_CMD_REQ_DATA pReq)
{
	if(pReq == NULL)
	{
		ML_COMMAND_PRINT_INFO0("ML_COMMAND_SetKey input param error");
		ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_PARAMETER_ERROR, NULL, 0);
		return 1;
	}
	
	memset(tuya_ble_current_para.auth_settings.auth_key, 0 , sizeof(tuya_ble_current_para.auth_settings.auth_key));
	memcpy(tuya_ble_current_para.auth_settings.auth_key, pReq->req, sizeof(tuya_ble_current_para.auth_settings.auth_key));

	ML_COMMAND_PRINT_INFO1("set key is %b", TRACE_BINARY(sizeof(tuya_ble_current_para.auth_settings.auth_key), \
						tuya_ble_current_para.auth_settings.auth_key));
	ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
	tuya_ble_storage_save_auth_settings();
	return 0;
}

uint32_t ML_COMMAND_GetKey(P_ML_CMD_REQ_DATA pReq)
{
	if(pReq == NULL)
	{
		ML_COMMAND_PRINT_INFO0("ML_COMMAND_GetKey input param error");
		ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_PARAMETER_ERROR, NULL, 0);
		return 1;
	}

	memcpy(g_cmdRespData, tuya_ble_current_para.auth_settings.auth_key, sizeof(tuya_ble_current_para.auth_settings.auth_key));
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, sizeof(tuya_ble_current_para.auth_settings.auth_key));
	
	return 0;
}
/*
uint32_t ML_COMMAND_SetMac(P_ML_CMD_REQ_DATA pReq)
{
	if(pReq == NULL)
	{
		ML_COMMAND_PRINT_INFO0("ML_COMMAND_SetMac input param error");
		ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_PARAMETER_ERROR, NULL, 0);
		return 1;
	}

	memset(tuya_ble_current_para.auth_settings.mac_string, 0 , sizeof(tuya_ble_current_para.auth_settings.mac_string));
	memcpy(tuya_ble_current_para.auth_settings.mac_string, pReq->req, sizeof(tuya_ble_current_para.auth_settings.mac_string));

	ML_COMMAND_PRINT_INFO1("set mac is %b", TRACE_BINARY(sizeof(tuya_ble_current_para.auth_settings.mac_string), tuya_ble_current_para.auth_settings.mac_string));
	ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
	tuya_ble_storage_save_auth_settings();
	return 0;
}*/

uint32_t ML_COMMAND_SetMac(P_ML_CMD_REQ_DATA pReq)
{
 if(pReq == NULL)
 {
  ML_COMMAND_PRINT_INFO0("ML_COMMAND_SetMac input param error");
  ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_PARAMETER_ERROR, NULL, 0);
  return 1;
 }
 uint8_t u8Temp[12] = {0};
 tuya_ble_gap_addr_t bt_addr;
  
 memset(tuya_ble_current_para.auth_settings.mac_string, 0 , sizeof(tuya_ble_current_para.auth_settings.mac_string));
 memcpy(tuya_ble_current_para.auth_settings.mac_string, pReq->req, sizeof(tuya_ble_current_para.auth_settings.mac_string));
 tuya_ble_asciitohex(tuya_ble_current_para.auth_settings.mac_string, u8Temp);
 memset(tuya_ble_current_para.auth_settings.mac, 0 , sizeof(tuya_ble_current_para.auth_settings.mac));
 memcpy(tuya_ble_current_para.auth_settings.mac, u8Temp+1, sizeof(tuya_ble_current_para.auth_settings.mac));
 memcpy(bt_addr.addr, u8Temp+1, sizeof(bt_addr.addr));
 
 tuya_ble_inverted_array(bt_addr.addr,6);
 if(tuya_ble_gap_addr_set(&bt_addr)!=TUYA_BLE_SUCCESS)
    {
        APP_PRINT_TRACE0("GAP ADDR SET failed!");
    }
    else
    {
        APP_PRINT_TRACE0("GAP ADDR SET SUCCESSED!");
    }
 
 
 ML_COMMAND_PRINT_INFO1("set mac is %b", TRACE_BINARY(sizeof(tuya_ble_current_para.auth_settings.mac_string), tuya_ble_current_para.auth_settings.mac_string));
 ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
 tuya_ble_storage_save_auth_settings();
 return 0;
}

uint32_t ML_COMMAND_GetMac(P_ML_CMD_REQ_DATA pReq)
{
	if(pReq == NULL)
	{
		ML_COMMAND_PRINT_INFO0("ML_COMMAND_GetMac input param error");
		ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_PARAMETER_ERROR, NULL, 0);
		return 1;
	}

	memcpy(g_cmdRespData, tuya_ble_current_para.auth_settings.mac_string, sizeof(tuya_ble_current_para.auth_settings.mac_string));
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, sizeof(tuya_ble_current_para.auth_settings.mac_string));

	return 0;
}


/*****************************************************************************
 �� �� ��  : ML_COMMAND_MotorControl
 ��������  : ������Խӿ�
 �������  : PCMD_REQ_DATA pReq
 �������  : ��
 �� �� ֵ  :

*****************************************************************************/
uint32_t ML_COMMAND_MotorControl(P_ML_CMD_REQ_DATA pReq)
{
	uint8_t flag = pReq->req[0];
	switch (flag)
	{
		case 1:	//��ת
			driver_motor_control(EM_MOTOR_CTRL_ON, 0xFFFFFFFF);
			break;
			
		case 2:  //��ת
			driver_motor_control(EM_MOTOR_CTRL_BACK, 0xFFFFFFFF);
			break;
		
		default:  //ֹͣ
			driver_motor_control(EM_MOTOR_CTRL_OFF, 0xFFFFFFFF);
			break;
		
	}
	g_cmdRespData[0] = flag;
 	ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 1);
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_SetProtoPwd
 ��������  : ����ͨ������
 �������  : PCMD_REQ_DATA pReq      
             PCMD_RESP_DATA *ppResp  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��5��13��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_SetProtoPwd(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t pwd = 0;

    if (pReq->len != 5)
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp,COMP_CODE_CMD_DATA_LEN_ERROR, NULL, 0);
        return 1;
    }

   pwd = (pReq->req[0] << 24)
          | (pReq->req[1] << 16)
          | (pReq->req[2] << 8)
          | (pReq->req[3]);
    
    if (0 == SYSSET_SetProtoPwd(pwd))
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
    }
    else
    {
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_CMD_DATA_ERROR, NULL, 0);
    }
    
    return 0;
}


/*****************************************************************************
 �� �� ��  : COMMAND_CmdTypeFpProc
 ��������  : ָ���������
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��29��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
void ML_COMMAND_CmdTypeFpProc(P_ML_CMD_REQ_DATA pReq)
{
    uint32_t ret;
	
    switch(pReq->cmd2)
    {
        case CMD_FP_CAPTURE_START:
        case CMD_FP_TEST_CAPTURE_START:
            {
                uint8_t *pbuf;
                uint32_t len;
				suspend_task();
                Sensor_GetSpiImgInfo(&pbuf, &len);
                g_cmdRespData[0] = ((len & 0xFF00) >> 8);
                g_cmdRespData[1] = len & 0xFF;

                ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, g_cmdRespData, 2);
            }
            break;

        case CMD_FP_CAPTURE_READ_DATA:
            ML_COMMAND_RespGetCmdCapture(pReq);
            break;

		case CMD_FP_UPLOAD_FTR_INFO:
        	ML_COMMAND_UploadFTRInfo(pReq);
        	break;

        case CMD_FP_UPLOAD_FTR_DATA:
        	ML_COMMAND_UploadFTRData(pReq);
        	break;

		case CMD_FP_DOWNLOAD_FTR_INFO:
        	ML_COMMAND_DownloadFTRInfo(pReq);
        	break;

        case CMD_FP_MATCH_START:
        case CMD_FP_MATCH_START_UID:
        case CMD_FP_REGISTER_START:
        case CMD_FP_STORAGE_FTR_START:
        case CMD_FP_CLEAR_RECORD_START:
		case CMD_FP_DOWNLOAD_FTR_DATA:
            ret = ML_COMMAND_Post(pReq);
            ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, ret, NULL, 0);
            break;

        case CMD_FP_STORAGE_FTR_UPDATE:
            if ((g_FtrInfoUpdateFlag == g_FtrUpdateFlag) || (!(g_sysSetting.sys_policy & SYS_POLICY_SELF_LEARN)))
            {
                ML_COMMAND_PRINT_INFO0("un\r\n");
                ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp ,COMP_CODE_NONE_UPDATE , NULL , 0);
            }
            else
            {         
                ret = ML_COMMAND_Post(pReq);
                ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, ret, NULL, 0);
            }
	
            break;
    
        case CMD_FP_MATCH_GET_RESULT:
        case CMD_FP_MATCH_GET_RESULT_UID:   
        case CMD_FP_REGISTER_GET_RESULT:
        case CMD_FP_STORAGE_FTR_GET_RESULT:
        case CMD_FP_CLEAR_RECORD_GET_RESULT:
        case CMD_FP_STORAGE_FTR_UPDATE_GET_RESULT:
            if(1 == ML_COMMAND_IsRespInvalid(pReq))
            {
                ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_NO_REQ_CMD, NULL, 0);
            }
            else
            {
                g_pstMLCmdResp = &g_stGetResultResp;
            }				
            break;
            
        case CMD_FP_REGISTER_CANCEL:			
            ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
            break;

        case CMD_FP_QUERY_FINGER_DISTRIBUTION:
            ML_COMMAND_QueryFingerDistribution(pReq);
            break;

        case CMD_FP_QUERY_FINGER_PRESENT:
            ML_COMMAND_QueryFingerPresent(pReq);
            break;

		case CMD_FP_QUERY_FINGER_EXIST: //��ѯָ��ID�Ƿ����
			ML_COMMAND_FingerIDIsExist(pReq);
			break;

		case CMD_FP_AUTO_REGISTER:	//�Զ�ע��
			ML_COMMAND_AutoRegisterFinger(pReq);
			break;
			
		case CMD_FP_MATCH_SYN:   //ƥ������
			ML_COMMAND_MatchFingerSyn(pReq);
			break;
		
		case CMD_FP_CLEAR_RECORD_SYN: //���ָ��
			ML_COMMAND_ClearRecord(pReq);
			break;
        
        default:
            ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_UNKOWN_CMD, NULL, 0);
			resume_task(false);
            break;
    }


}

/*****************************************************************************
 �� �� ��  : COMMAND_CmdTypeSysProc
 ��������  : ϵͳ�������
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��29��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
void ML_COMMAND_CmdTypeSysProc(P_ML_CMD_REQ_DATA pReq)
{
    switch(pReq->cmd2)
    {
    	case CMD_SYS_REBOOT:
			ML_COMMAND_Reboot();	
			break;
		
        case CMD_SYS_GET_TEMPLATE_CNT:
            ML_COMMAND_GetTemplateCnt(pReq);
            break;

        case CMD_SYS_GET_SENSOR_GAIN:
            ML_COMMAND_GetSensorGain(pReq);
            break;

        case CMD_SYS_GET_ENROLL_NUM:
            ML_COMMAND_GetEnrollMaxNum(pReq);
            break;

        case CMD_SYS_SET_ENROLL_MAX_NUM:
            ML_COMMAND_SetEnrollMaxNum(pReq);
            break;

        case CMD_SYS_GET_POLICY:
            ML_COMMAND_GetSystemPolicy(pReq);
            break;

        case CMD_SYS_SET_POLICY:
            ML_COMMAND_SetSystemPolicy(pReq);
            break;
        
        case CMD_SYS_SET_LED_CTRL_INFO:
            ML_COMMAND_LedControl(pReq);
            break;
        
        case CMD_SYS_POWER_SAVING:
            ML_COMMAND_PowerSaving(pReq);
            break;
        
        default:
            ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_UNKOWN_CMD, NULL, 0);
            break;
    }
}

/*****************************************************************************
 �� �� ��  : COMMAND_CmdTypeMtProc
 ��������  : ά���������
 �������  : PCMD_REQ_DATA pReq
             PCMD_RESP_DATA *ppResp
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��12��29��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
void ML_COMMAND_CmdTypeMtProc(P_ML_CMD_REQ_DATA pReq)
{
    switch(pReq->cmd2)
    {
        case CMD_MT_READ_ID:
	        ML_COMMAND_ReadBoardId(pReq);
	        break;
        
        case CMD_MT_WRITE_ID:
	        ML_COMMAND_WriteBoardId(pReq);
	        break;
        
        case CMD_MT_HEARTBEAT:
	        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);
	        break;
	
		case CMD_MT_SET_PROTO_PWD: //����ͨѶ����
			ML_COMMAND_SetProtoPwd(pReq);
			break;

        case CMD_MT_GET_SW_VERSION:
	        ML_COMMAND_ReadSWVersion(pReq);
	        break;
        
        case CMD_MT_READ_SN:
        	ML_COMMAND_ReadBoardSN(pReq);
        	break;

		case CMD_MT_MOTOR_TEST: //�����������
			ML_COMMAND_MotorControl(pReq);
			break;

		case CMD_MT_BAT_ADC_TEST:
			COMMAND_BAT_ADC_Test(pReq);
			break;
    
        case CMD_MT_READ_BOARD_SERIES_NUM:
	        ML_COMMAND_ReadSerialNumber(pReq);
	        break;
        
        case CMD_MT_WRITE_BOARD_SERIES_NUM:
	        ML_COMMAND_WriteSerialNumber(pReq);
	        break;
		
		case CMD_TUYA_UUID_SET:
			ML_COMMAND_SetUUID(pReq);
			break;

		case CMD_TUYA_UUID_GET:
			ML_COMMAND_GetUUID(pReq);
			break;

		case CMD_TUYA_KEY_SET:
			ML_COMMAND_SetKey(pReq);
			break;

		case CMD_TUYA_KEY_GET:
			ML_COMMAND_GetKey(pReq);
			break;

		case CMD_TUYA_MAC_SET:
			ML_COMMAND_SetMac(pReq);
			break;

		case CMD_TUYA_MAC_GET:
			ML_COMMAND_GetMac(pReq);
			break;
        
        default:
	        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_UNKOWN_CMD, NULL, 0);
	        break;
    }
}

uint32_t ML_COMMAND_DownloadStart(P_ML_CMD_REQ_DATA pReq)
{
	uint16_t deleteId = 0;

	MLAPI_DeleteFTR(ERASE_ALL_FINGER, 0, &deleteId);	
		
    ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_OK, NULL, 0);

	os_delay(200);
	switch_to_test_mode(UPGRADE_MODE);
	WDG_SystemReset(RESET_ALL_EXCEPT_AON, RESET_REASON_HW);
	
    return 0;
}



/*****************************************************************************
 �� �� ��  : COMMAND_CmdTypeUpdateProc
 ��������  : �����������
 �������  : PCMD_REQ_DATA pReq      
             PCMD_RESP_DATA *ppResp  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2017��1��6��
    ��    ��   : li dong
    �޸�����   : �����ɺ���

*****************************************************************************/
void ML_COMMAND_CmdTypeUpdateProc(P_ML_CMD_REQ_DATA pReq)
{
	int index = 0;
	uint8_t send_data[10] = {0};
	uint32_t file_len = 0;
	uint16_t file_crc = 0;
    switch(pReq->cmd2)
    {
        case CMD_UPDATE_START_DOWNLOAD:        
        ML_COMMAND_DownloadStart(pReq);
        break;
        
        case CMD_UPDATE_EXCHANGE:        
//        ML_COMMAND_ExchangeDownloadStart(pReq, ppResp);
        break;

		case CMD_UPDATA_TRANSFER_FILE:
		file_len = (pReq->req[0] << 24) | (pReq->req[1] << 16) | (pReq->req[2] << 8) | pReq->req[3];
		file_crc = (pReq->req[4] << 8) | pReq->req[5];
		APP_PRINT_INFO3("file_len is %d(%x), file_crc is 0x%x", file_len, file_len, file_crc);
		send_data[0] = (index >> 8) & 0xFF;
		send_data[1] = index & 0xFF; 
		request_upgrade_cmd(0x03, send_data, 2);	
		break;

		case CMD_UPDATA_TRANS_COMPLETE:
		if(index < 16)
		{
			index++;
			request_upgrade_cmd(0x03, send_data, 2);
		}
		
		break;

		case CMD_UPDATA_SYS_PARA:
		ML_COMMAND_UpdateSystemPara(pReq);
		break;

        default:
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_UNKOWN_CMD, NULL, 0);
        break;
    }
}


uint32_t ML_COMMAND_ReqProc(P_ML_CMD_REQ_DATA pReq)
{
    if (NULL == pReq)
    {
        return 1;
    }
    //֡������
    if (pReq->pwd != g_sysSetting.pwd)
    {
        ML_COMMAND_PRINT_INFO2("Frame head error 0x%08x -> 0x%08x.\r\n", g_sysSetting.pwd, pReq->pwd);
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_PASSWORD_ERROR, NULL, 0);
        if (((CMD_TYPE_FINGERPRINT == pReq->cmd1) && (CMD_FP_MATCH_SYN == pReq->cmd2))
        || ((CMD_TYPE_FINGERPRINT == pReq->cmd1) && (CMD_FP_CLEAR_RECORD_SYN == pReq->cmd2))
        || ((CMD_TYPE_FINGERPRINT == pReq->cmd1) && (CMD_FP_AUTO_REGISTER == pReq->cmd2)))
        {
            UTRNS_SendFrame_ML(g_pstMLCmdResp);
        }
        return 0;
    }

    //֡У����
    if (0x00 != ML_COMMAND_IsChksumOk(pReq))
    {
        ML_COMMAND_PRINT_INFO0("Frame chksum error.\r\n");
        ML_COMMAND_RespStartCmd(pReq, &g_pstMLCmdResp, COMP_CODE_CHECKSUM_ERROR, NULL, 0);
        if (((CMD_TYPE_FINGERPRINT == pReq->cmd1) && (CMD_FP_MATCH_SYN == pReq->cmd2))
        || ((CMD_TYPE_FINGERPRINT == pReq->cmd1) && (CMD_FP_CLEAR_RECORD_SYN == pReq->cmd2))
        || ((CMD_TYPE_FINGERPRINT == pReq->cmd1) && (CMD_FP_AUTO_REGISTER == pReq->cmd2)))
        {
            UTRNS_SendFrame_ML(g_pstMLCmdResp);
        }
        return 0;
    }
	
    //APP_PRINT_INFO2("cmd1: 0x%x, cmd2: 0x%x \r\n",pReq->cmd1,pReq->cmd2);
    
    switch (pReq->cmd1)
    {
        case CMD_TYPE_FINGERPRINT:
        ML_COMMAND_CmdTypeFpProc(pReq);
        break;

        case CMD_TYPE_SYSTEM:
        ML_COMMAND_CmdTypeSysProc(pReq);
        break;

        case CMD_TYPE_MAINTAINCE:
        ML_COMMAND_CmdTypeMtProc(pReq);
        break;

        case CMD_TYPE_UPDATE:
        ML_COMMAND_CmdTypeUpdateProc(pReq);
        break;

        default:
        ML_COMMAND_RespStartCmd(pReq,  &g_pstMLCmdResp, COMP_CODE_UNKOWN_CMD, NULL, 0);
        break;
    }
 
    UTRNS_SendFrame_ML(g_pstMLCmdResp);
	
    return 0;
}

/*****************************************************************************
 �� �� ��  : COMMAND_IsRespInvalid
 ��������  : �ж���Ӧ��Ϣ�Ƿ����
 �������  : PCMD_REQ_DATA pReq  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��12��24��
    ��    ��   : jack
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t ML_COMMAND_IsRespInvalid(P_ML_CMD_REQ_DATA pReq)
{
    if(g_stGetResultResp.flag == RESP_FLAG_NULL)
    {
        ML_COMMAND_PRINT_INFO1("resp flag error:%d\r\n", g_stGetResultResp.flag);
        return 1;
    }

    if((g_stGetResultResp.ack1 != pReq->cmd1)
        ||(g_stGetResultResp.ack2 != pReq->cmd2))
    {
        ML_COMMAND_PRINT_INFO2("resp cmd error cmd1:%d, cmd2:%d\r\n", g_stGetResultResp.ack1, g_stGetResultResp.ack2);
        return 1;
    }

    return 0;
}

uint32_t ML_COMMAND_Post(P_ML_CMD_REQ_DATA pReq)
{
	int i;

	//���������æ�͵ȴ�һ����
	for(i = 0; i < 10; i++)
	{
		if(g_RunningFlag == false)
	    {
	        if(os_sem_take(command_resp_sem_handle, 0) == false)
	        {
	            ML_COMMAND_PRINT_INFO0("resp sem take fail");
				if(i == 9)
	            	return COMP_CODE_CMD_NOT_FINISHED;
				else
					os_delay(100);
	        }
			else
			{
				break;
			}
	    }
	    else
	    {
	        ML_COMMAND_PRINT_INFO0("g_RunningFlag state is busy");
	        if(i == 9)
            	return COMP_CODE_CMD_NOT_FINISHED;
			else
				os_delay(100);
	    }
	}
    
    
    protocol_command_cpy(pReq);
    
    T_MENU_MSG io_uart_main_msg;
    io_uart_main_msg.type = MENU_MSG_TYPE_TASK;
    io_uart_main_msg.subtype = IO_MSG_UART_RX;
    if(false == menu_task_msg_send(&io_uart_main_msg))
    {
        APP_PRINT_INFO0("io_uart_background_task_deal send msg to main task fail");
    }
    
    os_sem_give(command_req_sem_handle);
    
    return COMP_CODE_OK;
}


void ML_COMMAND_ReqDeal(P_ML_CMD_REQ_DATA pReq)
{
    uint8_t cmd;
    cmd = pReq->cmd2; 

    ML_COMMAND_RespGetCmd(pReq, COMP_CODE_CMD_NOT_FINISHED, NULL, 0, RESP_FLAG_RUNNING);
    
    switch(cmd)
    {
        case CMD_FP_CAPTURE_START:
        
        break;
        
        case CMD_FP_REGISTER_START:
            ML_COMMAND_RegisterFingerStart(pReq);
			
        break;
        
        case CMD_FP_STORAGE_FTR_START:
            ML_COMMAND_RegisterStorageFtr(pReq);
        break;
        
        case CMD_FP_MATCH_START:
            ML_COMMAND_MatchFingerStart(pReq);			
        break;
        
        case CMD_FP_MATCH_START_UID:
            ML_COMMAND_MatchFingerStart_UID(pReq);
        break;
        
        case CMD_FP_CLEAR_RECORD_START:
            ML_COMMAND_ClearRecord(pReq);
        break;
        
        case CMD_FP_STORAGE_FTR_UPDATE:
            ML_COMMAND_UpdateFTR(pReq);
        break;

		case CMD_FP_DOWNLOAD_FTR_DATA:
        	ML_COMMAND_DownloadFTRData(pReq);
			break;
    }
	
}


#endif /* APPLICATION_PROTOCOL_COMMAND_C_ */
