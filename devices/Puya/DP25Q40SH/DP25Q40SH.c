/******************************************************************************\
	Programmer total 13 tasks driver setting:
POWER_ON_FUN							POWER_OFF_FUN
READ_FUN			PROGRAM_FUN		VERIFY_FUN
ERASE_FUN			BLANK_CHECK_FUN	ILLEGAL_CHECK_FUN
ID_CHECK_FUN		SECURE_FUN			CHECKSUM_FUN
PROTECT_FUN								UNPROTECT_FUN			
\*******************************************************************************/

#include "DP25Q40SH.h"
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
#include "h\Hspi.h"



/**************************************************************\
Notes: User define variable here,
          the area is UserVarityAreaMin ~ UserVarityAreaMin
\**************************************************************/
/************ IO Port define ****************/
uInt32 CS_Port ;
uInt32 WP_Port ;
uInt32 HOLD_Port ;
uInt32 RST_Port ;
uInt32 CS2_Port ;


/*********************************************************************/
typedef struct {
		uInt32 DirReg ; /* each pin use 2 bits, 00=In, 01=Out, 10=Fun, 11=res */
		uInt32 DataReg ;
		uInt32 PUDReg ; /* each pin use 2 bits, 00=Dis, 01=PullDown, 10=PullUP, 11=Not available */
		uInt32 BitReg ;
		}TDataPort ;


/************ MPU IO Port ReMap define ****************/
TDataPort CSPin ;
TDataPort WPPin ;
TDataPort HOLDPin ;
TDataPort RSTPin ;
TDataPort CS2Pin ;



/**************************************************************\
Notes: User define varity here,
          the area is UserVarityAreaMin ~ UserVarityAreaMin
\**************************************************************/
uShort PageDataBuf[256] ; /* Max Buffer Size be 256 word */
uChar SpiRxDataBuf[1024] ; /* Max Buffer Szie <=64K Bytes */
uShort BlankPageFlag ;
uShort BlockEndFlag ;
uInt32 DeviceAddress ;
uShort ProgPageSize ;
uShort ChipEraseTime ;
uChar ChipSPIMaxClkDelay ;
uChar PrtRegMaskBit ;
uChar EntEBACmd ;
uChar ExitEBACmd ;


/******** Control Command Macro Define **************/
#define	WriteEnableCmd		0x06
#define	WriteDisableCmd		0x04
#define	ReadStatusCmd		0x05
#define	WriteStatusCmd		0x01
#define	ReadDataCmd		    0x03
#define	FastReadDataCmd	    0x0B
#define	PageProgCmd		    0x02
#define	SectorEraseCmd		0xD8
#define	BulkEraseCmd		0xC7
#define	PwrDownCmd		    0xB9
#define	ReadEleSignCmd		0xAB
#define	ReadIDCmd			0x9F
#define   ReadStatus1Cmd        0X35
#define   ReadConfiguerRegister        0X15
 


#define	EntSecureOTP		0xB1
#define	ExitSecureOTP		0xC1
#define	ReadSecureLock		0x2B
#define	WriteSecureLock		0x2F
#define	EnSOOutputRY		0x70
#define	DisSOOutputRY		0x80
#define Enter4ByteModeCmd	0xB7
#define Exit4ByteModeCmd	0xE9
#define WPSELCmd			0x68


/* below two command jsut for 25L1605, 3205, 6405 */
#define	EntExtend8Kb		0xA5
#define	ExitExtend8Kb		0xB5


/******* Status Control Bits **********/
#define	StatusWIPBit			0x01
#define	StatusWELBit			0x02
#define	StatusProtectBits		0x1C
#define	StatusProtectBits128P	0x3C
#define	StatusWriteDisBit		0x80


/*** Control Bit Macro Definition, Refer to *.spc file setting ***/
#define	LockOn				0x01
#define	ProgramOn			0x02 /* bit x = 1 means On */
#define	VerifyOn			0x04
#define	CheckOn			    0x08
#define	ChecksumOn			0x10
#define	EraseExt8KOn		0x20
#define	SecRegWPSELBit		0x80
#define	SecuredOTPRead		0x01


/* MUST delay enough time let FPGA read 8 bit serial data & finish checking 4 socket data!! */
#define	SPIRWDelay()		(Delay_10nsX(ChipSPIMaxClkDelay)) /* 10ns scale, refer to sck speed!  */



/****************************************************************\
Notes: User define yoursel subroutine here

\****************************************************************/
void GetDeviceParameterInfo(void) ;
uChar ReadStatusData(uChar cmd) ;
uChar WriteStatusWELBit(void) ;
void SendControlCmd(uChar SendCmd) ;
uChar ReadLockRegister(void) ;
void Get1PageDataChkBlank(void) ;
void Enter4ByteMode(void) ;
void Exit4ByteMode(void) ;
void SetCSPinStatus(uChar PinStatus) ;
void SetWPPinStatus(uChar PinStatus) ;
void SetHOLDPinStatus(uChar PinStatus) ;
void SetRSTPinStatus(uChar PinStatus) ;
void SetCS2PinStatus(uChar PinStatus) ;
void WriteStatusData(uChar cmd, uChar data) ;
void Protect_Polling(void);



