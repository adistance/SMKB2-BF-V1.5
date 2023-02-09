/* 
 * filesys.c
 *
 *  Created on: 2020年8月17日
 *      Author: sks
 */
#include "action.h"
#include "files_ftrsys.h"
#include "filesys.h"
#include "string.h"
#include "driver_led.h"
#include <rtl876x.h>
#include "trace.h"


#if 0
#define FILESYS_PRINT_INFO0   APP_PRINT_TRACE0
#define FILESYS_PRINT_INFO1   APP_PRINT_TRACE1
#define FILESYS_PRINT_INFO2   APP_PRINT_TRACE2
#define FILESYS_PRINT_INFO3   APP_PRINT_TRACE3
#define FILESYS_PRINT_INFO4   APP_PRINT_TRACE4
#else
#define FILESYS_PRINT_INFO0(...)
#define FILESYS_PRINT_INFO1(...)
#define FILESYS_PRINT_INFO2(...)
#define FILESYS_PRINT_INFO3(...)
#define FILESYS_PRINT_INFO4(...)
#endif

ST_STORAGE_FP_LINK g_stAllFtrHead;
unsigned short g_filesysAdminCount = 0;

uint32_t CRC32_calc(uint8_t *Data , uint32_t Len)
{
#if 0
    uint32_t uwCRCValue;

    uwCRCValue = HAL_CRC_Calculate(&CrcHandle, (uint32_t *)Data, Len/4);

    return uwCRCValue;
#else
    uint32_t chksum = 0;
    uint32_t i;
    for(i = 0; i < Len; i++)
    {
        chksum += Data[i];
    }
    return chksum;
#endif
}

unsigned int GetChksum(uint8_t *p, uint32_t num)
{
    uint32_t i;
    unsigned int sum = 0;
    
    for (i = 0; i < num; i++)
    {
        sum += p[i];
    }

    return sum;
}


int fileSys_ReadSYNoteBook(unsigned char sec, unsigned char *buff, unsigned char length)
{
    if((length != SY_NOTE_LEN) || (sec > SY_MAX_NOTE))
    {
        return -1;          
    }
   
    memcpy(buff, g_stAllFtrHead.sy_note_Head[sec], SY_NOTE_LEN);

    return 0;
}

