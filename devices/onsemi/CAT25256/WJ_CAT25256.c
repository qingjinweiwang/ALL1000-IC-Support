/******************************************************************************\
	Programmer total 13 tasks driver setting:
POWER_ON_FUN							POWER_OFF_FUN
READ_FUN			PROGRAM_FUN		VERIFY_FUN
ERASE_FUN			BLANK_CHECK_FUN	ILLEGAL_CHECK_FUN
ID_CHECK_FUN		SECURE_FUN			CHECKSUM_FUN
PROTECT_FUN								UNPROTECT_FUN
\*******************************************************************************/

#include "WJ_CAT25256.h"
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
Notes: Use 1 time power on/off control flag: 1=Use 1 time, 0=Many time
\**************************************************************/
#define OnlyUse1TimePowerCtrl		1

/**************************************************************\
Notes: user-defined variables here,
          the area is UserVarityAreaMin ~ UserVarityAreaMax
\**************************************************************/
uShort PageDataBuf[256] ; /* Max Buffer Size be 256 word */
uShort BlockEndFlag ;
uInt32 DeviceAddress ;
uShort ProgPageSize ;
uChar ChipSPIMaxClkDelay ;
uShort PrtRegMaskBit ;
uShort PrtRegDefaultBit;


/******** Control Command Macro Define **************/
#define	WriteEnableCmd		0x06
#define	WriteDisableCmd		0x04
#define	ReadStatusCmd		0x05
#define	WriteStatusCmd		0x01

#define	ReadDataCmd		    0x03
#define	PageProgCmd		    0x02

#define	CATAddrMask			0x7FFFUL

/******* Status Control Bits **********/
#define	StatusWIPBit			0x01
#define	StatusWELBit			0x02
#define	StatusArrayProtectMask	0x0C


#define	GDevPageChkCnt		256 /* Byte data counter */
/* MUST delay enough time to let FPGA read 8 bit serial data & finish checking 4 socket data!! */
#define	SPIRWDelay()		(Delay_10nsX(ChipSPIMaxClkDelay)) /* 10ns scale, refer to sck speed! 1/3 */
#define	SetSPIDivider	(FPGAOffset|0xB000<<1) /* for setting FPGA output frequency divider */


/****************************************************************\
Notes: User define yourself subroutine here

\****************************************************************/

void SelectActiveSockets(void);
void SendCATAddr(uInt32 AddrData);
void GetDeviceParameterInfo(void);
uShort CAT25256ReadStatus(void);
uChar CAT25256WaitStatus(uChar ExpectData, uChar StatusMask, uShort PollingTimeCnt, uShort DelayUs);
uChar CAT25256WriteEnable(void);
uChar CAT25256WriteStatus(uChar StatusData, uChar StatusMask, uChar ErrorCode);


/********************* Power On ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar PowerOnFun(void)
{
#if OnlyUse1TimePowerCtrl
	if (Use1TimePowerOnMode)
		return SUCCESS; /* if already in power on status then return back */
	else
		Use1TimePowerOnMode = 1;
#endif

	Set_VccDAC() ; /* setting DAC array to rise VCC/VPP/VPE/VPS/VPIO volts level */
	Set_VihDAC() ;
	Set_VppDAC() ;
	Set_VpioDAC() ;
	Delay_1ms(50) ; /* delay for stable voltage */

	GangRiseAllPassSocketVccIccPin() ; /* open VCC/VPP/VPE/VPS/VPIO pin driving */
	Delay_1ms(50) ; /* delay for stable voltage */
	GetDeviceParameterInfo() ;
	Delay_1ms(50) ;
	SelectActiveSockets();
	/* Ensure CS high >= 250ns (CAT25256 min tSHS) before falling edge. */
	Delay_10nsX(30);

	return SUCCESS ;
}



/********************* Power Off ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar PowerOffFun(void)
{
#if OnlyUse1TimePowerCtrl
	if (Use1TimePowerOnMode && SysCmdBuf(0))
		return SUCCESS; /* if command buffer have other process then donot power off */
	else
		Use1TimePowerOnMode = 0;
