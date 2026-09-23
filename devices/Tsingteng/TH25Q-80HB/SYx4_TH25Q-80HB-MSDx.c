/******************************************************************************\
	Programmer total 13 tasks driver setting:
POWER_ON_FUN							POWER_OFF_FUN
READ_FUN			PROGRAM_FUN		VERIFY_FUN
ERASE_FUN			BLANK_CHECK_FUN	ILLEGAL_CHECK_FUN
ID_CHECK_FUN		SECURE_FUN			CHECKSUM_FUN
PROTECT_FUN								UNPROTECT_FUN			
\*******************************************************************************/

#include "SYx4_TH25Q-80HB-MSDx.h"
#include "h\Disp.h"
#include "h\Key.h"
#include "h\FirmWare.h"
#include "h\BlockSectorDef.h"
#include "h\Type.h"
#include "h\SystemTask.h"
#include "h\Subroutine.h"
#include "h\ErrorCode.h"
#include "h\PassedCode.h"
#include "h\USBCom.h"
#include "h\CFControl.h"
#include "h\StatusCode.h"
#include "h\hs0_mmc.h"


/**************************************************************\
Notes: User define varity here,
          the area is UserVarityAreaMin ~ UserVarityAreaMin
\**************************************************************/
uShort PageDataBuf[256] ; /* Max Buffer Size be 256 word */
uShort BlankPageFlag ;
uShort BlockEndFlag ;
uInt32 DeviceAddress ;
uShort ProgPageSize ;
uShort ChipEraseTime ;
uChar ChipSPIMaxClkDelay ;
uShort PrtRegMaskBit ;
uChar PrtReg0DefaultBit;
uChar PrtReg1DefaultBit;


/******** Control Command Macro Define **************/
#define	WriteEnableCmd		0x06
#define	WriteDisableCmd		0x04
#define	ReadStatusCmd		0x05  /* Status Register low byte (S7-S0) */
#define	ReadStatus1Cmd		0x35  /* Status Register high byte (S15-S8), keep template name */
#define	WriteStatusCmd		0x01  /* Write Status Register: S7-S0 then S15-S8 */

#define	ReadDataCmd		    0x03  /* 3-byte address read */
#define	FastReadDataCmd	    0x0B  /* 3-byte address fast read */
#define	PageProgCmd		    0x02  /* 3-byte address page program */
#define	BulkEraseCmd		0xC7  /* Chip Erase (60h equivalent) */
#define	ReadIDCmd			0x9F

#define 	ReadSecurityCmd		0x48
#define 	ProgramSecurityCmd 	0x42
#define 	EraseSecurityCmd	0x44


/******* Status Control Bits **********/
#define	StatusWIPBit			0x01
#define	StatusWELBit			0x02
#define	StatusLBMaskBit			0x38  /* LB3-LB1 in SR high byte(S13-S11), OTP permanent lock */


/*** Control Bit Macro Definition, Refer to *.spc file setting ***/
#define	ProgramOn			0x01 /* bit x = 1 means On */
#define	VerifyOn			0x02
#define	EraseOn				0x04
#define	CheckOn			0x08
#define	ChecksumOn			0x10
#define	StatusCheckOn		0x80 /* for status reg verify/blank enable */


#define	GDevPageChkCnt		256 /* Byte data counter */
/* MUST delay enough time let FPGA read 8 bit serial data & finish checking 4 socket data!! */
#define	SPIRWDelay()		(Delay_10nsX(ChipSPIMaxClkDelay)) /* 10ns scale, refer to sck speed!  */



/****************************************************************\
Notes: User define yoursel subroutine here

\****************************************************************/
void GetDeviceParameterInfo(void) ;
uShort ReadStatusData(void) ;
uChar CheckAllStatusData(uChar TempData) ;
uChar WriteStatusWELBit(void) ;
void Get1PageDataChkBlank(void) ;
uChar CheckAllStatus1Data(uChar DevData) ;



/********************* Power On ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar PowerOnFun(void)
{
	Set_VccDAC() ; /* setting DAC array to rise VCC/VPP/VPE/VPS/VPIO volts level */
	Set_VihDAC() ;
	Set_VppDAC() ;
	Set_VpioDAC() ;
	Delay_1ms(50) ; /* delay for stable volte */
	
	GangRiseAllPassSocketVccIccPin() ; /* open VCC/VPP/VPE/VPS/VPIO pin driving */

	Delay_1ms(50) ; /* delay for stable volte */
	GetDeviceParameterInfo() ;
	Delay_1ms(50) ;
	return SUCCESS ;
}




/********************* Power On ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar PowerOffFun(void)
{
	Fall_VpioIpioAll() ;
	Fall_VppIppAll() ;
	Fall_VihAll() ;
	Fall_VccAll() ;
	Delay_1ms(50) ; /* delay for stable volte */
	return SUCCESS ;
}



/***************** Read Subroutine **************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar ReadFun(void)
{
stuShort DevWordData ;
uShort TempCnt ;

	DeviceAddress = (GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{ /* if status regs */
		if( GetCurrBlkNum() == Noun_Block4 )
		{
			PtrData(GEnSelSckNum) = EnSckACtrlFlag ; /* read data path: select socket A */
			Delay_10nsX(10) ;
			DevWordData.all = ReadStatusData() ;
			SaveSRAMByteData( DevWordData.Byte.LoByte ) ; /* save SR low byte (05h) */
			SaveSRAMByteData( DevWordData.Byte.HiByte ) ; /* save SR high byte (35h) */
			PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* restore all pass socket */
			Delay_10nsX(3) ;
		}
		return SUCCESS ;
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{ /* factory register & user register */
		PtrData(GEnSelSckNum) = EnSckACtrlFlag ; /* read data path: select socket A explicitly */
		Delay_10nsX(10) ;
		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = ReadSecurityCmd ; /* send cmd Byte */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = 0xFF ; /* Send Dummy byte */
		SPIRWDelay() ;

		while(1)
		{
			PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let fpga read device data */
			SPIRWDelay() ;
			DevWordData.Byte.LoByte = PtrData(OneDataCmd) ; /* the second data is the true data */
			Delay_10nsX(10) ;
			PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let fpga read device data */
			SPIRWDelay() ;
			DevWordData.Byte.HiByte = PtrData(OneDataCmd) ; /* the second data is the true data */
			Delay_10nsX(10) ;
			ReadDataBuf[ReadBufCnt/2] = DevWordData.all ;

			ReadBufCnt += 2 ;
			DevCurrBlkLen.all -= 2 ;
			if( SaveChkBlkLenEnd() ) /* checking block length end */
			{
				PtrData(SetCSPinCmd) = 1 ;
				Delay_10nsX(10) ;
				PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* restore all pass socket */
				Delay_10nsX(3) ;
				return SUCCESS ;
			}
		}
	}
	else
	{ /* Read main flash */
		PtrData(GEnSelSckNum) = EnSckACtrlFlag ; /* default select socket A */

		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = ReadDataCmd ; /* 03h: Read Data, 3-byte address */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send A23-A16 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send A15-A8 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send A7-A0 */
		SPIRWDelay() ;

		while(1)
		{
			for(TempCnt=0; TempCnt<MaxPktByteSize; TempCnt+=2)
			{
				PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let fpga read device data */
				SPIRWDelay() ;
				DevWordData.Byte.LoByte = PtrData(OneDataCmd) ; /* the second data is the true data */
				Delay_10nsX(10) ;
				PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let fpga read device data */
				SPIRWDelay() ;
				DevWordData.Byte.HiByte = PtrData(OneDataCmd) ; /* the second data is the true data */
				Delay_10nsX(10) ;
				ReadDataBuf[(ReadBufCnt+TempCnt)/2] = DevWordData.all ;
			}

			ReadBufCnt += MaxPktByteSize ;
			DevCurrBlkLen.all -= MaxPktByteSize ;
			if( SaveChkBlkLenEnd() ) /* checking block length end */
			{
				PtrData(SetCSPinCmd) = 1 ;
				Delay_10nsX(10) ;
				PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* restore all pass socket */
				Delay_10nsX(3) ;
				return SUCCESS ;
			}
		}
	}
	return SUCCESS ;
}

