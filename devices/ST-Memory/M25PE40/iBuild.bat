@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

@REM ================================================================================================
@REM For ALL-1000G RVDS project.
@REM ================================================================================================
@REM [2026-05-15] V1.0   - Release
@REM ================================================================================================
@REM Usage:
@REM    1. Place this batch file in the same folder as Driver:
@REM    ALL1000_DEV/
@REM      ├─ Ahelp1/
@REM      ├─ Algo1/
@REM      ├─ Driver_G/
@REM      │    ├─ ALL1000Code/
@REM      │    ├─ Your_Driver1/
@REM      │    │    ├─ ibuild.bat
@REM      │    │    ├─ Driver1.c
@REM      │    │    ├─ Driver1.h
@REM      │    │    └─ Driver1.spc
@REM      │    └─ Your_Driver2/
@REM      ├─ Driver_ISP/
@REM      ├─ FPGA/
@REM      └─ ALL-1000Mult.exe
@REM    2. Set RVDS_LIB and ARMCC_PATH.
@REM    3. Open CMD in the Driver folder path and run : "110 <Driver Name>".
@REM ================================================================================================

@REM ================================================================================================
set "RVDS_LIB=C:\PROGRA~2\ARM\RVCT\Data\2.2\349\include\windows"
set "ARMCC_PATH=C:\PROGRA~2\ARM\RVCT\Programs\2.2\349\win_32-pentium"
@REM set "RVDS_LIB=C:\PROGRA~1\ARM\RVCT\Data\2.2\349\include\windows"
@REM set "ARMCC_PATH=C:\PROGRA~1\ARM\RVCT\Programs\2.2\349\win_32-pentium"
@REM ================================================================================================
set "Driver_Name=WAx4_M25PE40-VMPxxx"
set "Driver_Root=%~dp0"
set "Driver_Root=%Driver_Root:~0,-1%"

set "Driver_C=%Driver_Root%\%Driver_Name%.c"
set "Driver_Axf=%Driver_Root%\%Driver_Name%.axf"
set "Driver_Bin=%Driver_Root%\%Driver_Name%.bin"
set "Driver_Spc=%Driver_Root%\%Driver_Name%.spc"
set "Driver_O=%Driver_Root%\%Driver_Name%.o"
set "Map_Txt=%Driver_Root%\map.txt"

for %%I in ("%Driver_Root%\..") do set "Driver_Dir=%%~fI\"
set "Object_Lib=%Driver_Dir%ALL1000Code"
set "Header_Lib=%Driver_Dir%ALL1000Code\h"
for %%I in ("%Driver_Root%\..") do set "Driver_Dir=%%~fI\"
set "Algo1_Path=D:\Hi-Lo\ALL1000\Algo1\"
    
set "ARMCC=%ARMCC_PATH%\armcc.exe"
set "ARMLINK=%ARMCC_PATH%\armlink.exe"
set "FROMELF=%ARMCC_PATH%\fromelf.exe"

set "RVDS_PARAM= --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend"
@REM set "RVDS_PARAM= --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend --diag_suppress=1296"

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
    echo [FAIL] Cannot find %ARMCC%.
    exit /b 1
)
if not exist "%ARMLINK%" (
    echo [FAIL] Cannot find %ARMLINK%.
    exit /b 1
)
if not exist "%FROMELF%" (
    echo [FAIL] Cannot find %FROMELF%.
    exit /b 1
)


REM Step 2: Build the dirver project

echo --------------------------------------------------------------------------------------------------------------------------------
echo Compilation information:
echo --------------------------------------------------------------------------------------------------------------------------------
"%ARMCC%" -c %RVDS_PARAM% %Include_Files% "%Driver_C%" -o "%Driver_O%"
echo --------------------------------------------------------------------------------------------------------------------------------
if errorlevel 1 (
    echo [FAIL] Compilation failed.
    goto CLEAN_UP
)
if not exist "%Driver_O%" (
    echo [FAIL] Object file not generated: "%Driver_O%".
    goto CLEAN_UP
)

"%ARMLINK%" %Obj_Files% -o "%Driver_Axf%" --map --ro_base=0x30000000 --rw_base=0x30200000 --first=startup.o(Init) --entry=0x30000000 --info=sizes --info=totals --list "%Map_Txt%"
if errorlevel 1 (
    echo [FAIL] Linking failed.
    goto CLEAN_UP
)
if not exist "%Driver_Axf%" (
    echo [FAIL] Object file not generated: "%Driver_Axf%".
    goto CLEAN_UP
)

"%FROMELF%" --bin "%Driver_Axf%" --output "%Driver_Bin%"
if errorlevel 1 (
    echo [FAIL] Format conversion failed.
    goto CLEAN_UP
)
if not exist "%Driver_Bin%" (
    echo [FAIL] Object file not generated: "%Driver_Bin%".
    goto CLEAN_UP
)

echo [SUCCESS] Build successful.


REM Step 3: Update the dirver bin file

copy /Y "%Driver_Bin%" "%Algo1_Path%" >nul
if errorlevel 1 (
    echo [FAIL] Copy BIN File to Algo1 failed.
) else (
    echo [SUCCESS] Update "%Algo1_Path%%Driver_Name%.bin" successful.
)

copy /Y "%Driver_Spc%" "%Algo1_Path%" >nul
if errorlevel 1 (
    echo [FAIL] Copy SPC File to Algo1 failed.
) else (
    echo [SUCCESS] Update "%Algo1_Path%%Driver_Name%.spc" successful.
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