#endif

	Fall_VpioIpioAll() ;
	Fall_VppIppAll() ;
	Fall_VihAll() ;
	Fall_VccAll() ;
	Delay_1ms(50) ; /* delay for stable voltage */
	return SUCCESS ;
}




/****************** Read Subroutine **************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar ReadFun(void)
{
	stuShort DevWordData;
	uShort TempCnt, ReadByteCnt;

	SelectActiveSockets();

	DeviceAddress = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();
	DeviceAddress &= CATAddrMask;

	if (IfSpecialBitBlk())
	{
		if (LSB(PrtRegMaskBit))
		{
			DevWordData.Byte.LoByte = CAT25256ReadStatus() & LSB(PrtRegMaskBit);
			SRAMPointer = 0x00;
			SaveSRAMByteData(DevWordData.Byte.LoByte);
		}

		return SUCCESS;
	}

	PtrData(SetCSPinCmd) = 0;
	Delay_10nsX(10);
	PtrData(OneDataCmd) = ReadDataCmd;
	SPIRWDelay();
	SendCATAddr(DeviceAddress);

	while (1)
	{
		ReadByteCnt = (DevCurrBlkLen.all > MaxPktByteSize) ? MaxPktByteSize : LSW(DevCurrBlkLen.all);
		for (TempCnt = 0; TempCnt < ReadByteCnt; TempCnt += 2)
		{
			DevWordData.all = 0xFFFF;
			PtrData(GDevChkDataLoInc0) = 0xFF;
			SPIRWDelay();
			DevWordData.Byte.LoByte = PtrData(OneDataCmd);
			SPIRWDelay();

			if ((TempCnt + 1) < ReadByteCnt)
			{
				PtrData(GDevChkDataLoInc0) = 0xFF;
				SPIRWDelay();
				DevWordData.Byte.HiByte = PtrData(OneDataCmd);
				SPIRWDelay();
			}

			ReadDataBuf[(ReadBufCnt + TempCnt) / 2] = DevWordData.all;
		}

		ReadBufCnt += ReadByteCnt;
		DevCurrBlkLen.all -= ReadByteCnt;
		if (SaveChkBlkLenEnd())
		{
			PtrData(SetCSPinCmd) = 1;
			Delay_1us(10);   /* extra CS high time */
			return SUCCESS;
		}
	}
}