/****************************************************************\
	IO Port Define, CAN NOT change functin name!!! 
\****************************************************************/
void DmmIOPortListDefine(void) /* CAN NOT change functin name!!! */
{
PinPty PortSetting ;
/****************************************************************\
	for serial pin control can use below way to change Port type to Pin type,
	then can speed system. BUT ONLY for Digital/Z-Digital type pin. 
	Power type pin(VCC,VPP,VIO) CANNOT use this way.
\****************************************************************/
/* for XP1 system, CAN NOT use NCX define */
	PortSetting.all = DmmIOPortDefine[fwPmpIndex][0] ; /* each pin use 2 bits, 00=In, 01=Out, 10=Fun, 11=res */
	CSPin.DirReg = SDIP48_PortDirReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port dir reg addr */
	CSPin.DataReg = SDIP48_PortDataReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port data reg addr */
	CSPin.PUDReg = SDIP48_PortUpDnReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port pull-down reg addr */
	CS_Port = SDIP48_PortDataReMapTbl[(PortSetting.Byte.PinNum-1)*2+1] ; /* remap get port bit */
	CSPin.BitReg = (1<<CS_Port) ; /* remap get port bit */

	PortSetting.all = DmmIOPortDefine[fwPmpIndex][1] ; /* each pin use 2 bits, 00=In, 01=Out, 10=Fun, 11=res */
	WPPin.DirReg = SDIP48_PortDirReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port dir reg addr */
	WPPin.DataReg = SDIP48_PortDataReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port data reg addr */
	WPPin.PUDReg = SDIP48_PortUpDnReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port pull-down reg addr */
	WP_Port = SDIP48_PortDataReMapTbl[(PortSetting.Byte.PinNum-1)*2+1] ; /* remap get port bit */
	WPPin.BitReg = (1<<WP_Port) ; /* remap get port bit */

	PortSetting.all = DmmIOPortDefine[fwPmpIndex][2] ; /* each pin use 2 bits, 00=In, 01=Out, 10=Fun, 11=res */
	HOLDPin.DirReg = SDIP48_PortDirReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port dir reg addr */
	HOLDPin.DataReg = SDIP48_PortDataReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port data reg addr */
	HOLDPin.PUDReg = SDIP48_PortUpDnReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port pull-down reg addr */
	HOLD_Port = SDIP48_PortDataReMapTbl[(PortSetting.Byte.PinNum-1)*2+1] ; /* remap get port bit */
	HOLDPin.BitReg = (1<<HOLD_Port) ; /* remap get port bit */

	PortSetting.all = DmmIOPortDefine[fwPmpIndex][3] ; /* each pin use 2 bits, 00=In, 01=Out, 10=Fun, 11=res */
	RSTPin.DirReg = SDIP48_PortDirReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port dir reg addr */
	RSTPin.DataReg = SDIP48_PortDataReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port data reg addr */
	RSTPin.PUDReg = SDIP48_PortUpDnReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port pull-down reg addr */
	RST_Port = SDIP48_PortDataReMapTbl[(PortSetting.Byte.PinNum-1)*2+1] ; /* remap get port bit */
	RSTPin.BitReg = (1<<RST_Port) ; /* remap get port bit */

	PortSetting.all = DmmIOPortDefine[fwPmpIndex][4] ; /* each pin use 2 bits, 00=In, 01=Out, 10=Fun, 11=res */
	CS2Pin.DirReg = SDIP48_PortDirReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port dir reg addr */
	CS2Pin.DataReg = SDIP48_PortDataReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port data reg addr */
	CS2Pin.PUDReg = SDIP48_PortUpDnReMapTbl[(PortSetting.Byte.PinNum-1)*2] ; /* remap get port pull-down reg addr */
	CS2_Port = SDIP48_PortDataReMapTbl[(PortSetting.Byte.PinNum-1)*2+1] ; /* remap get port bit */
	CS2Pin.BitReg = (1<<CS2_Port) ; /* remap get port bit */

/* each pin use 2 bits, 00=In, 01=Out, 10=Fun, 11=res */
	PtrMcuReg(CSPin.DirReg) = PtrMcuReg(CSPin.DirReg) & (~(0x3<<(CS_Port*2))) |(OutDir<<(CS_Port*2)) ; /* set output */
	PtrMcuReg(WPPin.DirReg) = PtrMcuReg(WPPin.DirReg) & (~(0x3<<(WP_Port*2))) |(OutDir<<(WP_Port*2)) ; /* set output */
	PtrMcuReg(HOLDPin.DirReg) = PtrMcuReg(HOLDPin.DirReg) & (~(0x3<<(HOLD_Port*2))) |(OutDir<<(HOLD_Port*2)) ; /* set output */
	PtrMcuReg(RSTPin.DirReg) = PtrMcuReg(RSTPin.DirReg) & (~(0x3<<(RST_Port*2))) |(OutDir<<(RST_Port*2)) ; /* set output */
	PtrMcuReg(CS2Pin.DirReg) = PtrMcuReg(CS2Pin.DirReg) & (~(0x3<<(CS2_Port*2))) |(OutDir<<(CS2_Port*2)) ; /* set output */
}





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
	
	GetDeviceParameterInfo() ;
	SpiGPIOPortSet(ChipSPIMaxClkTable[fwSpcIndex]); /* initial Spi Port setting */
	SelectSPIPortType(SpiPortTypeDefine[fwPmpIndex]) ;

	SetCSPinStatus(1) ;
	SetWPPinStatus(1) ;
	SetHOLDPinStatus(1) ;
	SetRSTPinStatus(1) ;
	SetCS2PinStatus(1) ;
	Delay_1ms(5) ; /* delay for stable volte */

	GangRiseAllSocketVccIccPin() ; /* open VCC/VPP/VPE/VPS/VPIO pin driving */
	Delay_1ms(100) ; /* delay for stable volte */
	return SUCCESS ;
}




/********************* Power On ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar PowerOffFun(void)
{
	SelectSPIPortType(Dis_SelSPI) ;

	Fall_VpioIpioAll() ;
	Fall_VppIppAll() ;
	Fall_VihAll() ;
	Fall_VccIccAll() ;
	Delay_1ms(100) ; /* delay for stable volte */
	return SUCCESS ;
}




