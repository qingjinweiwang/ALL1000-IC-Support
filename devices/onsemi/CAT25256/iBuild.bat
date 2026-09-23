@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

@REM ================================================================================================
@REM For ALL-1000G RVDS project.
@REM ================================================================================================
@REM [2026-05-15] V1.3   - Release modify by DeepSeek 
@REM                       1.add spc file copy
@REM                       2.err shows red
@REM                       3.warning shows yellow
@REM                       4.success shows green
@REM ================================================================================================
@REM Usage:
@REM    1. Place this batch file in the same folder as Driver:
@REM    ALL1000G_DEV/
@REM      ├─ Ahelp1/
@REM      ├─ Algo1/
@REM      ├─ Driver/
@REM      │    ├─ ALL1000Code/
@REM      │    ├─ Your_Driver1/
@REM      │    │    ├─ ibuild.bat
@REM      │    │    ├─ Driver1.c
@REM      │    │    ├─ Driver1.h
@REM      │    │    └─ Driver1.spc
@REM      │    └─ Your_Driver2/
@REM      ├─ FPGA/
@REM      └─ ALL-1000Mult.exe
@REM    2. Set RVDS_LIB and ARMCC_PATH.
@REM    3. Open CMD in the Driver folder path and run : "110 <Driver Name>".
@REM ================================================================================================

@REM ================================================================================================
@REM set "RVDS_LIB=C:\PROGRA~2\ARM\RVCT\Data\2.2\349\include\windows"
@REM set "ARMCC_PATH=C:\PROGRA~2\ARM\RVCT\Programs\2.2\349\win_32-pentium"
set "RVDS_LIB=C:\PROGRA~2\ARM\RVCT\Data\2.2\349\include\windows"
set "ARMCC_PATH=C:\PROGRA~2\ARM\RVCT\Programs\2.2\349\win_32-pentium"
@REM ================================================================================================

@REM ---------- 初始化 ANSI 颜色 ----------
for /f %%a in ('echo prompt $E^| cmd') do set "ESC=%%a"
set "RED=%ESC%[31m"
set "GREEN=%ESC%[32m"
set "YELLOW=%ESC%[33m"
set "RESET=%ESC%[0m"

set "Driver_Name=%~1"
set "Driver_Root=%~dp0"
set "Driver_Root=%Driver_Root:~0,-1%"

set "Driver_C=%Driver_Root%\%Driver_Name%.c"
set "Driver_Axf=%Driver_Root%\%Driver_Name%.axf"
set "Driver_Bin=%Driver_Root%\%Driver_Name%.bin"
set "Driver_O=%Driver_Root%\%Driver_Name%.o"
set "Map_Txt=%Driver_Root%\map.txt"

for %%I in ("%Driver_Root%\..") do set "Driver_Dir=%%~fI\"
set "Object_Lib=%Driver_Dir%ALL1000Code"
set "Header_Lib=%Driver_Dir%ALL1000Code\h"
for %%I in ("%Driver_Root%\..\..") do set "Dev_Root=%%~fI\"
set "Algo1_Path=D:\HILO\ALL1000G_DEV\Algo1\"

set "ARMCC=%ARMCC_PATH%\armcc.exe"
set "ARMLINK=%ARMCC_PATH%\armlink.exe"
set "FROMELF=%ARMCC_PATH%\fromelf.exe"

set "RVDS_PARAM= --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend --diag_suppress=1296"

set "Include_Files="
set "Include_Files=%Include_Files% -I"%Driver_Root%""
set "Include_Files=%Include_Files% -I"%Header_Lib%""
set "Include_Files=%Include_Files% -I"%Object_Lib%""
set "Include_Files=%Include_Files% -J"%RVDS_LIB%""
set "Include_Files=%Include_Files% -J"%RVDS_LIB%\rw""

set "Obj_Files="
set "Obj_Files=%Obj_Files% "%Object_Lib%\_stackheap.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\CFControl.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\Disp.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\FPGADrv.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\FWDMM.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\hs0_mmc.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\Hspi.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\Initial.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\ISR.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\Key.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\Main.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\MemInit.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\MMUCache_asm.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\SHA.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\startup.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\Subroutine.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\SysClock.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\SystemTask.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\uMROS.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\UpDnLoad.o""
set "Obj_Files=%Obj_Files% "%Object_Lib%\USBCom.o""
set "Obj_Files=%Obj_Files% "%Driver_O%""