/****************** Program Subroutine **********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar ProgramFun(void)
{
	uShort RamWordData, PageTempCnt, WriteByteCnt, PageRemain, BufferCnt;
	uChar StatusData;

	SelectActiveSockets();

	DeviceAddress = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();
	DeviceAddress &= CATAddrMask;

	if (IfSpecialBitBlk())
	{
		return SUCCESS;
	}

	StatusData = CAT25256ReadStatus();
	if (StatusData & StatusArrayProtectMask)
	{
		SysStatusCode = E_ProtectStatusFail;
		return FAIL;
	}

	while (1)
	{
		PageRemain = ProgPageSize - (uShort)(DeviceAddress % ProgPageSize);
		WriteByteCnt = (DevCurrBlkLen.all < PageRemain) ? LSW(DevCurrBlkLen.all) : PageRemain;

		for (BufferCnt = 0; BufferCnt < WriteByteCnt; BufferCnt += 2)
			PageDataBuf[BufferCnt / 2] = CFCardReadWordData();

		if (!CAT25256WriteEnable())
		{
			SysStatusCode = E_ProgramNoDataFail;
			return FAIL;
		}

		PtrData(SetCSPinCmd) = 0;
		Delay_10nsX(10);
		PtrData(OneDataCmd) = PageProgCmd;
		SPIRWDelay();
		SendCATAddr(DeviceAddress);

		for (PageTempCnt = 0; PageTempCnt < WriteByteCnt; PageTempCnt += 2)
		{
			RamWordData = PageDataBuf[PageTempCnt / 2];
			PtrData(OneDataCmd) = LSB(RamWordData);
			SPIRWDelay();
			if ((PageTempCnt + 1) < WriteByteCnt)
			{
				PtrData(OneDataCmd) = MSB(RamWordData);
				SPIRWDelay();
			}
		}

		SPIRWDelay();
		PtrData(SetCSPinCmd) = 1;
		Delay_10nsX(10);

		if (!CAT25256WaitStatus(0x00, StatusWIPBit, 5000, 5))
		{
			SysStatusCode = E_ProgramNoDataFail;
			return FAIL;
		}

		DeviceAddress += WriteByteCnt;
		DramBufCnt -= WriteByteCnt;
		DevCurrBlkLen.all -= WriteByteCnt;
		BlockEndFlag = GetChkBlkLenEnd();
		if (BlockEndFlag)
			return SUCCESS;

		if (!(DevCurrBlkLen.all & 0x0FFF))
		{
			HandleEvent_EP0();
			Delay_1ms(10);
		}
	}
}

/****************** Verify Subroutine ************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar VerifyFun(void)
{
	uChar DevByteData;
	uShort RamWordData, TempCnt, CheckByteCnt;

	SelectActiveSockets();

	DeviceAddress = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();
	DeviceAddress &= CATAddrMask;

	if (IfSpecialBitBlk())
	{
		return SUCCESS;
	}

	PtrData(SetCSPinCmd) = 0;
	Delay_10nsX(10);
	PtrData(OneDataCmd) = ReadDataCmd;
	SPIRWDelay();
	SendCATAddr(DeviceAddress);

	PtrData(GSetAllMaskDataLo) = 0xFF;
	while (1)
	{
		CheckByteCnt = (DevCurrBlkLen.all > GDevPageChkCnt) ? GDevPageChkCnt : LSW(DevCurrBlkLen.all);
		PtrData(GClrPageChkStatus) = 0x0000;
		for (TempCnt = 0; TempCnt < CheckByteCnt; TempCnt += 2)
		{
			RamWordData = CFCardReadWordData();
			PtrData(GDevChkDataLoInc0) = LSB(RamWordData);
			SPIRWDelay();
			if ((TempCnt + 1) < CheckByteCnt)
			{
				PtrData(GDevChkDataLoInc0) = MSB(RamWordData);
				SPIRWDelay();
			}
		}

		SPIRWDelay();
		DevByteData = PtrData(GChkAllSckStatus);
		if (DevByteData)
		{
			if (GangCheckIfAllSocketError(DevByteData))
			{
				PtrData(SetCSPinCmd) = 1;
				Delay_1us(10);   /* extra CS high time */
				SysStatusCode = E_VerifyFail;
				return FAIL;
			}
		}

		DramBufCnt -= CheckByteCnt;
		DevCurrBlkLen.all -= CheckByteCnt;
		if (GetChkBlkLenEnd())
		{
			PtrData(SetCSPinCmd) = 1;
			Delay_1us(10);   /* extra CS high time */
			return SUCCESS;
		}

		if (!(DevCurrBlkLen.all & 0xFFFFF))
		{
			HandleEvent_EP0();
			Delay_1ms(10);
		}
	}
}


/******************* Erase Subroutine ***********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar EraseFun(void)
{
	/* CAT25256 is EEPROM. There is no sector/chip erase command. */
	return SUCCESS ;
}



/****************** Blank Check ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar BlankCheckFun(void)
{
	uChar DevByteData;
	uShort TempCnt, CheckByteCnt;

	DeviceAddress = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();
	DeviceAddress &= CATAddrMask;

	if (IfSpecialBitBlk())
		return SUCCESS;

	SelectActiveSockets();

	PtrData(SetCSPinCmd) = 0;
	Delay_10nsX(10);
	PtrData(OneDataCmd) = ReadDataCmd;
	SPIRWDelay();
	SendCATAddr(DeviceAddress);

	PtrData(GSetAllMaskDataLo) = 0xFF;
	while (1)
	{
		CheckByteCnt = (DevCurrBlkLen.all > GDevPageChkCnt) ? GDevPageChkCnt : LSW(DevCurrBlkLen.all);
		PtrData(GClrPageChkStatus) = 0x0000;
		for (TempCnt = 0; TempCnt < CheckByteCnt; TempCnt += 2)
		{
			PtrData(GDevChkDataLoInc0) = 0xFF;
			SPIRWDelay();
			if ((TempCnt + 1) < CheckByteCnt)
			{
				PtrData(GDevChkDataLoInc0) = 0xFF;
				SPIRWDelay();
			}
		}
		SPIRWDelay();
		DevByteData = PtrData(GChkAllSckStatus);
		if (DevByteData)
		{
			if (GangCheckIfAllSocketError(DevByteData))
			{
				PtrData(SetCSPinCmd) = 1;
				Delay_1us(10);   /* extra CS high time */
				SysStatusCode = E_BlankFail;
				return FAIL;
			}
		}
		DevCurrBlkLen.all -= CheckByteCnt;
		if (!DevCurrBlkLen.all)
		{
			PtrData(SetCSPinCmd) = 1;
			Delay_1us(10);   /* extra CS high time */
			return SUCCESS;
		}
		if (!(DevCurrBlkLen.all & 0xFFFFF))
		{
			HandleEvent_EP0();
			Delay_1ms(10);
		}
	}
}


