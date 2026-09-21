/*
V1.3 2026-09-08
1. First version adapted for MICRON N25Q256Ax3ESFxxx.
2. Main Flash size is 32MB, address range 00000000h-01FFFFFFh.
3. Enter volatile 4-byte address mode after power-on and use standard
   03h/02h commands with 4-byte addresses.
4. JEDEC ID (9Fh) is 20h-BAh-19h.
5. Support Page Program (02h), Bulk Erase (C7h), Status Register (05h/01h)
   and Flag Status Register (70h/50h).
6. Support 64-byte OTP data array plus one OTP control byte at address 40h.
   OTP read uses 4Bh and OTP program uses 42h.
7. OTP does not support erase. The control byte bit0 is used as the
   permanent lock request: bit0=1 keeps OTP unlocked, while an explicit
   bit0=0 request programs address 40h and permanently locks the OTP array.
8. OTP Read/Program/Verify/BlankCheck/Checksum are controlled by the
   driver OTP operation-enable bits.
   */
/******************************************************************************\
	Programmer total 13 tasks driver setting:
POWER_ON_FUN							POWER_OFF_FUN
READ_FUN			PROGRAM_FUN		VERIFY_FUN
ERASE_FUN			BLANK_CHECK_FUN	ILLEGAL_CHECK_FUN
ID_CHECK_FUN		SECURE_FUN			CHECKSUM_FUN
PROTECT_FUN								UNPROTECT_FUN
\*******************************************************************************/

#include "WAx4_N25Q256Ax3ESFxxx.h"
#include "h/Disp.h"
#include "h/Key.h"
#include "h/FirmWare.h"
#include "h/BlockSectorDef.h"
#include "h/Type.h"
#include "h/SystemTask.h"
#include "h/Subroutine.h"
#include "h/ErrorCode.h"
#include "h/PassedCode.h"
#include "h/USBCom.h"
#include "h/CFControl.h"
#include "h/StatusCode.h"
#include "h/hs0_mmc.h"

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

/******** Control Command Macro Define **************/
#define WriteEnableCmd          0x06  /* Write Enable, set WEL bit */
#define WriteDisableCmd         0x04  /* Write Disable, clear WEL bit */
#define ReadStatusCmd           0x05  /* Read 8-bit Status Register (S7-S0) */
#define WriteStatusCmd          0x01  /* Write 8-bit Status Register (S7-S0) */

#define ReadDataCmd             0x03  /* Read Data in 4-byte address mode */
#define FastReadDataCmd         0x0B  /* Fast Read in 4-byte address mode */
#define PageProgCmd             0x02  /* Page Program in 4-byte address mode */
#define SubsectorEraseCmd       0x20  /* 4KB Subsector Erase in 4-byte address mode */
#define SectorEraseCmd          0xD8  /* 64KB Sector Erase in 4-byte address mode */
#define BulkEraseCmd            0xC7  /* Bulk Erase entire 32MB main array */
#define Enter4ByteAddrModeCmd   0xB7  /* Enter 4-byte address mode; A13 requires WREN */
#define ReadIDCmd               0x9F  /* Read JEDEC ID, N25Q256A = 20h-BAh-19h */
#define ReadFlagStatusCmd       0x70  /* Read Flag Status Register */
#define ClearFlagStatusCmd      0x50  /* Clear Flag Status Register error bits */
#define ReadOTPArrayCmd         0x4B  /* Read 64-byte OTP array and control byte */
#define ProgramOTPArrayCmd      0x42  /* Program OTP array or control byte */
#define ReadNVCRCmd             0xB5  /* Read 16-bit NVCR, low byte first */
#define WriteNVCRCmd            0xB1  /* Write 16-bit NVCR, low byte first */

/******* Status Control Bits **********/
#define	StatusWIPBit			0x01
#define	StatusWELBit			0x02
#define FlagStatusProgramErrorMask 0x12 /* Program failure + protection failure */
#define FlagStatusEraseErrorMask   0x22 /* Erase failure + protection failure */
#define OTPProgramOn            0x01 /* SRAM[3] bit0: permit irreversible OTP programming. */
#define OTPVerifyOn             0x02 /* SRAM[3] bit1: verify OTP data and lock bit. */
#define OTPBlankCheckOn         0x08 /* SRAM[3] bit3: blank-check OTP data and lock bit. */
#define OTPChecksumOn           0x10 /* SRAM[3] bit4: include OTP CF/DRAM data in checksum. */
#define OTPDataSize             64
#define OTPControlAddr          0x40
#define OTPBlockSize            65
#define OTPControlMask          0x01
#define NVCRLowVerifyMask       0xDF /* NVCR bit5 is Reserved/Don't Care. */
#define NVCRHighVerifyMask      0xFF
#define NVCRReservedBit5        0x0020
#define NVCRWriteTimeoutMs      3500