/****************** Program Subroutine **********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar ProgramFun(void)
{
uChar StatusData ;
uShort RamWordData, PageTempCnt, PollingTimeCnt ;

	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
	Delay_10nsX(3) ;

	DeviceAddress = (GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{
#if 0
		if( !ProtectFun() )
		{
			SysStatusCode = E_ProtectFail ; /* sending error type code */
			return FAIL ;
		}				
#endif
		return SUCCESS ;
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{ /* factory register & user register */
		SRAMPointer = 0x03 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & ProgramOn) )
		{
			CFCardReadFillSector() ; /* if have other data then need write finish one sector */
			return SUCCESS ;
		}

		/* LB1-LB3 为 OTP 锁定位,锁定后 Program Security Registers 会被芯片静默忽略,先检查 */
		PtrData(GSetAllMaskDataLo) = (0x08 << (GetCurrBlkNum() - Noun_Block1)) ; /* LB1/LB2/LB3 in SR high byte */
		if( GangCheckIfAllSocketError(CheckAllStatus1Data(0x00)) )
		{ /* all socket locked: security register is read-only permanently */
			SysStatusCode = E_UserPrtRegProgramFail ; /* sending error type code */
			return FAIL ;
		}

		while(1)
		{
			if( !WriteStatusWELBit() )
			{
				SysStatusCode = E_ProgramNoDataFail ; /* sending error type code */
				return FAIL ;
			}
			PtrData(SetCSPinCmd) = 0 ; /* set chip CS port to Vil */
			Delay_10nsX(10) ;
			PtrData(OneDataCmd) = ProgramSecurityCmd ; /* send cmd Byte */
			SPIRWDelay() ;
			PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
			SPIRWDelay() ;
			PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
			SPIRWDelay() ;
			PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
			SPIRWDelay() ;

			for(PageTempCnt=0; PageTempCnt<ProgPageSize; PageTempCnt+=2) /* get page program counter */
			{
				RamWordData = CFCardReadWordData() ;
				PtrData(OneDataCmd) = LSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
				SPIRWDelay() ;
				PtrData(OneDataCmd) = MSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
				SPIRWDelay() ;
			}
			SPIRWDelay() ;
			PtrData(SetCSPinCmd) = 1 ; /* set chip CS port to Vih */
			Delay_10nsX(10) ;
			Delay_1us(400) ; /* NEED delay page time here before polling WIP, refer to tPP */
			PtrData(GSetAllMaskDataLo) = StatusWIPBit ;  /* only check bit0, send byte checking mask data to FPGA */
			PollingTimeCnt = 3000 ;
			while(1)
			{
				StatusData = CheckAllStatusData(0x00) ; /* only check bit0, checking all socket & get check result */
				if( !StatusData ) /* if all socket WIP bit = 0 means program ok */
					break ;
				else if( !(--PollingTimeCnt) )
				{ /* have some socket check error */
					if( GangCheckIfAllSocketError(StatusData) )
					{ /* if all socket error then cancle function */
						SysStatusCode = E_UserPrtRegProgramFail ; /* sending error type code */
						return FAIL ;
					}
					break ; /* go on process other pass socket */
				}
				Delay_1us(5) ;
			}

			DeviceAddress += ProgPageSize ;
			DramBufCnt -= ProgPageSize ;
			DevCurrBlkLen.all -= ProgPageSize ;
			if( GetChkBlkLenEnd() )
			{
				return SUCCESS ;
			}
		}
	}
	else
	{ /* Program main flash */
		Get1PageDataChkBlank() ; /* get first page data */
		while(1)
		{
			if( BlankPageFlag )
			{ /* if blank page data then skip to next page address */
				DeviceAddress += ProgPageSize ;
				DramBufCnt -= ProgPageSize ;
				DevCurrBlkLen.all -= ProgPageSize ;
				BlockEndFlag = GetChkBlkLenEnd() ; /* checking block length end */
				if( !BlockEndFlag ) /* if not the block end then goon check next page data */
					Get1PageDataChkBlank() ;
			}
			else
			{
				if( !WriteStatusWELBit() )
				{
					SysStatusCode = E_ProgramNoDataFail ; /* sending error type code */
					return FAIL ;
				}
				PtrData(SetCSPinCmd) = 0 ; /* set chip CS port to Vil */
				Delay_10nsX(10) ;
				PtrData(OneDataCmd) = PageProgCmd ; /* 02h: Page Program, 3-byte address */
				SPIRWDelay() ;
				PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send A23-A16 */
				SPIRWDelay() ;
				PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send A15-A8 */
				SPIRWDelay() ;
				PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send A7-A0 */
				SPIRWDelay() ;

				for(PageTempCnt=0; PageTempCnt<ProgPageSize; PageTempCnt+=2) /* get page program counter */
				{
					RamWordData = PageDataBuf[PageTempCnt/2] ;
					PtrData(OneDataCmd) = LSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
					SPIRWDelay() ;
					PtrData(OneDataCmd) = MSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
					SPIRWDelay() ;
				}
				SPIRWDelay() ;
				PtrData(SetCSPinCmd) = 1 ; /* set chip CS port to Vih */
				Delay_10nsX(10) ;

				DeviceAddress += ProgPageSize ;
				DramBufCnt -= ProgPageSize ;
				DevCurrBlkLen.all -= ProgPageSize ;
				BlockEndFlag = GetChkBlkLenEnd() ; /* checking block length end */
				if( !BlockEndFlag ) /* if not the block end then goon check next page data */
					Get1PageDataChkBlank() ;

				Delay_1us(400) ; /* NEED delay page time here before polling WIP, refer to tPP */
				PtrData(GSetAllMaskDataLo) = StatusWIPBit ;  /* only check bit0, send byte checking mask data to FPGA */
				PollingTimeCnt = 3000 ;
				while(1)
				{
					StatusData = CheckAllStatusData(0x00) ; /* only check bit0, checking all socket & get check result */
					if( !StatusData ) /* if all socket WIP bit = 0 means program ok */
						break ;
					else if( !(--PollingTimeCnt) )
					{ /* have some socket check error */
						if( GangCheckIfAllSocketError(StatusData) )
						{ /* if all socket error then cancle function */
							SysStatusCode = E_ProgramNoDataFail ; /* sending error type code */
							return FAIL ;
						}
						break ; /* go on process other pass socket */
					}
					Delay_1us(5) ;
				}
			}

			if( BlockEndFlag ) /* checking block length end */
				return SUCCESS ;
			
			if( !(DevCurrBlkLen.all & 0xFFFFF) )
			{ /* above 16M bit=2S handshake 1 time to PC, handshake period time MUST <=4S */
				HandleEvent_EP0() ;
				Delay_1ms(10) ; /* NEED delay 10ms here */
			}
		}
	}
	return SUCCESS ;
}

