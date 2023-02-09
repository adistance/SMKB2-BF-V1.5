/*
 * mlapi.c
 *
 *  Created on: 2020��8��17��
 *      Author: sks
 */
#include "trace.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "string.h"
#include "stdlib.h"

#include "mlapi.h"
#include "fpsapi.h"
#include "fpsapierrors.h"
#include "driver_sensor.h"
#include "driver_flash.h"
#include "filesys.h"
#include "action.h"
#include "system_setting.h"
#include "driver_led.h"
#include "app_task.h"
#include "action.h"

#if 0
#define MLAPI_PRINT_INFO0   APP_PRINT_TRACE0
#define MLAPI_PRINT_INFO1   APP_PRINT_TRACE1
#define MLAPI_PRINT_INFO2   APP_PRINT_TRACE2
#define MLAPI_PRINT_INFO3   APP_PRINT_TRACE3
#define MLAPI_PRINT_INFO4   APP_PRINT_TRACE4
#else
#define MLAPI_PRINT_INFO0(...)
#define MLAPI_PRINT_INFO1(...)
#define MLAPI_PRINT_INFO2(...)
#define MLAPI_PRINT_INFO3(...)
#define MLAPI_PRINT_INFO4(...)
#endif

#define FCFG1_BASE              0x50001000  // FCFG1
#define FCFG2_BASE              0x50002000  // FCFG2

#define FCFG1_O_MAC_BLE_0       0x000002E8  // MAC BLE Address 0
#define FCFG1_O_MAC_BLE_1       0x000002EC  // MAC BLE Address 1
#define HWREG(x)                (*((volatile unsigned long *)(x)))

#define UNUSED_PARA(x) ((x)=(x))
extern uint8_t g_QuitAtOnce;

extern uint32_t get_cpu_clock(void);

void fingerprint_algoInit(void)
{
    AfisParams_t stParam;
//    AlgoGlobalInit();  //ϵͳ��ʼ��ʱ��Ҫ����һ�Σ������㷨sram��ʼ��
    GetDefaultAfisParams(&stParam);

	MLAPI_PRINT_INFO2("nMaxFtrSize is %d, nMaxSetNum is %d", stParam.nMaxFtrSize, stParam.nMaxSetNum);
	MLAPI_PRINT_INFO1("CalcMaxFtrSize(14) %d", CalcMaxFtrSize(21));
	MLAPI_PRINT_INFO1("CalcMaxFtrSize(12) %d", CalcMaxFtrSize(20));
	MLAPI_PRINT_INFO1("CalcMaxFtrSize(13) %d", CalcMaxFtrSize(22));

	stParam.nMaxSetNum = 21;
	stParam.nMaxFtrSize = CalcMaxFtrSize(stParam.nMaxSetNum);
	MLAPI_PRINT_INFO2("nMaxFtrSize is %d, nMaxSetNum is %d", stParam.nMaxFtrSize, stParam.nMaxSetNum);
	
    SetAfisParams(&stParam);
	
}

uint32_t MLAPI_SystemInit(void)
{
	//driver_flash_read_id(MF_DEVICE_ID);
	fingerprint_algoInit();
    SYSSET_Init();
    fileSys_Init();

	AlgoGlobalInit();

	action_AfisInit(DEFAULT_ALG_ENROLL_NUM);
    
    return COMP_CODE_OK;
}

