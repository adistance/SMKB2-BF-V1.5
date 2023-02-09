ECHO off
rem

:: Convert audio binary to HEX
:: without flash secure boot
 .\flash_download\hex\srec_cat "bin\app.bin"   -binary -offset 0x80D000 -o "objects\app.hex" -intel
 .\flash_download\hex\srec_cat "bin\app.bin"   -binary -offset 0x80D000 -o "flash_download\hex\app.hex" -intel
 
:: with ota header
:: .\flash_download\hex\srec_cat "flash_download\hex\OTAHeader_Bank0.bin"   -binary -offset 0x802000 -o "flash_download\hex\OTAHeader_Bank0.hex" -intel

:: with flash secure boot
:: .\flash_download\hex\srec_cat "bin\app.bin"   -binary -offset 0x811000 -o "objects\app.hex" -intel
:: .\flash_download\hex\srec_cat "bin\app.bin"   -binary -offset 0x811000 -o "flash_download\hex\app.hex" -intel