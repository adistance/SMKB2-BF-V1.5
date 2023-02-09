/*
 * action.c
 *
 *  Created on: 2020年8月17日
 *      Author: sks
 */
#include "action.h"
#include "fpsapi.h"
#include "fpsapierrors.h"
#include "files_ftrsys.h"
#include "filesys.h"
#include "driver_sensor.h"
#include "driver_flash.h"
#include "system_setting.h"
#include "trace.h"

#include "mlapi.h"         //测试用
#include "driver_led.h"
#include "record.h"
#include "sensor.h"

#if 0
#define ACTION_DEBUG(x) Printf x
#else
#define ACTION_DEBUG(x)
#endif

#define UNUSED_PARA(x) ((x)=(x))

ENROLL_PARA g_stEnrollPara = {0};

__align(4) uint8_t g_FtrBufTmp[SENSOR_FTR_BUFFER_TMP] = {0};
__align(4) uint8_t g_EnrollFtrBuf[SENSOR_FTR_BUFFER_MAX] = {0};

uint8_t g_FtrUpdateFlag = false;
uint8_t g_FtrInfoUpdateFlag = false;

uint8_t* g_pUpdateFTRData = NULL;
uint16_t g_UpdateFTRIndex = 0xFFFF;
uint32_t g_UpdateFTRLen = 0;
uint32_t g_FtrLenTmp = 0;


uint16_t g_algMatchScoreThres = ALG_MATCH_THRESHOLD;


/*****************************************************************************
 函 数 名  : action_AfisInit
 功能描述  : 指纹算法初始化
 输入参数  : int nEnrollMaxNum
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年10月25日
    作    者   : li dong
    修改内容   : 新生成函数

*****************************************************************************/
void action_AfisInit(int nEnrollMaxNum)
{
    AfisParams_t stPara;

    //AlgoGlobalInit();
    GetDefaultAfisParams(&stPara);
    stPara.nEnrollMaxNum = nEnrollMaxNum;
    g_algMatchScoreThres = stPara.nMatchScoreThres;

    if(g_stAllFtrHead.flashEraNum < 32850)
    {
    }
    else if(g_stAllFtrHead.flashEraNum < 43543)
    {
        stPara.nFadingSpeed = 1;
    }
    else if(g_stAllFtrHead.flashEraNum < 80000)
    {
        stPara.nFadingSpeed = 2;
    }
    else
    {
        stPara.nSelfLearningStrategy = 7;
    }	

	stPara.nMaxSetNum = 21;
	stPara.nMaxFtrSize = CalcMaxFtrSize(stPara.nMaxSetNum);
	
    SetAfisParams(&stPara);
}

void action_AfisInit_FirstEnroll(uint16_t index)
{
    action_AfisInit(g_sysSetting.alg_enroll_num);

    memset(g_EnrollFtrBuf , 0 , SENSOR_FTR_BUFFER_MAX);
    g_stEnrollPara.pFeature = &g_EnrollFtrBuf[0];
    g_stEnrollPara.length = 0;
    g_stEnrollPara.progress = 0;
    g_stEnrollPara.storage_addr = index;
}