/****************** Verify Subroutine ************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar VerifyFun(void)
{
uChar DevByteData ;
uShort RamWordData, TempCnt ;

	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
	Delay_10nsX(3) ;

	DeviceAddress = (GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{ /* if status regs */
		SRAMPointer = 0x03 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & StatusCheckOn) )
			return SUCCESS ;

		if( GetCurrBlkNum() == Noun_Block4 )
		{
			SRAMPointer = 0x00 ;
			RamWordData = GetSRAMByteData() & LSB(PrtRegMaskBit) ;
			PtrData(GSetAllMaskDataLo) = LSB(PrtRegMaskBit) ;  /* send byte checking mask data to FPGA */
			DevByteData = CheckAllStatusData(RamWordData) ; /* checking all socket & get check result */
			if( DevByteData )
			{ /* have some socket check error */
				if( GangCheckIfAllSocketError(DevByteData) )
				{ /* if all socket error then cancle function */
					SysStatusCode = E_ProtectStatusFail ; /* sending error type code */
					return FAIL ;
				}
			}

			RamWordData = GetSRAMByteData() & MSB(PrtRegMaskBit) ;
			PtrData(GSetAllMaskDataLo) = MSB(PrtRegMaskBit) ;  /* send byte checking mask data to FPGA */
			DevByteData = CheckAllStatus1Data(RamWordData) ; /* checking all socket & get check result */
			if( DevByteData )
			{ /* have some socket check error */
				if( GangCheckIfAllSocketError(DevByteData) )
				{ /* if all socket error then cancle function */
					SysStatusCode = E_ProtectStatusFail ; /* sending error type code */
					return FAIL ;
				}
			}	
		}
		return SUCCESS ;
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{ /* factory register & user register */
		SRAMPointer = 0x03 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & VerifyOn) )
		{
			CFCardReadFillSector() ; /* if have other data then need write finish one sector */
			return SUCCESS ;
		}

		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = ReadSecurityCmd ; /* send cmd Byte */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = 0xFF ; /* Send Dummy byte */
		SPIRWDelay() ;
        
		PtrData(GSetAllMaskDataLo) = 0xFF ;  /* send byte checking mask data to FPGA */
		while(1)
		{
			RamWordData = CFCardReadWordData() ;
			PtrData(GClrPageChkStatus) = 0x0000 ;  /* send checking mask data to FPGA */
			PtrData(GDevChkDataLoInc0) = LSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
			SPIRWDelay() ;
			PtrData(GDevChkDataLoInc0) = MSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
			SPIRWDelay() ;
			SPIRWDelay() ;
			DevByteData = PtrData(GChkAllSckStatus) ; /* get fpga checking all socket result */
			if( DevByteData )
			{ /* have some socket check error */
				if( GangCheckIfAllSocketError(DevByteData) )
				{ /* if all socket error then cancle function */
					PtrData(SetCSPinCmd) = 1 ;
					Delay_10nsX(10) ;
					SysStatusCode = E_HiddenROMVerifyFail ; /* sending error type code */
					return FAIL ;
				}
			}

			DramBufCnt -= 2 ;
			DevCurrBlkLen.all -= 2 ;
			if( GetChkBlkLenEnd() ) /* checking block length end */
			{
				PtrData(SetCSPinCmd) = 1 ;
				Delay_10nsX(10) ;
				return SUCCESS ;
			}
		}
	}
	else
	{ /* Verify main flash */
		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = ReadDataCmd ; /* 03h: Read Data, 3-byte address */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send A23-A16 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send A15-A8 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send A7-A0 */
		SPIRWDelay() ;

		PtrData(GSetAllMaskDataLo) = 0xFF ;  /* send byte checking mask data to FPGA */
		while(1)
		{
			PtrData(GClrPageChkStatus) = 0x0000 ;  /* send checking mask data to FPGA */
			for(TempCnt=0; TempCnt<GDevPageChkCnt; TempCnt+=2)
			{
				RamWordData = CFCardReadWordData() ; /* send data to fpga & read all socket data */
				PtrData(GDevChkDataLoInc0) = LSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
				SPIRWDelay() ;
				PtrData(GDevChkDataLoInc0) = MSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
				SPIRWDelay() ;
			}
			SPIRWDelay() ;
			DevByteData = PtrData(GChkAllSckStatus) ; /* get fpga checking all socket result */
			if( DevByteData )
			{ /* have some socket check error */
				if( GangCheckIfAllSocketError(DevByteData) )
				{ /* if all socket error then cancle function */
					PtrData(SetCSPinCmd) = 1 ;
					Delay_10nsX(10) ;
					SysStatusCode = E_VerifyFail ; /* sending error type code */
					return FAIL ;
				}
			}

			DramBufCnt -= GDevPageChkCnt ;
			DevCurrBlkLen.all -= GDevPageChkCnt ;
			if( GetChkBlkLenEnd() ) /* checking block length end */
			{
				PtrData(SetCSPinCmd) = 1 ;
				Delay_10nsX(10) ;
				return SUCCESS ;
			}
			if( !(DevCurrBlkLen.all & 0xFFFFF) )
			{ /* above 16M bit=2S handshake 1 time to PC, handshake period time MUST <=4S */
				HandleEvent_EP0() ;
				Delay_1ms(10) ; /* NEED delay 10ms here */
			}
		}
	}
	return SUCCESS ;
}

