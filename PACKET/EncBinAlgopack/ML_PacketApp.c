#include "time.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define FLASH_DRV_ADDERSS			(0)
#define FP_ALGO_ADDRESS				(4*1024)

const char* flash_path              ="mlsto.bin";
const char* algo_path             	="FpsAlgo.bin";

int main(int argc,char **argv)
{
	int flash_len = 0,algo_len = 0;
	char updata_path[100];
	char *buffer = NULL;

	if(argc != 2)
	{
		printf("Parameter quantity error!\r\nParameter Number:%d\r\n",argc);
		getchar();
		return 1;
	}

	FILE *flash_fp=fopen(flash_path,"rb");			//flash_fp文件指针
	if(NULL == flash_fp)
	{
		printf("File %s open failed!\r\n",flash_path);
		fclose(flash_fp);
		getchar();
		return 1;
	}

	FILE *algo_fp=fopen(algo_path,"rb");			//algo_fp文件指针
	if(NULL == algo_fp)
	{
		printf("File %s open failed!\r\n",algo_path);
		fclose(flash_fp);
		fclose(algo_fp);
		getchar();
		return 1;
	}

	FILE *updata_fp= fopen(argv[1],"wb");
	if(NULL == updata_fp)
	{
		printf("File %s open failed!\r\n",argv[1]);
		fclose(flash_fp);
		fclose(algo_fp);
		fclose(updata_fp);
		getchar();
		return 1;
	}

	fseek(flash_fp,0, SEEK_END );
	flash_len = ftell(flash_fp);						//计算flash的大�?

	fseek(algo_fp,0, SEEK_END );
	algo_len = ftell(algo_fp);                           //计算algo的大�?

	fseek(flash_fp ,0 ,SEEK_SET );
	fseek(algo_fp ,0 ,SEEK_SET );						//将指针都偏移到文件开始地址

	buffer = (char *)malloc(128*1024);		//512K
	memset(buffer , 0xff , 128*1024);

	fread(buffer + FLASH_DRV_ADDERSS ,1 ,flash_len ,flash_fp);			            //把loader的内容全部读出来放在buffer�?
	fread(buffer + FP_ALGO_ADDRESS ,1 ,algo_len ,algo_fp);			                    //把app的内容全部读出来放在buffer�?
	fwrite(buffer,1 ,128*1024 , updata_fp);	                            //把buffer中的内容一次性写进新的文件中

	fclose(flash_fp);
	fclose(algo_fp);
    fclose(updata_fp);

	free(buffer);
	return 0;
}
