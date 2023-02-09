#include "password.h"
#include "trace.h"
#include "app_crypto.h"

//(3+16)*30+6+4 = 580 byte
typedef struct
{
    uint8_t unPswFlag;
    uint8_t unPswId;
    uint8_t unPswLen;
	uint8_t unPswBuf[STORE_MAX_PASSWORD_LEN];
	
}__attribute__((packed)) STORAGE_PSW_NODE;


typedef struct
{
    STORAGE_PSW_NODE pswHead[STORE_MAX_PASSWORD_NUM];
	uint8_t idHead[(STORE_MAX_PASSWORD_NUM/8)+1];
    uint32_t pswNum;
    uint32_t chk;
}__attribute__((packed)) ST_STORAGE_PSW_LINK;

static  ST_STORAGE_PSW_LINK g_stAllPswHead;
#define PSW_HEAD_ALL_LENGTH         (sizeof(ST_STORAGE_PSW_LINK))


int GetStorePswNum(void)
{
	return g_stAllPswHead.pswNum;
}


int PswIdToIndex(unsigned short id)
{
    int i;

    for(i = 0; i < STORE_MAX_PASSWORD_NUM; i++)
    {
        if((USE_FLAG == g_stAllPswHead.pswHead[i].unPswFlag) && (id == g_stAllPswHead.pswHead[i].unPswId))
            return i;
    }

    return -1;
	
}

void GetExistPswList(unsigned char *listBuf)
{
	int id = 0, i = 0;
	
	for (id = 1; id < STORE_MAX_PASSWORD_NUM; id++)
	{
		if (PswIdToIndex(id) >= 0)
		{
			listBuf[i] = id;
			i++;
		}
	}
}

int CopyPassword(unsigned short id, unsigned char *PswBuffer)
{
	int ret = 0;
	uint8_t len = 0;
	uint8_t passWord[] = {1, 2, 3, 4, 5, 6};
	
	ret = PswIdToIndex(id);
	if (ret > -1)
	{
		len = g_stAllPswHead.pswHead[id].unPswLen;
		PswBuffer[0] = len;
		memcpy(&PswBuffer[1], g_stAllPswHead.pswHead[id].unPswBuf, len);
	}
	else
	{
		PswBuffer[0] = sizeof(passWord);
		memcpy(&PswBuffer[1],passWord, sizeof(passWord));
		return -1;
	}

	return ret;
}

unsigned int GetUnuseSmallestId(unsigned short StartId , unsigned short *id)
{
    unsigned char i,j;
    unsigned char unId = 0;
    unsigned int maxIdNum = sizeof(g_stAllPswHead.idHead);

	i = StartId/8;
	j = StartId%8;

    for( ; i < maxIdNum; i++)
    {
        if(0xff != g_stAllPswHead.idHead[i])
        {
            for( ; j < 8 ; j++)
            {
                if(0 == ((g_stAllPswHead.idHead[i] >> (7 - j)) & 1))
                {
                	unId = ((i << 3) + j);
					*id = unId;
                    return 0;
                }
            }
        }
		j = 0;
    }

    return 1;
}