/******************* Erase Subroutine ***********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar EraseFun(void)
{
uShort StatusData, PollingTimeCnt, RamWordData ;
stuShort CurrSRData ;

	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
	Delay_10nsX(3) ;

	DeviceAddress = (GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfFirstTimeEntry() )
	{
		/*
		 * CE 仅在 (BP2-BP0=000 且 CMP=0) 或 (BP2-BP0=111 且 CMP=1) 时才会被芯片执行,
		 * 其余保护态下 CE 被芯片静默忽略。Erase 不解保护(平台惯例, UnProtect 为上位机独立任务),
		 * 这里先检查保护状态, 保护态直接报 Erase 失败, 避免假 PASS。
		 * Gang 模式下读回值取自 A 槽, 默认各槽芯片状态一致。
		 */
		CurrSRData.all = ReadStatusData() ; /* 读当前 SR(05h 低字节 + 35h 高字节) */
		if( !( ((CurrSRData.Byte.LoByte & 0x1C) == 0x00 && !(CurrSRData.Byte.HiByte & 0x40))
		    || ((CurrSRData.Byte.LoByte & 0x1C) == 0x1C &&  (CurrSRData.Byte.HiByte & 0x40)) ) )
		{
			SysStatusCode = E_EraseFail ; /* chip is protected, erase aborted */
			return FAIL ;
		}
		if( !WriteStatusWELBit() )
		{
			SysStatusCode = E_EraseFail ; /* sending error type code */
			return FAIL ;
		}
		PtrData(SetCSPinCmd) = 0 ; /* set chip CS port to Vil */
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = BulkEraseCmd ; /* send cmd Byte */
		SPIRWDelay() ;
		Delay_10nsX(10) ; /* ensure fpga send finish data before CS pin rise high */
		PtrData(SetCSPinCmd) = 1 ; /* set chip CS port to Vih */
		Delay_1ms(50) ;

		PtrData(GSetAllMaskDataLo) = StatusWIPBit ;  /* only check bit0, send byte checking mask data to FPGA */
		PollingTimeCnt = (ChipEraseTime * 100) ; /* divide to 10ms scale for one loop check */
		while(1)
		{ /* TH25Q-80HB chip erase: typ 5.2ms, max 7.8ms */
			StatusData = CheckAllStatusData(0x00) ; /* only check bit0, checking all socket & get check result */
			if( !StatusData ) /* if all socket WIP bit = 0 means program ok */
				break ;
			else if( !(--PollingTimeCnt) )
			{
				if( GangCheckIfAllSocketError(StatusData) )
				{ /* if all socket error then cancle function */
					SysStatusCode = E_EraseFail ; /* sending error type code */
					return FAIL ;
				}
				break ; /* go on process other pass socket */
			}
			Delay_1ms(10) ;
		}
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{/* Erase Security Register  */
		SRAMPointer = 0x03 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & EraseOn) )
			return SUCCESS ;

		/* LB1-LB3 为 OTP 锁定位,锁定后 Erase Security Registers 会被芯片静默忽略,先检查 */
		PtrData(GSetAllMaskDataLo) = (0x08 << (GetCurrBlkNum() - Noun_Block1)) ; /* LB1/LB2/LB3 in SR high byte */
		if( GangCheckIfAllSocketError(CheckAllStatus1Data(0x00)) )
		{ /* all socket locked: security register is read-only permanently */
			SysStatusCode = E_EraseFail ; /* sending error type code */
			return FAIL ;
		}

		if( !WriteStatusWELBit() )
		{
			SysStatusCode = E_EraseFail ; /* sending error type code */
			return FAIL ;
		}

		PtrData(SetCSPinCmd) = 0 ; /* set chip CS port to Vil */
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = EraseSecurityCmd ; /* send cmd Byte */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
		SPIRWDelay() ;
		Delay_10nsX(10) ; /* ensure fpga send finish data before CS pin rise high */
		PtrData(SetCSPinCmd) = 1 ; /* set chip CS port to Vih */
		Delay_1ms(50) ;

		PtrData(GSetAllMaskDataLo) = StatusWIPBit ;  /* only check bit0, send byte checking mask data to FPGA */
		PollingTimeCnt = 200 ; /* divide to 10ms scale for one loop check */
		while(1)
		{ /* TH25Q-80HB security register erase time = tSE, max 7.6ms */
			StatusData = CheckAllStatusData(0x00) ; /* only check bit0, checking all socket & get check result */
			if( !StatusData ) /* if all socket WIP bit = 0 means program ok */
				break ;
			else if( !(--PollingTimeCnt) )
			{
				if( GangCheckIfAllSocketError(StatusData) )
				{ /* if all socket error then cancle function */
					SysStatusCode = E_EraseFail ; /* sending error type code */
					return FAIL ;
				}
				break ; /* go on process other pass socket */
			}
			Delay_1ms(10) ;

			if( !(PollingTimeCnt & 0x7F) )
			{ /* above 16M bit=2S handshake 1 time to PC, handshake period time MUST <=4S */
				HandleEvent_EP0() ;
				Delay_1ms(10) ; /* NEED delay 10ms here */
			}
		}
	}
	return SUCCESS ;
}



/****************** Blank Check ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar BlankCheckFun(void)
{
uChar DevByteData, RamWordData ;
uShort TempCnt ;

	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
	Delay_10nsX(3) ;

	DeviceAddress = (GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{ /* if status regs */
		SRAMPointer = 0x03 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & StatusCheckOn) )
			return SUCCESS ;

		if( GetCurrBlkNum() == Noun_Block4 )
		{
			RamWordData = (uChar)(DevBlkVirgin & LSB(PrtRegMaskBit)) ;
			PtrData(GSetAllMaskDataLo) = LSB(PrtRegMaskBit) ;  /* send byte checking mask data to FPGA */
			DevByteData = CheckAllStatusData(RamWordData) ; /* checking all socket & get check result */
			if( DevByteData )
			{ /* have some socket check error */
				if( GangCheckIfAllSocketError(DevByteData) )
				{ /* if all socket error then cancle function */
					SysStatusCode = E_ProtectStatusFail ; /* sending error type code */
					return FAIL ;
				}
			}
				
			RamWordData = (uChar)(DevBlkVirgin & MSB(PrtRegMaskBit)) ;
			PtrData(GSetAllMaskDataLo) = MSB(PrtRegMaskBit) ;  /* send byte checking mask data to FPGA */
			DevByteData = CheckAllStatus1Data(RamWordData) ; /* checking all socket & get check result */
			if( DevByteData )
			{ /* have some socket check error */
				if( GangCheckIfAllSocketError(DevByteData) )
				{ /* if all socket error then cancle function */
					SysStatusCode = E_ProtectStatusFail ; /* sending error type code */
					return FAIL ;
				}
			}
		}
		return SUCCESS ;
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{ /* factory register & user register */
		SRAMPointer = 0x03 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & CheckOn) )
			return SUCCESS ;

		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = ReadSecurityCmd ; /* send cmd Byte */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = 0xFF ; /* Send Dummy byte */
		SPIRWDelay() ;
        
		PtrData(GSetAllMaskDataLo) = 0xFF ; /* send byte checking mask data to FPGA */
		while(1)
		{
			PtrData(GClrPageChkStatus) = 0x0000 ;  /* send checking mask data to FPGA */
			PtrData(GDevChkDataLoInc0) = 0xFF ; /* send checking data to let fpga to read all socket data */
			SPIRWDelay() ;
			PtrData(GDevChkDataLoInc0) = 0xFF ; /* send checking data to let fpga to read all socket data */
			SPIRWDelay() ;
			SPIRWDelay() ;
			DevByteData = PtrData(GChkAllSckStatus) ; /* get fpga checking all socket result */
			if( DevByteData )
			{ /* have some socket check error */
				if( GangCheckIfAllSocketError(DevByteData) )
				{ /* if all socket error then cancle function */
					PtrData(SetCSPinCmd) = 1 ;
					Delay_10nsX(10) ;
					SysStatusCode = E_HiddenROMBlankFail ; /* sending error type code */
					return FAIL ;
				}
			}

			DevCurrBlkLen.all -= 2 ;
			if( !DevCurrBlkLen.all ) /* checking block length end */
			{
				PtrData(SetCSPinCmd) = 1 ;
				Delay_10nsX(10) ;
				return SUCCESS ;
			}
		}
	}
	else
	{ /* main flash */
		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = ReadDataCmd ; /* 03h: Read Data, 3-byte address */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send A23-A16 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send A15-A8 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send A7-A0 */
		SPIRWDelay() ;

		PtrData(GSetAllMaskDataLo) = 0xFF ; /* send byte checking mask data to FPGA */
		while(1)
		{
			PtrData(GClrPageChkStatus) = 0x0000 ;  /* send checking mask data to FPGA */
			for(TempCnt=0; TempCnt<GDevPageChkCnt; TempCnt+=2)
			{
				PtrData(GDevChkDataLoInc0) = 0xFF ; /* send data to fpga & read all socket data */
				SPIRWDelay() ;
				PtrData(GDevChkDataLoInc0) = 0xFF ; /* send data to fpga & read all socket data */
				SPIRWDelay() ;
			}
			SPIRWDelay() ;
			DevByteData = PtrData(GChkAllSckStatus) ; /* get fpga checking all socket result */
			if( DevByteData )
			{ /* have some socket check error */
				if( GangCheckIfAllSocketError(DevByteData) )
				{ /* if all socket error then cancle function */
					PtrData(SetCSPinCmd) = 1 ;
					Delay_10nsX(10) ;
					SysStatusCode = E_BlankFail ; /* sending error type code */
					return FAIL ;
				}
			}

			DevCurrBlkLen.all -= GDevPageChkCnt ;
			if( !DevCurrBlkLen.all )
			{
				PtrData(SetCSPinCmd) = 1 ;
				Delay_10nsX(10) ;
				return SUCCESS ;
			}

			if( !(DevCurrBlkLen.all & 0xFFFFF) )
			{ /* above 16M bit=2S handshake 1 time to PC, handshake period time MUST <=4S */
				HandleEvent_EP0() ;
				Delay_1ms(10) ; /* NEED delay 10ms here */
			}
		}
	}
	return SUCCESS ;
}


