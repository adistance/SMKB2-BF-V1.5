@echo on

rem ���û�������

set HW_VER=EL610C_TY
set SW_VER=101
rem Э��汾 00--ML
set PROTOCOL_VER_ML_TZ=00
set PACK_DATE=%date:~0,4%%date:~5,2%%date:~8,2%

set APPPATH=%cd%\..\..\bin
set OUTPUTPATH=%cd%

rem set APPBIN=%APPPATH%\ML_FPM.bin  //��дҪ�����bin�ļ�·��
set APPBIN=%APPPATH%\app.bin
set OUTPUTFILE_ML_TZ=%OUTPUTPATH%\%HW_VER%_%PROTOCOL_VER_ML_TZ%_v%SW_VER%_ML_TZ_%PACK_DATE%.bin
rem set APPSIZE=88 //Ҫ��app����д4K�ı���
set APPSIZE=112
set UPDATE_CRC_BLOCK_SIZE=4

set SystemSettingPath_ML_TZ=%cd%\..\SystemSetting_ML_TZ.xml

rem ��������
UpdateEnc.exe %APPBIN% %OUTPUTFILE_ML_TZ% %APPSIZE% %UPDATE_CRC_BLOCK_SIZE% %SystemSettingPath_ML_TZ%


rem call AutoPacketApp.bat