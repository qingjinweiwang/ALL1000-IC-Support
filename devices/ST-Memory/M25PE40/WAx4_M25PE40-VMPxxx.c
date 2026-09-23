
/******************************************************************************\
	Programmer total 13 tasks driver setting:
POWER_ON_FUN							POWER_OFF_FUN
READ_FUN			PROGRAM_FUN		VERIFY_FUN
ERASE_FUN			BLANK_CHECK_FUN	ILLEGAL_CHECK_FUN
ID_CHECK_FUN		SECURE_FUN			CHECKSUM_FUN
PROTECT_FUN								UNPROTECT_FUN			
\*******************************************************************************/

#include "WAx4_M25PE40-VMPxxx.h"
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
#define ReadStatusCmd           0x05  /* Read 8-bit Status Register (S7-S0) */
#define WriteStatusCmd          0x01  /* Write 8-bit Status Register (S7-S0) */

#define ReadDataCmd             0x03  /* 3-byte address normal read */
#define FastReadDataCmd         0x0B  /* 3-byte address + 1 dummy byte fast read */
#define PageWriteCmd            0x0A  /* 3-byte address page write, 1~256 bytes */
#define PageProgCmd             0x02  /* 3-byte address page program, 1~256 bytes */
#define BulkEraseCmd            0xC7  /* Bulk erase entire 512KB main array */
#define ReadIDCmd               0x9F  /* Read JEDEC ID, M25PE40 = 20h-80h-13h */

/******* Status Control Bits **********/
#define	StatusWIPBit			0x01
#define	StatusWELBit			0x02

#define	GDevPageChkCnt		256 /* Byte data counter */
/* MUST delay enough time let FPGA read 8 bit serial data & finish checking 4 socket data!! */
#define	SPIRWDelay()		(Delay_10nsX(ChipSPIMaxClkDelay)) /* 10ns scale, refer to sck speed!  */

/****************************************************************\
Notes: User define yoursel subroutine here

\****************************************************************/
void GetDeviceParameterInfo(void) ;
uChar ReadStatusData(void) ;
uChar CheckAllStatusData(uChar TempData) ;
uChar WriteStatusWELBit(void) ;
uChar WriteStatusBPBits(uChar BPData) ;
void Get1PageDataChkBlank(void) ;