/******************* ID Check ******************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar IDCheckFun(void)
{
	uChar MftID, DevIDLo;

	PtrData(GSetAllMaskDataLo) = 0xFF; /* send byte checking mask data to FPGA */

	PtrData(SetCSPinCmd) = 0;
	Delay_10nsX(10);
	PtrData(OneDataCmd) = ReadIDCmd; /* send cmd Byte */
	SPIRWDelay();
	PtrData(GClrPageChkStatus) = 0x0000;			  /* clear fpga page check cnt start from 0 */
	PtrData(GDevChkDataLoInc0) = (uChar)ManufactID(); /* send checking data to let fpga to read all socket data */
	SPIRWDelay();
	MftID = PtrData(GChkAllSckStatus); /* get fpga checking all socket result */
	if (GangCheckIfAllSocketError(MftID))
	{								  /* if all socket error then cancle function */
		PtrData(SetCSPinCmd) = 1;	  /* error path must also release CS */
		Delay_10nsX(10);
		SysStatusCode = E_AutoIDFail; /* sending error type code */
		return FAIL;
	}

	PtrData(GClrPageChkStatus) = 0x0000;			/* clear fpga page check cnt start from 0 */
	PtrData(GDevChkDataLoInc0) = MSB(DeviceIDLo()); /* send checking data to let fpga to read all socket data */
	SPIRWDelay();
	DevIDLo = PtrData(GChkAllSckStatus); /* get fpga checking all socket result */
	if (GangCheckIfAllSocketError(DevIDLo))
	{								  /* if all socket error then cancle function */
		PtrData(SetCSPinCmd) = 1;	  /* error path must also release CS */
		Delay_10nsX(10);
		SysStatusCode = E_AutoIDFail; /* sending error type code */
		return FAIL;
	}

	PtrData(GClrPageChkStatus) = 0x0000;			/* clear fpga page check cnt start from 0 */
	PtrData(GDevChkDataLoInc0) = LSB(DeviceIDLo()); /* send checking data to let fpga to read all socket data */
	SPIRWDelay();
	DevIDLo = PtrData(GChkAllSckStatus); /* get fpga checking all socket result */
	PtrData(SetCSPinCmd) = 1;
	Delay_10nsX(10);
	if (GangCheckIfAllSocketError(DevIDLo))
	{								  /* if all socket error then cancle function */
		SysStatusCode = E_AutoIDFail; /* sending error type code */
		return FAIL;
	}
	return SUCCESS;
}

/******************* CheckSumFun***************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar CheckSumFun(void)
{
uShort RamWordData ;
uInt32 ChecksumAll ;

	if( IfSpecialBitBlk() )
	{	/* if special bit block then read boot lock status */
		return SUCCESS ;
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{ /* if Externed Block Address */
		SRAMPointer = 0x03 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & ChecksumOn) )
		{
			CFCardReadFillSector() ; /* if have other data then need write finish one sector */
			return SUCCESS ;
		}

		ChecksumAll = (StdChecksumHi<<16) + StdChecksumLo ; /* get hi-lo 32 bit checksum data */
		while(1)
		{
			RamWordData = CFCardReadWordData() ;
			ChecksumAll += (MSB(RamWordData) + LSB(RamWordData)) ;

			DramBufCnt -= 2 ;
			DevCurrBlkLen.all -= 2 ;
			if( GetChkBlkLenEnd() ) /* checking block length end */
			{
				StdChecksumLo = LSW(ChecksumAll) ;
				StdChecksumHi = MSW(ChecksumAll) ;
				return SUCCESS ;
			}
		}
	}
	else
	{ /* Main Flash Block */
		SysStdChecksum() ;
	}
	return SUCCESS ;
}