#define	GDevPageChkCnt		256 /* Byte data counter */
/* MUST delay enough time let FPGA read 8 bit serial data & finish checking 4 socket data!! */
#define	SPIRWDelay()		(Delay_10nsX(ChipSPIMaxClkDelay)) /* 10ns scale, refer to sck speed!  */

/****************************************************************\
Notes: User define yoursel subroutine here

\****************************************************************/
void GetDeviceParameterInfo(void) ;
uChar ReadStatusData(void) ;
uChar ReadFlagStatusData(void) ;
uChar CheckAllStatusData(uChar TempData) ;
uChar WriteStatusWELBit(void) ;
void Get1PageDataChkBlank(void) ;
void Send4ByteAddress(uInt32 Address) ;
void ClearFlagStatusData(void) ;
uChar CheckAllFlagStatusData(uChar DevData) ;
void SkipOTPBlockData(void) ;
uChar ReadOTPArrayData(uChar Address, uChar *Data, uChar Count) ;
uChar ProgramOTPArray(uChar Address, uChar *Data, uChar Count) ;
uChar VerifyOTPArray(uChar Address, uChar *Data, uChar Count, uChar Mask) ;
uShort ReadNVCRData(void) ;
uChar WriteNVCRData(uShort Data) ;
uChar CheckAllNVCRData(uShort Data) ;
uChar NVCRDataIsValid(uShort Data) ;
uChar Enter4ByteAddressMode(void) ;

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
	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
	if( !Enter4ByteAddressMode() ){
		SysStatusCode = E_AutoIDFail ;
		return FAIL ;
	}
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
uChar OTPData[OTPDataSize], OTPControlData ;

	DeviceAddress =((uInt32)GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{ /* Status/NVCR SRAM blocks and virtual driver control. */
		if( GetCurrBlkNum() == Noun_Block2 )
		{
			PtrData(GEnSelSckNum) = EnSckACtrlFlag ; /* read data path: select socket A */
			Delay_10nsX(10) ;
			DevWordData.all = ReadStatusData() ;
			SRAMPointer = 0x00 ;
			SaveSRAMByteData( DevWordData.Byte.LoByte ) ; /* save SR low byte (05h) */
			PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* restore all pass socket */
			Delay_10nsX(3) ;
		}
		else if( GetCurrBlkNum() == Noun_Block3 )
		{
			PtrData(GEnSelSckNum) = EnSckACtrlFlag ; /* upload one deterministic socket */
			Delay_10nsX(10) ;
			DevWordData.all = ReadNVCRData() ;
			SRAMPointer = 0x01 ;
			SaveSRAMByteData( DevWordData.Byte.LoByte ) ;
			SRAMPointer = 0x02 ;
			SaveSRAMByteData( DevWordData.Byte.HiByte ) ;
			PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
			Delay_10nsX(3) ;
		}
		else if( GetCurrBlkNum() == Noun_BlkPtStatus )
		{
			/* SRAM[3] is virtual driver control; never send a corresponding IC command. */
			return SUCCESS ;
		}
		return SUCCESS ;
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{ /* OTP: read 64 data bytes and the control byte in separate transactions. */
		PtrData(GEnSelSckNum) = EnSckACtrlFlag ;
		Delay_10nsX(10) ;

		if( !ReadOTPArrayData(0x00, OTPData, OTPDataSize) )
		{
			PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
			Delay_10nsX(3) ;
			SysStatusCode = E_ReadFail ;
			return FAIL ;
		}
		if( !ReadOTPArrayData(OTPControlAddr, &OTPControlData, 1) )
		{
			PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
			Delay_10nsX(3) ;
			SysStatusCode = E_ReadFail ;
			return FAIL ;
		}

		for(TempCnt=0; TempCnt<OTPDataSize; TempCnt+=2)
		{
			DevWordData.Byte.LoByte = OTPData[TempCnt] ;
			DevWordData.Byte.HiByte = OTPData[TempCnt+1] ;
			ReadDataBuf[ReadBufCnt/2] = DevWordData.all ;
			ReadBufCnt += 2 ;
		}
		DevWordData.Byte.LoByte = OTPControlData ;
		DevWordData.Byte.HiByte = (uChar)DevBlkVirgin ; /* upload transport padding only */
		ReadDataBuf[ReadBufCnt/2] = DevWordData.all ;
		ReadBufCnt++ ;
		DeviceAddress += OTPBlockSize ;
		DevCurrBlkLen.all -= OTPBlockSize ;
		SaveChkBlkLenEnd() ;

		PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
		Delay_10nsX(3) ;
		return SUCCESS ;
	}
	else
	{ /* Read main flash */
		PtrData(GEnSelSckNum) = EnSckACtrlFlag ; /* default select socket A */

		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = ReadDataCmd ; /* 03h: Read Data in 4-byte address mode */
		SPIRWDelay() ;
		Send4ByteAddress(DeviceAddress) ;

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
			DeviceAddress += MaxPktByteSize ;
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
}

/****************** Program Subroutine **********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar ProgramFun(void)
{
uChar StatusData ;
uShort RamWordData, PageTempCnt, PollingTimeCnt ;
uChar OTPData[OTPDataSize], OTPControlData, OTPControlProgramData ;

	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
	Delay_10nsX(3) ;

	DeviceAddress = ((uInt32)GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
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
	{ /* Program the 64-byte OTP array; lock only on an explicit control bit0=0. */
		SRAMPointer = 0x03 ;
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & OTPProgramOn) )
		{
			SkipOTPBlockData() ;
			return SUCCESS ;
		}

		for(PageTempCnt=0; PageTempCnt<OTPDataSize; PageTempCnt+=2)
		{
			RamWordData = CFCardReadWordData() ;
			OTPData[PageTempCnt] = LSB(RamWordData) ;
			OTPData[PageTempCnt+1] = MSB(RamWordData) ;
		}
		RamWordData = CFCardReadWordData() ; /* word-aligned transport includes one padding byte */
		OTPControlData = LSB(RamWordData) ;

		if( !ProgramOTPArray(0x00, OTPData, OTPDataSize) )
		{
			SysStatusCode = E_HiddenROMProgramFail ;
			return FAIL ;
		}
		if( !VerifyOTPArray(0x00, OTPData, OTPDataSize, 0xFF) )
		{
			SysStatusCode = E_HiddenROMProgramFail ;
			return FAIL ;
		}

		OTPControlProgramData = OTPControlData ;
		if( !(OTPControlData & OTPControlMask) )
		{
			/* Only an explicit 0 request may perform the irreversible OTP lock. */
			OTPControlProgramData = 0xFE ;
			if( !ProgramOTPArray(OTPControlAddr, &OTPControlProgramData, 1) )
			{
				SysStatusCode = E_HiddenROMProgramFail ;
				return FAIL ;
			}
		}
		if( !VerifyOTPArray(OTPControlAddr, &OTPControlProgramData, 1, OTPControlMask) )
		{
			SysStatusCode = E_HiddenROMProgramFail ;
			return FAIL ;
		}

		DramBufCnt -= OTPBlockSize ;
		DevCurrBlkLen.all -= OTPBlockSize ;
		DeviceAddress += OTPBlockSize ;
		GetChkBlkLenEnd() ;
		return SUCCESS ;
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
				ClearFlagStatusData() ;
				if( !WriteStatusWELBit() )
				{
					SysStatusCode = E_ProgramNoDataFail ; /* sending error type code */
					return FAIL ;
				}
				PtrData(SetCSPinCmd) = 0 ; /* set chip CS port to Vil */
				Delay_10nsX(10) ;
				PtrData(OneDataCmd) = PageProgCmd ; /* 02h: Page Program in 4-byte address mode */
				SPIRWDelay() ;
				Send4ByteAddress(DeviceAddress) ;

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

				Delay_1us(400) ; /* NEED delay page time here before polling WIP, refer to tPP */
				PtrData(GSetAllMaskDataLo) = StatusWIPBit | StatusWELBit ;  /* only check bit0, send byte checking mask data to FPGA */
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
						PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
						Delay_10nsX(3) ;
						break ; /* go on process other pass socket */
					}
					Delay_1us(5) ;
				}

				PtrData(GSetAllMaskDataLo) = FlagStatusProgramErrorMask ;
				StatusData = CheckAllFlagStatusData(0x00) ;
				if( StatusData )
					{
						if( GangCheckIfAllSocketError(StatusData) )
						{
							SysStatusCode = E_ProgramNoDataFail ;
							return FAIL ;
						}
						PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
						Delay_10nsX(3) ;
					}
				DeviceAddress += ProgPageSize ;
				DramBufCnt -= ProgPageSize ;
				DevCurrBlkLen.all -= ProgPageSize ;

				BlockEndFlag = GetChkBlkLenEnd() ;

				if( !BlockEndFlag )
					Get1PageDataChkBlank() ;
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
}