int StorePassword(unsigned short id, unsigned char *PswBuffer, unsigned int length)
{
    int ret;
	short unIndex = 0;
	uint8_t flag = 0;

	if((0 == length) || (length > STORE_MAX_PASSWORD_LEN) )
        return -1;
	
	if (id >= STORE_MAX_PASSWORD_NUM)
	{
		return -1;
	}
	
	unIndex = PswIdToIndex(id);
	if(unIndex < 0)     //id不存在
    {
		unIndex = id;
		g_stAllPswHead.pswNum++;
    }
    else                //id存在
    {
  		flag = 1;
    }
	
	g_stAllPswHead.pswHead[unIndex].unPswFlag   = USE_FLAG;
	g_stAllPswHead.pswHead[unIndex].unPswId     = id;
	g_stAllPswHead.pswHead[unIndex].unPswLen    = length;
	memcpy(&g_stAllPswHead.pswHead[unIndex].unPswBuf[0], PswBuffer, length);
    g_stAllPswHead.idHead[(id >> 3)]|= (1 << ( 7 - (id & 0x7)));
   	g_stAllPswHead.chk = GetChksum((unsigned char *)&g_stAllPswHead , PSW_HEAD_ALL_LENGTH - 4 );

	ret = flashWriteBuffer((unsigned char *)&g_stAllPswHead, PASSWORD_ADDRESS, PSW_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
    if(0 != ret)
		return  ret;

	return flag;
}


int MatchPassword(uint8_t *passwordBuf, uint8_t passwordLen, uint8_t matchIdNum)
{
	uint16_t i;
	uint8_t unPasswordLen = 0, compareNum = 0;
	int retCmp = 0;

	if (matchIdNum > STORE_MAX_PASSWORD_NUM)
	{
		return -1;
	}

	if (0 == GetStorePswNum)
	{
		return -1;
	}
	
	for(i = 0; i < matchIdNum; i++)
    {
		if(USE_FLAG == g_stAllPswHead.pswHead[i].unPswFlag)
		{
			unPasswordLen = g_stAllPswHead.pswHead[i].unPswLen;
			if (passwordLen >= unPasswordLen)
			{
				compareNum = 0;
				while(compareNum <= passwordLen-unPasswordLen)
				{
					retCmp = memcmp(&passwordBuf[compareNum], &g_stAllPswHead.pswHead[i].unPswBuf[0], unPasswordLen);
					if(0 == retCmp)
					{
						return g_stAllPswHead.pswHead[i].unPswId;
					}
					compareNum++; 
				}
			}

		}
	}
		
	return -1;
}

void DeleteAllPassword(void)
{
   // uint8_t pw[6] = {1,2,3,4,5,6}; 
	memset((unsigned char *)&g_stAllPswHead, 0x00, PSW_HEAD_ALL_LENGTH);
	g_stAllPswHead.chk = GetChksum((unsigned char *)&g_stAllPswHead , PSW_HEAD_ALL_LENGTH - 4 );

	flashWriteBuffer((unsigned char *)&g_stAllPswHead , PASSWORD_ADDRESS , PSW_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
   // app_crypto_set_pw(pw, 6);

	return ;
}

int DeleteOnePassword(unsigned short id)
{
    short index = 0;
    unsigned char unId = 0;

    if(id >= STORE_MAX_PASSWORD_NUM)
        return -1;

    index = PswIdToIndex(id);
    if(0 > index)//不存在
    {
        return 1;
    }

    unId = g_stAllPswHead.pswHead[index].unPswId;

    g_stAllPswHead.idHead[(unId >> 3)]         &= ~(1 << (7 - (unId & 0x7)));
	g_stAllPswHead.pswHead[index].unPswFlag   = 0;
    g_stAllPswHead.pswHead[index].unPswId    = 0;
    g_stAllPswHead.pswHead[index].unPswLen     = 0;
	memset(&g_stAllPswHead.pswHead[index].unPswBuf[0], 0, STORE_MAX_PASSWORD_LEN);
    g_stAllPswHead.pswNum--;
	g_stAllPswHead.chk = GetChksum((unsigned char *)&g_stAllPswHead , PSW_HEAD_ALL_LENGTH - 4 );
    

	flashWriteBuffer((unsigned char *)&g_stAllPswHead , PASSWORD_ADDRESS , PSW_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);

	return 0;
}


int PswFileInit(void)
{
    int i,ret;
	unsigned int chksum = 0;

	//APP_PRINT_INFO1("password len is %d", PSW_HEAD_ALL_LENGTH);

    //读取主区
    memset((unsigned char *)&g_stAllPswHead, 0xff, PSW_HEAD_ALL_LENGTH);
    for(i = 0; i < 3; i++ )
    {
        ret = flashReadBuffer((unsigned char *)&g_stAllPswHead, PASSWORD_ADDRESS, PSW_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);
        if(0 != ret)
        {
            return ret;
        }

        chksum = GetChksum((unsigned char *)&g_stAllPswHead, PSW_HEAD_ALL_LENGTH - 4 );
        if(chksum == g_stAllPswHead.chk)
        {
            break;
        }
		else
		{
			if (i >= 2)
			{
				memset((unsigned char *)&g_stAllPswHead, 0x00, PSW_HEAD_ALL_LENGTH);
				g_stAllPswHead.chk = GetChksum((unsigned char *)&g_stAllPswHead , PSW_HEAD_ALL_LENGTH - 4 );

				flashWriteBuffer((unsigned char *)&g_stAllPswHead , PASSWORD_ADDRESS , PSW_HEAD_ALL_LENGTH, EM_FLASH_CTRL_FTL);	
                APP_PRINT_INFO0("g_stAllPswHead reset\r\n");
            }
		}
		  
    }

	APP_PRINT_INFO2("ps num is %d, data is %b", g_stAllPswHead.pswNum, TRACE_BINARY(sizeof(g_stAllPswHead), &g_stAllPswHead));
    if(g_stAllPswHead.pswNum != 0)
    {
        //app_crypto_set_pw(g_stAllPswHead.pswHead[1].unPswBuf,  g_stAllPswHead.pswHead[1].unPswLen);
    }
    return 0;
}