/******************* Protect Subroutine **********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar ProtectFun(void)
{
uShort PollingTimeCnt;
uChar SR1Data ;
uChar SR2Data ;
uChar StatusData ;
stuShort CurrSRData ;

    /*
     * SRAM[0x00]：上位机下发的 SR 低字节目标值(BP4-BP0)
     * SRAM[0x01]：上位机下发的 SR 高字节目标值(CMP)
     *
     * GetSRAMByteData() 调用后 SRAMPointer 自动增加。
     *
     * 读-改-写,只改目标位,保留芯片原有状态：
     * 低字节：写入 BP4-BP0(bit6-bit2),保留 SRP0 等非目标位
     * 高字节：只写 CMP(bit6),保留 QE/SRP1/LB1-LB3(LBx 为 OTP 位)
     *
     * 注意：Gang 模式下读回值取自 A 槽,默认各槽芯片出厂状态一致。
     */
    PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
    Delay_10nsX(3) ;

    SRAMPointer = 0x00 ;

    SR1Data = GetSRAMByteData() & LSB(PrtRegMaskBit) ;
    SR2Data = GetSRAMByteData() & MSB(PrtRegMaskBit) ;

    CurrSRData.all = ReadStatusData() ; /* 读当前 SR(05h 低字节 + 35h 高字节) */

    SR1Data |= ( CurrSRData.Byte.LoByte & (uChar)(~LSB(PrtRegMaskBit)) ) ; /* 保留 SRP0 等非目标位 */
    SR2Data |= ( CurrSRData.Byte.HiByte & (uChar)(~MSB(PrtRegMaskBit)) ) ; /* 保留 QE/SRP1/LBx 等非目标位 */

    /**********************************************************
     * 06h Write Enable
     * 01h Write Status Register：先写低字节(S7-S0)再写高字节(S15-S8),
     * 一次命令写满 16 位(只写 1 字节会把 CMP/QE 清 0)。
     **********************************************************/
    if( !WriteStatusWELBit() )
    {
        SysStatusCode = E_ProtectFail ;
        return FAIL ;
    }

    PtrData(SetCSPinCmd) = 0 ;
    Delay_10nsX(10) ;

    PtrData(OneDataCmd) = WriteStatusCmd ;      /* 01h */
    SPIRWDelay() ;

    PtrData(OneDataCmd) = SR1Data ;             /* S7-S0：BP4-BP0 */
    SPIRWDelay() ;

    PtrData(OneDataCmd) = SR2Data ;             /* S15-S8：CMP */
    SPIRWDelay() ;

    Delay_10nsX(10) ;
    PtrData(SetCSPinCmd) = 1 ;
    Delay_10nsX(10) ;

    /*
     * 等待 SR.WIP 清零(05h 读低字节, tW max 4ms)。
     */
    PtrData(GSetAllMaskDataLo) = StatusWIPBit ;

    PollingTimeCnt = 100 ;

    while(1)
    {
        StatusData = CheckAllStatusData(0x00) ;

        if( !StatusData )
        {
            break ;
        }

        if( !(--PollingTimeCnt) )
        {
            if( GangCheckIfAllSocketError(StatusData) )
            {
                SysStatusCode = E_ProtectFail ;
                return FAIL ;
            }

            break ;
        }

        Delay_1ms(1) ;
    }

    /*
     * 读回并校验低字节 BP4-BP0(05h)。
     */
    PtrData(GSetAllMaskDataLo) = LSB(PrtRegMaskBit) ;

    StatusData = CheckAllStatusData(SR1Data) ;

    if( StatusData )
    {
        if( GangCheckIfAllSocketError(StatusData) )
        {
            SysStatusCode = E_ProtectFail ;
            return FAIL ;
        }
    }

    /*
     * 读回并校验高字节 CMP(35h)。
     */
    PtrData(GSetAllMaskDataLo) = MSB(PrtRegMaskBit) ;

    StatusData = CheckAllStatus1Data(SR2Data) ;

    if( StatusData )
    {
        if( GangCheckIfAllSocketError(StatusData) )
        {
            SysStatusCode = E_ProtectFail ;
            return FAIL ;
        }
    }

    return SUCCESS ;
}



/******************* Unprotect Subroutine ********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar UnProtectFun(void)
{
uShort PollingTimeCnt;
uChar SR1Data ;
uChar SR2Data ;
uChar StatusData ;
stuShort CurrSRData ;

    /*
     * 解保护：目标值取 H 文件中的默认值(BP4-BP0=0, CMP=0),
     * 读-改-写保留非目标位(SRP0/QE/SRP1/LB1-LB3,LBx 为 OTP 位)。
     *
     * 注意：Gang 模式下读回值取自 A 槽,默认各槽芯片出厂状态一致。
     */
    PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
    Delay_10nsX(3) ;

    SR1Data = PrtReg0DefaultBit & LSB(PrtRegMaskBit) ;
    SR2Data = PrtReg1DefaultBit & MSB(PrtRegMaskBit) ;

    CurrSRData.all = ReadStatusData() ; /* 读当前 SR(05h 低字节 + 35h 高字节) */

    SR1Data |= ( CurrSRData.Byte.LoByte & (uChar)(~LSB(PrtRegMaskBit)) ) ; /* 保留 SRP0 等非目标位 */
    SR2Data |= ( CurrSRData.Byte.HiByte & (uChar)(~MSB(PrtRegMaskBit)) ) ; /* 保留 QE/SRP1/LBx 等非目标位 */

    /**********************************************************
     * 06h Write Enable
     * 01h Write Status Register：先低字节后高字节,一次写满 16 位
     **********************************************************/
    if( !WriteStatusWELBit() )
    {
        SysStatusCode = E_UnprotectFail ;
        return FAIL ;
    }

    PtrData(SetCSPinCmd) = 0 ;
    Delay_10nsX(10) ;

    PtrData(OneDataCmd) = WriteStatusCmd ;      /* 01h */
    SPIRWDelay() ;

    PtrData(OneDataCmd) = SR1Data ;             /* S7-S0：BP4-BP0=0 */
    SPIRWDelay() ;

    PtrData(OneDataCmd) = SR2Data ;             /* S15-S8：CMP=0 */
    SPIRWDelay() ;

    Delay_10nsX(10) ;
    PtrData(SetCSPinCmd) = 1 ;
    Delay_10nsX(10) ;

    /*
     * 等待 SR.WIP 清零(05h 读低字节, tW max 4ms)。
     */
    PtrData(GSetAllMaskDataLo) = StatusWIPBit ;

    PollingTimeCnt = 100 ;

    while(1)
    {
        StatusData = CheckAllStatusData(0x00) ;

        if( !StatusData )
        {
            break ;
        }

        if( !(--PollingTimeCnt) )
        {
            if( GangCheckIfAllSocketError(StatusData) )
            {
                SysStatusCode = E_UnprotectFail ;
                return FAIL ;
            }

            break ;
        }

        Delay_1ms(1) ;
    }

    /*
     * 校验低字节 BP4-BP0 已经清零(05h)。
     */
    PtrData(GSetAllMaskDataLo) = LSB(PrtRegMaskBit) ;

    StatusData = CheckAllStatusData(SR1Data) ;

    if( StatusData )
    {
        if( GangCheckIfAllSocketError(StatusData) )
        {
            SysStatusCode = E_UnprotectFail ;
            return FAIL ;
        }
    }

    /*
     * 校验高字节 CMP 已经清零(35h)。
     */
    PtrData(GSetAllMaskDataLo) = MSB(PrtRegMaskBit) ;

    StatusData = CheckAllStatus1Data(SR2Data) ;

    if( StatusData )
    {
        if( GangCheckIfAllSocketError(StatusData) )
        {
            SysStatusCode = E_UnprotectFail ;
            return FAIL ;
        }
    }

    return SUCCESS ;
}




