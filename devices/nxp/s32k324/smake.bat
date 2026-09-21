@echo off
setlocal enabledelayedexpansion

:START
cls

REM ==============================================
REM RVDS Project Compilation&Copy Script
REM Usage: cmake
REM Note:   You need set *.bin target file at TARGET_DIR and RVDS_BIN_PATH
REM         This script should be placed in the project folder, at the same level as the .c file
REM         The project folder name must be same as the .c file
REM ==============================================

REM Get the directory where the batch file is located (i.e., the project folder)
set "PROJECT_ROOT=%~dp0"
REM Remove the trailing backslash
set "PROJECT_ROOT=%PROJECT_ROOT:~0,-1%"

REM Extract the folder name as project name
for %%I in ("%PROJECT_ROOT%") do set "PROJECT_NAME=%%~nxI"

REM Build file paths
set "SOURCE_FILE=%PROJECT_ROOT%\%PROJECT_NAME%.c"
set "HEADER_FILE=%PROJECT_ROOT%\%PROJECT_NAME%.h"

REM Use the same output path as the RVDS project
set "RVDS_OUTPUT_DIR=%PROJECT_ROOT%\RVDS\%PROJECT_NAME%_Data\Debug\ObjectCode"
set "TEST_OUTPUT_DIR=%PROJECT_ROOT%\RVDS\Output_scp"

REM .via file paths
set "OBJECTS_VIA=%RVDS_OUTPUT_DIR%\Objects.via"
set "USER_INCLUDES_VIA=%RVDS_OUTPUT_DIR%\User Includes.via"
set "SYSTEM_INCLUDES_VIA=%RVDS_OUTPUT_DIR%\System Includes.via"

echo.
echo [Configuration Parameters]
echo Project Name: %PROJECT_NAME%
echo Project Folder: "%PROJECT_ROOT%"
echo Source File: "%SOURCE_FILE%"
echo Header File: "%HEADER_FILE%"
echo RVDS Output Directory: "%RVDS_OUTPUT_DIR%"
echo Test Output Directory: "%TEST_OUTPUT_DIR%"
echo Objects.via Path: "%OBJECTS_VIA%"
echo User Includes.via Path: "%USER_INCLUDES_VIA%"
echo System Includes.via Path: "%SYSTEM_INCLUDES_VIA%"

REM Set compiler path
set "RVDS_BIN_PATH=C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium"
set "ARMCC=%RVDS_BIN_PATH%\armcc.exe"
set "ARMLINK=%RVDS_BIN_PATH%\armlink.exe"
set "FROMELF=%RVDS_BIN_PATH%\fromelf.exe"

echo.
echo [0/5] Checking compiler tools...

REM Check if compiler exists
if not exist "%ARMCC%" (
    echo Error: Cannot find armcc.exe
    goto PROMPT_RESTART
)

REM Check if necessary files exist
if not exist "%SOURCE_FILE%" (
    echo Error: Cannot find source file "%SOURCE_FILE%"
    goto PROMPT_RESTART
)

if not exist "%OBJECTS_VIA%" (
    echo Error: Cannot find Objects.via file "%OBJECTS_VIA%"
    goto PROMPT_RESTART
)

REM Create output directories
echo Creating RVDS output directory: "%RVDS_OUTPUT_DIR%"
if not exist "%RVDS_OUTPUT_DIR%" mkdir "%RVDS_OUTPUT_DIR%"

echo Creating test output directory: "%TEST_OUTPUT_DIR%"
if not exist "%TEST_OUTPUT_DIR%" mkdir "%TEST_OUTPUT_DIR%"

REM ==============================================
REM Step 1: Read Objects.via to get link order
REM ==============================================
echo.
echo [1/5] Reading Objects.via file to get link order...

set "OBJ_FILES="
for /f "usebackq delims=" %%i in ("%OBJECTS_VIA%") do (
    set "OBJ_FILES=!OBJ_FILES! %%i"
)

echo Number of object files in Objects.via: 
set "COUNT=0"
for %%i in (%OBJ_FILES%) do set /a COUNT+=1
echo Total !COUNT! object files

REM ==============================================
REM Step 2: Read include paths and compile source file
REM ==============================================
echo.
powershell -Command "Write-Host '----------------------------------------------------------------------------------------------' -ForegroundColor Red"
echo [2/5] Reading include paths and compiling source file...

REM Read User Includes.via
set "INCLUDE_FLAGS="
if exist "%USER_INCLUDES_VIA%" (
    for /f "usebackq tokens=*" %%i in ("%USER_INCLUDES_VIA%") do (
        set "INCLUDE_FLAGS=!INCLUDE_FLAGS! %%i"
    )
)