@REM ================================================================================================
echo.

REM Step 1: check compilation tools

if not exist "%ARMCC%" (
    echo %RED%[FAIL] Cannot find %ARMCC%.%RESET%
    exit /b 1
)
if not exist "%ARMLINK%" (
    echo %RED%[FAIL] Cannot find %ARMLINK%.%RESET%
    exit /b 1
)
if not exist "%FROMELF%" (
    echo %RED%[FAIL] Cannot find %FROMELF%.%RESET%
    exit /b 1
)


REM Step 2: Build the driver project

echo --------------------------------------------------------------------------------------------------------------------------------
echo Compilation information:
echo --------------------------------------------------------------------------------------------------------------------------------

REM 执行编译，并将输出重定向到临时文件以便着色
set "BUILD_TMP=%TEMP%\build_%random%.tmp"
"%ARMCC%" -c %RVDS_PARAM% %Include_Files% "%Driver_C%" -o "%Driver_O%" > "%BUILD_TMP%" 2>&1
set "CC_ERR=%errorlevel%"

REM 逐行回显编译输出，警告用黄色，错误用红色
for /f "usebackq delims=" %%L in ("%BUILD_TMP%") do (
    set "line=%%L"
    echo !line! | findstr /I "warning:" >nul
    if not errorlevel 1 (
        echo %YELLOW%!line!%RESET%
    ) else (
        echo !line! | findstr /I "error:" >nul
        if not errorlevel 1 (
            echo %RED%!line!%RESET%
        ) else (
            echo !line!
        )
    )
)
del "%BUILD_TMP%" 2>nul

echo --------------------------------------------------------------------------------------------------------------------------------
if %CC_ERR% neq 0 (
    echo %RED%[FAIL] Compilation failed.%RESET%
    goto CLEAN_UP
)
if not exist "%Driver_O%" (
    echo %RED%[FAIL] Object file not generated: "%Driver_O%".%RESET%
    goto CLEAN_UP
)

"%ARMLINK%" %Obj_Files% -o "%Driver_Axf%" --map --ro_base=0x30000000 --rw_base=0x30200000 --first=startup.o(Init) --entry=0x30000000 --info=sizes --info=totals --list "%Map_Txt%"
if errorlevel 1 (
    echo %RED%[FAIL] Linking failed.%RESET%
    goto CLEAN_UP
)
if not exist "%Driver_Axf%" (
    echo %RED%[FAIL] Output file not generated: "%Driver_Axf%".%RESET%
    goto CLEAN_UP
)

"%FROMELF%" --bin "%Driver_Axf%" --output "%Driver_Bin%"
if errorlevel 1 (
    echo %RED%[FAIL] Format conversion failed.%RESET%
    goto CLEAN_UP
)
if not exist "%Driver_Bin%" (
    echo %RED%[FAIL] Binary file not generated: "%Driver_Bin%".%RESET%
    goto CLEAN_UP
)

echo %GREEN%[SUCCESS] Build successful.%RESET%


REM Step 3: Update the driver bin and spc files

copy /Y "%Driver_Bin%" "%Algo1_Path%" >nul
if errorlevel 1 (
    echo %RED%[FAIL] Copy BIN file to Algo1 failed.%RESET%
) else (
    echo %GREEN%[SUCCESS] Updated "%Algo1_Path%%Driver_Name%.bin".%RESET%
)

set "Driver_Spc=%Driver_Root%\%Driver_Name%.spc"
if exist "%Driver_Spc%" (
    copy /Y "%Driver_Spc%" "%Algo1_Path%" >nul
    if errorlevel 1 (
        echo %RED%[FAIL] Copy SPC file to Algo1 failed.%RESET%
    ) else (
        echo %GREEN%[SUCCESS] Updated "%Algo1_Path%%Driver_Name%.spc".%RESET%
    )
) else (
    echo %YELLOW%[WARNING] SPC file not found: "%Driver_Spc%", skipping copy.%RESET%
)


REM Step 4: Clean all generated files

:CLEAN_UP
del /f /q %Driver_Axf% 2>nul
del /f /q %Map_Txt% 2>nul
del /f /q %Driver_O% 2>nul
del /f /q %Driver_Bin% 2>nul
@REM echo.
echo. > con

exit /b