/****************** Read Subroutine **************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar ReadFun_4Byte(void)
{
stuShort DevWordData ;
uShort TempCnt ;

	DeviceAddress = (GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;

	if( IfSpecialBitBlk() )
	{	
		
	/********* lock register control *********/
		SRAMPointer = 0x00 ; /* to get sram EFA control status */
	     // DevWordData.all = ReadStatusData();
		 DevWordData.all =  ReadStatusData(ReadStatusCmd);
		  	SaveSRAMByteData( DevWordData.all) ; /* save protect register */
			
		  SRAMPointer = 0x01 ; /* to get sram EFA control status */
	      //DevWordData.all = ReadStatusData();
		  DevWordData.all =  ReadStatusData(ReadStatus1Cmd);
		  	SaveSRAMByteData( DevWordData.all) ; /* save protect register */
			
		  SRAMPointer = 0x02 ; /* to get sram EFA control status */
	      //DevWordData.all = ReadStatusData();
	      DevWordData.all =  ReadStatusData(ReadConfiguerRegister);
		  	SaveSRAMByteData( DevWordData.all) ; /* save protect register */
			

	       SRAMPointer = 0x03 ; /* to get sram EFA control status */	
		  // DevWordData.all = ReadStatusData();
	     	SaveSRAMByteData( DevWordData.all) ; /* save protect register */
	
		return SUCCESS ;
	}
	
	else
	{ /* Read main flash */
		//Enter4ByteMode() ;
		while(1)
		{
			SetCSPinStatus(0) ;
			Delay_10nsX(10) ;
			ArmSpi_TxReg() = ReadDataCmd ; /* send cmd Byte */
			SPIRWDelay() ;
			//ArmSpi_TxReg() = MSB(MSW(DeviceAddress)) ; /* send HiHi Addr */
			//SPIRWDelay() ;
			ArmSpi_TxReg() = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
			SPIRWDelay() ;
			ArmSpi_TxReg() = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
			SPIRWDelay() ;
			ArmSpi_TxReg() = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
			SPIRWDelay() ;

			SetArmSpi_RxDataInt(MaxPktByteSize, SpiRxDataBuf) ; /* set Arm Spi to Rx data from flash by Int */
			while( (!SpiRxIntEnd) && (SpiIntWaitTime--) ) ; /* wait Rx Int & MAX 1500ms for Dead Lock */
			SetCSPinStatus(1) ;
			Delay_10nsX(10) ;

			for(TempCnt=0; TempCnt<MaxPktByteSize; TempCnt+=2)
			{
				DevWordData.Byte.LoByte = *RxBufAddr++ ; /* the second data is the true data */
				DevWordData.Byte.HiByte = *RxBufAddr++ ; /* the second data is the true data */
				ReadDataBuf[(ReadBufCnt+TempCnt)/2] = DevWordData.all ;
			}

			ReadBufCnt += MaxPktByteSize ;
			DevCurrBlkLen.all -= MaxPktByteSize ;
			if( SaveChkBlkLenEnd() ) /* checking block length end */
			{
				Exit4ByteMode() ;
				return SUCCESS ;
			}

			DeviceAddress += MaxPktByteSize ;
		}
	}
	return SUCCESS ;
}

/****************** Program Subroutine **********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar ProgramFun_4Byte(void)
{
uChar StatusData ;
uShort RamWordData, PageTempCnt, PollingTimeCnt ;

	DeviceAddress = (GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{
		if( !ProtectFun() )
		{
			SysStatusCode = E_ProtectFail ; /* sending error type code */
			return FAIL ;
		}
			
	/********* lock register control *********/
		SRAMPointer = 0x01 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ; /* read secure control status */
		if( !(RamWordData & ProgramOn) )
			return SUCCESS ; /* if donot need check then return */

		if( RamWordData & LockOn )
			SendControlCmd(WriteSecureLock) ;
		Delay_1ms(50) ;

		/* WPSEL enable */
		SRAMPointer = 0x02;
		RamWordData = GetSRAMByteData();
		if(RamWordData & SecRegWPSELBit)
		{
			if( !WriteStatusWELBit() )
			{
				SysStatusCode = E_UserPrtRegProgramFail ; /* sending error type code */
				return FAIL ;
			}
			SendControlCmd(WPSELCmd);
			Delay_1ms(50);

			PollingTimeCnt = 6000 ;
			while(1)
			{ /* sending writing cmd to set WEL bit */
				StatusData = ReadLockRegister() & SecRegWPSELBit ; /* only check bit0, checking all socket & get check result */
				if( !StatusData ) /* if all socket WIP bit = 0 means program ok */
					break ;
				else if( !(--PollingTimeCnt) )
				{ /* have some socket check error */
					SysStatusCode = E_UserPrtRegProgramFail ;
					return FAIL ;
				}
				Delay_1us(20) ;
			}
		}		
		return SUCCESS ;
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{ /* factory register & user register */
		SRAMPointer = 0x01 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & ProgramOn) )
		{
			CFCardReadFillSector() ; /* if have other data then need write finish one sector */
			return SUCCESS ;
		}

		//Enter4ByteMode() ;
		SendControlCmd(EntEBACmd) ; /* enter extended block */
		while(1)
		{
			if( !WriteStatusWELBit() )
			{
				SysStatusCode = E_ProgramNoDataFail ; /* sending error type code */
				return FAIL ;
			}

			SetCSPinStatus(0) ; /* set chip CS port to Vil */
			Delay_10nsX(10) ;
			ArmSpi_TxReg() = PageProgCmd ; /* send cmd Byte */
			SPIRWDelay() ;
			//ArmSpi_TxReg() = MSB(MSW(DeviceAddress)) ; /* send HiHi Addr */
			//SPIRWDelay() ;
			ArmSpi_TxReg() = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
			SPIRWDelay() ;
			ArmSpi_TxReg() = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
			SPIRWDelay() ;
			ArmSpi_TxReg() = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
			SPIRWDelay() ;

			for(PageTempCnt=0; PageTempCnt<ProgPageSize; PageTempCnt+=2) /* get page program counter */
			{
				RamWordData = CFCardReadWordData() ;
				ArmSpi_TxReg() = LSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
				SPIRWDelay() ;
				ArmSpi_TxReg() = MSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
				SPIRWDelay() ;
			}
			SetCSPinStatus(1) ; /* set chip CS port to Vih */
			Delay_10nsX(10) ;

			PollingTimeCnt = 3000 ;
			while(1)
			{
				StatusData = ReadStatusData(ReadStatusCmd)& StatusWIPBit ; /* only check bit0, checking all socket & get check result */
				if( !StatusData ) /* if all socket WIP bit = 0 means program ok */
					break ;
				else if( !(--PollingTimeCnt) )
				{ /* have some socket check error */
					SysStatusCode = E_ProgramNoDataFail ; /* sending error type code */
					return FAIL ;
				}
				Delay_1us(5) ;
			}

			DeviceAddress += ProgPageSize ;
			DramBufCnt -= ProgPageSize ;
			DevCurrBlkLen.all -= ProgPageSize ;
			if( GetChkBlkLenEnd() )
			{
				SendControlCmd(ExitEBACmd) ; /* exit extended block */
				Exit4ByteMode() ;
				return SUCCESS ;
			}
		}
	}
	else
	{ /* Program main flash */
		SendControlCmd(0x98);   /* gang block unlock */
	//Enter4ByteMode() ;

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

				SetCSPinStatus(0) ; /* set chip CS port to Vil */
				Delay_10nsX(10) ;
				ArmSpi_TxReg() = PageProgCmd ; /* send cmd Byte */
				SPIRWDelay() ;
				//ArmSpi_TxReg() = MSB(MSW(DeviceAddress)) ; /* send HiHi Addr */
				//SPIRWDelay() ;
				ArmSpi_TxReg() = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
				SPIRWDelay() ;
				ArmSpi_TxReg() = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
				SPIRWDelay() ;
				ArmSpi_TxReg() = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
				SPIRWDelay() ;
#if 1
				SetArmSpi_TxDataInt(ProgPageSize, (U8*)PageDataBuf) ; /* set Arm Spi to Rx data from flash by Int */
				while( (!SpiTxIntEnd) && (SpiIntWaitTime--) ) ; /* wait Tx Int & MAX 1500ms for Dead Lock */
				while ( ChkIfTxFifoEmpty() && (SpiIntWaitTime--)); /* check if Tx Fifo counter empty */
				while ((!ChkIfTxDone()) && (SpiIntWaitTime--)); /* check if Tx Flag done */
#else
				for(PageTempCnt=0; PageTempCnt<ProgPageSize; PageTempCnt+=2) /* get page program counter */
				{
					RamWordData = PageDataBuf[PageTempCnt/2] ;
					ArmSpi_TxReg() = LSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
					SPIRWDelay() ;
					ArmSpi_TxReg() = MSB(RamWordData) ; /* send checking data to let fpga to read all socket data */
					SPIRWDelay() ;
				}
#endif
				SetCSPinStatus(1) ; /* set chip CS port to Vih */
				Delay_10nsX(10) ;
				Delay_1us(10) ; /* MUST delay some time to stable 256M,512M,1G device */
				DeviceAddress += ProgPageSize ;
				DramBufCnt -= ProgPageSize ;
				DevCurrBlkLen.all -= ProgPageSize ;
				BlockEndFlag = GetChkBlkLenEnd() ; /* checking block length end */
				if( !BlockEndFlag ) /* if not the block end then goon check next page data */
					Get1PageDataChkBlank() ;

				PollingTimeCnt = 3000 ;
				while(1)
				{
					StatusData = ReadStatusData(ReadStatusCmd)& StatusWIPBit ; /* only check bit0, checking all socket & get check result */
					if( !StatusData ) /* if all socket WIP bit = 0 means program ok */
						break ;
					else if( !(--PollingTimeCnt) )
					{ /* have some socket check error */
						SysStatusCode = E_ProgramNoDataFail ; /* sending error type code */
						return FAIL ;
					}
					Delay_1us(5) ;
				}
			}

			if( BlockEndFlag ) /* checking block length end */
			{
		                Exit4ByteMode() ;
		                return SUCCESS ;
			}
		}
	}
	return SUCCESS ;
}