/******************* Secure Subroutine ***********************\
Notes: Permanently lock the selected Security Registers via LB1-LB3 (OTP).

\**********************************************************/
uChar SecureFun(void)
{
uShort PollingTimeCnt;
uChar SR1Data ;
uChar SR2Data ;
uChar StatusData ;
stuShort CurrSRData ;

    /*
     * 第一阶段暂时关闭 Secure/LB OTP 功能：
     * RMW 读回值取自单槽再广播写所有槽, 各槽 QE/LB/SRP 状态不一致时会交叉污染,
     * 待实现逐槽 Read-Modify-Write 后重新开放
     * (需同时恢复 .spc 的 LB 选项与 DevAlgoIdx_Table 的 SecureFun 注册)。
     */
    return SUCCESS ;

    /*
     * SRAM[0x01] 的 LB1-LB3(bit3-bit5)为上位机选择的永久锁定项。
     *
     * LB 位为 OTP：一旦写入 1, 对应 Security Register 永久只读, 不可撤销。
     * 未选择任何锁定项时直接返回成功。
     */
    SRAMPointer = 0x01 ;

    SR2Data = GetSRAMByteData() & StatusLBMaskBit ;

    if( !SR2Data )
    {
        return SUCCESS ;
    }

    /*
     * 读-改-写：只置位选中的 LB, 保留 BP/CMP/QE/SRPx 等其余位。
     * Gang 模式下读回值取自 A 槽, 默认各槽芯片出厂状态一致。
     */
    CurrSRData.all = ReadStatusData() ; /* 读当前 SR(05h 低字节 + 35h 高字节) */

    SR1Data = CurrSRData.Byte.LoByte ;              /* 低字节原样写回(01h 对 WIP/WEL 无效) */
    SR2Data |= CurrSRData.Byte.HiByte ;             /* 高字节 = 原值 | 新锁定位 */

    /**********************************************************
     * 06h Write Enable
     * 01h Write Status Register：先低字节后高字节,一次写满 16 位
     **********************************************************/
    if( !WriteStatusWELBit() )
    {
        SysStatusCode = E_SecureFail ;
        return FAIL ;
    }

    PtrData(SetCSPinCmd) = 0 ;
    Delay_10nsX(10) ;

    PtrData(OneDataCmd) = WriteStatusCmd ;      /* 01h */
    SPIRWDelay() ;

    PtrData(OneDataCmd) = SR1Data ;             /* S7-S0：原样写回 */
    SPIRWDelay() ;

    PtrData(OneDataCmd) = SR2Data ;             /* S15-S8：置位 LBx */
    SPIRWDelay() ;

    Delay_10nsX(10) ;
    PtrData(SetCSPinCmd) = 1 ;
    Delay_10nsX(10) ;

    /*
     * 等待 SR.WIP 清零(05h 读低字节, tW max 4ms)。
     */
    PtrData(GSetAllMaskDataLo) = StatusWIPBit ;

    PollingTimeCnt = 100 ;

    while(1)
    {
        StatusData = CheckAllStatusData(0x00) ;

        if( !StatusData )
        {
            break ;
        }

        if( !(--PollingTimeCnt) )
        {
            if( GangCheckIfAllSocketError(StatusData) )
            {
                SysStatusCode = E_SecureFail ;
                return FAIL ;
            }

            break ;
        }

        Delay_1ms(1) ;
    }

    /*
     * 读回并校验 LBx 已经置位(35h)。
     */
    PtrData(GSetAllMaskDataLo) = StatusLBMaskBit ;

    StatusData = CheckAllStatus1Data(SR2Data) ;

    if( StatusData )
    {
        if( GangCheckIfAllSocketError(StatusData) )
        {
            SysStatusCode = E_SecureFail ;
            return FAIL ;
        }
    }

    return SUCCESS ;
}




/**********************************************************\
		Subrountine Function Define Here
\**********************************************************/
void GetDeviceParameterInfo(void)
{
	ProgPageSize = ChipPageSizeTable[fwBlkIndex] ;
	ChipEraseTime = ChipEraseTimeTable[fwBlkIndex] ;
	ChipSPIMaxClkDelay = ChipSPIMaxClkTable[fwBlkIndex] ;
	ChipSPIMaxClkDelay = ((1000*8)/(25))/ChipSPIMaxClkDelay ;  /* FPGA SPI Delay Time = (1/xxM)*8Bit/(10ns*2.5 Scale) */
	PrtRegMaskBit = ProtectRegisterTable[fwBlkIndex] ;
	PrtReg0DefaultBit=ProtectRegister0DefaultTable[fwBlkIndex] ;
 	PrtReg1DefaultBit=ProtectRegister1DefaultTable[fwBlkIndex] ;
}


/******************************************************/
uShort ReadStatusData(void)
{
stuShort TempData ;

	/* 不在此选择槽位：调用者(Gang 功能)已在入口恢复全部有效槽,
	   此处切换槽位不恢复会导致后续 WREN/WRSR/CE 只作用单槽 */
	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ReadStatusCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let fpga read device data */
	SPIRWDelay() ;
	TempData.Byte.LoByte = PtrData(OneDataCmd) ;
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(50) ;

	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ReadStatus1Cmd ; /* send cmd Byte */
	SPIRWDelay() ;
	PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let fpga read device data */
	SPIRWDelay() ;
	TempData.Byte.HiByte = PtrData(OneDataCmd) ;
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;
	
	return ( TempData.all ) ;
}


uChar CheckAllStatusData(uChar DevData)
{
	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ReadStatusCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	PtrData(GClrPageChkStatus) = 0x0000 ; /* clear fpga page check cnt start from 0 */
	PtrData(GDevChkDataLoInc0) = DevData ; /* send checking data to let fpga to read all socket data */
	SPIRWDelay() ;
	Delay_10nsX(2) ;
	DevData = PtrData(GChkAllSckStatus) ; /* get fpga checking all socket result */
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;
	return (DevData) ;
}


uChar CheckAllStatus1Data(uChar DevData)
{
	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ReadStatus1Cmd ; /* send cmd Byte */
	SPIRWDelay() ;
	PtrData(GClrPageChkStatus) = 0x0000 ; /* clear fpga page check cnt start from 0 */
	PtrData(GDevChkDataLoInc0) = DevData ; /* send checking data to let fpga to read all socket data */
	SPIRWDelay() ;
	Delay_10nsX(2) ;
	DevData = PtrData(GChkAllSckStatus) ; /* get fpga checking all socket result */
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;
	return (DevData) ;
}


/***********************************************************/
uChar WriteStatusWELBit(void)
{
uChar PollingTimeCnt, StatusData;

    PtrData(SetCSPinCmd) = 0;
    Delay_10nsX(10);

    PtrData(OneDataCmd) = WriteEnableCmd;
    SPIRWDelay();

    Delay_10nsX(10);
    PtrData(SetCSPinCmd) = 1;
    Delay_10nsX(10);

    /*
     * SR1 bit1 = WEL。
     * 期望读取结果为1。
     */
    PtrData(GSetAllMaskDataLo) = StatusWELBit;

    PollingTimeCnt = 100;

    while(1)
    {
        StatusData = CheckAllStatusData(StatusWELBit);

        if(!StatusData)
        {
            return SUCCESS;
        }
        else if(!(--PollingTimeCnt))
        {
            if(GangCheckIfAllSocketError(StatusData))
            {
                return FAIL;
            }

            return SUCCESS;
        }

        Delay_1us(20);
    }
}



///////////////////////////////////////////////////////
//////    If Blank Page Data then return Success /////////
///////////////////////////////////////////////////////
void Get1PageDataChkBlank(void)
{
uShort BufferCnt ;

	BlankPageFlag = 0xFFFF ;
	for(BufferCnt=0; BufferCnt<ProgPageSize; BufferCnt+=2)
	{ /* divide by 2 for byte data width */
		PageDataBuf[BufferCnt/2] = CFCardReadWordData() ;
		BlankPageFlag &= PageDataBuf[BufferCnt/2] ;
	}
	if( BlankPageFlag != 0xFFFF )
		BlankPageFlag = 0x0000 ;
}