/********************* Power On ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar PowerOnFun(void)
{
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;

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
	PtrData(SetCSPinCmd) = 1 ;
	Delay_10nsX(10) ;

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

	DeviceAddress = ((uInt32)GetBlkChipStartAddrHi()<<16)+ GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{ /* if status regs */
		if( GetCurrBlkNum() == Noun_Block1 )
		{
			PtrData(GEnSelSckNum) = EnSckACtrlFlag ; /* read data path: select socket A */
			Delay_10nsX(10) ;
			DevWordData.all = ReadStatusData() ;
			SaveSRAMByteData( DevWordData.Byte.LoByte) ;
			PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* restore all pass socket */
			Delay_10nsX(10) ;
		}
		return SUCCESS ;
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

	DeviceAddress = ((uInt32)GetBlkChipStartAddrHi()<<16)+ GetBlkChipStartAddrLo() ;
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

				/* A rejected Page Program leaves WEL set although WIP is already 0. */
				PtrData(GSetAllMaskDataLo) = StatusWELBit ;
				StatusData = CheckAllStatusData(0x00) ;
				if( StatusData )
				{
					if( GangCheckIfAllSocketError(StatusData) )
					{
						SysStatusCode = E_ProgramNoDataFail ;
						return FAIL ;
					}
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

	DeviceAddress = ((uInt32)GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{ /* if status regs */
		if( GetCurrBlkNum() == Noun_Block1 )
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
		}
		return SUCCESS ;
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
		if( !UnProtectFun() )
		{
			SysStatusCode = E_EraseFail ;
			return FAIL ;
		}

		PtrData(GSetAllMaskDataLo) = LSB(PrtRegMaskBit) ;
		StatusData = CheckAllStatusData(0x00) ;
		if( StatusData )
		{
			if( GangCheckIfAllSocketError(StatusData) )
			{
				SysStatusCode = E_EraseFail ;
				return FAIL ;
			}
		}

		if( !WriteStatusWELBit() )
		{
			SysStatusCode = E_EraseFail ;
			return FAIL ;
		}

		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = BulkEraseCmd ;
		SPIRWDelay() ;
		Delay_10nsX(10) ;
		PtrData(SetCSPinCmd) = 1 ;
		Delay_10nsX(10) ;

		PtrData(GSetAllMaskDataLo) = StatusWIPBit ;
		PollingTimeCnt = 100 ;
		while(1)
		{
			StatusData = CheckAllStatusData(StatusWIPBit) ;
			if( !StatusData )
				break ;
			if( !(--PollingTimeCnt) )
			{
				if( GangCheckIfAllSocketError(StatusData) )
				{
					SysStatusCode = E_EraseFail ;
					return FAIL ;
				}
				break ;
			}
			Delay_1us(20) ;
		}

		PollingTimeCnt = (ChipEraseTime * 100) ;
		while(1)
		{
			StatusData = CheckAllStatusData(0x00) ;
			if( !StatusData )
				break ;
			if( !(--PollingTimeCnt) )
			{
				if( GangCheckIfAllSocketError(StatusData) )
				{
					SysStatusCode = E_EraseFail ;
					return FAIL ;
				}
				break ;
			}
			Delay_1ms(10) ;
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

	DeviceAddress = ((uInt32)GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{ /* if status regs */
		if( GetCurrBlkNum() == Noun_Block1 )
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
		}
		return SUCCESS ;
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
}

/******************* ID Check ******************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/

uChar IDCheckFun(void)
{
	uChar MftID, DevIDLo;

	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask);
	Delay_10nsX(10);
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
	if( IfSpecialBitBlk() )
	{	/* Status register and virtual driver-control blocks are not checksummed. */
		return SUCCESS ;
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
uChar SR1Data ;

    PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
    Delay_10nsX(3) ;

    SRAMPointer = 0x00 ;
    SR1Data = GetSRAMByteData() & LSB(PrtRegMaskBit) ;

    if( !WriteStatusBPBits(SR1Data) )
    {
        SysStatusCode = E_ProtectFail ;
        return FAIL ;
    }
    return SUCCESS ;
}

/******************* Unprotect Subroutine ********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar UnProtectFun(void)
{
    PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ; /* enable all pass socket before gang operation */
    Delay_10nsX(3) ;

    if( !WriteStatusBPBits(PrtReg0DefaultBit) )
    {
        SysStatusCode = E_UnprotectFail ;
        return FAIL ;
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
	PrtReg0DefaultBit=ProtectRegisterDefaultTable[fwBlkIndex] ;
}

/******************************************************/
uChar ReadStatusData(void)
{
	/* 不在此选择槽位：调用者(Gang 功能)已在入口恢复全部有效槽,
	此处切换槽位不恢复会导致后续 WREN/WRSR/CE 只作用单槽 */
uChar TempData ;

    TempData = 0x00 ;
    PtrData(SetCSPinCmd) = 0 ;
    Delay_10nsX(10) ;
    PtrData(OneDataCmd) = ReadStatusCmd ; /* 05h: Read 8-bit Status Register */
    SPIRWDelay() ;
    PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let FPGA read device data */
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

/***********************************************************/
uChar WriteStatusBPBits(uChar BPData)
{
uShort PollingTimeCnt ;
uChar StatusData ;

    BPData &= LSB(PrtRegMaskBit) ; /* SRWD=0; only BP2-BP0 are written */

    if( !WriteStatusWELBit() )
        return FAIL ;

    PtrData(SetCSPinCmd) = 0 ;
    Delay_10nsX(10) ;
    PtrData(OneDataCmd) = WriteStatusCmd ;
    SPIRWDelay() ;
    PtrData(OneDataCmd) = BPData ;
    SPIRWDelay() ;
    Delay_10nsX(10) ;
    PtrData(SetCSPinCmd) = 1 ;
    Delay_10nsX(10) ;

    PtrData(GSetAllMaskDataLo) = StatusWIPBit ;
    PollingTimeCnt = 20 ; /* tW max is 15ms */
    while(1)
    {
        StatusData = CheckAllStatusData(0x00) ;
        if( !StatusData )
            break ;
        if( !(--PollingTimeCnt) )
        {
            if( GangCheckIfAllSocketError(StatusData) )
                return FAIL ;
            break ;
        }
        Delay_1ms(1) ;
    }

    PtrData(GSetAllMaskDataLo) = LSB(PrtRegMaskBit) ;
    StatusData = CheckAllStatusData(BPData) ;
    if( StatusData )
    {
        if( GangCheckIfAllSocketError(StatusData) )
            return FAIL ;
    }

    return SUCCESS ;
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
		if( !WriteStatusWELBit() )
		{
			SNDataBuf(1) = SNDataBuf(0) ;
			SNDataBuf(2) = SKBInputFlag ;
			SysStatusCode = E_DeviceSerialCodeSetFail ;
			Send_Status_Code() ;
			PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
			Delay_10nsX(3) ;
			return ;
		}

		PtrData(SetCSPinCmd) = 0 ; /* set chip CS port to Vil */
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = PageWriteCmd ; /* 0Ah: Page Write, 3-byte address */
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
		Delay_10nsX(10) ;

		PtrData(GSetAllMaskDataLo) = StatusWIPBit ;
		PollingTimeCnt = 30 ; /* tPW max is 23ms */
		while(1)
		{
			StatusData = CheckAllStatusData(0x00) ;
			if( !StatusData )
				break ;
			if( !(--PollingTimeCnt) )
			{
				SNDataBuf(1) = SNDataBuf(0) ;
				SNDataBuf(2) = SKBInputFlag ;
				if( GangCheckIfAllSocketError(StatusData) )
				{
					SysStatusCode = E_DeviceSerialCodeSetFail ;
					Send_Status_Code() ;
				}
				PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
				Delay_10nsX(3) ;
				return ;
			}
			Delay_1ms(1) ;
		}

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