/*****************************************************************************
 �� �� ��: MLAPI_Enrollment
 ��������  : ע��ָ������
 �������  : uint8_t uEnrollNum
            **��ǰע�����
 �������  : uint16_t *uUnuseId
            **return COMP_CODE_OK���ȡ��ǰ��Сδʹ��ID
            **return COMP_CODE_STORAGE_REPEAT_FINGERPRINT���ȡ���ظ�ID
            uint8_t *uProgress ע�����
            uint16_t usTimeout ��ʱʱ��
            **return ok���ȡ��ǰע�����

 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ    :
  1.��    ��  : 2019��08��20��
    ��    ��  : Alex
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t MLAPI_Enrollment(uint8_t uEnrollNum, uint16_t *uUnuseId, uint8_t *uProgress, uint16_t usTimeout)
{
    int32_t ret;
    uint32_t comp_code;
    uint16_t Fp_Index = 0xFFFF;
    g_FtrUpdateFlag = false;

    *uUnuseId = fileSys_getUnuseSmallestIndex();
    if(*uUnuseId >= STORE_MAX_FTR)
    {
        comp_code = COMP_CODE_STORAGE_IS_FULL;
        goto out;
    }

    MLAPI_PRINT_INFO2("[MLAPI] ApiRegisterFinger uEnrollNum:%d, uUnuseId:%d\r\n", uEnrollNum, *uUnuseId);

    if(uEnrollNum < 0x01)
    {
        MLAPI_PRINT_INFO0("[MLAPI] if(uEnrollNum < 0x01)\r\n");
        comp_code = COMP_CODE_CMD_DATA_ERROR;
        goto out;
    }

    //��һ��ע��
    if (1 == uEnrollNum)
    {
        action_AfisInit_FirstEnroll(*uUnuseId);
    }

    /*********** �ɼ�ָ��ͼƬ *************/
    ret = Sensor_WaitAndCapture(usTimeout);
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

    if(ret != SENSOR_COMP_CODE_OK)
    {
        MLAPI_PRINT_INFO1("[MLAPI] Sensor_WaitAndCapture Sensor_WaitAndCapture ret:%d", ret);
        goto out;
    }

   
    /************** ������ȡ **************/
    ret = action_Extract(SENSOR_IMAGE_BUFFER);
    if (ret != 0)
    {
        MLAPI_PRINT_INFO1("[MLAPI] Sensor_WaitAndCapture action_Extract ret:%d", ret);
        comp_code = COMP_CODE_FINGER_EXTRACT_ERROR;
        goto out;
    }
    
    /************** �ж��ظ�ָ�� *************/
    if (0 != (g_sysSetting.sys_policy & SYS_POLICY_REPEAT_CHECK))
    {
        memcpy(g_ImgBuf, g_stEnrollPara.pFeature, g_stEnrollPara.length);

        ret = action_MatchEx(&Fp_Index);
        if (ret < g_algMatchScoreThres)
        {
            //MLAPI_PRINT_INFO0(("Not Match,new one\r\n"));
        }
        else
        {
            MLAPI_PRINT_INFO1("[MLAPI] Match,same as FP%d\r\n", Fp_Index);
            *uUnuseId = Fp_Index;
            comp_code = COMP_CODE_STORAGE_REPEAT_FINGERPRINT;
            memcpy(g_stEnrollPara.pFeature, g_ImgBuf, g_stEnrollPara.length);
            goto out;
        }

        memcpy(g_stEnrollPara.pFeature, g_ImgBuf, g_stEnrollPara.length);

    }

    /************* ����ƴ�� **************/
    ret = action_Enrollment(uEnrollNum, uProgress, *uUnuseId);
    switch (ret)
    {
        case PR_OK:
        case PR_ENROLL_OK:
        case PR_ENROLL_COMPLETE:
            comp_code = COMP_CODE_OK;
            break;

        case PR_ENROLL_LARGE_MOVE:
            comp_code = COMP_CODE_CAPTURE_LARGE_MOVE;
            break;

        case PR_ENROLL_ALMOST_NOMOVE:
            comp_code = COMP_CODE_CAPTURE_NO_MOVE;
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

        case PR_ENROLL_LOW_MOISTNESS:
            comp_code = COMP_CODE_ENROLL_LOW_MOISTNESS;
            break;

        default:
            comp_code = COMP_CODE_OTHER_ERROR;
            break;
    }

out:
    return comp_code;
}

