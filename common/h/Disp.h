
/*****************************************************************************/
/*****************************************************************************/
#ifndef DISP_H_
#define DISP_H_

#include "Type.h"


/*****************************************************************************\
			Constant Definitions
\*****************************************************************************/
/**** LCD control command ****/
#define LCDLineNum				8
#define LCDLineMaxCnt			21 /* line0~3 for device info, line4~7 for device status */
#define LCD_MAX_CHARCNT		(LCDLineNum*LCDLineMaxCnt)
extern uChar LCMDram[LCDLineNum][LCDLineMaxCnt] ;

/*********************************************************************/
typedef struct {
		uChar Seconds ;
		uChar Minutes ;
		uChar Hours ;
		uChar Day ;
		uChar Date ;
		uChar Month ;
		uChar Year ;

		uChar Seconds_AL1 ;
		uChar Minutes_AL1 ;		
		uChar Hours_AL1 ;
		uChar Date_AL1 ;

		uChar Minutes_AL2 ;
		uChar Hours_AL2 ;
		uChar Date_AL2 ;

		uChar Control ;
		uChar Status ;
		uChar AgingOffset ;
		uChar TemperatureMSB ;
		uChar TemperatureLSB ;
		}TCalendar;

extern TCalendar CalendarTimer ; /* Timer use for record LOG & calculate program times */


/*********************************************************************/
typedef struct {
		uChar TSec ;
		uChar TMin ;
		uChar THour ;
		uChar TDate ;
		uChar TMonth ;
		uChar TYear ;
		}Ttimer ;

extern Ttimer StartRunTimer, EndRunTimer, UsingTimer ; /* Timer use for record LOG & calculate program times */


/*****************************************************************************\
			Global Variables Declarations
\*****************************************************************************/







