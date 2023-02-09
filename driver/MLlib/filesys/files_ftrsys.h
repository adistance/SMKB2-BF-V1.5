#ifndef _FILES_FTRSYS_H
#define _FILES_FTRSYS_H

#include "mlapi.h"

#define FTRHEAD_SEC_MAX_NUM                 (1)             //8K
#define SINGER_FTRHEAD_NUM                  (25)            //单sec存放数

#define FTRHEAD_DATALENGTH                  (64)            //有效数据
#define FTRHEAD_DATAALLLENGTH               (70)            //有效数据
#define LENG_SINGERFTRINFO                  (FTRHEAD_DATAALLLENGTH * SINGER_FTRHEAD_NUM + 4)   //该sector存放的info+4个字节的flash擦写次数（不可超过4k）


typedef struct{
    unsigned char *ftrBuffer;
    unsigned int ftrleng;
    short ftr_Index;
    unsigned short ftr_updatetimes;    
    unsigned char FtrInfo_Sec;
} ftrInfo_para, *p_ftrInfo_para;

int ftrsys_read_ftrinfo(p_ftrInfo_para p_ftrpara);
int ftrsys_SaveFtrInfo(p_ftrInfo_para p_ftrpara);
int ftrsys_get_MaxFlashWriteTimes(void);


#endif