/*****************************************************************************
 �� �� ��: MLAPI_EditFP
 ��������  : �޸��Ѿ�ע���ָ��
 �������  : uint8_t uEnrollNum
            **��ǰע�����
 �������  : uint16_t *uUnuseId ������Ҫ�޸ĵ�ָ��ID����0��ʼ
            **return COMP_CODE_OK���ȡ��ǰ��Сδʹ��ID
            **return COMP_CODE_STORAGE_REPEAT_FINGERPRINT���ȡ���ظ�ID
            uint8_t *uProgress ע�����
            uint16_t usTimeout ��ʱʱ��
            **return ok���ȡ��ǰע�����

 �� �� ֵ: uint32_t 32 bits Error Code

*****************************************************************************/
uint32_t MLAPI_EditFP(uint8_t uEnrollNum, uint16_t *uUnuseId, uint8_t *uProgress, uint16_t usTimeout)
{
    int32_t ret;
    uint32_t comp_code;
    uint16_t Fp_Index = 0xFFFF;
    g_FtrUpdateFlag = false;


    MLAPI_PRINT_INFO2("[MLAPI] MLAPI_EditFP uEnrollNum:%d, uUnuseId:%d\r\n", uEnrollNum, *uUnuseId);

    if(uEnrollNum < 0x01)
    {
        MLAPI_PRINT_INFO0("[MLAPI] if(uEnrollNum < 0x01)\r\n");
        comp_code = COMP_CODE_CMD_DATA_ERROR;
        goto out;
    }

    //��һ��ע��
    if (1 == uEnrollNum)
    {
        action_AfisInit_FirstEnroll(*uUnuseId);
    }

    /*********** �ɼ�ָ��ͼƬ *************/
    ret = Sensor_WaitAndCapture(usTimeout);
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

    if(ret != SENSOR_COMP_CODE_OK)
    {
        MLAPI_PRINT_INFO1("[MLAPI] Sensor_WaitAndCapture Sensor_WaitAndCapture ret:%d", ret);
        goto out;
    }

   
    /************** ������ȡ **************/
    ret = action_Extract(SENSOR_IMAGE_BUFFER);
    if (ret != 0)
    {
        MLAPI_PRINT_INFO1("[MLAPI] Sensor_WaitAndCapture action_Extract ret:%d", ret);
        comp_code = COMP_CODE_FINGER_EXTRACT_ERROR;
        goto out;
    }
    
    /************** �ж��ظ�ָ�� *************/
    if (0 != (g_sysSetting.sys_policy & SYS_POLICY_REPEAT_CHECK))
    {
        memcpy(g_ImgBuf, g_stEnrollPara.pFeature, g_stEnrollPara.length);

        ret = action_MatchEx(&Fp_Index);
        if (ret < g_algMatchScoreThres)
        {
            //MLAPI_PRINT_INFO0(("Not Match,new one\r\n"));
        }
        else
        {
            MLAPI_PRINT_INFO1("[MLAPI] Match,same as FP%d\r\n", Fp_Index);
            *uUnuseId = Fp_Index;
            comp_code = COMP_CODE_STORAGE_REPEAT_FINGERPRINT;
            memcpy(g_stEnrollPara.pFeature, g_ImgBuf, g_stEnrollPara.length);
            goto out;
        }

        memcpy(g_stEnrollPara.pFeature, g_ImgBuf, g_stEnrollPara.length);

    }

    /************* ����ƴ�� **************/
    ret = action_Enrollment(uEnrollNum, uProgress, *uUnuseId);
    switch (ret)
    {
        case PR_OK:
        case PR_ENROLL_OK:
        case PR_ENROLL_COMPLETE:
            comp_code = COMP_CODE_OK;
            break;

        case PR_ENROLL_LARGE_MOVE:
            comp_code = COMP_CODE_CAPTURE_LARGE_MOVE;
            break;

        case PR_ENROLL_ALMOST_NOMOVE:
            comp_code = COMP_CODE_CAPTURE_NO_MOVE;
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

        case PR_ENROLL_LOW_MOISTNESS:
            comp_code = COMP_CODE_ENROLL_LOW_MOISTNESS;
            break;

        default:
            comp_code = COMP_CODE_OTHER_ERROR;
            break;
    }

out:
    return comp_code;
}