/************************************************************\
		Firmware or System Using Subroutine
		CAN NOT Modify!!!
\************************************************************/
uChar GetDrvElementLen(void)
{
	DevDrvVersion =  DevDRV_Version ; /* get driver version */
	GangType = SystemGang ;
	return (uChar)(sizeof(DrvElement_Table) / 5) ;
}


/*******************************************************************************/
/*                                       0           1             2           3         4-5-6-7                         8-max */
/* Download: EP0 format: WrSck+SNStep+SNType+SNLen+4DevAddrHH-HM-LM-LL + SNData0,1,2...max=(512-8) */
/* Upload: EP0 format:      WrSck+SckOkNg+SckEnDis+SNLen+4DevAddr + SNData0,1,2...max=(512-8) */
/*******************************************************************************/
void DevSNProgAndVerify(void)
{
uShort TempCnt, SNBufCnt, SNWrAddrOffset ;
uInt32 DevSNStrAddr ;
uChar SNPageBuff[256] ;

	if( (~GangErrSckMask) & SNDataBuf(0) ) /* bitx=1 enable prog, 0=disable, here prog only 1 sck per loop */
	{ /* if current sck be OK status, then goon wr SN */
		SNBufCnt = SNDataBuf(3) ;/* SN cnt max <=256=1page prog length protect */
		DevSNStrAddr = (SNDataBuf(4)<<24) + (SNDataBuf(5)<<16) + (SNDataBuf(6)<<8) + SNDataBuf(7) ; /* get dev SN str addr */
		DeviceAddress = DevSNStrAddr & (~0xFF) ; /* get dev SN page str addr */
		SNWrAddrOffset = DevSNStrAddr - DeviceAddress ; /* get dev SN page offset addr */
		if( (SNWrAddrOffset + SNBufCnt) > 256 )
		{/* SN code overflow 1 prog page then error return for protect */
			SNDataBuf(1) = SNDataBuf(0) ; /* set sck wr SN 0=pass, 1=ng */
			SNDataBuf(2) = SKBInputFlag ; /* set sck enable/disable flag */

			if( GangCheckIfAllSocketError(SNDataBuf(0)) )
			{ /* checking if all sck error, then update disp */
				SysStatusCode = E_DeviceSerialCodeSetFail ; /* sending error type code */
				Send_Status_Code() ;
			}
			return ;
		}

	/////// If need change SN code to new Key Code, add Algo here! //////////////////////
	/// here JUST for one example algo, NEED save new key code back to SNDataBuf[8,9....] for upload to PC ///
//		for(TempCnt=0; TempCnt<SNBufCnt; TempCnt++) 
//			SNDataBuf(8+TempCnt) = SNDataBuf(8+TempCnt) * 2 ;
//		.........
//          SNDataBuf(3) = 32 ; /* update new key code length */
	/////////////////////////////////////////////////////////////////////////////

	/////// read SN page data before write SN ///////////
		PtrData(GEnSelSckNum) = SNDataBuf(0) ; /* get write sck num, 1=enable check */
		Delay_10nsX(10) ;
		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = FastReadDataCmd ; /* 0Bh: Fast Read, 3-byte address */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send A23-A16 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send A15-A8 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send A7-A0 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = 0xFF ; /* Dumy Send */
		SPIRWDelay() ;

		for(TempCnt=0; TempCnt<256; TempCnt++)
		{ /* read back Device SN page data */
			PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let fpga read device data */
			SPIRWDelay() ;
			SNPageBuff[TempCnt] = PtrData(OneDataCmd) ; /* the second data is the true data */
		}
		PtrData(SetCSPinCmd) = 1 ;
		Delay_10nsX(10) ;

		for(TempCnt=0; TempCnt<SNBufCnt; TempCnt++) /* merging new SN data in buffer */
			SNPageBuff[SNWrAddrOffset+TempCnt] = SNDataBuf(8+TempCnt) ;

	////// write back SN page data to device /////////
		PtrData(SetCSPinCmd) = 0 ; /* set chip CS port to Vil */
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = WriteEnableCmd ; /* send cmd Byte */
		SPIRWDelay() ;
		Delay_10nsX(10) ; /* ensure fpga send finish data before CS pin rise high */
		PtrData(SetCSPinCmd) = 1 ; /* set chip CS port to Vih */
		Delay_10nsX(100) ;

		PtrData(SetCSPinCmd) = 0 ; /* set chip CS port to Vil */
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = PageProgCmd ; /* 02h: Page Program, 3-byte address */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send A23-A16 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send A15-A8 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send A7-A0 */
		SPIRWDelay() ;

		for(TempCnt=0; TempCnt<256; TempCnt++)
		{
			PtrData(OneDataCmd) = SNPageBuff[TempCnt] ;
			SPIRWDelay() ;
		}
		SPIRWDelay() ;
		PtrData(SetCSPinCmd) = 1 ; /* set chip CS port to Vih */
		Delay_1ms(10) ;

	/////// read back SN page data for verifying ///////////
		PtrData(GEnSelSckNum) = SNDataBuf(0) ; /* get write sck num */
		Delay_10nsX(10) ;
		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = FastReadDataCmd ; /* 0Bh: Fast Read, 3-byte address */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(MSW(DeviceAddress)) ; /* send A23-A16 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = MSB(LSW(DeviceAddress)) ; /* send A15-A8 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = LSB(LSW(DeviceAddress)) ; /* send A7-A0 */
		SPIRWDelay() ;
		PtrData(OneDataCmd) = 0xFF ; /* Dumy Send */
		SPIRWDelay() ;

		for(TempCnt=0; TempCnt<256; TempCnt++)
		{ /* read back Device SN page data */
			PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let fpga read device data */
			SPIRWDelay() ;
			SNBufCnt = PtrData(OneDataCmd) ; /* the second data is the true data */
			if( SNPageBuff[TempCnt] != SNBufCnt )
			{ /* if data check error then set flag & power off */
				PtrData(SetCSPinCmd) = 1 ;
				Delay_10nsX(10) ;
				SNDataBuf(1) = SNDataBuf(0) ; /* set sck wr SN 0=pass, 1=ng */
				SNDataBuf(2) = SKBInputFlag ; /* set sck enable/disable flag */

				if( GangCheckIfAllSocketError(SNDataBuf(0)) )
				{ /* checking if all sck error, then update disp */
					SysStatusCode = E_DeviceSerialCodeSetFail ; /* sending error type code */
					Send_Status_Code() ;
				}
				PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask); /* re-enable all pass socket */
				Delay_10nsX(3);
				return ;
			}
		}
		PtrData(SetCSPinCmd) = 1 ;
		Delay_10nsX(10) ;
		SNDataBuf(1) = 0x00 ; /* set sck wr SN 0=pass, 1=ng */
		SNDataBuf(2) = SKBInputFlag ; /* set sck enable/disable flag */

		PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask); /* re-enable all pass socket */
		Delay_10nsX(3);
	}
}

