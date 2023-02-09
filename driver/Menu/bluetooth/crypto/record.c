
#include "record.h"
#include "trace.h"
#include "driver_rtc.h"
#include "tuya_ble_app_demo.h"
#include <peripheral_app.h>
#include "os_sched.h"

#define STORE_INLINE_MAX_FTR			(50)

__align(4) ST_STORAGE_BLE_RECORD_LINK g_stBleRecordHead;

static char updataFlag = 0;

int bleRecordGet(uint8_t *data, uint8_t *len, uint8_t nPage)
{
	if(data == NULL || len == NULL)
	{
		APP_PRINT_INFO0("bleRecordGet2 input param error");
		return 0;
	}
	
	int tmpLen = 0;
	uint16_t num = 0;
	short temp_num;
	uint8_t getPageNum = 0;
	num = g_stBleRecordHead.recordNum;
	
	if (num > BLE_RECORD_MAX_NUM)
		num = BLE_RECORD_MAX_NUM;
	
	temp_num = (num - (nPage-1)*BLE_RECORD_PAGE_MAX_SIZE);
    if(temp_num <= 0)
	{
		getPageNum = 0;
	}
    else if(temp_num < BLE_RECORD_PAGE_MAX_SIZE)
	{
		getPageNum = num-(nPage-1)*BLE_RECORD_PAGE_MAX_SIZE;
	}
    else
    {
        getPageNum = BLE_RECORD_PAGE_MAX_SIZE;
    }

	tmpLen = getPageNum * sizeof(STORAGE_RECORD_NODE);

	//APP_PRINT_INFO4("tmpLen %d, count:%d, nPage:%d, getPageNum = %d\r\n",tmpLen, num, nPage, getPageNum);
	
	*len = tmpLen + 2;
	data[0] = 1; //1��ʾ��ȷ�ظ���0�Ǵ���ظ�	
	data[1] = getPageNum; //ָ�Ƹ���

	//ÿ�λ�ȡ���µ�20��ָ�Ƽ�¼����
	memcpy(data+2, (uint8_t *)(&g_stBleRecordHead.record[(nPage - 1)*BLE_RECORD_PAGE_MAX_SIZE]), tmpLen); //������¼����	

	return 0;
}

//ͬ��������Ϣ������������true��ͬ�����ֱ�����flash��Ϊfalse��ͬ���겻���flash
void BleRecordSync(bool bClean)
{
	uint8_t i;

	if(g_stBleRecordHead.recordNum > BLE_RECORD_MAX_NUM)
		g_stBleRecordHead.recordNum = BLE_RECORD_MAX_NUM;

	if(g_stBleRecordHead.recordNum > 0)
	{
		for(i = 0; i < g_stBleRecordHead.recordNum; i++)
		{
			tuya_ble_get_record(g_stBleRecordHead.record[i].utcTime, g_stBleRecordHead.record[i].ucNumId - 1);
		}

		memset((unsigned char *)&g_stBleRecordHead , 0, BLE_RECORD_HEAD_ALL_LENGTH);
		g_stBleRecordHead.chk = GetChksum((unsigned char *)&g_stBleRecordHead , BLE_RECORD_HEAD_ALL_LENGTH - 4 );

		if(bClean)
			flashWriteBuffer((unsigned char *)&g_stBleRecordHead , RECORD_ADDRESS, sizeof(g_stBleRecordHead), EM_FLASH_CTRL_FTL);
	}
}

//���ÿ�����¼
void BleRecordSet(eModeCtrl moid, uint16_t index)
{
	uint16_t num;
       
	num = g_stBleRecordHead.recordNum;
	APP_PRINT_INFO2("Record get utc:%d, num is %d\r\n", rtc_get_time(), num);

	//���������ļ�¼��ʱ�������ɵ����ݸ��ǵ������µ����ݷ��������
	if (num >= BLE_RECORD_MAX_NUM)
	{
		num = BLE_RECORD_MAX_NUM - 1;
		g_stBleRecordHead.recordNum = BLE_RECORD_MAX_NUM;

		memmove(&g_stBleRecordHead.record[0], &g_stBleRecordHead.record[1], sizeof(g_stBleRecordHead.record[0])*(BLE_RECORD_MAX_NUM-1));
	}
	
	g_stBleRecordHead.record[num].ucNumId = index;
	g_stBleRecordHead.record[num].ucType = moid;
	g_stBleRecordHead.record[num].bUseFlag = true;
	g_stBleRecordHead.record[num].utcTime = rtc_get_time();
	
	g_stBleRecordHead.recordNum++;
		
	g_stBleRecordHead.chk = GetChksum((unsigned char *)&g_stBleRecordHead , BLE_RECORD_HEAD_ALL_LENGTH - 4 );

	if(app_get_bt_real_state() == 2) //����������,�����ϴ�������¼
	{
		if(g_stBleRecordHead.recordNum > 1) //����2�����ϵļ�¼���ϴ�������flash
			BleRecordSync(true);
		else     							//ֻ�е�ǰ��һ����û��Ҫ��flash������
			BleRecordSync(false);
	}
	else //û�������������Ȱ����ݱ�������
	{
		flashWriteBuffer((unsigned char *)&g_stBleRecordHead , RECORD_ADDRESS, sizeof(g_stBleRecordHead), EM_FLASH_CTRL_FTL);
	}
	    
	return;
}


