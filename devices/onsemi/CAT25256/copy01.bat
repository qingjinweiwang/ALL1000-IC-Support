@echo off
setlocal enabledelayedexpansion

:: Set UTF-8 encoding
chcp 65001 >nul

:: Read run count (saved in %temp%\tmp_count.txt)
set "count_file=%temp%\tmp_count.txt"
if exist "%count_file%" (
    set /p run_count=<"%count_file%"
) else (
    set run_count=0
)
set /A run_count+=1
echo %run_count% > "%count_file%"

:loop
:: Define source and target folders
set "source_folder=%~dp0RVDS"
set "target_folder=D:\Program Files\ALL1000\Algo1"

:: Create target folder if it doesn't exist
if not exist "%target_folder%" mkdir "%target_folder%" 2>nul

:: Check if source folder exists
if not exist "%source_folder%" (
    echo [ERROR] Source folder "%source_folder%" not found, please check path!
    pause
    exit /b
)

:: Initialize variables for storing filenames
set "copied_bin_files="
set "copied_spc_files="

:: Copy .bin files
echo Copying .bin files, please wait...
set "found_bin=0"
for %%f in ("%source_folder%\*.bin") do (
    copy "%%~f" "%target_folder%" >nul
    set "copy_err=!errorlevel!"
    if !copy_err! EQU 0 (
        set "found_bin=1"
        set "copied_bin_files=!copied_bin_files! %%~nxf"
    )
)

:: Copy .spc files (.spc files in script directory)
echo Copying .spc files, please wait...
set "found_spc=0"
for %%f in ("%~dp0\*.spc") do (
    copy "%%~f" "%target_folder%" >nul
    set "copy_err=!errorlevel!"
    if !copy_err! EQU 0 (
        set "found_spc=1"
        set "copied_spc_files=!copied_spc_files! %%~nxf"
    )
)

:: If neither file type was copied, prompt and loop again
if %found_bin%==0 if %found_spc%==0 (
    echo [WARNING] No .bin or .spc files found!
    pause
    goto loop
)

:: Get current time, format using %DATE% and %TIME%
set "current_date=%DATE%"
set "current_time=%TIME%"
set "formatted_time=%current_date% %current_time:~0,5%"

:: Output success message
echo.
echo ----------------------------------------------------------------
echo [SUCCESS] Files copied successfully!
echo [TIME] Completion time: %formatted_time%

:: Show copied .bin files with modification time
if %found_bin%==1 (
    echo [Copied .bin files]:
    for %%f in (%copied_bin_files%) do (
        for /f "tokens=1,2 delims= " %%A in ('dir /T:W "%source_folder%\%%f" ^| find /i "%%~nxf"') do (
            echo   - %%f   [Modified: %%A %%B]
        )
    )
)

:: Show copied .spc files with modification time
if %found_spc%==1 (
    echo [Copied .spc files]:
    for %%f in (%copied_spc_files%) do (
        for /f "tokens=1,2 delims= " %%A in ('dir /T:W "%~dp0\%%f" ^| find /i "%%~nxf"') do (
            echo   - %%f   [Modified: %%A %%B]
        )
    )
)

echo.

:: Clear screen and reset counter after 20 runs
if %run_count% GEQ 20 (
    echo [CLEANUP] Clearing screen after 20 runs...
    timeout /t 2 >nul
    cls
    set run_count=0
    echo %run_count% > "%count_file%"
)

echo Press any key to run again, or close window to exit.
pause
goto loop