/*****************************************************************************
 �� �� ��: MLAPI_StorageFtr
 ��������  : ����ָ������
 �������  : uint16_t uStorageId
            **��Ҫ�����ID
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ  :
  1.��    ��  : 2019��08��20��
    ��    ��  : Alex
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t MLAPI_StorageFtr(uint16_t uStorageId, uint16_t u16NumId)
{
    int32_t ret;
    uint32_t comp_code = COMP_CODE_OK;
    MLAPI_PRINT_INFO0("[MLAPI] ApiRegisterStorageFtr!\r\n");
    if(STORE_MAX_FTR <= uStorageId)
    {
        comp_code = COMP_CODE_STORAGE_IS_FULL;
        goto out;
    }
    ret = action_StoreFtr(uStorageId, u16NumId);
    switch (ret)
    {
        case FILE_SYSTEM_OK:
        comp_code = COMP_CODE_OK;
        break;

        case FILE_SYSTEM_STORAGE_FULL:
        comp_code = COMP_CODE_STORAGE_IS_FULL;
        break;

        case FILE_SYSTEM_PARAMETER_ERROR:
        comp_code = COMP_CODE_PARAMETER_ERROR;
        break;

        case FILE_SYSTEM_FLASH_ID_ERROR:
        comp_code = COMP_CODE_FLASH_ID_ERROR;
        break;

        case FILE_SYSTEM_FTR_CRC_ERR:
        comp_code = COMP_CODE_FTR_CRC_ERR;
        break;

        default:
        comp_code = COMP_CODE_OTHER_ERROR;
        break;
    }

out:
    return comp_code;
}

/*****************************************************************************
 �� �� ��  : MLAPI_Match
 ��������  : ָ��ƥ��
 �������  : uint32_t usTimeout
            **��ָ��ѹ��ʱʱ��1000Ϊ1s
 �������  : uint16_t *uResult
            **��ȡƥ���� 1:�ɹ� 2��ʧ��
            uint16_t *uScore
            **��ȡƥ�����
            uint16_t *uMatchId
            **��ȡƥ�䵽��ID
 �� �� ֵ  : uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2019��08��20��
    ��    ��   : Alex
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t MLAPI_Match(uint16_t *uResult, uint16_t *uScore, uint16_t *uMatchId, uint32_t usTimeout)
{
    int32_t ret;
    uint32_t comp_code;
    uint16_t Fp_Index = 0xFFFF;
    

    if ((NULL == uResult) || (NULL == uScore) || (NULL == uMatchId))
    {
        comp_code = COMP_CODE_CMD_DATA_ERROR;
        goto out;

    }
    *uResult = 0;
    *uScore = 0;
    *uMatchId = 0xFFFF;

#if(TEST_FPS_SPEED == 1)
     timerStart('C');
#endif

    ret = Sensor_WaitAndCapture(usTimeout);
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

#if(TEST_FPS_SPEED == 1)
     timerEnd();
#endif

    if (comp_code != COMP_CODE_OK)
    {
        goto out;
    }
    
    if(set_system_clock(SYSTEM_90MHZ) == 0)
    {
        DBG_DIRECT("set system clock 90M Fail");
    }
    
#if(TEST_FPS_SPEED == 1)
    timerStart('E');
#endif
    ret = action_Extract(SENSOR_IMAGE_BUFFER);
    if (ret != 0)
    {
        comp_code = COMP_CODE_FINGER_EXTRACT_ERROR;
        goto out;
    }
    
#if(TEST_FPS_SPEED == 1)    
    timerEnd();
#endif

#if(TEST_FPS_SPEED == 1) 
    timerStart('M');
#endif

    ret = action_Match(&Fp_Index);

#if(TEST_FPS_SPEED == 1) 
    timerEnd();
#endif   

    if (ret < 0)
    {
        MLAPI_PRINT_INFO1("[MLAPI] match fail ret:0x%x\r\n", ret);
        comp_code = COMP_CODE_FINGER_MATCH_ERROR;
        goto out;
    }
    else
    {
        //���ƥ�����ͷ���
        if (ret >= g_algMatchScoreThres)
        {
            *uResult = 1;
			
        }
        else
        {
            *uResult = 0;
        }
        *uScore = ret;
        *uMatchId = Fp_Index;
        comp_code = COMP_CODE_OK;
    }
    
    MLAPI_PRINT_INFO2("[MLAPI] match success id:%d, score:%d\r\n", *uMatchId, *uScore);
out: 
    if(90000000 == get_cpu_clock())
    {
        if(set_system_clock(SYSTEM_40MHZ) == 0)
            DBG_DIRECT("set system clock 40M Fail");
    }
    
    return comp_code;
}


/*****************************************************************************
 �� �� ��: MLAPI_DeleteFTR
 ��������  : ���ָ������
 �������  : uint8_t uClearMode
            **���ģʽ 0������ɾ�� 1��ȫ��ɾ�� 2������ɾ��
            uint16_t uFpNum
            **����ɾ����ID����
            uint16_t* uFpIdx
            **����ɾ����һ��ID������ɾ����ID
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ  :
  1.��    ��  : 2019��08��20��
    ��    ��  : Alex
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t MLAPI_DeleteFTR(uint8_t uClearMode, uint16_t uFpNum, uint16_t* uFpIdx)
{
    uint32_t comp_code = 0;
    uint32_t ret = 0;

    if (NULL == uFpIdx)
    {
        comp_code = COMP_CODE_CMD_DATA_ERROR;
        goto out;
    }

    switch (uClearMode)
    {
        case ERASE_ALL_FINGER:
            ret = fileSys_deleteAllFtr();
            if (0 == ret)
            {
                comp_code = COMP_CODE_OK;
            }
            else
            {
                comp_code = COMP_CODE_HARDWARE_ERROR;
            }
            break;

        case ERASE_BATCH_FINGER:                //��δʵ��
            ret = fileSys_deleteBatchFtr(uFpIdx, uFpNum);
            if (0 == ret)
            {
                comp_code = COMP_CODE_OK;
            }
            else if (FILE_SYSTEM_PARAMETER_ERROR == ret)
            {
                comp_code = COMP_CODE_INVALID_FINGERPRINT_ID;
            }
            else
            {
                comp_code = COMP_CODE_HARDWARE_ERROR;
            }
            break;

        case ERASE_SINGLE_FINGER:
            ret = fileSys_deleteOneFtr(uFpIdx[0]);
            if (0 == ret)
            {
                comp_code = COMP_CODE_OK;
            }
            else if ((FILE_SYSTEM_PARAMETER_ERROR == ret) || (FILE_SYSTEM_FLASH_ID_ERROR == ret))
            {
                comp_code = COMP_CODE_INVALID_FINGERPRINT_ID;
            }
            else
            {
                comp_code = COMP_CODE_HARDWARE_ERROR;
            }
            break;

		case ERASE_NUMBER_FINGER:
			ret = fileSys_deleteNumberFtr(uFpIdx[0]);
			if (0 == ret)
            {
                comp_code = COMP_CODE_OK;
            }
            else if ((FILE_SYSTEM_PARAMETER_ERROR == ret) || (FILE_SYSTEM_FLASH_ID_ERROR == ret))
            {
                comp_code = COMP_CODE_INVALID_FINGERPRINT_ID;
            }
            else
            {
                comp_code = COMP_CODE_HARDWARE_ERROR;
            }
			break;

        default:
            comp_code = COMP_CODE_CMD_DATA_ERROR;
            break;

    }

    out:
    g_FtrUpdateFlag = false;
    return comp_code;
}



uint32_t MLAPI_UpdateFTR(void)
{
    int32_t ret = 0;
    uint32_t comp_code = 0;

    if(g_FtrUpdateFlag)
    {
        MLAPI_PRINT_INFO1("[MLAPI] ud\r\n, g_UpdateFTRLen is %d", g_UpdateFTRLen);
        ret = fileSys_storeFtrAndUpdateFtrHead(g_UpdateFTRIndex, 0, g_pUpdateFTRData, g_UpdateFTRLen);
    }
    else if(g_FtrInfoUpdateFlag)
    {
        MLAPI_PRINT_INFO1("[MLAPI] ui, g_UpdateFTRLen is %d\r\n", g_UpdateFTRLen);
        ret = fileSys_Ftrinfo_StoreAndUpdate(g_UpdateFTRIndex, g_pUpdateFTRData, g_UpdateFTRLen);
    }
    else
    {
        comp_code = COMP_CODE_NONE_UPDATE;
        return comp_code;
    }

    if (0 == ret)
    {
        comp_code = COMP_CODE_OK;
    }
    else
    {
        comp_code = COMP_CODE_HARDWARE_ERROR;
    }

    g_FtrUpdateFlag = false;
    g_FtrInfoUpdateFlag = false;

    return comp_code;
}

/****************************************************************************
 �� �� ��: MLAPI_QueryFingerPresent
 ��������  : ��ѯ��ָ��λ״̬
 �������  : ��
 �������  : ��
 �� �� ֵ: 1:��ָ��λ 0����ָ����λ
 ���ú���  :
 ��������  :

 �޸���ʷ    :
 1.��    ��  : 2019��11��16��
   ��    ��  : Curry
   �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t MLAPI_QueryFingerPresent(void)
{
	app_wdg_reset();
    return Sensor_FingerCheck();
}


/*****************************************************************************
 �� �� ��  : MLAPI_PowerSaving
 ��������  : ��������ģʽ
 �������  : uint8_t uSleepMode
            **0����ͨ���� 1���������
 �������  : ��
 �� �� ֵ  : uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2019��08��20��
    ��    ��   : Alex
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t MLAPI_PowerSaving(uint8_t uSleepMode)
{
    SLEEP_MODE_T emSleepMode;
    uint32_t comp_code = 0;
    uint32_t nRet = 0;

    //�������Ͳ��Ϸ�
    if ((SLEEP_NORMAL_MODE != uSleepMode) && (SLEEP_DEEP_MODE != uSleepMode))
    {
        comp_code = COMP_CODE_CMD_DATA_ERROR;
        goto out;
    }
    emSleepMode = (SLEEP_MODE_T)uSleepMode;
    //MCU��������ģʽ
    switch(emSleepMode)
    {
        case SLEEP_NORMAL_MODE://��ӦоƬ��Power Downģʽ
        {
            nRet = Sensor_Sleep();
            if (0 != nRet)
            {
                comp_code = COMP_CODE_FINGER_PRESENT;
                break;
            }
            comp_code = COMP_CODE_OK;
            break;
        }

        case SLEEP_DEEP_MODE: //��ӦоƬDeep Power Downģʽ
        {
 //           nRet = Sensor_DeepSleep();
            if (0 != nRet)
            {
                comp_code = COMP_CODE_FINGER_PRESENT;
                break;
            }
            comp_code = COMP_CODE_OK;
            break;
        }
        default:
            break;
    }
out:
    return comp_code;
}

void MLAPI_AbortCommand(void)
{
    g_QuitAtOnce = 1;
}

/*****************************************************************************
 �� �� ��: MLAPI_GetFPVersion
 ��������  : ��ȡ�汾��Ϣ
 �������  : ST_FP_VER stVer  �汾��Ϣ
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ    :
  1.��    ��  : 2019��11��16��
    ��    ��  : 
    �޸�����   : �����ɺ���

*****************************************************************************/
void MLAPI_GetFPVersion(PST_FP_VER stVer)
{
    char StrTemp[5] = {0}, flag = 0;
    const char *AlgoVersion = GetAfisVersion();
    stVer = stVer;
    StrTemp[0] = AlgoVersion[0];
	StrTemp[1] = AlgoVersion[2];
    
	if('.' == AlgoVersion[5])
	{
		StrTemp[2] = '0';
		StrTemp[3] = AlgoVersion[4];
        flag = 1;
	}
	else
	{
		StrTemp[2] = AlgoVersion[4];
		StrTemp[3] = AlgoVersion[5];
	}
    
	StrTemp[4] = '\0';
	stVer->usAlgoMainVer = atoi(StrTemp);
    
    if(flag)    strncpy(StrTemp, &AlgoVersion[6], 4);
	else        strncpy(StrTemp, &AlgoVersion[7], 4);
    
	StrTemp[4] = '\0';
	stVer->usAlgoSubVer = atoi(StrTemp);
    strncpy(StrTemp,(char *) &g_sysSetting.board_id[13], 3);
    StrTemp[3] = '\0';
    stVer->usDriverVer = atoi(StrTemp);
    
    return;
}