void BleRecordUpdata(void)
{
    int ret;
    if(updataFlag == 1)
    {
    	//APP_PRINT_INFO1("BleRecordUpdata is %b", TRACE_BINARY(sizeof(g_stBleRecordHead), &g_stBleRecordHead));
        updataFlag = 0;		
		ret = flashWriteBuffer((unsigned char *)&g_stBleRecordHead , RECORD_ADDRESS, sizeof(g_stBleRecordHead), EM_FLASH_CTRL_FTL);
		if(ret != 0)
			APP_PRINT_INFO0("update record fail\r\n");
        
    }
}

void BleRecordInfoReset(void)
{
    memset((unsigned char *)&g_stBleRecordHead , 0, BLE_RECORD_HEAD_ALL_LENGTH);
	g_stBleRecordHead.chk = GetChksum((unsigned char *)&g_stBleRecordHead , BLE_RECORD_HEAD_ALL_LENGTH - 4 );
	flashWriteBuffer((unsigned char *)&g_stBleRecordHead , RECORD_ADDRESS, sizeof(g_stBleRecordHead) , EM_FLASH_CTRL_FTL);
}


void BleProductInfoInit(void)
{
    int i;
    unsigned int chksum = 0;

	//BleRecordInfoReset();
	APP_PRINT_INFO1("record len is %d", BLE_RECORD_HEAD_ALL_LENGTH);

    for(i = 0; i < 3 ; i++)
    {
        flashReadBuffer((unsigned char *)&g_stBleRecordHead, RECORD_ADDRESS, BLE_RECORD_HEAD_ALL_LENGTH,  EM_FLASH_CTRL_FTL);
        chksum = GetChksum((unsigned char *)&g_stBleRecordHead , BLE_RECORD_HEAD_ALL_LENGTH - 4 );
        if(chksum == g_stBleRecordHead.chk)
        {
        	APP_PRINT_INFO0("g_stBleRecordHead chksum suc");
            break;
        }
        else
        {
            if (i == 2)
            {
                memset((unsigned char *)&g_stBleRecordHead , 0, BLE_RECORD_HEAD_ALL_LENGTH);
	            g_stBleRecordHead.chk = GetChksum((unsigned char *)&g_stBleRecordHead , BLE_RECORD_HEAD_ALL_LENGTH - 4 );
                flashWriteBuffer((unsigned char *)&g_stBleRecordHead , RECORD_ADDRESS, sizeof(g_stBleRecordHead) , EM_FLASH_CTRL_FTL);     
                APP_PRINT_INFO0("g_stBleRecordHead reset\r\n");
            }
			os_delay(100);
        }
    }

	APP_PRINT_INFO1("g_stBleRecordHead is %b", TRACE_BINARY(sizeof(g_stBleRecordHead), &g_stBleRecordHead));
  
}

void GetFpList(uint8_t* listBuf)
{
    int i = 0, j = 2, ret = 0;
    
    listBuf[1] = fileSys_getStoreFtrNum();
	APP_PRINT_INFO1("---fileSys_getStoreFtrNum--- %d\n", fileSys_getStoreFtrNum());
    if (listBuf[1] > STORE_INLINE_MAX_FTR)
    {
        listBuf[0] = 0;
    }
    else
    {
        listBuf[0] = 1;
        for (i = 0; i < STORE_INLINE_MAX_FTR; ++i)
		{
			ret = fileSys_IdToIndex(i);
			if (ret != -1)
			{
				listBuf[j] = i+1;
				j++;
			}
		}
    }
    return;
}