//-------------  ASCII字符库----------------------------------------------------------------
//说明：该字库为5*7点阵字符字模库，但字库格式为8*8点阵
//字模数据，第8行为行间距， 第6-8列为字间距。可以使用
//6*8点阵、7*8点阵、8*8点阵形式的字符显示
static const uChar AsciiTable[] =
{
	0x00,0x00,0x00,0x00,0x00,0x00,    /*" "=00H*/
	0x00,0x00,0x00,0x4F,0x00,0x00,    /*"!"=01H*/
	0x00,0x00,0x07,0x00,0x07,0x00,    /*"""=02h*/
	0x00,0x14,0x7F,0x14,0x7F,0x14,    /*"#"=03h*/
	0x00,0x24,0x2A,0x7F,0x2A,0x12,    /*"$"=04h*/
	0x00,0x23,0x13,0x08,0x64,0x62,    /*"%"=05h*/
	0x00,0x36,0x49,0x55,0x22,0x50,    /*"&"=06h*/
	0x00,0x00,0x05,0x03,0x00,0x00,    /*"'"=07h*/
	0x00,0x00,0x1C,0x22,0x41,0x00,    /*"("=08h*/
	0x00,0x00,0x41,0x22,0x1C,0x00,    /*")"=09h*/
	0x00,0x14,0x08,0x3E,0x08,0x14,    /*"*"=0Ah*/
	0x00,0x08,0x08,0x3E,0x08,0x08,    /*"+"=0Bh*/
	0x00,0x00,0x50,0x30,0x00,0x00,    /*";"=0Ch*/
	0x00,0x08,0x08,0x08,0x08,0x08,    /*"-"=0Dh*/
	0x00,0x00,0x60,0x60,0x00,0x00,    /*"."=0Eh*/
	0x00,0x20,0x10,0x08,0x04,0x02,    /*"/"=0Fh*/
	0x00,0x3E,0x51,0x49,0x45,0x3E,    /*"0"=10h*/
	0x00,0x00,0x42,0x7F,0x40,0x00,    /*"1"=11h*/
	0x00,0x42,0x61,0x51,0x49,0x46,    /*"2"=12h*/
	0x00,0x21,0x41,0x45,0x4B,0x31,    /*"3"=13h*/
	0x00,0x18,0x14,0x12,0x7F,0x10,    /*"4"=14h*/
	0x00,0x27,0x45,0x45,0x45,0x39,    /*"5"=15h*/
	0x00,0x3C,0x4A,0x49,0x49,0x30,    /*"6"=16h*/
	0x00,0x01,0x01,0x79,0x05,0x03,    /*"7"=17h*/
	0x00,0x36,0x49,0x49,0x49,0x36,    /*"8"=18h*/
	0x00,0x06,0x49,0x49,0x29,0x1E,    /*"9"=19h*/
	0x00,0x00,0x36,0x36,0x00,0x00,    /*":"=1Ah*/
	0x00,0x00,0x56,0x36,0x00,0x00,    /*";"=1Bh*/
	0x00,0x08,0x14,0x22,0x41,0x00,    /*"<"=1Ch*/
	0x00,0x14,0x14,0x14,0x14,0x14,    /*"="=1Dh*/
	0x00,0x00,0x41,0x22,0x14,0x08,    /*">"=1Eh*/
	0x00,0x02,0x01,0x51,0x09,0x06,    /*"?"=1Fh*/
	0x00,0x32,0x49,0x79,0x41,0x3E,    /*"@"=20h*/
	0x00,0x7E,0x11,0x11,0x11,0x7E,    /*"A"=21h*/
	0x00,0x41,0x7F,0x49,0x49,0x36,    /*"B"=22h*/
	0x00,0x3E,0x41,0x41,0x41,0x22,    /*"C"=23h*/
	0x00,0x41,0x7F,0x41,0x41,0x3E,    /*"D"=24h*/
	0x00,0x7F,0x49,0x49,0x49,0x49,    /*"E"=25h*/
	0x00,0x7F,0x09,0x09,0x09,0x01,    /*"F"=26h*/
	0x00,0x3E,0x41,0x41,0x49,0x7A,    /*"G"=27h*/
	0x00,0x7F,0x08,0x08,0x08,0x7F,    /*"h"=28h*/
	0x00,0x00,0x41,0x7F,0x41,0x00,    /*"I"=29h*/
	0x00,0x20,0x40,0x41,0x3F,0x01,    /*"J"=2Ah*/
	0x00,0x7F,0x08,0x14,0x22,0x41,    /*"K"=2Bh*/
	0x00,0x7F,0x40,0x40,0x40,0x40,    /*"L"=2Ch*/
	0x00,0x7F,0x02,0x0C,0x02,0x7F,    /*"M"=2Dh*/
	0x00,0x7F,0x06,0x08,0x30,0x7F,    /*"N"=2Eh*/
	0x00,0x3E,0x41,0x41,0x41,0x3E,    /*"O"=2Fh*/
	0x00,0x7F,0x09,0x09,0x09,0x06,    /*"P"=30h*/
	0x00,0x3E,0x41,0x51,0x21,0x5E,    /*"Q"=31h*/
	0x00,0x7F,0x09,0x19,0x29,0x46,    /*"R"=32h*/
	0x00,0x26,0x49,0x49,0x49,0x32,    /*"S"=33h*/
	0x00,0x01,0x01,0x7F,0x01,0x01,    /*"T"=34h*/
	0x00,0x3F,0x40,0x40,0x40,0x3F,    /*"U"=35h*/
	0x00,0x1F,0x20,0x40,0x20,0x1F,    /*"V"=36h*/
	0x00,0x7F,0x20,0x18,0x20,0x7F,    /*"W"=37h*/
	0x00,0x63,0x14,0x08,0x14,0x63,    /*"X"=38h*/
	0x00,0x07,0x08,0x70,0x08,0x07,    /*"Y"=39h*/
	0x00,0x61,0x51,0x49,0x45,0x43,    /*"Z"=3Ah*/
	0x00,0x00,0x7F,0x41,0x41,0x00,    /*"["=3Bh*/
	0x00,0x02,0x04,0x08,0x10,0x20,    /*"\"=3Ch*/
	0x00,0x00,0x41,0x41,0x7F,0x00,    /*"]"=3Dh*/
	0x00,0x04,0x02,0x01,0x02,0x04,    /*"^"=3Eh*/
	0x00,0x40,0x40,0x40,0x40,0x40,    /*"_"=3Fh*/
	0x00,0x01,0x02,0x04,0x00,0x00,    /*"`"=40h*/
	0x00,0x20,0x54,0x54,0x54,0x78,    /*"a"=41h*/
	0x00,0x7F,0x48,0x44,0x44,0x38,    /*"b"=42h*/
	0x00,0x38,0x44,0x44,0x44,0x28,    /*"c"=43h*/
	0x00,0x38,0x44,0x44,0x48,0x7F,    /*"d"=44h*/
	0x00,0x38,0x54,0x54,0x54,0x18,    /*"e"=45h*/
	0x00,0x00,0x08,0x7E,0x09,0x02,    /*"f"=46h*/
	0x00,0x0C,0x52,0x52,0x4C,0x3E,    /*"g"=47h*/
	0x00,0x7F,0x08,0x04,0x04,0x78,    /*"h"=48h*/
	0x00,0x00,0x44,0x7D,0x40,0x00,    /*"i"=49h*/
	0x00,0x20,0x40,0x44,0x3D,0x00,    /*"j"=4Ah*/
	0x00,0x00,0x7F,0x10,0x28,0x44,    /*"k"=4Bh*/
	0x00,0x00,0x41,0x7F,0x40,0x00,    /*"l"=4Ch*/
	0x00,0x7C,0x04,0x78,0x04,0x78,    /*"m"=4Dh*/
	0x00,0x7C,0x08,0x04,0x04,0x78,    /*"n"=4Eh*/
	0x00,0x38,0x44,0x44,0x44,0x38,    /*"o'=4Fh*/
	0x00,0x7E,0x0C,0x12,0x12,0x0C,    /*"p"=50h*/
	0x00,0x0C,0x12,0x12,0x0C,0x7E,    /*"q"=51h*/
	0x00,0x7C,0x08,0x04,0x04,0x08,    /*"r"=52h*/
	0x00,0x58,0x54,0x54,0x54,0x64,    /*"s"=53h*/
	0x00,0x04,0x3F,0x44,0x40,0x20,    /*"t"=54h*/
	0x00,0x3C,0x40,0x40,0x3C,0x40,    /*"u"=55h*/
	0x00,0x1C,0x20,0x40,0x20,0x1C,    /*"v"=56h*/
	0x00,0x3C,0x40,0x30,0x40,0x3C,    /*"w"=57h*/
	0x00,0x44,0x28,0x10,0x28,0x44,    /*"x"=58h*/
	0x00,0x1C,0xA0,0xA0,0x90,0x7C,    /*"y"=59h*/
	0x00,0x44,0x64,0x54,0x4C,0x44,    /*"z"=5Ah*/
	0x00,0x00,0x08,0x36,0x41,0x00,    /*"{"=5Bh*/
	0x00,0x00,0x00,0x77,0x00,0x00,    /*"|"=5Ch*/
	0x00,0x00,0x41,0x36,0x08,0x00,    /*"}"=5Dh*/
	0x00,0x02,0x01,0x02,0x04,0x02,    /*"~"=5Fh*/
	0x00,0xFF,0xFF,0xFF,0xFF,0xFF,    /*" "=0x60*/
} ;
