int fileSys_FtrHead_master_test(void)
{
    int ret;
    unsigned int chksum = 0;
    ST_STORAGE_FP_LINK stFtrPara = {0};
    
    ret = flashReadBuffer((unsigned char *)&stFtrPara , FTR_HEAD_ADDRESS , FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    if(0 != ret)
    {
        FILESYS_PRINT_INFO0("read master ftrHead error!\r\n");
        return ret;
    }
    
    chksum = CRC32_calc((unsigned char *)&stFtrPara , FTR_HEAD_ALL_LENGTH - 4 );
    
    if(chksum != stFtrPara.chk)
    {
        FILESYS_PRINT_INFO0("master crc error\r\n");
        return FILE_SYSTEM_FTR_CRC_ERR;
    }
    
    return FILE_SYSTEM_OK;
}

int fileSys_FtrHead_slave_test(void)
{
    int ret;
    unsigned int chksum = 0;
    ST_STORAGE_FP_LINK stFtrPara = {0};
    
    ret = flashReadBuffer((unsigned char *)&stFtrPara, FTR_HEAD_BACKUP_ADDRESS , FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    if(0 != ret)
    {
        FILESYS_PRINT_INFO0("read backup ftrHead error!\r\n");
        return ret;
    }
    
    chksum = CRC32_calc((unsigned char *)&stFtrPara , FTR_HEAD_ALL_LENGTH - 4 );
    
    if(chksum != stFtrPara.chk)
    {

        FILESYS_PRINT_INFO0("backup crc error\r\n");
        return FILE_SYSTEM_FTR_CRC_ERR;
    }

    return FILE_SYSTEM_OK;
}

int fileSys_Write_Ftr_Head(void)
{

	flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    if(FILE_SYSTEM_OK != fileSys_FtrHead_master_test())
    {
        return -1;
    }

    flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_BACKUP_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    if(FILE_SYSTEM_OK != fileSys_FtrHead_slave_test())
    {  
        return -1;
    }
    
    return FILE_SYSTEM_OK;
}

int fileSys_WriteSYNoteBook(unsigned char sec, unsigned char *buff, unsigned char length)
{
    int ret = 0;

    if((length != SY_NOTE_LEN) || (sec > SY_MAX_NOTE))
    {
        return -1;          
    }

    memcpy(g_stAllFtrHead.sy_note_Head[sec], buff, SY_NOTE_LEN);
    
	g_stAllFtrHead.chk = CRC32_calc((unsigned char *)&g_stAllFtrHead , FTR_HEAD_ALL_LENGTH - 4 );

    ret = fileSys_Write_Ftr_Head();
    if(FILE_SYSTEM_OK != ret)
    {
        FILESYS_PRINT_INFO0("Update_Ftr_Head  fail\r\n");
        return ret;
    }
    return 0;
}

uint8_t fileSys_Is_Buff_Clear(unsigned char *buff, unsigned int num, unsigned int value)
{
    uint32_t i;
    uint8_t *p;    

    p = buff;
    
    for (i = 0; i < num; i++)
    {
        if (*(p + i) != value)
        {
            return 0;
        }
    }
  
    return 1;
}

void fileSys_QuickSort(unsigned char *arr, int low, int high)
{
    int i = low;
    int j = high;
    int k;
    unsigned char temp;

    temp = arr[low];
    k = g_stAllFtrHead.updatetime[temp];

    while (i < j)
    {
        while((g_stAllFtrHead.updatetime[arr[j]] <= k) && (i < j))
        j--;
        arr[i] = arr[j];

        while((g_stAllFtrHead.updatetime[arr[i]] >= k) && (j > i))
        i++;
        arr[j] = arr[i];
    }

    arr[i] = temp;

    if(i > low)
    {
        fileSys_QuickSort(arr, low, j - 1);     // 排序k左边
    }

    if(j < high)
    {
        fileSys_QuickSort(arr, j + 1, high);    // 排序k右边
    }
}

int fileSys_getStoreFtrNum(void)
{
    return g_stAllFtrHead.ftrNum;
}

int fileSys_getUseIndex(unsigned char *index_buff, unsigned int *index_num)
{
    unsigned short i = 0;
    unsigned short num = 0;

    for(i = 0; i < STORE_MAX_FTR; i++)
    {
        if(USE_FLAG == g_stAllFtrHead.ftrHead[i].unFlag)
        {
            index_buff[num] = i;
            num++;
        }
    }

    *index_num = num;

    return 0;
}

int fileSys_rankFtrHead(unsigned char *index_buff, unsigned int num)
{
//    unsigned char indexTmp;
    int i;
    ftrInfo_para ftr_para;

#if (QUICKSORT == 1)
    Printf("adminNum:%d, ftrNum:%d\r\n",g_filesysAdminCount, num);
    for(i=0; i<num; i++)
    {
        Printf("#I:%-2d - T:%-3d ", index_buff[i], g_stAllFtrHead.updatetime[index_buff[i]]);
        if((i > 2) && ((i + 1)%10 == 0))
        {
            Printf("\r\n");
        }
    }
    Printf("\r\n");
#endif

    if((2 >= num) || (g_filesysAdminCount >= num))
    {
//        Printf("no sort and return\r\n");
        return 0;
    }

    i = (g_filesysAdminCount == 0) ? 0 : g_filesysAdminCount;

    for(; i < num; i++)
    {
        ftr_para.FtrInfo_Sec = index_buff[i] / SINGER_FTRHEAD_NUM;
        ftr_para.ftr_Index = index_buff[i];
        ftr_para.ftrBuffer = g_EnrollFtrBuf;
        ftr_para.ftrleng = g_stAllFtrHead.ftrHead[index_buff[i]].unLength;

        if(0 != ftrsys_read_ftrinfo(&ftr_para))
        {
            ftr_para.ftr_updatetimes = 0;
        }

        if(ftr_para.ftr_updatetimes > g_stAllFtrHead.updatetime[index_buff[i]])
        {
            g_stAllFtrHead.updatetime[index_buff[i]] = ftr_para.ftr_updatetimes;
        }
    }

    if((num - g_filesysAdminCount) > 2)
    {
        fileSys_QuickSort(index_buff, g_filesysAdminCount, num - 1);
    }

#if (QUICKSORT == 1)
    else
    {
        Printf("num - g_filesysAdminCount less than 2\r\n");
    }
    for(i=0; i<num; i++)
    {
        Printf("#I:%-2d - T:%-3d ", index_buff[i], g_stAllFtrHead.updatetime[index_buff[i]]);
        if((i > 2) && ((i + 1)%10 == 0))
        {
            Printf("\r\n");
        }
    }
    Printf("\r\n\r\n\r\n");
#endif

    return 0;
}


unsigned short fileSys_getUnuseSmallestIndex(void)
{
    unsigned short i = 0;

    for(i = 0; i < STORE_MAX_FTR; i++)
    {
        if (USE_FLAG != g_stAllFtrHead.ftrHead[i].unFlag)
        {
            break;
        }
    }

    return i;
}

int fileSys_IdToIndex(unsigned short id)
{
    int i;

    for(i = 0; i < STORE_MAX_FTR; i++)
    {
        if((USE_FLAG == g_stAllFtrHead.ftrHead[i].unFlag) && (id == g_stAllFtrHead.ftrHead[i].unId))
            return i;
    }

    return -1;
}

unsigned short fileSys_GetNumId(unsigned short id)
{
    int i;

    for(i = 0; i < STORE_MAX_FTR; i++)
    {
        if((USE_FLAG == g_stAllFtrHead.ftrHead[i].unFlag) && (id == g_stAllFtrHead.ftrHead[i].unId))
            return g_stAllFtrHead.ftrHead[i].unNumID;
    }

    return 0;
}


unsigned short fileSys_indexToId(unsigned short index)
{
    return g_stAllFtrHead.ftrHead[index].unId;
}

unsigned char fileSys_getUnuseSmallestSector()
{
    unsigned char i,j;
    unsigned char unSec = DEFAULT_SECTOR;

    for(i = 0; i < 20; i++)
    {
        if(0xff != g_stAllFtrHead.secHead[i])
        {
            for(j = 0 ; j < 8 ; j++)
            {
                if(0 == ((g_stAllFtrHead.secHead[i] >> (7 - j)) & 1))
                {
                    break;
                }
            }
            unSec = ((i << 3) + j);
            break;
        }
    }
    return unSec;
}

void ACTION_HighAndLowChange(unsigned char *data, const unsigned int length)
{
    unsigned char temp;
    unsigned char i;

    for(i = 0; i < length; i++)
    {
        temp = ((data[i] & 0x80) >> 7)
             | ((data[i] & 0x40) >> 5)
             | ((data[i] & 0x20) >> 3)
             | ((data[i] & 0x10) >> 1)
             | ((data[i] & 0x08) << 1)
             | ((data[i] & 0x04) << 3)
             | ((data[i] & 0x02) << 5)
             | ((data[i] & 0x01) << 7);

        data[i] = temp;
    }
}

void fileSys_getIdDistribute(unsigned char * IdIndexBuff)
{
    memcpy(IdIndexBuff , g_stAllFtrHead.idHead , sizeof(g_stAllFtrHead.idHead));
    ACTION_HighAndLowChange(IdIndexBuff , sizeof(g_stAllFtrHead.idHead));
}

int fileSys_storeFtr(unsigned short id, unsigned char *ftrBuffer, unsigned char *unSec ,unsigned int length)
{
    int ret = 0;
    unsigned char unUseSec = 0;
    uint32_t  CrcTemp;
	uint32_t baseAddr = FLASH_FP_SAVE_ADDR;

    unUseSec = fileSys_getUnuseSmallestSector();
    if((1 + STORE_MAX_FTR) < unUseSec)
    {
        return FILE_SYSTEM_STORAGE_FULL;
    }

    CrcTemp = CRC32_calc(ftrBuffer , length);
    ftrBuffer[length] = (CrcTemp >> 0) & 0xFF;
    ftrBuffer[length+1] = (CrcTemp >> 8) & 0xFF;
    ftrBuffer[length+2] = (CrcTemp >> 16) & 0xFF;
    ftrBuffer[length+3] = (CrcTemp >> 24) & 0xFF;

    FILESYS_PRINT_INFO1("[FILE]:fileSys_storeFtr address:0x%x", baseAddr + unUseSec * FLASH_SECTOR_SIZE_12K);
    ret = flashWriteBuffer(ftrBuffer, baseAddr + unUseSec * FLASH_SECTOR_SIZE_12K, (length+4), EM_FLASH_CTRL_INSIDE);

    if(COMP_CODE_OK != ret)
        return ret;

    memset(ftrBuffer, 0xFF, length+4);
    *unSec = unUseSec;
    /**********************读取当前FTR，校验CRC*******************************/

    flashReadBuffer(ftrBuffer, baseAddr + unUseSec * FLASH_SECTOR_SIZE_12K, (length+4), EM_FLASH_CTRL_INSIDE);
   
    CrcTemp = ((ftrBuffer[length+3] << 24) | (ftrBuffer[length+2] << 16) | (ftrBuffer[length+1] << 8) | ftrBuffer[length]);
    if (0 != CrcTemp - CRC32_calc(ftrBuffer , length))
    {
        FILESYS_PRINT_INFO0("Save ftr_crc_err");
        return FILE_SYSTEM_FTR_CRC_ERR;
    }

    return FILE_SYSTEM_OK;
}

int fileSys_Ftrinfo_StoreAndUpdate(unsigned short id, unsigned char *ftrBuffer, unsigned int leng)
{
    short ftr_Index = 0;
    ftrInfo_para ftr_para;
    int ret;

    ftr_Index = fileSys_IdToIndex(id);
    if(ftr_Index < 0)
    {
        return 1;
    }

    ftr_para.FtrInfo_Sec = ftr_Index / SINGER_FTRHEAD_NUM;
    ftr_para.ftr_Index = ftr_Index;
    ftr_para.ftrBuffer = ftrBuffer;
    ftr_para.ftrleng = leng;
    ftr_para.ftr_updatetimes = ++g_stAllFtrHead.updatetime[ftr_Index];

    ret = ftrsys_SaveFtrInfo(&ftr_para);
    if(0 != ret)
    {
        FILESYS_PRINT_INFO1("[FILE]:ftrsys_save_ftrinfo:%d\r\n",ret);
        return ret;
    }

    return FILE_SYSTEM_OK;
}

int fileSys_UpdateFtrHead(unsigned short id, unsigned short numId, unsigned char unSec,unsigned int length)
{
    int ret = 0;
    short unIndex = 0;
    unsigned char unOldSec = 0;

    unIndex = fileSys_IdToIndex(id);
    if(unIndex < 0)     //id不存在，获取最小index，ftr数量+1
    {
        unIndex = fileSys_getUnuseSmallestIndex();
        g_stAllFtrHead.ftrNum++;
        g_stAllFtrHead.updatetime[unIndex] = DEFAULT_INDEX;
        FILESYS_PRINT_INFO2("[FILE]:id[%d] no used, save index:%d", id, unIndex);
    }
    else                //id存在，将之前的sector设置为未使用
    {
        unOldSec = g_stAllFtrHead.ftrHead[unIndex].unSec;
        g_stAllFtrHead.secHead[(unOldSec >> 3)] &= ~(1 << (7 - (unOldSec & 0x7)));
        g_stAllFtrHead.updatetime[unIndex]++;
        if(g_stAllFtrHead.updatetime[unIndex] > 0xfffe)
        {
            g_stAllFtrHead.updatetime[unIndex] = 0xfffe;
        }
        FILESYS_PRINT_INFO4("[FILE]:id[%d] used index:%d, old sector:%d, old length:%d", id, unIndex, unOldSec, g_stAllFtrHead.ftrHead[unIndex].unLength);
    }

    g_stAllFtrHead.secHead[(unSec >> 3)]         |= (1 << ( 7 - (unSec & 0x7)));
    g_stAllFtrHead.idHead[(id >> 3)]            |= (1 << ( 7 - (id & 0x7)));
    g_stAllFtrHead.ftrHead[unIndex].unFlag      = USE_FLAG;
    g_stAllFtrHead.ftrHead[unIndex].unSec       = unSec;
    g_stAllFtrHead.ftrHead[unIndex].unId        = id;
    g_stAllFtrHead.ftrHead[unIndex].unLength    = (unsigned short)length;
	if(numId != 0) //不等于0才保存，成员ID范围是1-0x64
		g_stAllFtrHead.ftrHead[unIndex].unNumID    = numId;
//    g_stAllFtrHead.flashEraNum = flashEraNum;

    g_stAllFtrHead.chk = CRC32_calc((unsigned char *)&g_stAllFtrHead , FTR_HEAD_ALL_LENGTH - 4 );
    
    ret = flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    if(FILE_SYSTEM_OK != ret)
        return ret;
    
    ret = flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_BACKUP_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    if(FILE_SYSTEM_OK != ret)
        return ret;

    FILESYS_PRINT_INFO2("[FILE]:save success new sector:%d, length=%d", unSec , length);
    return FILE_SYSTEM_OK;
}

unsigned char fileSys_checkIDExist(unsigned short id)
{
    unsigned char idByte,idBit;
    unsigned char ret = 0;

    idByte = g_stAllFtrHead.idHead[(id >> 3)];
    idBit  = id & 0x7;
    ret = ((idByte >> (7 - idBit)) & 0x1);

    return ret;
}


int fileSys_storeFtrAndUpdateFtrHead(unsigned short id, unsigned short numId, unsigned char *ftrBuffer, unsigned int length)
{
    unsigned char unSec = 0;
    int ret;

    if((0 == length) || (length > (FLASH_SECTOR_SIZE_12K - 4)) )
    {
    	ret = FILE_SYSTEM_PARAMETER_ERROR;
    	goto storeError;
    }

    ret = fileSys_storeFtr(id, ftrBuffer, &unSec, length);
    if(FILE_SYSTEM_OK != ret)
    {
        FILESYS_PRINT_INFO0(" [FILE]:fileSys_storeFtr Err");
        goto storeError;
    }

    ret = fileSys_UpdateFtrHead(id, numId, unSec,length);
    if(FILE_SYSTEM_OK != ret)
    {
    	goto storeError;
    }
	ledAutoCtrl(AUTO_LED_STORAGE_OK);	
    return FILE_SYSTEM_OK;
	
storeError:
	ledAutoCtrl(AUTO_LED_FAIL);
	return ret;
}

int fileSys_readFtr(unsigned short index, unsigned char *readBuffer, unsigned int *length)
{
    unsigned int readLen = g_stAllFtrHead.ftrHead[index].unLength;
    unsigned char unSec  = g_stAllFtrHead.ftrHead[index].unSec;
    unsigned int CrcTemp;
	uint32_t baseAddr = FLASH_FP_SAVE_ADDR;
//    ftrInfo_para ftr_para;
//    unsigned char FtrheadTmp[FTRHEAD_DATALENGTH] = {0};

    if(index > STORE_MAX_FTR)
    {
        return FILE_SYSTEM_PARAMETER_ERROR;
    }

    if(USE_FLAG != g_stAllFtrHead.ftrHead[index].unFlag)
    {
        return FILE_SYSTEM_PARAMETER_ERROR;
    }

    if(readLen > (FLASH_SECTOR_SIZE_12K - 4) )
    {
        return FILE_SYSTEM_READ_FTR_ERROR;
    }

    if(USE_FLAG != g_stAllFtrHead.ftrHead[index].unFlag)
    {
        return FILE_SYSTEM_READ_FTR_ERROR;
    }

    *length = readLen;


    flashReadBuffer(readBuffer, (baseAddr + unSec * FLASH_SECTOR_SIZE_12K), (readLen+4), EM_FLASH_CTRL_INSIDE);

    CrcTemp = ((readBuffer[(*length)+3] << 24) | (readBuffer[(*length)+2] << 16) | (readBuffer[(*length)+1] << 8) | readBuffer[(*length)]);

    if (0 != CrcTemp - CRC32_calc(readBuffer , readLen))
    {
        FILESYS_PRINT_INFO0("[FILE]:read ftr_crc_err\r\n");
        return FILE_SYSTEM_FTR_CRC_ERR;
    }

//    ftr_para.FtrInfo_Sec = index / SINGER_FTRHEAD_NUM;
//    ftr_para.ftr_Index = index;
//    ftr_para.ftrBuffer = readBuffer;
//    ftr_para.ftrleng = *length;
//    memcpy(FtrheadTmp, readBuffer, FTRHEAD_DATALENGTH);
//
//    if(0 != ftrsys_read_ftrinfo(&ftr_para))
//    {
//        Printf("ftr info error\r\n");
//        memcpy(readBuffer, FtrheadTmp, FTRHEAD_DATALENGTH);
//    }

    return FILE_SYSTEM_OK;
}

uint32_t fileSys_deleteAllFtr(void)
{
    uint32_t ret = FILE_SYSTEM_OK;
    memset((unsigned char *)&g_stAllFtrHead , 0 , FTR_HEAD_ALL_LENGTH);
//    g_stAllFtrHead.flashEraNum = FlashEraseNum;
    g_stAllFtrHead.chk = CRC32_calc((unsigned char *)&g_stAllFtrHead , FTR_HEAD_ALL_LENGTH - 4 );


    ret = flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    if(ret != COMP_CODE_OK)
        goto out;
    
    ret = flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_BACKUP_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
out:
    if(COMP_CODE_OK != ret)
    {
        FILESYS_PRINT_INFO0("[FILE]:Update_Ftr_Head fail");
        return ret;
    }
    return ret;
}

uint32_t fileSys_deleteBatchFtr(unsigned short ausFPIdx[],unsigned short usFPNum)
{
    uint32_t ret = 0;
    short index;

    unsigned char unSec         = 0;
    unsigned short unId         = 0;
    unsigned int i              = 0;
    unsigned char unUseSec      = 0xFF;     //使用扇区编号

    if(0 == g_stAllFtrHead.ftrNum)
    {
        return ret;
    }

    for(i = 0; i < usFPNum; i++)
    {
        index = fileSys_IdToIndex(ausFPIdx[i]);
        if(0 > index)
        {
            continue;
        }

        unId  = g_stAllFtrHead.ftrHead[index].unId;
        unSec = g_stAllFtrHead.ftrHead[index].unSec;
        if(unSec < unUseSec) unUseSec = unSec;      //保存较小的扇区号，为了提高写的速度

        g_stAllFtrHead.idHead[(unId >> 3)]         &= ~(1 << (7 - (unId & 0x7)));
        g_stAllFtrHead.secHead[(unSec >> 3)]       &= ~(1 << (7 - (unSec & 0x7)));
        g_stAllFtrHead.ftrHead[index].unFlag   = DEFAULT_FLAG;
        g_stAllFtrHead.ftrHead[index].unSec    = DEFAULT_SECTOR;
        g_stAllFtrHead.ftrHead[index].unId     = DEFAULT_ID;
        g_stAllFtrHead.ftrHead[index].unLength = DEFAULT_LENGTH;

        g_stAllFtrHead.ftrNum--;
//        g_stAllFtrHead.flashEraNum             = FlashEraseNum;
    }
    g_stAllFtrHead.chk = CRC32_calc((unsigned char *)&g_stAllFtrHead , FTR_HEAD_ALL_LENGTH - 4 );
    

    ret = flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    if(ret != COMP_CODE_OK)
        goto out;
    
    ret = flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_BACKUP_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL); 
out:
    if(COMP_CODE_OK != ret)
    {
        FILESYS_PRINT_INFO0("[FILE]:Update_Ftr_Head fail\r\n");
        return ret;
    }
    return ret;
}

uint32_t fileSys_deleteOneFtr(unsigned short id)
{
    uint32_t ret            = FILE_SYSTEM_OK;
    short index             = 0;
    uint8_t unSec           = 0;
    uint8_t unId            = 0;

    if(id > 0xff)
        return FILE_SYSTEM_PARAMETER_ERROR;

    index = fileSys_IdToIndex(id);
    if(0 > index)
    {
        FILESYS_PRINT_INFO1("[FILE]:id [%d] not exist!\r\n",id);
        return FILE_SYSTEM_FLASH_ID_ERROR;
    }

    unSec = g_stAllFtrHead.ftrHead[index].unSec;
    unId = g_stAllFtrHead.ftrHead[index].unId;

    g_stAllFtrHead.idHead[(unId >> 3)]         &= ~(1 << (7 - (unId & 0x7)));
    g_stAllFtrHead.secHead[(unSec >> 3)]       &= ~(1 << (7 - (unSec & 0x7)));
    g_stAllFtrHead.ftrHead[index].unFlag   = DEFAULT_FLAG;
    g_stAllFtrHead.ftrHead[index].unSec    = DEFAULT_SECTOR;
    g_stAllFtrHead.ftrHead[index].unId     = DEFAULT_ID;
    g_stAllFtrHead.ftrHead[index].unLength = DEFAULT_LENGTH;
    g_stAllFtrHead.ftrNum--;
//    g_stAllFtrHead.flashEraNum             = FlashEraseNum;
    
    g_stAllFtrHead.chk = CRC32_calc((unsigned char *)&g_stAllFtrHead , FTR_HEAD_ALL_LENGTH - 4 );

    ret = flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    if(ret != COMP_CODE_OK)
        goto out;
    
    ret = flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_BACKUP_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
out:
    if(COMP_CODE_OK != ret)
    {
        FILESYS_PRINT_INFO0("[FILE]:Update_Ftr_Head fail\r\n");
        return ret;
    }
    return ret;
}

//删除成员下面的所有指纹
uint32_t fileSys_deleteNumberFtr(unsigned short id)
{
    uint32_t ret            = FILE_SYSTEM_OK;
    uint8_t unSec           = 0;
    uint8_t unId            = 0;
	uint8_t i;

    if(id > 0x64 || id < 1)
        return FILE_SYSTEM_PARAMETER_ERROR;

	for(i = 0; i < STORE_MAX_FTR; i++)
	{
		if(g_stAllFtrHead.ftrHead[i].unNumID == id)
		{
			unSec = g_stAllFtrHead.ftrHead[i].unSec;
    		unId = g_stAllFtrHead.ftrHead[i].unId;

			g_stAllFtrHead.idHead[(unId >> 3)] &= ~(1 << (7 - (unId & 0x7)));
    		g_stAllFtrHead.secHead[(unSec >> 3)] &= ~(1 << (7 - (unSec & 0x7)));

			g_stAllFtrHead.ftrHead[i].unFlag   = DEFAULT_FLAG;
		    g_stAllFtrHead.ftrHead[i].unSec    = DEFAULT_SECTOR;
		    g_stAllFtrHead.ftrHead[i].unId     = DEFAULT_ID;
		    g_stAllFtrHead.ftrHead[i].unLength = DEFAULT_LENGTH;
			g_stAllFtrHead.ftrHead[i].unNumID  = DEFAULT_ID;
		    g_stAllFtrHead.ftrNum--;
		}
	}

    g_stAllFtrHead.chk = CRC32_calc((unsigned char *)&g_stAllFtrHead , FTR_HEAD_ALL_LENGTH - 4 );

    ret = flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    if(ret != COMP_CODE_OK)
        goto out;
    
    ret = flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_BACKUP_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
out:
    if(COMP_CODE_OK != ret)
    {
        FILESYS_PRINT_INFO0("[FILE]:Update_Ftr_Head fail\r\n");
        return ret;
    }
    return ret;
}


int fileSys_Init(void)
{
    int i;
    unsigned int chksum = 0;
    ST_STORAGE_FP_LINK stFtrPara = {0};
    unsigned char stoStatMaster = SETTING_STORAGE_NULL;
    unsigned char stoStatSlave = SETTING_STORAGE_NULL;

    //读取主区
    for(i = 0; i < 3 ; i++ )
    {

        flashReadBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);    
        chksum = CRC32_calc((unsigned char *)&g_stAllFtrHead , FTR_HEAD_ALL_LENGTH - 4 );

        if(chksum == g_stAllFtrHead.chk)
        {
            FILESYS_PRINT_INFO1("[FILE]:master chksum 0x%x OK!\r\n", chksum);
            stoStatMaster = SETTING_STORAGE_OK;
            break;
        }
        else
        {
            FILESYS_PRINT_INFO3("[FILE]:master chksum 0x%x != 0x%x error %d!\r\n", chksum, g_stAllFtrHead.chk, i);
            stoStatMaster = SETTING_STORAGE_CHK_ERR;
        }
    }
    //读取备份区
    for(i = 0; i < 3 ; i++ )
    {
        flashReadBuffer((unsigned char *)&stFtrPara, FTR_HEAD_BACKUP_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
        chksum = CRC32_calc((unsigned char *)&stFtrPara , FTR_HEAD_ALL_LENGTH - 4 );

        if(chksum == stFtrPara.chk)
        {
            FILESYS_PRINT_INFO1("[FILE]:backup chksum 0x%x OK!\r\n", chksum);
            stoStatSlave = SETTING_STORAGE_OK;
            break;
        }
        else
        {
            FILESYS_PRINT_INFO3("[FILE]:backup chksum 0x%x != 0x%x error %d!\r\n", chksum, stFtrPara.chk, i);
            stoStatSlave = SETTING_STORAGE_CHK_ERR;
        }
    }
    if ((SETTING_STORAGE_OK == stoStatMaster) && (SETTING_STORAGE_OK == stoStatSlave))
    {
        //主区和备份区数据都有效
        if(stFtrPara.chk != g_stAllFtrHead.chk)
        {
            FILESYS_PRINT_INFO0("[FILE]:slave chk error, update!\r\n");
            flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_BACKUP_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);          
        }
//        FILESYS_PRINT_INFO0("[FILE]:master and slave OK!\r\n");
    }
    else if ((SETTING_STORAGE_OK != stoStatMaster) && (SETTING_STORAGE_OK == stoStatSlave))
    {
        //主区数据无效，备份区数据有效，将备份区数据同步到主区
        FILESYS_PRINT_INFO0("[FILE]:master fail , slave OK!\r\n");
        memcpy(&g_stAllFtrHead, &stFtrPara, FTR_HEAD_ALL_LENGTH);

        flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);       
    }
    else if ((SETTING_STORAGE_OK == stoStatMaster) && (SETTING_STORAGE_OK != stoStatSlave))
    {
        //主区数据有效，备份区数据无效，将主区数据同步到备份区
        FILESYS_PRINT_INFO0("[FILE]:master OK , slave fail!\r\n");
        flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_BACKUP_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    }
    else
    {
        FILESYS_PRINT_INFO0("[FILE]:Not finger!\r\n");
        memset((unsigned char *)&g_stAllFtrHead , 0 , FTR_HEAD_ALL_LENGTH);
        g_stAllFtrHead.chk = CRC32_calc((unsigned char *)&g_stAllFtrHead , FTR_HEAD_ALL_LENGTH - 4 );

        //flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
        //flashWriteBuffer((unsigned char *)&g_stAllFtrHead, FTR_HEAD_BACKUP_ADDRESS, FTR_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    }

    return FILE_SYSTEM_OK;
}