/****************** Verify Subroutine ************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar VerifyFun(void)
{
uChar DevByteData ;
uShort RamWordData, TempCnt ;
uChar OTPData[OTPDataSize], OTPControlData ;

	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
	Delay_10nsX(3) ;

	DeviceAddress = ((uInt32)GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{ /* Status verify is always active; no customer enable bit is required. */
		if( GetCurrBlkNum() == Noun_Block2 )
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
				PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
				Delay_10nsX(3) ;
			}
		}
		else if( GetCurrBlkNum() == Noun_Block3 )
		{
			return SUCCESS ;
		}
		return SUCCESS ;
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{ /* Verify 64 OTP data bytes, then control bit0 in a separate 4Bh transaction. */
		SRAMPointer = 0x03 ;
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & OTPVerifyOn) )
		{
			SkipOTPBlockData();
			return SUCCESS ;
		}

		for(TempCnt=0; TempCnt<OTPDataSize; TempCnt+=2)
		{
			RamWordData = CFCardReadWordData() ;
			OTPData[TempCnt] = LSB(RamWordData) ;
			OTPData[TempCnt+1] = MSB(RamWordData) ;
		}
		RamWordData = CFCardReadWordData() ;
		OTPControlData = LSB(RamWordData) ;

		if( !VerifyOTPArray(0x00, OTPData, OTPDataSize, 0xFF) )
		{
			SysStatusCode = E_HiddenROMVerifyFail ;
			return FAIL ;
		}
		if( !VerifyOTPArray(OTPControlAddr, &OTPControlData, 1, OTPControlMask) )
		{
			SysStatusCode = E_HiddenROMVerifyFail ;
			return FAIL ;
		}

		DramBufCnt -= OTPBlockSize ;
		DevCurrBlkLen.all -= OTPBlockSize ;
		DeviceAddress += OTPBlockSize ;
		GetChkBlkLenEnd() ;
		return SUCCESS ;
	}
	else
	{ /* Verify main flash */
		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = ReadDataCmd ; /* 03h: Read Data in 4-byte address mode */
		SPIRWDelay() ;
		Send4ByteAddress(DeviceAddress) ;

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
			DeviceAddress += GDevPageChkCnt ;
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
}