/*****************************************************************************\
			Functions Declarations
\*****************************************************************************/
extern void Put_String (uChar LineNum, uChar addr, char *str) ;
extern void ClearLcdRamBuffer(void) ;
extern void InitialLCDSetting(void) ;
extern void Write_LcdData(uChar data);
extern void Write_LcdCMD(uChar cmd);
extern void PrintASCII(uChar x, uChar y) ;
extern void Write_LcdRam(void);
extern uChar ChangeDramToChar(uChar TempDram) ;



/********************************************************/
/*********** below for system display ***********************/
extern void Disp_SelfTestFail(void) ;
extern void Disp_FWSecureCheckFail(void) ;
extern void Disp_DriverInitialFail(void) ;
extern void Disp_Cmp_Prj_Info(void) ;
extern void Disp_SelfTesting(void) ;
extern void Disp_DriverInitializing(void) ;
extern void Disp_FPGAConfiguring(void) ;
extern void Disp_SelfTestPass(void) ;
extern void Disp_DriverInitialPass(void) ;
extern void Disp_Standby(void) ;
extern void Disp_PassFailCounter(void) ;
extern void Disp_TotalUseTimer(void) ;



/******************************************************************\
		Below subroutine function for error/fail display control 

\******************************************************************/
extern void Disp_CFCardTestFail(void) ;
extern void Disp_FPGAConfigureFail(void) ;
extern void Disp_DACCalibrateFail(void) ;
extern void Disp_ChkBlkPtyFail(void) ;
extern void Disp_DevicePowerOnFail(void) ;
extern void Disp_DevicePowerOffFail(void) ;
extern void Disp_DeviceInitialFail(void) ;
extern void Disp_DeviceReadFail(void) ;
extern void Disp_DeviceBlankCheckFail(void) ;
extern void Disp_DeviceIllegalCheckFail(void) ;
extern void Disp_DeviceEraseFail(void) ;
extern void Disp_DeviceProgramFail(void) ;
extern void Disp_DeviceVerifyFail(void) ;
extern void Disp_DeviceHiVerifyFail(void) ;
extern void Disp_DeviceLoVerifyFail(void) ;
extern void Disp_DeviceProtectFail(void) ;
extern void Disp_DeviceUnprotectFail(void) ;
extern void Disp_HiddenROMEraseFail(void) ; /* same for Extend Block Erase Fail */
extern void Disp_HiddenROMProgramFail(void) ; /* same for Extend Block Program Fail */
extern void Disp_HiddenROMVerifyFail(void) ; /* same for Extend Block Verify Fail */
extern void Disp_HiddenROMBlankFail(void) ; /* same for Extend Block Blank Fail */
extern void Disp_DeviceProtectStatusFail(void) ;
extern void Disp_UserPrtRegProgramFail(void) ;
extern void Disp_UserPrtRegVerifyFail(void) ;
extern void Disp_UserPrtRegBlankFail(void) ;
extern void Disp_DeviceSecureFail(void) ;
extern void Disp_FunctionalTestFail(void) ;
extern void Disp_DeviceIDCheckFail(void) ;
extern void Disp_DeviceBakBlockOverLimitPass(void) ;
extern void Disp_DeviceBakBlockOverLimitFail(void) ;
extern void Disp_DeviceBadBootBlockFail(void) ;
extern void Disp_DevicePartitionTableFail(void) ;
extern void Disp_ChecksumCalculateFail(void) ;
extern void Disp_DevicePinConnectCheckFail(void) ;
extern void Disp_ChecksumCompareFail(void) ;
extern void Disp_SKBCodeCheckFail(void) ;
extern void Disp_DataDownloadFail(void) ;
extern void Disp_DataUploadFail(void) ;


