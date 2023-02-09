#ifndef __PASSWORD_H__
#define __PASSWORD_H__

#include "filesys.h"


#define STORE_MAX_PASSWORD_LEN               (16)
#define STORE_MAX_PASSWORD_NUM               (5)

#define USE_FLAG    	            		(1)

int PswFileInit(void);
int GetStorePswNum(void);
int StorePassword(unsigned short id, unsigned char *PswBuffer, unsigned int length);
int MatchPassword(uint8_t *passwordBuf, uint8_t passwordLen, uint8_t matchIdNum);
int CopyPassword(unsigned short id, unsigned char *PswBuffer);
void DeleteAllPassword(void);
unsigned int GetUnuseSmallestId(unsigned short StartId , unsigned short *id);
void GetExistPswList(unsigned char *listBuf);
int DeleteOnePassword(unsigned short id);


#endif