/******************* Erase Subroutine ***********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/

uChar EraseFun(void)
{
uShort StatusData, PollingTimeCnt ;

	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
	Delay_10nsX(3) ;

	if( IfFirstTimeEntry() )
	{
		ClearFlagStatusData() ;

		if( !WriteStatusWELBit() )
		{
			SysStatusCode = E_EraseFail ;
			return FAIL ;
		}
		PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
		Delay_10nsX(3) ;
		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = BulkEraseCmd ;	/* N25Q256A Bulk Erase: C7h */
		SPIRWDelay() ;
		Delay_10nsX(10) ;
		PtrData(SetCSPinCmd) = 1 ;
		Delay_1ms(1000) ;
		PtrData(GSetAllMaskDataLo) = StatusWIPBit ;
		PollingTimeCnt = (ChipEraseTime * 100) ;
		while(1)
		{
			StatusData = CheckAllStatusData(0x00) ;
			if( !StatusData )
				break ;
			else if( !(--PollingTimeCnt) )
			{
				if( GangCheckIfAllSocketError(StatusData) )
				{
					SysStatusCode = E_EraseFail ;
					return FAIL ;
				}
				PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
				Delay_10nsX(3) ;
				break ;
			}
			Delay_1ms(10) ;
			if( !(PollingTimeCnt & 0x7F) )
			{
				HandleEvent_EP0() ;
				Delay_1ms(10) ;
			}
		}
		PtrData(GSetAllMaskDataLo) = FlagStatusEraseErrorMask ;
		StatusData = CheckAllFlagStatusData(0x00) ;
		if( StatusData )
		{
			if( GangCheckIfAllSocketError(StatusData) )
			{
				SysStatusCode = E_EraseFail ;
				return FAIL ;
			}
			PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
			Delay_10nsX(3) ;
		}
	}

	return SUCCESS ;
}