/******************* ID Check ******************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar IDCheckFun(void)
{
	/* CAT25256 has no JEDEC 0x9F ID. Treat ID check as not applicable. */
	return SUCCESS ;
}


/******************* CheckSumFun***************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar CheckSumFun(void)
{
	/* CAT25256 has only the main EEPROM array. Use standard checksum. */
	SysStdChecksum() ;
	return SUCCESS ;
}



/******************* Protect Subroutine **********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar ProtectFun(void)
{
	uChar StatusData;

	StatusData = SpecialBitBuf(0) & LSB(PrtRegMaskBit);
	return CAT25256WriteStatus(StatusData, LSB(PrtRegMaskBit), E_ProtectFail);
}


/******************* Unprotect Subroutine ********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar UnProtectFun(void)
{
	return CAT25256WriteStatus(0x00, LSB(PrtRegMaskBit), E_UnprotectFail);
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
uChar SNPageBuff[64] ;

	if ((~GangErrSckMask) & SNDataBuf(0) ) /* bitx=1 enable prog, 0=disable, here prog only 1 sck per loop */
	{ /* if current sck be OK status, then go on wr SN */
		SNBufCnt = SNDataBuf(3) ;/* SN cnt max <=64=1page prog length protect */
		DevSNStrAddr = (SNDataBuf(4)<<24) + (SNDataBuf(5)<<16) + (SNDataBuf(6)<<8) + SNDataBuf(7) ; /* get dev SN str addr */
		DevSNStrAddr &= CATAddrMask;
		DeviceAddress = DevSNStrAddr & (~((uInt32)64 - 1)) ; /* get dev SN page str addr */
		DeviceAddress &= CATAddrMask;
		SNWrAddrOffset = DevSNStrAddr - DeviceAddress ; /* get dev SN page offset addr */
		if ((SNWrAddrOffset + SNBufCnt) > 64 )
		{/* SN code overflow 1 prog page then error return for protect */
			SNDataBuf(1) = SNDataBuf(0) ; /* set sck wr SN 0=pass, 1=ng */
			SNDataBuf(2) = SKBInputFlag ; /* set sck enable/disable flag */

			if (GangCheckIfAllSocketError(SNDataBuf(0)) )
			{ /* checking if all sck error, then update disp */
				SysStatusCode = E_DeviceSerialCodeSetFail ; /* sending error type code */
				Send_Status_Code() ;
			}
			return ;
		}

	/////// If need change SN code to new Key Code, add Algo here! //////////////////////
	/// here JUST for one example algo, NEED save new key code back to SNDataBuf[8,9....] for upload to PC ///
//		for (TempCnt=0; TempCnt<SNBufCnt; TempCnt++)
//			SNDataBuf(8+TempCnt) = SNDataBuf(8+TempCnt) * 2 ;
//		.........
//          SNDataBuf(3) = 32 ; /* update new key code length */
	/////////////////////////////////////////////////////////////////////////////

	/////// read SN page data before write SN ///////////
		PtrData(GEnSelSckNum) = SNDataBuf(0) ; /* get write sck num, 1=enable check */
		Delay_10nsX(10) ;
		PtrData(SetCSPinCmd) = 0 ;
		Delay_10nsX(10) ;
		PtrData(OneDataCmd) = ReadDataCmd ; /* send cmd Byte */
		SPIRWDelay() ;
		SendCATAddr(DeviceAddress);

		for (TempCnt=0; TempCnt<64; TempCnt++)
		{ /* read back Device SN page data */
			PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let fpga read device data */
			SPIRWDelay() ;
			SNPageBuff[TempCnt] = PtrData(OneDataCmd) ; /* the second data is the true data */
		}
		PtrData(SetCSPinCmd) = 1 ;
		Delay_10nsX(10) ;

		for (TempCnt=0; TempCnt<SNBufCnt; TempCnt++) /* merging new SN data in buffer */
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
		PtrData(OneDataCmd) = PageProgCmd ; /* send cmd Byte */
		SPIRWDelay() ;
		SendCATAddr(DeviceAddress);

		for (TempCnt=0; TempCnt<64; TempCnt++)
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
		PtrData(OneDataCmd) = ReadDataCmd ; /* send cmd Byte */
		SPIRWDelay() ;
		SendCATAddr(DeviceAddress);

		for (TempCnt=0; TempCnt<64; TempCnt++)
		{ /* read back Device SN page data */
			PtrData(GDevChkDataLoInc0) = 0xFF ; /* dummy send to let fpga read device data */
			SPIRWDelay() ;
			SNBufCnt = PtrData(OneDataCmd) ; /* the second data is the true data */
			if (SNPageBuff[TempCnt] != SNBufCnt )
			{ /* if data check error then set flag & power off */
				PtrData(SetCSPinCmd) = 1 ;
				Delay_10nsX(10) ;
				SNDataBuf(1) = SNDataBuf(0) ; /* set sck wr SN 0=pass, 1=ng */
				SNDataBuf(2) = SKBInputFlag ; /* set sck enable/disable flag */

				if (GangCheckIfAllSocketError(SNDataBuf(0)) )
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




/**********************************************************\
		CAT25256 helper routines
\**********************************************************/

/*
 * Select all currently valid sockets and keep CS high before the next SPI command.
 * GangErrSckMask bits are excluded so failed sockets do not affect later checks.
 */
void SelectActiveSockets(void)
{
	PtrData(SetCSPinCmd) = 1 ;	/* Drive CS high to prevent floating during FPGA idle */
	PtrData(GSetAllErrSckMask) = (uShort)GangErrSckMask;
	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask);
	Delay_10nsX(3);
}

/*
 * Send CAT25256 16-bit byte address after READ/WRITE opcode.
 * Only A14:A0 are significant, so the address is masked to 0x0000-0x7FFF.
 */
void SendCATAddr(uInt32 AddrData)
{
	AddrData &= CATAddrMask;
	PtrData(OneDataCmd) = MSB(LSW(AddrData));
	SPIRWDelay();
	PtrData(OneDataCmd) = LSB(LSW(AddrData));
	SPIRWDelay();
}

/*
 * Read the single CAT25256 status register with RDSR(0x05).
 * Return value is the low byte; callers mask WIP/WEL/BP bits as needed.
 */
uShort CAT25256ReadStatus(void)
{
	stuShort TempData;

	PtrData(SetCSPinCmd) = 0;
	Delay_10nsX(10);
	PtrData(OneDataCmd) = ReadStatusCmd;
	SPIRWDelay();
	PtrData(GDevChkDataLoInc0) = 0xFF;
	SPIRWDelay();
	TempData.Byte.LoByte = PtrData(OneDataCmd);
	PtrData(SetCSPinCmd) = 1;
	Delay_10nsX(50);

	return TempData.Byte.LoByte;
}

/*
 * Poll RDSR on all active sockets until (status & StatusMask) equals ExpectData.
 * Returns FAIL only when every active socket has failed before the timeout exits.
 */
uChar CAT25256WaitStatus(uChar ExpectData, uChar StatusMask, uShort PollingTimeCnt, uShort DelayUs)
{
	uChar StatusData;

	if (!StatusMask)
		return SUCCESS;

	SelectActiveSockets();
	PtrData(GSetAllMaskDataLo) = StatusMask;
	while (1)
	{
		PtrData(SetCSPinCmd) = 1;
		Delay_10nsX(30);
		PtrData(SetCSPinCmd) = 0;
		Delay_10nsX(10);
		PtrData(OneDataCmd) = ReadStatusCmd;
		SPIRWDelay();
		PtrData(GClrPageChkStatus) = 0x0000;
		PtrData(GDevChkDataLoInc0) = ExpectData;
		SPIRWDelay();
		Delay_1us(10);
		Delay_10nsX(2);
		StatusData = PtrData(GChkAllSckStatus);
		PtrData(SetCSPinCmd) = 1;
		Delay_10nsX(10);

		if (!StatusData)
			break;
		else if (!(--PollingTimeCnt))
		{
			if (GangCheckIfAllSocketError(StatusData))
				return FAIL;
			break;
		}

		if (DelayUs)
			Delay_1us(DelayUs);
	}

	return SUCCESS;
}

/*
 * Send WREN(0x06), then verify WEL bit is set before any write/status operation.
 */
uChar CAT25256WriteEnable(void)
{
	SelectActiveSockets();
	Delay_10nsX(30);

	PtrData(SetCSPinCmd) = 0;
	Delay_10nsX(10);
	PtrData(OneDataCmd) = WriteEnableCmd;
	SPIRWDelay();
	Delay_1us(2);
	PtrData(SetCSPinCmd) = 1;
	Delay_10nsX(10);

	return CAT25256WaitStatus(StatusWELBit, StatusWELBit, 100, 20);
}

/*
 * Write selected status register protection bits with WRSR(0x01).
 * StatusMask limits the modified bits; ErrorCode is reported on write/verify failure.
 */
uChar CAT25256WriteStatus(uChar StatusData, uChar StatusMask, uChar ErrorCode)
{
	StatusData &= StatusMask;
	if (!StatusMask)
		return SUCCESS;

	if (!CAT25256WriteEnable())
	{
		SysStatusCode = ErrorCode;
		return FAIL;
	}

	PtrData(SetCSPinCmd) = 0;
	Delay_10nsX(10);
	PtrData(OneDataCmd) = WriteStatusCmd;
	SPIRWDelay();
	PtrData(OneDataCmd) = StatusData;
	SPIRWDelay();
	Delay_1us(2);
	PtrData(SetCSPinCmd) = 1;
	Delay_10nsX(30);

	if (!CAT25256WaitStatus(0x00, StatusWIPBit, 5000, 5))
	{
		SysStatusCode = ErrorCode;
		return FAIL;
	}

	if (!CAT25256WaitStatus(StatusData, StatusMask, 2000, 1000))
	{
		SysStatusCode = ErrorCode;
		return FAIL;
	}

	return SUCCESS;
}

/**********************************************************\
		Subroutine Function Define Here
\**********************************************************/
/*
 * Load per-device parameters from WJ_CAT25256.h tables and configure SPI timing.
 * This must run after power is stable and before read/program/verify operations.
 */
void GetDeviceParameterInfo(void)
{
	ProgPageSize = ChipPageSizeTable[fwBlkIndex];
	ChipSPIMaxClkDelay = ChipSPIMaxClkTable[fwBlkIndex];
	switch (ChipSPIMaxClkDelay) { /* set frequency by ChipSPIMaxClkDelay */
		case 16:
			PtrData(SetSPIDivider) = 0x0001;
			ChipSPIMaxClkDelay = 14; /* add write time */
			break;
		case 8:
			PtrData(SetSPIDivider) = 0x0002;
			ChipSPIMaxClkDelay = 6; /* add write time */
			break;
		case 5:
			PtrData(SetSPIDivider) = 0x0003;
			ChipSPIMaxClkDelay = 5;
			break;
		case 4:
			PtrData(SetSPIDivider) = 0x0003;
			ChipSPIMaxClkDelay = 4;
			break;
		default:
			break;
	}
	Delay_10nsX(3);
	ChipSPIMaxClkDelay = ((1000 * 8) / 25) / ChipSPIMaxClkDelay; /* formula gives 320/freq_MHz */
	if (ChipSPIMaxClkDelay < 150)
		ChipSPIMaxClkDelay = 150; /* FPGA always takes 96/66MHz ~=1.45us per SPI byte regardless of SCK */
	PrtRegMaskBit = ProtectRegisterTable[fwBlkIndex];
	PrtRegDefaultBit = ProtectRegisterDefaultTable[fwBlkIndex];
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
