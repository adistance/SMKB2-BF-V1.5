#include "action.h"
#include "files_ftrsys.h"
#include "filesys.h"
#include "driver_sensor.h"
#include "string.h"


typedef struct{
    unsigned int BaseAdd;
    unsigned int Offset;
    unsigned int Len;
}FlashPoint, *FlashPoint_p;


typedef struct{
    FlashPoint DataPoint;
    unsigned char *databuff;
    unsigned char Sec;                         		//页数
    short Index;                               		//当前指纹编号index
    short Index_nun;                           		//当前表编号
    unsigned short FtrHeadInfo_updatetimes;      	//当前ftr的update次数
    unsigned char FtrHeadInfo[FTRHEAD_DATALENGTH];  //当前ftr头信息
    unsigned int FtrHeadInfo_check;             	//当前ftr头信息crc
    int FlashWriterNum;                         	//Flash操作次数
	unsigned char rsv[2];
} ftrsys_Object, *ftrsys_Object_p;

static ftrsys_Object s_odj_ftrsys;
uint8_t g_ucFTRInfoBuf[2048] = {0xFF};

unsigned int g_uiFlashEraseNum;
#define FTRINFO_DATABUFF                     g_ucFTRInfoBuf//      (g_EnrollFtrBuf + SENSOR_FTR_BUFFER_MAX - LENG_SINGERFTRINFO - 4)//(g_ImgBuf+ SENSOR_IMG_BUFFER_MAX-LENG_SINGERFTRINFO-4)//注意

/*****************************************************************************
类：FlashPoint 
功能描述  :     Flash操作可抽象成Flashz坐标点的操作。

数据成员：
 unsigned int BaseAdd;
 unsigned int Offset;
 unsigned int Len;

函数成员：
 FlashPoint_stor();
 FlashPoint_setbyLen();
 FlashPoint_setbyOffset();
 FlashPoint_movebyLen();
 FlashPoint_save();
 FlashPoint_read();
 
 修改历史      :
  1.日    期   : 2020年3月
    作    者   : phager
    修改内容   : 新生成函数

*****************************************************************************/
void FlashPoint_movebyLen(FlashPoint_p const me, int const dz)
{
    me->Len += (dz);
}

void FlashPoint_setbyLen(FlashPoint_p const me, int const len)
{
    me->Len = (len);
}


void FlashPoint_setbyOffset(FlashPoint_p const me, unsigned int const Offset)
{
    me->Offset = (Offset);
}


void FlashPoint_save(FlashPoint_p const me, unsigned char *Buffer)
{
    flashWriteBuffer(Buffer, me->BaseAdd + me->Offset, me->Len, EM_FLASH_CTRL_INSIDE);
}


void FlashPoint_read(FlashPoint_p const me, unsigned char *Buffer)
{
    flashReadBuffer(Buffer, me->BaseAdd + me->Offset, me->Len, EM_FLASH_CTRL_INSIDE);
}

void FlashPoint_stor(FlashPoint_p const me, unsigned int const BaseAdd, unsigned int const Offset, unsigned int const Len)
{
    me->BaseAdd = (BaseAdd);
    me->Offset = (Offset);
    me->Len = (Len);
}

void ftrsys_OBJ_setbyIndex(ftrsys_Object_p const me, short const index)
{
    me->Index = index;
    me->Index_nun = index % SINGER_FTRHEAD_NUM;
}

void ftrsys_OBJ_setbyftrinfo(ftrsys_Object_p const me, unsigned char *Buffer)
{
    memcpy(me->FtrHeadInfo, Buffer, FTRHEAD_DATALENGTH);
}

void ftrsys_OBJ_setbyftrinfocrc(ftrsys_Object_p const me, unsigned char *Buffer, unsigned int leng)
{
	me->FtrHeadInfo_check = CRC32_calc(Buffer, leng);
}

void ftrsys_OBJ_setbyftrUpdatetimes(ftrsys_Object_p const me, unsigned short Updatetimes)
{
	me->FtrHeadInfo_updatetimes = Updatetimes;
}

void ftrsys_OBJ_setbySec(ftrsys_Object_p const me, unsigned char const Sec)
{
    me->Sec = (Sec);
}