/****************** Blank Check ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar BlankCheckFun(void)
{
uChar DevByteData, OTPData[OTPDataSize], OTPControlData ;
uShort TempCnt ;

	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
	Delay_10nsX(3) ;

	DeviceAddress = ((uInt32)GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{
		if( GetCurrBlkNum() == Noun_Block2 )
			return SUCCESS ; /* Status protection bits do not participate in IC blank state. */
		else if( GetCurrBlkNum() == Noun_Block3 )
		{
			/* NVCR support retained for future use; disabled in current customer SPC. */
			return SUCCESS ;
		}
		return SUCCESS ;
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{ /* OTP is blank only when data is FFh and control bit0 is still 1. */
		SRAMPointer = 0x03 ;
		TempCnt = GetSRAMByteData() ;
		if( !(TempCnt & OTPBlankCheckOn) )
		{
			DeviceAddress += OTPBlockSize ;
			DevCurrBlkLen.all -= OTPBlockSize ;
			return SUCCESS ;
		}

		for(TempCnt=0; TempCnt<OTPDataSize; TempCnt++)
			OTPData[TempCnt] = 0xFF ;
		OTPControlData = OTPControlMask ;

		if( !VerifyOTPArray(0x00, OTPData, OTPDataSize, 0xFF) )
		{
			SysStatusCode = E_HiddenROMBlankFail ;
			return FAIL ;
		}
		if( !VerifyOTPArray(OTPControlAddr, &OTPControlData, 1, OTPControlMask) )
		{
			SysStatusCode = E_HiddenROMBlankFail ;
			return FAIL ;
		}

		DeviceAddress += OTPBlockSize ;
		DevCurrBlkLen.all -= OTPBlockSize ;
		return SUCCESS ;
	}
	else
	{ /* main flash */
		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = ReadDataCmd ; /* 03h: Read Data in 4-byte address mode */
		SPIRWDelay() ;
		Send4ByteAddress(DeviceAddress) ;

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

			DeviceAddress += GDevPageChkCnt ;
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
	{ /* OTP checksum is over 65 logical CF/DRAM bytes; byte 66 is padding. */
		DeviceAddress = ((uInt32)GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
		SRAMPointer = 0x03 ;
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & OTPChecksumOn) )
		{
			SkipOTPBlockData() ;
			return SUCCESS ;
		}

		ChecksumAll = ((uInt32)StdChecksumHi<<16) + StdChecksumLo ;
		while(1)
		{
			RamWordData = CFCardReadWordData() ;
			ChecksumAll += LSB(RamWordData) ;
			DramBufCnt-- ;
			DevCurrBlkLen.all-- ;
			DeviceAddress++ ;
			if( GetChkBlkLenEnd() )
			{
				StdChecksumLo = LSW(ChecksumAll) ;
				StdChecksumHi = MSW(ChecksumAll) ;
				return SUCCESS ;
			}

			ChecksumAll += MSB(RamWordData) ;
			DramBufCnt-- ;
			DevCurrBlkLen.all-- ;
			DeviceAddress++ ;
			if( GetChkBlkLenEnd() )
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
uChar StatusData ;
uChar CurrSRData ;

    /* SRAM[0x00] supplies BP3, TB and BP2-BP0; preserve all other SR bits. */
    PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
    Delay_10nsX(3) ;

    SRAMPointer = 0x00 ;

    SR1Data = GetSRAMByteData() & LSB(PrtRegMaskBit) ;

    CurrSRData = ReadStatusData() ;

    SR1Data |= ( CurrSRData & (uChar)(~LSB(PrtRegMaskBit)) ) ; /* Preserve SRWD and volatile bits. */

    /* N25Q256A command 01h accepts exactly one Status Register byte. */
    ClearFlagStatusData() ;
    if( !WriteStatusWELBit() )
    {
        SysStatusCode = E_ProtectFail ;
        return FAIL ;
    }

    PtrData(SetCSPinCmd) = 0 ;
    Delay_10nsX(10) ;

    PtrData(OneDataCmd) = WriteStatusCmd ;      /* 01h */
    SPIRWDelay() ;

    PtrData(OneDataCmd) = SR1Data ;             /* One-byte N25Q256A Status Register. */
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
     * Read back and verify BP3, TB and BP2-BP0 with 05h.
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

    return SUCCESS ;
}



/******************* Unprotect Subroutine ********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar UnProtectFun(void)
{
uShort PollingTimeCnt;
uChar SR1Data ;
uChar StatusData ;
uChar CurrSRData ;

    /* Clear BP3, TB and BP2-BP0 while preserving the remaining SR bits. */
    PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
    Delay_10nsX(3) ;

    SR1Data = PrtReg0DefaultBit & LSB(PrtRegMaskBit) ;

    CurrSRData = ReadStatusData() ;

    SR1Data |= ( CurrSRData & (uChar)(~LSB(PrtRegMaskBit)) ) ; /* Preserve SRWD and volatile bits. */

    /* N25Q256A command 01h accepts exactly one Status Register byte. */
    ClearFlagStatusData() ;
    if( !WriteStatusWELBit() )
    {
        SysStatusCode = E_UnprotectFail ;
        return FAIL ;
    }

    PtrData(SetCSPinCmd) = 0 ;
    Delay_10nsX(10) ;

    PtrData(OneDataCmd) = WriteStatusCmd ;      /* 01h */
    SPIRWDelay() ;

    PtrData(OneDataCmd) = SR1Data ;             /* One-byte N25Q256A Status Register. */
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

    return SUCCESS ;
}