/****************** Verify Subroutine ************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar VerifyFun_4Byte(void)
{
uChar DevByteData ;
uShort RamWordData, TempCnt ;
stuShort DevWordDataDMA ;

	DeviceAddress = (GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{	/* if special bit block then read boot lock status */
		RamWordData = GetSRAMByteData() & PrtRegMaskBit ;
		DevByteData = ReadStatusData(ReadStatusCmd)& PrtRegMaskBit ; /* checking all socket & get check result */
		if( RamWordData != DevByteData )
		{ /* have some socket check error */
			SysStatusCode = E_ProtectStatusFail ; /* sending error type code */
			return FAIL ;
		}
			
	/********* lock register control *********/
		SRAMPointer = 0x01 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ; /* read secure control status */
		if( !(RamWordData & VerifyOn) )
			return SUCCESS ; /* if donot need check then return */

		RamWordData &= LockOn ; /* mask lock bit */
		RamWordData = RamWordData<<1 ; /* move to user lock bit 1 */
		DevByteData = ReadLockRegister() & (LockOn<<1) ; /* checking all socket & get check result */
		if( RamWordData != DevByteData )
		{ /* have some socket check error */
			SysStatusCode = E_ProtectStatusFail ; /* sending error type code */
			return FAIL ;
		}
			
	    /* Verify WPSEL bit */
		SRAMPointer = 0x02 ;
		RamWordData = GetSRAMByteData() & SecRegWPSELBit ;
		DevByteData = ReadLockRegister() & SecRegWPSELBit ;
		if( RamWordData != DevByteData )
		{ /* have some socket check error */
			SysStatusCode = E_ProtectStatusFail ; /* sending error type code */
			return FAIL ;
		}		
		return SUCCESS ;
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{ /* factory register & user register */
		SRAMPointer = 0x01 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & VerifyOn) )
		{
			CFCardReadFillSector() ; /* if have other data then need write finish one sector */
			return SUCCESS ;
		}

		//Enter4ByteMode() ;
		SendControlCmd(EntEBACmd) ; /* enter extended block */

		SetCSPinStatus(0) ;
		Delay_10nsX(10) ;
		ArmSpi_TxReg() = ReadDataCmd ; /* send cmd Byte */
		SPIRWDelay() ;
		//ArmSpi_TxReg() = MSB(MSW(DeviceAddress)) ; /* send HiHi Addr */
		//SPIRWDelay() ;
		ArmSpi_TxReg() = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
		SPIRWDelay() ;
		ArmSpi_TxReg() = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
		SPIRWDelay() ;
		ArmSpi_TxReg() = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
		SPIRWDelay() ;

		SetArmSpi_RxDataInt(256, SpiRxDataBuf) ; /* set Arm Spi to Rx data from flash by Int */
		while( (!SpiRxIntEnd) && (SpiIntWaitTime--) ) ; /* wait Rx Int & MAX 1500ms for Dead Lock */
		SetCSPinStatus(1) ;
		Delay_10nsX(10) ;

		while(1)
		{
			RamWordData = CFCardReadWordData() ;
			DevWordDataDMA.Byte.LoByte = *RxBufAddr++ ; /* the second data is the true data */
			DevWordDataDMA.Byte.HiByte = *RxBufAddr++ ; /* the second data is the true data */
			if( RamWordData != DevWordDataDMA.all )
			{ /* have some socket check error */
				SysStatusCode = E_HiddenROMVerifyFail ; /* sending error type code */
				return FAIL ;
			}

			DramBufCnt -= 2 ;
			DevCurrBlkLen.all -= 2 ;
			if( GetChkBlkLenEnd() ) /* checking block length end */
			{
				SendControlCmd(ExitEBACmd) ; /* exit extended block */
				Exit4ByteMode() ;
				return SUCCESS ;
			}
		}
	}
	else
	{ /* main flash */
	//Enter4ByteMode() ;
		while(1)
		{
			SetCSPinStatus(0) ;
			Delay_10nsX(10) ;
			ArmSpi_TxReg() = ReadDataCmd ; /* send cmd Byte */
			SPIRWDelay() ;
			//ArmSpi_TxReg() = MSB(MSW(DeviceAddress)) ; /* send HiHi Addr */
			//SPIRWDelay() ;
			ArmSpi_TxReg() = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
			SPIRWDelay() ;
			ArmSpi_TxReg() = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
			SPIRWDelay() ;
			ArmSpi_TxReg() = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
			SPIRWDelay() ;

			SetArmSpi_RxDataInt(MaxPktByteSize, SpiRxDataBuf) ; /* set Arm Spi to Rx data from flash by Int */

			 /* use Rx data time to get CF card data to short time */
			for(TempCnt=0; TempCnt<MaxPktByteSize; TempCnt+=2)
			{
				PageDataBuf[TempCnt/2] = CFCardReadWordData() ; /* send data to fpga & read all socket data */
			}

			while( (!SpiRxIntEnd) && (SpiIntWaitTime--) ) ; /* wait Rx Int & MAX 1500ms for Dead Lock */
			SetCSPinStatus(1) ;
			Delay_10nsX(10) ;

			for(TempCnt=0; TempCnt<MaxPktByteSize; TempCnt+=2)
			{
				DevWordDataDMA.Byte.LoByte = *RxBufAddr++ ;
				DevWordDataDMA.Byte.HiByte = *RxBufAddr++ ;
				if( PageDataBuf[TempCnt/2] != DevWordDataDMA.all )
				{ /* have some socket check error */
					SysStatusCode = E_VerifyFail ; /* sending error type code */
					return FAIL ;
				}
			}

			DramBufCnt -= MaxPktByteSize ;
			DevCurrBlkLen.all -= MaxPktByteSize ;
			if( GetChkBlkLenEnd() ) /* checking block length end */
			{
				Exit4ByteMode() ;
				return SUCCESS ;
			}

			DeviceAddress += MaxPktByteSize ;
		}
	}
	return SUCCESS ;
}