REM Read System Includes.via
if exist "%SYSTEM_INCLUDES_VIA%" (
    for /f "usebackq tokens=*" %%i in ("%SYSTEM_INCLUDES_VIA%") do (
        set "INCLUDE_FLAGS=!INCLUDE_FLAGS! %%i"
    )
)

echo Include path flags read

REM Set compilation parameters
set "COMMON_FLAGS=--debug --cpu=ARM926EJ-S -O0 -Otime --fpu=None"

REM Use the same output path as the RVDS project
set "OBJ_FILE=%RVDS_OUTPUT_DIR%\%PROJECT_NAME%.o"
echo Output object file: "%OBJ_FILE%"

echo Executing compilation command...
"%ARMCC%" -c %COMMON_FLAGS% %INCLUDE_FLAGS% "%SOURCE_FILE%" -o "%OBJ_FILE%"
if errorlevel 1 (
    echo Error: Compilation failed!
    goto PROMPT_RESTART
)

REM Check if object file was generated
if not exist "%OBJ_FILE%" (  
    echo Error: Object file not generated: "%OBJ_FILE%"
    goto PROMPT_RESTART
)
echo Compilation successful! Object file generated
powershell -Command "Write-Host '----------------------------------------------------------------------------------------------' -ForegroundColor Red"

REM ==============================================
REM Step 3: Link object files using order from Objects.via
REM ==============================================
echo.
echo [3/5] Linking object files using order from Objects.via...

REM Place final output files in test directory
set "AXF_FILE=%TEST_OUTPUT_DIR%\%PROJECT_NAME%.axf"
set "MAP_FILE=%TEST_OUTPUT_DIR%\map.txt"

echo Output AXF file: "%AXF_FILE%"
echo Output MAP file: "%MAP_FILE%"

echo Executing link command...
"%ARMLINK%" %OBJ_FILES% -o "%AXF_FILE%" --map --ro_base=0x30000000 --rw_base=0x30200000 --first=startup.o(Init) --entry=0x30000000 --info=sizes --info=totals --list "%MAP_FILE%"

if errorlevel 1 (
    echo.
    echo ----------------------------------------------------------------
    echo Linking failed!
    echo ----------------------------------------------------------------
    goto PROMPT_RESTART
)
echo Linking successful!

REM ==============================================
REM Step 4: Generate binary file
REM ==============================================
echo.
echo [4/5] Generating binary file...
set "BIN_FILE=%TEST_OUTPUT_DIR%\%PROJECT_NAME%.bin"
echo Output BIN file: "%BIN_FILE%"

echo Executing command...
"%FROMELF%" --bin "%AXF_FILE%" --output "%BIN_FILE%"
if errorlevel 1 (
    echo Error: Failed to generate BIN file!
    goto PROMPT_RESTART
)

REM ==============================================
REM Step 5: Copy BIN file to target directory
REM ==============================================
echo.
echo [5/5] Copying BIN file to target directory...
set "TARGET_DIR=D:\Program Files\ALL1000\Algo1"
set "TARGET_BIN=%TARGET_DIR%\%PROJECT_NAME%.bin"

echo Target directory: "%TARGET_DIR%"
echo Target BIN file: "%TARGET_BIN%"

REM Check if target directory exists
if not exist "%TARGET_DIR%" (
    echo Error: Target directory does not exist: "%TARGET_DIR%"
    echo Creating target directory...
    mkdir "%TARGET_DIR%"
    if errorlevel 1 (
        echo Error: Failed to create target directory!
        goto PROMPT_RESTART
    )
)

echo.
echo ----------------------------------------------------------------
echo Compilation and deployment completed!
echo Project: %PROJECT_NAME%
echo Used .via files:
echo   Objects.via: "%OBJECTS_VIA%"
echo   User Includes.via: "%USER_INCLUDES_VIA%"
echo   System Includes.via: "%SYSTEM_INCLUDES_VIA%"
echo Generated files:
echo   AXF file: "%AXF_FILE%"
echo   BIN file: "%BIN_FILE%"
echo   MAP file: "%MAP_FILE%"
echo Deployment:
echo   Target directory: "%TARGET_DIR%"
echo   Deployed BIN file: "%TARGET_BIN%"
echo ----------------------------------------------------------------

REM Copy the BIN file
if exist "%BIN_FILE%" (
    echo Copying "%BIN_FILE%" to "%TARGET_BIN%"...
    copy "%BIN_FILE%" "%TARGET_BIN%"
    if errorlevel 1 (
        echo Error: Failed to copy BIN file!
        goto PROMPT_RESTART
    )
    echo [OK] BIN file copied successfully to target directory
) else (
    echo Error: Source BIN file not found: "%BIN_FILE%"
    goto PROMPT_RESTART
)



:PROMPT_RESTART
echo.
echo Press any key to compile again...
set /p "user_input=Enter your choice: "
if /i "%user_input%"=="Q" goto END
goto START

:END
echo.
echo Script terminated.
pause