/******************************************************************\
		Below subroutine function for Passed display control 

\******************************************************************/
extern void Disp_DeviceReading(void) ;
extern void Disp_DeviceBlankChecking(void) ;
extern void Disp_DeviceIllegalChecking(void) ;
extern void Disp_DeviceErasing(void) ;
extern void Disp_DeviceProgramming(void) ;
extern void Disp_DeviceVerifying(void) ;
extern void Disp_DeviceHiVerifying(void) ;
extern void Disp_DeviceLoVerifying(void) ;
extern void Disp_DeviceProtecting(void) ;
extern void Disp_DeviceUnprotecting(void) ;
extern void Disp_DeviceSecuring(void) ;
extern void Disp_FunctionalTesting(void) ;
extern void Disp_DeviceIDChecking(void) ;
extern void Disp_ChecksumCalculating(void) ;
extern void Disp_DevicePinConnectChecking(void) ;
extern void Disp_AutoModeDeviceInputWaiting(void) ;
extern void Disp_DataDownloading(void) ;
extern void Disp_DataUploading(void) ;


/******************************************************************\
		Below subroutine function for Passed display control 

\******************************************************************/
extern void Disp_CFCardTestPass(void) ;
extern void Disp_DACCalibratePass(void) ;
extern void Disp_DeviceReadPass(void) ;
extern void Disp_DeviceBlankCheckPass(void) ;
extern void Disp_DeviceIllegalCheckPass(void) ;
extern void Disp_DeviceErasePass(void) ;
extern void Disp_DeviceProgramPass(void) ;
extern void Disp_DeviceVerifyPass(void) ;
extern void Disp_DeviceHiVerifyPass(void) ;
extern void Disp_DeviceLoVerifyPass(void) ;
extern void Disp_DeviceProtectPass(void) ;
extern void Disp_DeviceUnprotectPass(void) ;
extern void Disp_DeviceSecurePass(void) ;
extern void Disp_FunctionalTestPass(void) ;
extern void Disp_DeviceIDCheckPass(void) ;
extern void Disp_ChecksumCalculatePass(void) ;
extern void Disp_DevicePinConnectCheckPass(void) ;
extern void Disp_ChecksumComparePass(void) ;
extern void Disp_DeviceControlModePass(void) ;
extern void Disp_DataDownloadPass(void) ;
extern void Disp_DataUploadPass(void) ;

#endif