void ftrsys_OBJ_setbyFlashWriteNum(ftrsys_Object_p const me, unsigned int const Num)
{
    me->FlashWriterNum = (Num);
}

void ftrsys_OBJ_setbyDatabuff(ftrsys_Object_p const me, unsigned char *Buffer)
{
    memset(Buffer, 0xff, LENG_SINGERFTRINFO+4);
    me->databuff = (Buffer);
}

void ftrsys_OBJ_setbyDatapoint(ftrsys_Object_p const me)
{
    FlashPoint_stor(&(me->DataPoint), FTR_INFO_BASE_ADDRESS, (me->Sec)*FLASH_SECTOR_SIZE_8K, LENG_SINGERFTRINFO);
}

//对读出来整个ftr进行校验
unsigned char ftrsys_OBJ_getftrinfo(ftrsys_Object_p const me, unsigned char *Buffer, unsigned int Ftrlen)
{    
    unsigned int Check;

    Check = me->FtrHeadInfo_check;
    memcpy(Buffer, me->FtrHeadInfo, FTRHEAD_DATALENGTH);

    if (0 != Check - CRC32_calc(Buffer, Ftrlen))
    {
        return 1;
    }
    
    return 0;
}

unsigned short ftrsys_OBJ_getftrUpdatetimes(ftrsys_Object_p const me)
{    
    return me->FtrHeadInfo_updatetimes;
}

unsigned char ftrsys_OBJ_getSec(ftrsys_Object_p const me)
{    
    return me->Sec;
}

void ftrsys_OBJ_load_FlashWriteNum(ftrsys_Object_p const me)
{    
    unsigned char buff[4] = {0};
    
    FlashPoint_setbyOffset(&(me->DataPoint), (me->Sec)*FLASH_PAGE_SIZE + FTRHEAD_DATAALLLENGTH * SINGER_FTRHEAD_NUM);
    FlashPoint_setbyLen(&(me->DataPoint), 4);
    FlashPoint_read(&(me->DataPoint), buff);
    
    me->FlashWriterNum = ((buff[3] << 24) | (buff[2] << 16) | (buff[1] << 8) | buff[0]);

}


void ftrsys_OBJ_load_FtrHeadInfo(ftrsys_Object_p const me)
{
    unsigned char buff[FTRHEAD_DATAALLLENGTH];
    unsigned char offset;

    FlashPoint_setbyOffset(&(me->DataPoint), (me->Sec)*FLASH_PAGE_SIZE + (me->Index_nun)*FTRHEAD_DATAALLLENGTH);
    FlashPoint_setbyLen(&(me->DataPoint), FTRHEAD_DATAALLLENGTH);
    FlashPoint_read(&(me->DataPoint), buff);

    memcpy(me->FtrHeadInfo, buff, FTRHEAD_DATALENGTH);

    offset = FTRHEAD_DATALENGTH;
    me->FtrHeadInfo_check = ((buff[offset + 3] << 24) | 
                            (buff[offset +2] << 16) | 
                            (buff[offset +1] << 8) | 
                            buff[offset]);
    offset += 4;
    me->FtrHeadInfo_updatetimes = ((buff[offset+1] << 8) | buff[offset]);
}

