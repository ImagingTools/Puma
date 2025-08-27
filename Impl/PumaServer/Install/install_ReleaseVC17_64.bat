@echo on
set OUTPUT_FILE=tempFile.txt
..\..\..\..\ImtCore\3rdParty\Python\3.8\python.exe ..\..\..\..\ImtCore\Build\GetSvnVersion.py -D ..\..\..\ -F %OUTPUT_FILE%
set /p APP_VERSION=< %OUTPUT_FILE% 
IF EXIST %OUTPUT_FILE% DEL /F %OUTPUT_FILE%

set PUMA_BUILD_DIR=..\..\..\Bin\Release_Qt6_VC17_x64
iscc PumaServer.iss