/*****************************************************************************
 函 数 名  : action_AfisInitEnrollNumIndex
 功能描述  : 初始化算法
 输入参数  : enrollNum 注册次数
 			index 指纹保存的地址ID
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
void action_AfisInitEnrollNumIndex(uint32_t enrollNum, uint16_t index)
{
    action_AfisInit(enrollNum);

    memset(g_EnrollFtrBuf , 0 , SENSOR_FTR_BUFFER_MAX);
    g_stEnrollPara.pFeature = &g_EnrollFtrBuf[0];
    g_stEnrollPara.length = 0;
    g_stEnrollPara.progress = 0;
    g_stEnrollPara.storage_addr = index;
}


/*****************************************************************************
 函 数 名  : action_Extract
 功能描述  : 指纹特征提取
 输入参数  : uint8_t mode
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
int32_t action_Extract(uint8_t *Imgbuff)
{
    RawImageInfo_t stRawImgInfo;
    uint8_t *pFtrBuf;
    uint32_t *pLen;
    int32_t rst = 0;

    pFtrBuf = g_FtrBufTmp;
    pLen = &g_FtrLenTmp;

    stRawImgInfo.pData= Imgbuff;
    stRawImgInfo.nHeight = FPSENSOR_IMAGE_HEIGHT;
    stRawImgInfo.nWidth = FPSENSOR_IMAGE_WIDTH;
    stRawImgInfo.nBpp = 0;
    stRawImgInfo.nReserved = 0;
    rst = ExtractFeature(&stRawImgInfo, pFtrBuf, pLen);

    return rst;
}

/*****************************************************************************
 函 数 名  : action_Enrollment
 功能描述  : 指纹注册
 输入参数  : uint8_t num
             uint8_t *pProgress
             uint32_t storage_index
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年10月25日
    作    者   : li dong
    修改内容   : 新生成函数

*****************************************************************************/
int32_t action_Enrollment(uint8_t num, uint8_t *pProgress, uint16_t storage_index)
{
    int32_t ret;
    uint32_t progress;
    progress = g_stEnrollPara.progress;

    ret = EnrollFeature(g_FtrBufTmp, &g_stEnrollPara.pFeature, &g_stEnrollPara.length, &g_stEnrollPara.progress);
    *pProgress = g_stEnrollPara.progress;
    if ((g_stEnrollPara.progress > progress)||(100 == g_stEnrollPara.progress))
    {
        progress = g_stEnrollPara.progress;
        APP_PRINT_TRACE3("1-ftr len = %d, progress = %d, ret = 0x%x.\r\n", g_stEnrollPara.length, g_stEnrollPara.progress, ret);
        ret = 0;
		ledAutoCtrl(AUTO_LED_CAPT_OK);
    }
    else
    {
        APP_PRINT_TRACE3("2-ftr len = %d, progress = %d, ret = 0x%x.\r\n", g_stEnrollPara.length, g_stEnrollPara.progress, ret);
		ledAutoCtrl(AUTO_LED_FAIL);
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : action_StoreFtr
 功能描述  : 指纹特征保存
 输入参数  : uint32_t storage_index
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年10月25日
    作    者   : li dong
    修改内容   : 新生成函数

*****************************************************************************/
int action_StoreFtr(unsigned int storage_index, unsigned short numId)
{
    int nRet = FILE_SYSTEM_OK;

    nRet = fileSys_storeFtrAndUpdateFtrHead(storage_index, numId, g_stEnrollPara.pFeature, g_stEnrollPara.length);

    ACTION_DEBUG(("StoreFtr ret:%d , Id:%d , g_stEnrollPara.length:%u\r\n" , nRet , storage_index , g_stEnrollPara.length));
    return nRet;
}

#if 0
/*****************************************************************************
 函 数 名  : action_Match
 功能描述  :  指纹特征匹配
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
int32_t action_MatchEx(uint16_t *Fp_Index)
{
    int32_t rst;
    uint32_t index[2], i, FtrIndexBufLen;
    uint32_t len, uTimeTick = 0;
    uint8_t *p_ftrTpl;
    uint32_t score, score_tmp;
    uint8_t FtrIndexBuf[STORE_MAX_FTR];

    UNUSED_PARA(uTimeTick);

    action_AfisInit(g_sysSetting.alg_enroll_num);

    p_ftrTpl = &g_EnrollFtrBuf[0];
    if(NULL == p_ftrTpl)
    {
        return -1;
    }


    index[0] = fileSys_getStoreFtrNum();
    if(index[0] == 0)
    {
        return -1;
    }

    fileSys_getUseIndex(FtrIndexBuf , &FtrIndexBufLen);
//    fileSys_rankFtrHead(FtrIndexBuf , FtrIndexBufLen);

    g_pUpdateFTRData = NULL;
    g_UpdateFTRLen = 0;
    g_UpdateFTRIndex = 0xFFFF;
    g_FtrUpdateFlag = false;

    score = 0;
    score_tmp = 0;

    for(i = 0; i < FtrIndexBufLen; i++)
    {
        memset(p_ftrTpl, 0x00, SENSOR_FTR_BUFFER_MAX);
        if (FILE_SYSTEM_OK == fileSys_readFtr(FtrIndexBuf[i], p_ftrTpl, &len))
        {
            rst = MatchFeature(p_ftrTpl,  &len, g_FtrBufTmp, g_FtrLenTmp, &score_tmp);
            if(score_tmp > score)
            {
                score = score_tmp;
                if(score >= g_algMatchScoreThres)
                {
                    *Fp_Index = fileSys_indexToId(FtrIndexBuf[i]);
                    if (0 != (g_sysSetting.sys_policy & SYS_POLICY_SELF_LEARN))
                    {
                        if (PR_MATCH_FTR_UPDATED == rst) //需要更新特征
                        {
                            g_pUpdateFTRData = p_ftrTpl;
                            g_UpdateFTRLen = len;
                            g_UpdateFTRIndex = (*Fp_Index);
                            g_FtrUpdateFlag = true;
                            g_FtrInfoUpdateFlag = false;

                        }
                        else if(PR_MATCH_FTR_HEAD_UPDATED == rst)
                        {
                            g_pUpdateFTRData = p_ftrTpl;
                            g_UpdateFTRLen = len;
                            g_UpdateFTRIndex = (*Fp_Index);
                            g_FtrUpdateFlag = false;
                            g_FtrInfoUpdateFlag = true;
                        }
                        else
                        {
                            g_pUpdateFTRData = NULL;
                            g_UpdateFTRLen = 0;
                            g_UpdateFTRIndex = 0xFFFF;
                            g_FtrUpdateFlag = false;
                            g_FtrInfoUpdateFlag = false;
                        }

                    }
                    return score;
                }

            }
        }
    }

//    ACTION_DEBUG(("finger NOT match, max score:%d.\r\n", score));
    return score;
}
#else

uint32_t action_ReadFtr(uint16_t usFtrId, uint16_t* pusFtrLen, uint8_t* pucFtrBuf)
{
    int ret = 0;
    unsigned int readLen = 0;
    unsigned char unSec  = 0;
	uint32_t baseAddr = FLASH_FP_SAVE_ADDR;
    short index = fileSys_IdToIndex(usFtrId);

    if(-1 == index)
    {
        return COMP_CODE_INVALID_FINGERPRINT_ID;
    }

    if(index > STORE_MAX_FTR)
    {
        ACTION_DEBUG(("MLAPI_ReadFtr_INDEX_ERROR\r\n"));
        return COMP_CODE_PARAMETER_ERROR;
    }

    readLen = g_stAllFtrHead.ftrHead[index].unLength;
    unSec  = g_stAllFtrHead.ftrHead[index].unSec;
    //printf("short:%d  readLen:%d  unSec:%d\r\n",index,readLen,unSec);
    if(readLen > (FLASH_SECTOR_SIZE_12K - 4))
    {
        ACTION_DEBUG(("MLAPI_ReadFtr_READ_LEN_ERROR\r\n"));
        return COMP_CODE_READ_FTR_ERROR;
    }

    if(USE_FLAG != g_stAllFtrHead.ftrHead[index].unFlag)
    {
        ACTION_DEBUG(("MLAPI_ReadFtr_READ_FLAG_ERROR\r\n"));
        return COMP_CODE_READ_FTR_ERROR;
    }
    *pusFtrLen = readLen;
    ret = flashReadBuffer_match(pucFtrBuf, baseAddr + unSec * FLASH_SECTOR_SIZE_12K, (readLen+4), EM_FLASH_CTRL_INSIDE); 

    if(COMP_CODE_OK != ret) return ret;

    return COMP_CODE_OK;
}

/*****************************************************************************
 函 数 名  : action_Match
 功能描述  :  指纹特征匹配
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : jack
    修改内容   : 新生成函数

*****************************************************************************/
int32_t action_Match(uint16_t *Fp_Index)
{
    uint8_t *p_ftrTpl;                      //指向注册的buf  
    uint8_t FtrIndexBuf[STORE_MAX_FTR];
    uint32_t FtrIndexBufLen;
    
    int32_t rst;
    uint32_t index[2], i;
    uint32_t len;
    uint32_t score = 0;
    
    unsigned int CrcTemp = 0;
    ftrInfo_para ftr_para;
    uint16_t pusFtrLen;
    int readret = 0;
    unsigned char FtrheadTmp[FTRHEAD_DATALENGTH] = {0};         

    action_AfisInit(g_sysSetting.alg_enroll_num);
    
    p_ftrTpl = &g_EnrollFtrBuf[0];
    if(NULL == p_ftrTpl)
    {
        return -1;
    }

    index[0] = fileSys_getStoreFtrNum();
    if(index[0] == 0)
    {
        return -1;
    }

    fileSys_getUseIndex(FtrIndexBuf , &FtrIndexBufLen);

#if 1//(TEST_FPS_SPEED != 1)
    fileSys_rankFtrHead(FtrIndexBuf , FtrIndexBufLen);
#endif

    g_pUpdateFTRData = NULL;
    g_UpdateFTRLen   = 0;
    g_UpdateFTRIndex = 0xFFFF;
    g_FtrUpdateFlag  = false;
    
    action_ReadFtr(g_stAllFtrHead.ftrHead[FtrIndexBuf[0]].unId, &pusFtrLen, g_ImgBuf);
    for(i = 0; i < FtrIndexBufLen; i++)
    {
		menu_sleep_event_control(MENU_SLEEP_EVENT_WAKEUP_BIT, true);
        if(COMP_CODE_OK != spiFlashTransfer_match_wait())
        {
            return -2;
        }

        //对数据进行一次校验
        CrcTemp = ((g_ImgBuf[pusFtrLen+3] << 24) | (g_ImgBuf[pusFtrLen+2] << 16) | (g_ImgBuf[pusFtrLen+1] << 8) | g_ImgBuf[pusFtrLen]);
        if (0 != CrcTemp - CRC32_calc(g_ImgBuf , pusFtrLen))
        {
            ACTION_DEBUG(("crc error dma!\r\n"));
            return -3;
        }

        ftr_para.FtrInfo_Sec = FtrIndexBuf[i] / SINGER_FTRHEAD_NUM;
        ftr_para.ftr_Index = FtrIndexBuf[i];
        ftr_para.ftrBuffer = g_ImgBuf;
        ftr_para.ftrleng = pusFtrLen;
        memcpy(FtrheadTmp, g_ImgBuf, FTRHEAD_DATALENGTH);

        if(0 != ftrsys_read_ftrinfo(&ftr_para))
        {
            memcpy(g_ImgBuf, FtrheadTmp, FTRHEAD_DATALENGTH);
        }

        len = pusFtrLen;
        memcpy(g_EnrollFtrBuf, g_ImgBuf, len);
        p_ftrTpl = g_EnrollFtrBuf;

        if(i != (FtrIndexBufLen-1))                                         //判断是否还需要启动读
        {
            readret = action_ReadFtr(g_stAllFtrHead.ftrHead[FtrIndexBuf[i+1]].unId, &pusFtrLen, g_ImgBuf);
        }
        else                                                                //不需要时赋值成功的目的是下面做匹配使用
        {
            readret = COMP_CODE_OK;
        }

        if(COMP_CODE_OK ==  readret)
        {
            rst = MatchFeature(p_ftrTpl,  &len, g_FtrBufTmp, g_FtrLenTmp, &score);

            if(score >= g_algMatchScoreThres)
            {
                *Fp_Index = fileSys_indexToId(FtrIndexBuf[i]);
                if (0 != (g_sysSetting.sys_policy & SYS_POLICY_SELF_LEARN))
                {
                    if (PR_MATCH_FTR_UPDATED == rst) //需要更新特征
                    {
                        g_pUpdateFTRData = p_ftrTpl;
                        g_UpdateFTRLen = len;
                        g_UpdateFTRIndex = (*Fp_Index);
                        g_FtrUpdateFlag = true;
                        g_FtrInfoUpdateFlag = false;

                    }
                    else if(PR_MATCH_FTR_HEAD_UPDATED == rst)
                    {
                        g_pUpdateFTRData = p_ftrTpl;
                        g_UpdateFTRLen = len;
                        g_UpdateFTRIndex = (*Fp_Index);
                        g_FtrUpdateFlag = false;
                        g_FtrInfoUpdateFlag = true;
                    }
                    else
                    {
                        g_pUpdateFTRData = NULL;
                        g_UpdateFTRLen = 0;
                        g_UpdateFTRIndex = 0xFFFF;
                        g_FtrUpdateFlag = false;
                        g_FtrInfoUpdateFlag = false;
                    }
                }
                spiFlashTransfer_match_wait();
//                Printf("match:%d\r\n", score);
				//APP_PRINT_INFO2("Fp_Index is %d %d", Fp_Index, FtrIndexBuf[i]);
				BleRecordSet(ModeFp, FtrIndexBuf[i]+1);
				BleRecordUpdata();
				ledAutoCtrl(AUTO_LED_MATCH_OK);
                return score;
            }

        }
        else
        {
            ACTION_DEBUG(("read error! 0x%x\r\n",readret));
        }
    }
    spiFlashTransfer_match_wait();
//    ACTION_DEBUG(("finger NOT match, max score:%d.\r\n", score));
	ledAutoCtrl(AUTO_LED_FAIL);
    return score;
}
#endif

/*****************************************************************************
 函 数 名  : SENSOR_MatchEx
 功能描述  : 快速匹配
 输入参数  : uint16_t *Fp_Index
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年3月30日
    作    者   : li dong
    修改内容   : 新生成函数

*****************************************************************************/
int32_t action_MatchEx(uint16_t *Fp_Index)
{
    uint32_t rst;
    uint32_t index[2], i;
    uint32_t len;
    uint8_t *p_ftrTpl;
    uint32_t score;
    uint8_t FtrIndexBuf[STORE_MAX_FTR];
    uint32_t FtrIndexBufLen;

    UNUSED_PARA(rst);

    action_AfisInit(g_sysSetting.alg_enroll_num);

    p_ftrTpl = &g_EnrollFtrBuf[0];
    if(NULL == p_ftrTpl)
    {
        return -1;
    }

    index[0] = fileSys_getStoreFtrNum();
    if(index[0] == 0)
    {
        return -1;
    }

    fileSys_getUseIndex(FtrIndexBuf , &FtrIndexBufLen);

    g_pUpdateFTRData = NULL;
    g_UpdateFTRLen = 0;
    g_UpdateFTRIndex = 0xFFFF;
    g_FtrUpdateFlag = false;
    score = 0;

    for(i = 0; i < FtrIndexBufLen; i++)
    {
        if (FILE_SYSTEM_OK == fileSys_readFtr(FtrIndexBuf[i], p_ftrTpl, &len))
        {
            MatchFeature(p_ftrTpl, &len, g_FtrBufTmp, g_FtrLenTmp, &score);
            if(score >= g_algMatchScoreThres)
            {
                *Fp_Index = fileSys_indexToId(FtrIndexBuf[i]);
                action_AfisInit(g_sysSetting.alg_enroll_num);
				//APP_PRINT_INFO2("Fp_Index is %d %d", Fp_Index, FtrIndexBuf[i]);
				BleRecordSet(ModeFp, FtrIndexBuf[i]+1);
				BleRecordUpdata();
                return score;
            }
        }
    }

    action_AfisInit(g_sysSetting.alg_enroll_num);
    return score;
}