/******************* Erase Subroutine ***********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar EraseFun(void)
{
uChar StatusData ;
uShort PollingTimeCnt ;

	if( IfFirstTimeEntry() )
	{
		if( !UnProtectFun() )
		{
			SysStatusCode = E_EraseFail ; /* sending error type code */
			return FAIL ;
		}
		if( !WriteStatusWELBit() )
		{
			SysStatusCode = E_EraseFail ; /* sending error type code */
			return FAIL ;
		}
		

		SetCSPinStatus(0) ; /* set chip CS port to Vil */
		Delay_10nsX(10) ;
		ArmSpi_TxReg() = 0x60; /* send cmd Byte */
		SPIRWDelay() ;
		Delay_10nsX(10) ; /* ensure fpga send finish data before CS pin rise high */
		SetCSPinStatus(1) ; /* set chip CS port to Vih */
		Delay_1ms(100) ;

             SetCSPinStatus(0) ; /* set chip CS port to Vil */
		Delay_10nsX(10) ;
		ArmSpi_TxReg() = 0xC7; /* send cmd Byte */
		SPIRWDelay() ;
		Delay_10nsX(10) ; /* ensure fpga send finish data before CS pin rise high */
		SetCSPinStatus(1) ; /* set chip CS port to Vih */
		Delay_1ms(100) ;
		
		PollingTimeCnt = (ChipEraseTime * 100) ; /* divide to 10ms scale for one loop check */
		#if 0
		  ReadStatusData(ReadStatusCmd);
	 Delay_1ms(100);
	 ReadStatusData(ReadStatus1Cmd);
	 Delay_1ms(100);
	 ReadStatusData(ReadConfiguerRegister);
	 Delay_1ms(100);
	 #endif
	 #if 1
		while(1)
		{ /* 1M bit typical erase time be 2 seconds */
			StatusData = ReadStatusData(ReadStatusCmd)& StatusWIPBit ; /* only check bit0, checking all socket & get check result */
			if( !StatusData ) /* if all socket WIP bit = 0 means program ok */
				break ;
			else if( !(--PollingTimeCnt) )
			{
				SysStatusCode = E_EraseFail ; /* sending error type code */
				return FAIL ;
			}
			Delay_1ms(15) ;
		}
		Delay_1ms(5000);
	#endif
	}

	return SUCCESS ;
}



/****************** Blank Check ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar BlankCheckFun_4Byte(void)
{
uChar DevByteData, RamWordData ;
uShort TempCnt ;

	DeviceAddress = (GetBlkChipStartAddrHi()<<16) + GetBlkChipStartAddrLo() ;
	if( IfSpecialBitBlk() )
	{	/* if special bit block then read boot lock status */
		RamWordData = (uChar)(DevBlkVirgin & PrtRegMaskBit) ;
		DevByteData = ReadStatusData(ReadStatusCmd) & PrtRegMaskBit ; /* checking all socket & get check result */
#if 0
		if( RamWordData != DevByteData )
		{ /* have some socket check error */
			SysStatusCode = E_ProtectStatusFail ; /* sending error type code */
			return FAIL ;
		}
		#endif
			
	/********* lock register control *********/
		SRAMPointer = 0x01 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ; /* read secure control status */
		if( !(RamWordData & CheckOn) )
			return SUCCESS ; /* if donot need check then return */

		RamWordData = (uChar)(DevBlkVirgin & LockOn) ; /* mask lock bit */
		RamWordData = RamWordData<<1 ; /* move to user lock bit 1 */
		DevByteData = ReadLockRegister() & (LockOn<<1) ; /* checking all socket & get check result */

		#if 0
		if( RamWordData != DevByteData )
		{ /* have some socket check error */
			SysStatusCode = E_ProtectStatusFail ; /* sending error type code */
			return FAIL ;
		}
		#endif
			
		/* Blankcheck WPSEL bit */
		SRAMPointer = 0x02 ;
		RamWordData = (uChar)(DevBlkVirgin & SecRegWPSELBit) ; /* mask other bit */
		DevByteData = ReadLockRegister() & SecRegWPSELBit ;