uChar SecureFun(void)
{
	
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
	PrtReg0DefaultBit=ProtectRegisterDefaultTable[fwBlkIndex] ;
}


/******************************************************/
uChar ReadStatusData(void)
{
uChar TempData ;

	/* 不在此选择槽位：调用者(Gang 功能)已在入口恢复全部有效槽,
	   此处切换槽位不恢复会导致后续 WREN/WRSR/CE 只作用单槽 */
	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ReadStatusCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let fpga read device data */
	SPIRWDelay() ;
	TempData = PtrData(OneDataCmd) ;
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;

	return TempData ;
}

uChar ReadFlagStatusData(void)
{
uChar TempData ;

    TempData = 0x00 ;
    PtrData(SetCSPinCmd) = 0 ;
    Delay_10nsX(10) ;
    PtrData(OneDataCmd) = ReadFlagStatusCmd ;   /* 70h */
    SPIRWDelay() ;
    PtrData(GDevChkDataLoInc0) = 0xFF ;
    SPIRWDelay() ;
    TempData = PtrData(OneDataCmd) ;

    PtrData(SetCSPinCmd) = 1 ;
    Delay_10nsX(10) ;

    return TempData ;
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


void Send4ByteAddress(uInt32 Address)
{
	PtrData(OneDataCmd) = MSB(MSW(Address)) ;
	SPIRWDelay() ;
	PtrData(OneDataCmd) = LSB(MSW(Address)) ;
	SPIRWDelay() ;
	PtrData(OneDataCmd) = MSB(LSW(Address)) ;
	SPIRWDelay() ;
	PtrData(OneDataCmd) = LSB(LSW(Address)) ;
	SPIRWDelay() ;
}


void ClearFlagStatusData(void)
{
	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ClearFlagStatusCmd ;
	SPIRWDelay() ;
	Delay_10nsX(10) ;
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;
}


uChar CheckAllFlagStatusData(uChar DevData)
{
	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ReadFlagStatusCmd ;
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


void SkipOTPBlockData(void)
{
uShort TempCnt ;

	for(TempCnt=0; TempCnt<OTPDataSize; TempCnt+=2)
		(void)CFCardReadWordData() ;
	(void)CFCardReadWordData() ; /* low byte is control; high byte is padding */

	DramBufCnt -= OTPBlockSize ;
	DevCurrBlkLen.all -= OTPBlockSize ;
	DeviceAddress += OTPBlockSize ;
	GetChkBlkLenEnd() ;
}


uChar ReadOTPArrayData(uChar Address, uChar *Data, uChar Count)
{
uChar TempCnt ;

	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ReadOTPArrayCmd ;
	SPIRWDelay() ;
	Send4ByteAddress((uInt32)Address) ;
	PtrData(OneDataCmd) = 0xFF ; /* Eight dummy clocks in extended SPI protocol. */
	SPIRWDelay() ;

	for(TempCnt=0; TempCnt<Count; TempCnt++)
	{
		PtrData(GDevChkDataLoInc0) = 0xFF ;
		SPIRWDelay() ;
		Data[TempCnt] = PtrData(OneDataCmd) ;
	}

	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;
	return SUCCESS ;
}


uChar ProgramOTPArray(uChar Address, uChar *Data, uChar Count)
{
uShort PollingTimeCnt ;
uChar StatusData, TempCnt ;

	ClearFlagStatusData() ;
	if( !WriteStatusWELBit() )
		return FAIL ;

	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ProgramOTPArrayCmd ;
	SPIRWDelay() ;
	Send4ByteAddress((uInt32)Address) ;
	for(TempCnt=0; TempCnt<Count; TempCnt++)
	{
		PtrData(OneDataCmd) = Data[TempCnt] ;
		SPIRWDelay() ;
	}
	Delay_10nsX(10) ;
	PtrData(SetCSPinCmd) = 1 ;
	Delay_1us(400) ;

	PtrData(GSetAllMaskDataLo) = StatusWIPBit ;
	PollingTimeCnt = 3000 ;
	while(1)
	{
		StatusData = CheckAllStatusData(0x00) ;
		if( !StatusData )
			break ;
		if( !(--PollingTimeCnt) )
		{
			if( GangCheckIfAllSocketError(StatusData) )
				return FAIL ;
			PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
			Delay_10nsX(3) ;
			break ;
		}
		Delay_1us(5) ;
	}

	PtrData(GSetAllMaskDataLo) = FlagStatusProgramErrorMask ;
	StatusData = CheckAllFlagStatusData(0x00) ;
	if( StatusData )
	{
		if( GangCheckIfAllSocketError(StatusData) )
			return FAIL ;
		PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
		Delay_10nsX(3) ;
	}
	return SUCCESS ;
}


uChar VerifyOTPArray(uChar Address, uChar *Data, uChar Count, uChar Mask)
{
uChar StatusData, TempCnt ;

	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ReadOTPArrayCmd ;
	SPIRWDelay() ;
	Send4ByteAddress((uInt32)Address) ;
	PtrData(OneDataCmd) = 0xFF ; /* One dummy byte. */
	SPIRWDelay() ;

	PtrData(GSetAllMaskDataLo) = Mask ;
	PtrData(GClrPageChkStatus) = 0x0000 ;
	for(TempCnt=0; TempCnt<Count; TempCnt++)
	{
		PtrData(GDevChkDataLoInc0) = Data[TempCnt] & Mask ;
		SPIRWDelay() ;
	}
	SPIRWDelay() ;
	StatusData = PtrData(GChkAllSckStatus) ;
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;
	if( StatusData )
	{
		if( GangCheckIfAllSocketError(StatusData) )
			return FAIL ;
		PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
		Delay_10nsX(3) ;
	}
	return SUCCESS ;
}


uShort ReadNVCRData(void)
{
stuShort TempData ;

	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ReadNVCRCmd ;
	SPIRWDelay() ;
	PtrData(GDevChkDataLoInc0) = 0xFF ;
	SPIRWDelay() ;
	TempData.Byte.LoByte = PtrData(OneDataCmd) ;
	PtrData(GDevChkDataLoInc0) = 0xFF ;
	SPIRWDelay() ;
	TempData.Byte.HiByte = PtrData(OneDataCmd) ;
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;
	return TempData.all ;
}


uChar CheckAllNVCRData(uShort Data)
{
uChar LowStatus, HighStatus ;

	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = ReadNVCRCmd ;
	SPIRWDelay() ;

	PtrData(GSetAllMaskDataLo) = NVCRLowVerifyMask ;
	PtrData(GClrPageChkStatus) = 0x0000 ;
	PtrData(GDevChkDataLoInc0) = LSB(Data) ;
	SPIRWDelay() ;
	Delay_10nsX(2) ;
	LowStatus = PtrData(GChkAllSckStatus) ;

	PtrData(GSetAllMaskDataLo) = NVCRHighVerifyMask ;
	PtrData(GClrPageChkStatus) = 0x0000 ;
	PtrData(GDevChkDataLoInc0) = MSB(Data) ;
	SPIRWDelay() ;
	Delay_10nsX(2) ;
	HighStatus = PtrData(GChkAllSckStatus) ;

	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;
	return (LowStatus | HighStatus) ;
}


uChar NVCRDataIsValid(uShort Data)
{
uChar XIPData, DriverData ;

	XIPData = (uChar)((Data >> 9) & 0x07) ;
	DriverData = (uChar)((Data >> 6) & 0x07) ;
	if( (XIPData == 5) || (XIPData == 6) )
		return FAIL ;
	if( (DriverData == 0) || (DriverData == 4) )
		return FAIL ;
	return SUCCESS ;
}


uChar WriteNVCRData(uShort Data)
{
uShort PollingTimeCnt ;
uChar StatusData ;

	ClearFlagStatusData() ;
	if( !WriteStatusWELBit() )
		return FAIL ;

	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = WriteNVCRCmd ;
	SPIRWDelay() ;
	PtrData(OneDataCmd) = LSB(Data) ; /* Datasheet requires low byte first. */
	SPIRWDelay() ;
	PtrData(OneDataCmd) = MSB(Data) ;
	SPIRWDelay() ;
	Delay_10nsX(10) ;
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;

	PtrData(GSetAllMaskDataLo) = StatusWIPBit ;
	PollingTimeCnt = NVCRWriteTimeoutMs ;
	while(1)
	{
		StatusData = CheckAllStatusData(0x00) ;
		if( !StatusData )
			break ;
		if( !(--PollingTimeCnt) )
		{
			if( GangCheckIfAllSocketError(StatusData) )
				return FAIL ;
			PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
			Delay_10nsX(3) ;
			break ;
		}
		Delay_1ms(1) ;
		if( !(PollingTimeCnt & 0x7F) )
		{
			HandleEvent_EP0() ;
			Delay_1ms(1) ;
		}
	}

	PtrData(GSetAllMaskDataLo) = FlagStatusProgramErrorMask ;
	StatusData = CheckAllFlagStatusData(0x00) ;
	if( StatusData )
	{
		if( GangCheckIfAllSocketError(StatusData) )
			return FAIL ;
		PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
		Delay_10nsX(3) ;
	}
	return SUCCESS ;
}

/**************** Enter 4-Byte Address Mode ******************\
Notes: Set and verify WEL before sending the B7h command.

\**********************************************************/
uChar Enter4ByteAddressMode(void)
{
	uChar FlagStatusData ;
	if( !WriteStatusWELBit() )
		return FAIL ;
	PtrData(SetCSPinCmd) = 0 ;
	Delay_10nsX(10) ;
	PtrData(OneDataCmd) = Enter4ByteAddrModeCmd ;
	SPIRWDelay() ;
	Delay_10nsX(10) ;
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;
    FlagStatusData = ReadFlagStatusData() ;
    if( !(FlagStatusData & 0x01) )
    {
        return FAIL ;
    }
	return SUCCESS ;
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

			PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask);
			Delay_10nsX(3);
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
uShort TempCnt, SNBufCnt, SNWrAddrOffset, PollingTimeCnt ;
uInt32 DevSNStrAddr ;
uChar SNPageBuff[256], StatusData ;

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
		PtrData(OneDataCmd) = FastReadDataCmd ; /* 0Bh: Fast Read in 4-byte address mode */
		SPIRWDelay() ;
		Send4ByteAddress(DeviceAddress) ;
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
		ClearFlagStatusData() ;
		if( !WriteStatusWELBit() )
		{
			SNDataBuf(1) = SNDataBuf(0) ;
			SNDataBuf(2) = SKBInputFlag ;
			return ;
		}

		PtrData(SetCSPinCmd) = 0 ; /* set chip CS port to Vil */
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = PageProgCmd ; /* 02h: Page Program in 4-byte address mode */
		SPIRWDelay() ;
		Send4ByteAddress(DeviceAddress) ;

		for(TempCnt=0; TempCnt<256; TempCnt++)
		{
			PtrData(OneDataCmd) = SNPageBuff[TempCnt] ;
			SPIRWDelay() ;
		}
		SPIRWDelay() ;
		PtrData(SetCSPinCmd) = 1 ; /* set chip CS port to Vih */
		Delay_1us(400) ;
		PtrData(GSetAllMaskDataLo) = StatusWIPBit ;
		PollingTimeCnt = 3000 ;
		while(1)
		{
			StatusData = CheckAllStatusData(0x00) ;
			if( !StatusData )
				break ;
			if( !(--PollingTimeCnt) )
			{
				GangCheckIfAllSocketError(StatusData) ;
				SNDataBuf(1) = SNDataBuf(0) ;
				SNDataBuf(2) = SKBInputFlag ;
				return ;
			}
			Delay_1us(5) ;
		}
		PtrData(GSetAllMaskDataLo) = FlagStatusProgramErrorMask ;
		StatusData = CheckAllFlagStatusData(0x00) ;
		if( StatusData )
		{
			GangCheckIfAllSocketError(StatusData) ;
			SNDataBuf(1) = SNDataBuf(0) ;
			SNDataBuf(2) = SKBInputFlag ;
			return ;
		}

	/////// read back SN page data for verifying ///////////
		PtrData(GEnSelSckNum) = SNDataBuf(0) ; /* get write sck num */
		Delay_10nsX(10) ;
		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = FastReadDataCmd ; /* 0Bh: Fast Read in 4-byte address mode */
		SPIRWDelay() ;
		Send4ByteAddress(DeviceAddress) ;
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