static int ftrsys_OBJ_load_DataBlock(ftrsys_Object_p const me)
{
    unsigned int Check;
    unsigned char Crc[4];
	
    FlashPoint_movebyLen(&(me->DataPoint), 4);
    FlashPoint_read(&(me->DataPoint), me->databuff);
    FlashPoint_movebyLen(&(me->DataPoint), -4);

    memcpy(Crc, (me->databuff + me->DataPoint.Len), 4);

    Check = ((Crc[3] << 24) | (Crc[2] << 16) | (Crc[1] << 8) | Crc[0]);

    if (0 != Check - CRC32_calc(me->databuff, me->DataPoint.Len))
    {       
        return 1;
    }
    return 0;
}
//64info+4CRC+2updatatimes... 4flashEase+4crc
static int ftrsys_OBJ_save(ftrsys_Object_p const me)
{
    unsigned int Check;
    unsigned short Updatetimes;
    unsigned char Crc[4];
    unsigned char *p;

    //先将FtrHeadInfo导入DataBlock
    p = me->databuff + (me->Index_nun)*FTRHEAD_DATAALLLENGTH;
        
    memcpy(p, me->FtrHeadInfo, FTRHEAD_DATALENGTH);

    p += FTRHEAD_DATALENGTH;

    //先将FtrHeadInfocrc导入DataBlock
	Check = me->FtrHeadInfo_check;
	Crc[0] = (Check >> 0) & 0xFF;
	Crc[1] = (Check >> 8) & 0xFF;
	Crc[2] = (Check >> 16) & 0xFF;
	Crc[3] = (Check >> 24) & 0xFF;
    memcpy(p, Crc, 4);

    p += 4;

    //先将Updatetimes导入DataBlock
	Updatetimes = me->FtrHeadInfo_updatetimes;
	Crc[0] = (Updatetimes >> 0) & 0xFF;
	Crc[1] = (Updatetimes >> 8) & 0xFF;
    memcpy(p, Crc, 2);

    //再将FlashWriterNum导入DataBlock
	Crc[0] = (me->FlashWriterNum >> 0) & 0xFF;
	Crc[1] = (me->FlashWriterNum >> 8) & 0xFF;
	Crc[2] = (me->FlashWriterNum >> 16) & 0xFF;
	Crc[3] = (me->FlashWriterNum >> 24) & 0xFF;
    memcpy((me->databuff + SINGER_FTRHEAD_NUM*FTRHEAD_DATAALLLENGTH), Crc, 4);		
    
    //再存DataBlock到flash中 
	Check = CRC32_calc(me->databuff, me->DataPoint.Len);
	Crc[0] = (Check >> 0) & 0xFF;
	Crc[1] = (Check >> 8) & 0xFF;
	Crc[2] = (Check >> 16) & 0xFF;
	Crc[3] = (Check >> 24) & 0xFF;
    memcpy((me->databuff + me->DataPoint.Len), Crc, 4);

    FlashPoint_movebyLen(&(me->DataPoint), 4);
    FlashPoint_save(&(me->DataPoint), me->databuff);

    //回读校验 
    FlashPoint_movebyLen(&(me->DataPoint), -4);

    FlashPoint_read(&(me->DataPoint), me->databuff);

    memcpy(Crc, (me->databuff + me->DataPoint.Len), 4);

    Check = ((Crc[3] << 24) | (Crc[2] << 16) | (Crc[1] << 8) | Crc[0]);
    if (0 != Check - CRC32_calc(me->databuff, me->DataPoint.Len))
    {       
        return 1;
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : ftrsys_get_MaxFlashWriteTimes
 功能描述  :     对外接口，获取擦写Flash的次数
 输入参数  :  void
 输出参数  : 擦写次数
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2020年3月
    作    者   : phager
    修改内容   : 新生成函数

*****************************************************************************/

int ftrsys_get_MaxFlashWriteTimes(void)
{
    int NumTemp = 0;
    int i;
    
    for(i= 0; i<FTRHEAD_SEC_MAX_NUM;i++)
    {
        ftrsys_OBJ_setbyDatabuff(&s_odj_ftrsys, FTRINFO_DATABUFF);
        ftrsys_OBJ_setbySec(&s_odj_ftrsys, i);
        ftrsys_OBJ_setbyDatapoint(&s_odj_ftrsys);

        if(ftrsys_OBJ_load_DataBlock(&s_odj_ftrsys))
        {
            s_odj_ftrsys.FlashWriterNum = 0;    
        }
        else
        {
            ftrsys_OBJ_load_FlashWriteNum(&s_odj_ftrsys);
        }

        if(s_odj_ftrsys.FlashWriterNum > NumTemp)
        {
            NumTemp = s_odj_ftrsys.FlashWriterNum;
        }
    }

    return NumTemp;
}


/*****************************************************************************
 函 数 名  : ftrsys_get_ftrinfo
 功能描述  :     对外接口，获取指定index的ftr头信息
 输入参数  :   
             unsigned char FtrInfo_Sec
             unsigned char *Buffer
             unsigned short index 
 输出参数  : 错误码
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2020年3月
    作    者   : phager
    修改内容   : 新生成函数

*****************************************************************************/
int ftrsys_read_ftrinfo(p_ftrInfo_para p_ftrpara)
{
    ftrsys_OBJ_setbyDatabuff(&s_odj_ftrsys, FTRINFO_DATABUFF);			//清buffer

    /*页不同，加载旧页*/
    if (p_ftrpara->FtrInfo_Sec != ftrsys_OBJ_getSec(&s_odj_ftrsys))     //判断是否是同一页       
    {
        ftrsys_OBJ_setbySec(&s_odj_ftrsys, p_ftrpara->FtrInfo_Sec);		//把页的值赋给全局变量
        ftrsys_OBJ_setbyDatapoint(&s_odj_ftrsys);						//确定该sector的地址信息和长度信息
        if(ftrsys_OBJ_load_DataBlock(&s_odj_ftrsys))					//加载数据块并对数据进行校验
        {
            return 1;    
        }
    }
    else
    {
        ftrsys_OBJ_setbyDatapoint(&s_odj_ftrsys);						//确定该sector的地址信息和长度信息
    }

    ftrsys_OBJ_setbyIndex(&s_odj_ftrsys, p_ftrpara->ftr_Index);			//根据index找到info位置
    ftrsys_OBJ_load_FtrHeadInfo(&s_odj_ftrsys);							//将info读出来

    if(0 != ftrsys_OBJ_getftrinfo(&s_odj_ftrsys, p_ftrpara->ftrBuffer, p_ftrpara->ftrleng))	//校验一下对不对
    {
        return 1;
    }

    p_ftrpara->ftr_updatetimes =  ftrsys_OBJ_getftrUpdatetimes(&s_odj_ftrsys);

    return 0;
}

/*****************************************************************************
 函 数 名  : ftrsys_SaveFtrInfo
 功能描述  :     对外接口，保存指定index的ftr头信息
 输入参数  :   
             unsigned char FtrInfo_OldSec
             unsigned char FtrInfo_unUseSec
             short ftr_Index
             unsigned char*ftrBuffer
             unsigned int leng

 输出参数  : 错误码
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2020年3月
    作    者   : phager
    修改内容   : 新生成函数

*****************************************************************************/
int ftrsys_SaveFtrInfo(p_ftrInfo_para p_ftrpara)
{
    int ret = 0;
    
    ftrsys_OBJ_setbyDatabuff(&s_odj_ftrsys, FTRINFO_DATABUFF);          //先将要用的buff清空(0xFF)
    ftrsys_OBJ_setbySec(&s_odj_ftrsys, p_ftrpara->FtrInfo_Sec);         //确定该info所在的sector

    ftrsys_OBJ_setbyDatapoint(&s_odj_ftrsys);                           //设置该info所在的4K空间的结构体信息(基地址，偏移地址，长度信息，长度包含了flash的擦写次数)
    ftrsys_OBJ_load_DataBlock(&s_odj_ftrsys);                           //将整个sector的读出来校验一下，Flash的擦写次数也校验进去了

    ftrsys_OBJ_setbyIndex(&s_odj_ftrsys, p_ftrpara->ftr_Index);         //确定index(根据id得到的)，确定在该sector中的第几个
    ftrsys_OBJ_setbyftrinfo(&s_odj_ftrsys, p_ftrpara->ftrBuffer);       //将新的info保存在结构体中
    ftrsys_OBJ_setbyftrinfocrc(&s_odj_ftrsys, p_ftrpara->ftrBuffer, p_ftrpara->ftrleng);    //将整个ftr进行CRC检验，然后保存在结构里中
    ftrsys_OBJ_setbyftrUpdatetimes(&s_odj_ftrsys, p_ftrpara->ftr_updatetimes);              //保存该info更新的次数
    ftrsys_OBJ_setbyFlashWriteNum(&s_odj_ftrsys, g_uiFlashEraseNum+1);      //保存flash擦写次数

    ret = ftrsys_OBJ_save(&s_odj_ftrsys);                               //写flash并对整个sector进行校验一次

    return ret;
}