#if 0
		if( RamWordData != DevByteData )
		{ /* have some socket check error */
			SysStatusCode = E_ProtectStatusFail ; /* sending error type code */
			return FAIL ;
		}
		#endif
		return SUCCESS ;
	}
	else if( DevBlkAttri.Bit.bBlkAttri_Res15 )
	{ /* factory register & user register */
		SRAMPointer = 0x01 ; /* to get sram EFA control status */
		RamWordData = GetSRAMByteData() ;
		if( !(RamWordData & CheckOn) )
			return SUCCESS ;
			
		//Enter4ByteMode() ;
		SendControlCmd(EntEBACmd) ; /* enter extended block */

		SetCSPinStatus(0) ;
		Delay_10nsX(10) ;
		ArmSpi_TxReg() = ReadDataCmd ; /* send cmd Byte */
		SPIRWDelay() ;
		//ArmSpi_TxReg() = MSB(MSW(DeviceAddress)) ; /* send HiHi Addr */
		//SPIRWDelay() ;
		ArmSpi_TxReg() = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
		SPIRWDelay() ;
		ArmSpi_TxReg() = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
		SPIRWDelay() ;
		ArmSpi_TxReg() = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
		SPIRWDelay() ;

		SetArmSpi_RxDataInt(256, SpiRxDataBuf) ; /* set Arm Spi to Rx data from flash by Int */
		while( (!SpiRxIntEnd) && (SpiIntWaitTime--) ) ; /* wait Rx Int & MAX 1500ms for Dead Lock */
		SetCSPinStatus(1) ;
		Delay_10nsX(10) ;

		while(1)
		{
			DevByteData = *RxBufAddr++ ; /* the second data is the true data */
			if( DevByteData != DevBlkVirgin )
			{ /* have some socket check error */
				SysStatusCode = E_HiddenROMBlankFail ; /* sending error type code */
				return FAIL ;
			}

			DevCurrBlkLen.all -- ;
			if( !DevCurrBlkLen.all ) /* checking block length end */
			{
				SendControlCmd(ExitEBACmd) ; /* exit extended block */
				Exit4ByteMode() ;
				return SUCCESS ;
			}
		}
	}
	else
	{ /* main flash */
	//	Enter4ByteMode() ;
		while(1)
		{
			SetCSPinStatus(0) ;
			Delay_10nsX(10) ;
			ArmSpi_TxReg() = ReadDataCmd ; /* send cmd Byte */
			SPIRWDelay() ;
			//ArmSpi_TxReg() = MSB(MSW(DeviceAddress)) ; /* send HiHi Addr */
			//SPIRWDelay() ;
			ArmSpi_TxReg() = LSB(MSW(DeviceAddress)) ; /* send MiHi Addr */
			SPIRWDelay() ;
			ArmSpi_TxReg() = MSB(LSW(DeviceAddress)) ; /* send MiLo Addr */
			SPIRWDelay() ;
			ArmSpi_TxReg() = LSB(LSW(DeviceAddress)) ; /* send LoLo Addr */
			SPIRWDelay() ;

			SetArmSpi_RxDataInt(MaxPktByteSize, SpiRxDataBuf) ; /* set Arm Spi to Rx data from flash by Int */
			while( (!SpiRxIntEnd) && (SpiIntWaitTime--) ) ; /* wait Rx Int & MAX 1500ms for Dead Lock */
			SetCSPinStatus(1) ;
			Delay_10nsX(10) ;

			for(TempCnt=0; TempCnt<MaxPktByteSize; TempCnt+=1)
			{
				DevByteData = *RxBufAddr++ ; /* the second data is the true data */
				if( DevByteData != DevBlkVirgin )
				{ /* have some socket check error */
					SysStatusCode = E_BlankFail ; /* sending error type code */
					return FAIL ;
				}
			}

			DevCurrBlkLen.all -= MaxPktByteSize ;
			if( !DevCurrBlkLen.all )
			{
				Exit4ByteMode() ;
				return SUCCESS ;
			}

			DeviceAddress += MaxPktByteSize ;
		}
	}
	return SUCCESS ;
}


/******************* ID Check ******************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar IDCheckFun(void) 
{
uChar MftID, DevIDHi, DevIDLo;

	SetCSPinStatus(0) ;
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = ReadIDCmd ; /* send cmd Byte */
	SPIRWDelay() ;

	SetArmSpi_RxDataInt(4, SpiRxDataBuf) ; /* set Arm Spi to Rx data from flash by Int */
	while( (!SpiRxIntEnd) && (SpiIntWaitTime--) ) ; /* wait Rx Int & MAX 1500ms for Dead Lock */
	SetCSPinStatus(1) ;
	Delay_10nsX(10) ;

	MftID = *RxBufAddr++ ; /* send checking data to let fpga to read all socket data */
	if( MftID != (uChar)ManufactID() )
	{ /* if all socket error then cancle function */
		ManufactErrID = MftID ;
		DeviceErrIDHi = 0x0000 ;
		DeviceErrIDLo = 0x0000 ;
		SysStatusCode = E_AutoIDFail ; /* sending error type code */
		return FAIL ;
	}

	DevIDHi = *RxBufAddr++ ; /* send checking data to let fpga to read all socket data */

	#if 0
	if( DevIDHi != MSB(DeviceIDLo()) )
	{ /* if all socket error then cancle function */
		ManufactErrID = MftID ;
		DeviceErrIDHi = DevIDHi ;
		DeviceErrIDLo = 0x0000 ;
		SysStatusCode = E_AutoIDFail ; /* sending error type code */
		return FAIL ;
	}

	DevIDLo = *RxBufAddr++ ;
	if( DevIDLo != LSB(DeviceIDLo()) )
	{ /* if all socket error then cancle function */
		ManufactErrID = MftID ;
		DeviceErrIDHi = DevIDHi ;
		DeviceErrIDLo = DevIDLo ;
		SysStatusCode = E_AutoIDFail ; /* sending error type code */
		return FAIL ;
	}
	#endif
	return SUCCESS ;
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
		SRAMPointer = 0x01 ; /* to get sram EFA control status */
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
uChar RamWordData, PollingTimeCnt, StatusData ;

