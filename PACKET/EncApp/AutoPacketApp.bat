@echo on

rem ���û�������

set HW_VER=EL605_WR_TY
set SW_VER=105
rem Э��汾 00--ML
set PROTOCOL_VER_ML_TZ=00
rem set PROTOCOL_VER_SY=12
set PACK_DATE=%date:~0,4%%date:~5,2%%date:~8,2%

set APPPATH=%cd%\..\..\bin
rem set APPPATH=%cd%\..\..\bin
set OUTPUTPATH=%cd%

rem set APPBIN=%APPPATH%\ML_FPM.bin  //��дҪ�����bin�ļ�·��
set APPBIN=%APPPATH%\app.bin
set OUTPUTFILE_ML_TZ=%OUTPUTPATH%\%HW_VER%_%PROTOCOL_VER_ML_TZ%_v%SW_VER%_ML_TZ_%PACK_DATE%.bin
rem set OUTPUTFILE_SY=%OUTPUTPATH%\%HW_VER%_%PROTOCOL_VER_SY%_v%SW_VER%_SY_%PACK_DATE%.bin
rem set APPSIZE=88 //Ҫ��app����д4K�ı���
set APPSIZE=152
set UPDATE_CRC_BLOCK_SIZE=4

set SystemSettingPath_ML_TZ=%cd%\..\SystemSetting_ML_TZ.xml
rem set SystemSettingPath_SY=%cd%\..\SystemSetting_SY.xml

rem ��������
ElockAppPack_Vs_V2.3.exe %APPBIN% %OUTPUTFILE_ML_TZ% %APPSIZE% %UPDATE_CRC_BLOCK_SIZE% %SystemSettingPath_ML_TZ%
rem UpdateEnc.exe %APPBIN% %OUTPUTFILE_SY% %APPSIZE% %UPDATE_CRC_BLOCK_SIZE% %SystemSettingPath_SY%

rem call AutoPacketApp.bat