/*****************************************************************************
 �� �� ��: MLAPI_GetFTRNum
 ��������  : ��ȡָ����������
 �������  : uint16_t* pusFtrNum,               FTR����     
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ    :
  1.��    ��  : 2019��11��16��
    ��    ��  : Curry
    �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t MLAPI_GetFTRNum(uint16_t* pusFtrNum)
{
    *pusFtrNum = g_stAllFtrHead.ftrNum;
    return COMP_CODE_OK;
}

/*****************************************************************************
 �� �� ��: MLAPI_GetShareMemory
 ��������  : ��ȡ�����ڴ�
 �������  : uint8_t **ppMem,  ���ڷ���mem��ַ
           uint32_t *pLen,   ���ڷ���mem����
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ    :
 1.��    ��  : 2019��11��16��
   ��    ��  :
   �޸�����   : �����ɺ���

*****************************************************************************/
void* MLAPI_GetShareMemory(int nSize)
{
    return Ucas_malloc(nSize);
}

/*****************************************************************************
 �� �� ��: MLAPI_FreeShareMemory
 ��������  : �ͷŹ����ڴ�
 �������  : ��
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
 ���ú���  :
 ��������  :

 �޸���ʷ    :
 1.��    ��  : 2019��11��16��
   ��    ��  :
   �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t MLAPI_FreeShareMemory(void *pBuff)
{
    Ucas_free(pBuff);
    return COMP_CODE_OK;
}

/*****************************************************************************
 �� �� ��: MLAPI_SetAdminCount
 ��������  : ���ù���Ա��
 �������  : uint16_t count�� ����Ա����0-99��ָ����������100ö��
 �������  : ��
 �� �� ֵ: uint32_t 32 bits Error Code
         COMP_CODE_CMD_DATA_ERROR:�����쳣
 �޸���ʷ    :
 1.��    ��  : 2020��1111��146��
   ��    ��  :joho
   �޸�����   : �����ɺ���

*****************************************************************************/
uint32_t MLAPI_SetAdminCount(uint16_t count)
{
    if(count >= STORE_MAX_FTR)
    {
        return COMP_CODE_CMD_DATA_ERROR;
    }

    g_filesysAdminCount = count;
    return COMP_CODE_OK;
}