#if 0
	if( !WriteStatusWELBit() )
	{
		SysStatusCode = E_ProtectFail ; /* sending error type code */
		return FAIL ;
	}
#endif



	SetCSPinStatus(0) ; /* set chip CS port to Vil */
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = WriteEnableCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	SetCSPinStatus(1) ; /* set chip CS port to Vih */
	Delay_10nsX(10) ;


#if 1
	SRAMPointer = 0x00 ;
	RamWordData = GetSRAMByteData() ;
	WriteStatusData(WriteStatusCmd, RamWordData);
	Delay_1ms(500);
#endif	




	SetCSPinStatus(0) ; /* set chip CS port to Vil */
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = WriteEnableCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	SetCSPinStatus(1) ; /* set chip CS port to Vih */
	Delay_10nsX(10) ;

	SRAMPointer = 0x01 ;
	RamWordData = GetSRAMByteData() ;
	WriteStatusData(0x31, RamWordData);
	Delay_1ms(500);
	

	SetCSPinStatus(0) ; /* set chip CS port to Vil */
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = WriteEnableCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	SetCSPinStatus(1) ; /* set chip CS port to Vih */
	Delay_10nsX(10) ;
	
#if 1	
	SRAMPointer = 0x02 ;
	RamWordData = GetSRAMByteData() ;
	WriteStatusData(0x11, RamWordData);
	Delay_1ms(500);
#endif


#if 1	
       ReadStatusData(ReadStatusCmd);
	 Delay_1ms(100);
	 ReadStatusData(ReadStatus1Cmd);
	 Delay_1ms(100);
	 ReadStatusData(ReadConfiguerRegister);
	 Delay_1ms(100);
#endif


#if 0
	PollingTimeCnt = 200 ;
	while(1)
	{
		StatusData = ReadStatusData(ReadStatusCmd) ; /* only check bit0, checking all socket & get check result */
		if( (StatusData & 0x01)== 0 ) /* if all socket WIP bit = 0 means program ok */
			break ;
			else if( !(--PollingTimeCnt) )
			{ /* have some socket check error */
				SysStatusCode = E_ProtectFail ; /* sending error type code */
				return FAIL ;
			}
			Delay_1ms(1) ;
	}
#endif



	#if 0
	PollingTimeCnt = 200 ;
	while(1)
	{
		StatusData = ReadStatusData(ReadStatusCmd) & PrtRegMaskBit ; /* only check bit0, checking all socket & get check result */
		if( RamWordData == StatusData ) /* if all socket WIP bit = 0 means program ok */
			break ;
			else if( !(--PollingTimeCnt) )
			{ /* have some socket check error */
				SysStatusCode = E_ProtectFail ; /* sending error type code */
				return FAIL ;
			}
			Delay_1ms(1) ;
	}
#endif
	return SUCCESS ;
}



/******************* Unprotect Subroutine ********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar UnProtectFun(void)
{
uChar PollingTimeCnt, StatusData ;

	if( !WriteStatusWELBit() )
	{
		SysStatusCode = E_UnprotectFail ; /* sending error type code */
		return FAIL ;
	}

	#if 0
	SetCSPinStatus(0) ; /* set chip CS port to Vil */
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = WriteStatusCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	ArmSpi_TxReg() = 0x00 ;
	SPIRWDelay() ;
	Delay_10nsX(10) ; /* ensure fpga send finish data before CS pin rise high */
	SetCSPinStatus(1) ; /* set chip CS port to Vih */
	Delay_1ms(100) ; /* MUST delay some time before checking */
	#endif
	SetCSPinStatus(0) ; /* set chip CS port to Vil */
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = WriteEnableCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	SetCSPinStatus(1) ; /* set chip CS port to Vih */
	Delay_10nsX(10) ;
	
       WriteStatusData(WriteStatusCmd, 0);
	   
	SetCSPinStatus(0) ; /* set chip CS port to Vil */
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = 0x04 ; /* send cmd Byte */
	SPIRWDelay() ;
	SetCSPinStatus(1) ; /* set chip CS port to Vih */
	Delay_10nsX(10) ;



	SetCSPinStatus(0) ; /* set chip CS port to Vil */
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = WriteEnableCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	SetCSPinStatus(1) ; /* set chip CS port to Vih */
	Delay_10nsX(10) ;
	
	WriteStatusData(0x31, 0);
	
	SetCSPinStatus(0) ; /* set chip CS port to Vil */
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = 0x04 ; /* send cmd Byte */
	SPIRWDelay() ;
	SetCSPinStatus(1) ; /* set chip CS port to Vih */
	Delay_10nsX(10) ;


	SetCSPinStatus(0) ; /* set chip CS port to Vil */
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = WriteEnableCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	SetCSPinStatus(1) ; /* set chip CS port to Vih */
	Delay_10nsX(10) ;

	
	WriteStatusData(0x11, 0);

	
	SetCSPinStatus(0) ; /* set chip CS port to Vil */
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = 0x04 ; /* send cmd Byte */
	SPIRWDelay() ;
	SetCSPinStatus(1) ; /* set chip CS port to Vih */
	Delay_10nsX(10) ;
	Delay_1ms(500);
	PtrData(GSetAllMaskDataLo) = StatusWIPBit ;  /* only check bit0, send byte checking mask data to FPGA */
	PollingTimeCnt = 200 ;
	while(1)
	{
		StatusData = ReadStatusData(ReadStatusCmd) & StatusWIPBit ; /* only check bit0, checking all socket & get check result */
		if( !StatusData ) /* if all socket WIP bit = 0 means program ok */
			break ;
		else if( !(--PollingTimeCnt) )
		{ /* have some socket check error */
			SysStatusCode = E_UnprotectFail ; /* sending error type code */
			return FAIL ;
		}
		Delay_1ms(1) ;
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
	ChipSPIMaxClkDelay = ChipSPIMaxClkTable[fwSpcIndex] ;
	ChipSPIMaxClkDelay = (ChipSPIMaxClkDelay+3)*(1000*8)/(25)/(33) ;  /* Arm SPI Delay Time = (1/xxM)*8Bit/(10ns*2.5 Scale), + 3 offset scales */
	PrtRegMaskBit = ProtectRegisterTable[fwBlkIndex] ;
	EntEBACmd = EntSecureOTP ;
	ExitEBACmd = ExitSecureOTP ;

	DmmIOPortListDefine() ;
}


/******************************************************/
uChar ReadStatusData(uChar cmd )
{
	SetCSPinStatus(0) ;
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = cmd ; /* send cmd Byte */
	SPIRWDelay() ;

	SetArmSpi_RxDataInt(4, SpiRxDataBuf) ; /* set Arm Spi to Rx data from flash by Int */
	while( (!SpiRxIntEnd) && (SpiIntWaitTime--) ) ; /* wait Rx Int & MAX 1500ms for Dead Lock */
	SetCSPinStatus(1) ;
	Delay_10nsX(10) ;
	return (*RxBufAddr) ;
}

void WriteStatusData(uChar cmd , uChar data) 
{
		SetCSPinStatus(0) ; /* set chip CS port to Vil */
		Delay_10nsX(10) ;
		ArmSpi_TxReg() = cmd ; /* send cmd Byte */
		SPIRWDelay() ;
		ArmSpi_TxReg() = data ;
		SPIRWDelay() ;
		Delay_10nsX(10) ; /* ensure fpga send finish data before CS pin rise high */
		SetCSPinStatus(1) ; /* set chip CS port to Vih */
		Delay_1ms(100) ; /* MUST delay some time before checking */
}


void Protect_Polling(void)
{
	uChar PollingTimeCnt;

}

/***************************************************************/
uChar ReadLockRegister(void)
{
	SetCSPinStatus(0) ;
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = ReadSecureLock ; /* send cmd Byte */
	SPIRWDelay() ;

	SetArmSpi_RxDataInt(4, SpiRxDataBuf) ; /* set Arm Spi to Rx data from flash by Int */
	while( (!SpiRxIntEnd) && (SpiIntWaitTime--) ) ; /* wait Rx Int & MAX 1500ms for Dead Lock */
	SetCSPinStatus(1) ;
	Delay_10nsX(10) ;
	return (*RxBufAddr) ;
}



/***********************************************************/
uChar WriteStatusWELBit(void)
{
uChar PollingTimeCnt, StatusData ;

	SetCSPinStatus(0) ; /* set chip CS port to Vil */
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = WriteEnableCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	SetCSPinStatus(1) ; /* set chip CS port to Vih */
	Delay_10nsX(10) ;

	PollingTimeCnt = 200 ;
	while(1)
	{ /* sending writing cmd to set WEL bit */
		StatusData = ReadStatusData(ReadStatusCmd)& StatusWIPBit ; /* only check bit0, checking all socket & get check result */
		if( !StatusData ) /* if all socket WIP bit = 0 means program ok */
			break ;
		else if( !(--PollingTimeCnt) )
		{ /* have some socket check error */
			return FAIL ;
		}
		Delay_1us(1) ;
	}
	return SUCCESS ;
}


/***************************************************************/
void SendControlCmd(uChar SendCmd)
{
	SetCSPinStatus(0) ;
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = SendCmd ; /* send cmd Byte */
	SPIRWDelay() ;
	Delay_10nsX(10) ; /* ensure fpga send finish data before CS pin rise high */
	SetCSPinStatus(1) ;
	Delay_10nsX(10) ;
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


void Enter4ByteMode(void)
{
	WriteStatusWELBit() ; /* for XMC 256B serial */

	SetCSPinStatus(0) ;
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = Enter4ByteModeCmd ;
	SPIRWDelay() ;
	SetCSPinStatus(1) ;
	Delay_10nsX(10) ;

	Delay_1us(10) ; /* MUST delay some time to stable 256M,512M,1G device */
}


void Exit4ByteMode(void)
{
	WriteStatusWELBit() ; /* for XMC 256B serial */

	SetCSPinStatus(0) ;
	Delay_10nsX(10) ;
	ArmSpi_TxReg() = Exit4ByteModeCmd ;
	SPIRWDelay() ;
	SetCSPinStatus(1) ;
	Delay_10nsX(10) ;

	Delay_1us(10) ; /* MUST delay some time to stable 256M,512M,1G device */
}


/***************************************************************/
void SetCSPinStatus(uChar PinStatus)
{
	if( PinStatus )
	{
		PtrMcuReg(CSPin.DataReg) |= (CSPin.BitReg) ; /* set CS=1 */
		nSSHigh() ; /* Disable Arm SPi Port, Page 415 */
		ResetArmSpiPortReg() ;
	}
	else
	{
		SetArmSpi_TxChlReg() ; /* Tx Channel On, Rx=Off */
		nSSLow() ; /* Enable Arm SPi Port, Page 415 */
		PtrMcuReg(CSPin.DataReg) &= (~(CSPin.BitReg)) ; /* set CS=0 */
	}
}


void SetWPPinStatus(uChar PinStatus)
{
	if( PinStatus )
		PtrMcuReg(WPPin.DataReg) |= (WPPin.BitReg) ; /* set WP=1 */
	else
		PtrMcuReg(WPPin.DataReg) &= (~(WPPin.BitReg)) ; /* set WP=0 */
}


void SetHOLDPinStatus(uChar PinStatus)
{
	if( PinStatus )
		PtrMcuReg(HOLDPin.DataReg) |= (HOLDPin.BitReg) ; /* set HOLD=1 */
	else
		PtrMcuReg(HOLDPin.DataReg) &= (~(HOLDPin.BitReg)) ; /* set HOLD=0 */
}


void SetRSTPinStatus(uChar PinStatus)
{
	if( PinStatus )
		PtrMcuReg(RSTPin.DataReg) |= (RSTPin.BitReg) ; /* set RST=1 */
	else
		PtrMcuReg(RSTPin.DataReg) &= (~(RSTPin.BitReg)) ; /* set RST=0 */
}


void SetCS2PinStatus(uChar PinStatus)
{
	if( PinStatus )
		PtrMcuReg(CS2Pin.DataReg) |= (CS2Pin.BitReg) ; /* set CS2=1 */
	else
		PtrMcuReg(CS2Pin.DataReg) &= (~(CS2Pin.BitReg)) ; /* set CS2=0 */
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


