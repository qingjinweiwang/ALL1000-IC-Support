
/******************************************************************************\
	Programmer total 13 tasks driver setting:
POWER_ON_FUN							POWER_OFF_FUN
READ_FUN			PROGRAM_FUN		VERIFY_FUN
ERASE_FUN			BLANK_CHECK_FUN	ILLEGAL_CHECK_FUN
ID_CHECK_FUN		SECURE_FUN			CHECKSUM_FUN
PROTECT_FUN								UNPROTECT_FUN
\*******************************************************************************/

#include "WJ_STM32G030F6P6.h"
#include "h\Type.h"
#include "h\Disp.h"
#include "h\Key.h"
#include "h\FirmWare.h"
#include "h\BlockSectorDef.h"
#include "h\SystemTask.h"
#include "h\Subroutine.h"
#include "h\ErrorCode.h"
#include "h\PassedCode.h"
#include "h\USBCom.h"
#include "h\CFControl.h"
#include "h\StatusCode.h"
#include "h\hs0_mmc.h"

/****************************************************************************\
			Type Definitions
\****************************************************************************/
/* Variable type Definitions: */
typedef	unsigned int	DWORD;
typedef	unsigned short WORD;
typedef	unsigned char BYTE;


/**************************************************************\
Notes: User define variable here,
		  the area is UserVarityAreaMin ~ UserVarityAreaMin
\**************************************************************/

/*********** For Blank Page *************/
uInt32 PageDataBuf[32]; /* Max Buffer Size be 32 D-word=128 Bytes */
uShort BlockEndFlag; /* if Seccess=End, Fail=Not End */
uShort OutDataLo, OutDataHi;
uInt32 SWD_Data;
uInt32 BlankPageFlag;
uShort ProgPageSize;
uShort Parity_Data;
/**************************************************************\
	FPGA Control Port Decode Address: 0xD2000~0xD2FF
	User should use the Macro Definition refer to FPGA file setting !!!
	User MUST MUST MUST CANNOT Use Other Address !!!
	Or System will be Destroy !!!!
\**************************************************************/
#define	SetSCKPinCmd			(FPGAOffset|0x8200<<1)
#define	SetRSTPinCmd			(FPGAOffset|0x8000<<1)
/// any more here, need also modify FPGA to support new Pin  ///
/**************************************************************\
	Set Easy FPGA Pin Number:A=1--48;
	then auto offset in FPGA: B=A+48, C=A+96, D=A+144
\**************************************************************/
#define	SetPinInPut			0x0000
#define	SetPinOutPut		0xF000

#define	SetPinDisLo			0x0000 /* when disable sck, this pin default be 0=Lo */
#define	SetPinDisHi			0x0F00 /* when disable sck, this pin default be 1=Hi */

///// FPGA Pin Num Define Here /////////////////////////////////////////////////
#define	SetVCCPinNum		(FPGAOffset|0xB000<<1) /* 0xB000 for easy FPGA pin setting */
#define	SetGNDPinNum		(FPGAOffset|0xB100<<1)
#define	SetSWDPinNum		(FPGAOffset|0xB200<<1) /* in/output dir, can also be TDO for JTAG */
#define	SetSCKPinNum		(FPGAOffset|0xB300<<1) /* output, can also be TCK for JTAG */
#define	SetRSTPinNum		(FPGAOffset|0xB400<<1) /* output */
#define	SetTMSPinNum		(FPGAOffset|0xB500<<1) /* output, can be define for other pin */
#define	SetTDIPinNum		(FPGAOffset|0xB600<<1) /* output, can be define for other pin */
#define	SetJCOMPinNum		(FPGAOffset|0xB700<<1) /* output, can be define for other pin */
/// any more here, need also modify FPGA to support new Pin  ///

///// FPGA Pin Num Define Here /////////////////////////////////////////////////
uChar VCCPinNum;
uChar GNDPinNum;
uChar SWDPinNum;
uChar SCKPinNum;
uChar RSTPinNum;
uChar TMSPinNum;
uChar TDIPinNum;
uChar JCOMPinNum;
/// any more here, need also modify FPGA to support new Pin  ///

/**************************************************************\
	Single type
\**************************************************************/
#define TTL_HI					0xFF
#define TTL_LO					0x00

#define BIT_MASK            0x0001
#define READ_BIT_MASK       0x01
#define ALL_BITS_16         0xFFFF
#define ALL_BITS_32         0xFFFFFFFFL
#define IDLE_BYTE           0x00
#define STATUS_LOW_NIBBLE   0x0F
#define USB_EVENT_MASK      0x0000FFFFUL

// /********* SWD parameter  ********************************/ //
/*
 TAR : Transter Address Register
 DRW : Data/Write Register
 CSW : The Control/Status Word

 In all phases data is transmitted LSB (least significant
 bit) first. The target will both sample and put data on the line on a rising clock edge.

 HOST CONSIST OF WITH BELOW
 START  AP/DP  W/R  ADDR[2..3]  PARITY   STOP   PARK
   1    0 / 1  0/1                        0       1

 Bit 5 is a parity bit. The parity bit is used by the target to verify
 the integrity of the request. This bit should be 1 if bits 1-4 contains an odd number of 1's. If the number
 of 1's are even, the parity bit should be zero. Bit 6 is the stop bit. This bit is always zero. Bit 7 is the
 park bit. This bit is always one.

 */
 // SWD header format
 // | Start | APnDP | RnW | ADDR[2:3] | Parity | STOP | Park |
/************************************************************************************************************/
#define DPACC_IDCODE_RD       0xA5
#define DPACC_ABORT_WR        0x81
#define DPACC_DP_CTRLSTAT_RD  0x8D
#define DPACC_CTRLSTAT_WR     0xA9
#define DPACC_SELECT_WR       0xB1
#define DPACC_READBUFF_RD     0xBD

#define APACC_CSW_WR          0xA3 //  1       1        0       0 0        1       0       1
#define APACC_TAR_WR          0x8B //  1       1        0       1 0        0       0       1
#define APACC_DRW_WR          0xBB //  1       1        0       1 1        1       0       1
#define APACC_DRW_RD          0x9F //  1       1        1       1 1        0       0       1
/****************************************************************\
Notes: User Macro Define Here

\****************************************************************/

#define DPIDR       (0x0BC11477L)    /* STM32G030 Cortex-M0+ */

/* STM32G030F6P6: 32 Kbytes Flash, 8 Kbytes SRAM */
#define FLASH_BASE       0x08000000L
#define FLASH_SIZE       0x00008000L
#define FLASH_PAGE_SIZE  0x00000800L

#define TOTAL_PAGE  (FLASH_SIZE / FLASH_PAGE_SIZE)

#define FALSH_APP_BASE  FLASH_BASE

#define DHCSR_ADDR             0xE000EDF0L
#define DHCSR_HALT_CMD         0xA05F0003L

#define AIRCR_ADDR             0xE000ED0C
#define AIRCR_SYSRESETREQ      0x05FA0004

/** STM32G0 Flash controller registers @ 0x40022000 **/
#define FLASH_REG_BASE  0x40022000L
#define FLASH_KEYR      0x40022008L
#define FLASH_OPTKEYR   0x4002200CL
#define FLASH_SR        0x40022010L
#define FLASH_CR        0x40022014L
#define FLASH_OPTR      0x40022020L
#define FLASH_WRP1AR    0x4002202CL
#define FLASH_WRP1BR    0x40022030L

/* FLASH_CR bit definitions */
#define FLASH_CR_OPTLOCK     0x40000000UL
#define FLASH_CR_LOCK        0x80000000UL
#define FLASH_CR_IDLE        0
#define FLASH_BLANK_WORD     ALL_BITS_32
#define FLASH_WAIT_LIMIT     5000

/* Unified error/clear mask macros (shared by ProgramFun/EraseFun) */
#define FLASH_ERROR_MASK 0x000000FA

#define FLASH_CLEAR_MASK 0x000000FB

/* FLASH_SR bit definitions */
#define FLASH_SR_EOP         0x00000001UL
#define FLASH_SR_BSY         0x00010000UL

/* FLASH unlock keys */
#define FLASH_KEY1           0x45670123UL
#define FLASH_KEY2           0xCDEF89ABUL
#define FLASH_OPTKEY1        0x08192A3BUL
#define FLASH_OPTKEY2        0x4C5D6E7FUL

/* .spc option data layout in SpecialBitBuf */
#define WRP_FIELDS_MASK    0x001F001FUL

#define OPTR_OTHER_MASK     0xFFFFFF00UL
#define OPTR_DEFAULT_HIGH   0xDFFFE100UL
#define PROTECT_RDP_MASK                 0x000000FFUL
#define PROTECT_RDP_LEVEL2               0x000000CCUL
#define PROTECT_WRP_START_MASK           0x0000001FUL
#define PROTECT_WRP_END_SHIFT            16
#define PROTECT_WRP_DISABLED_START       0x0000001FUL
#define PROTECT_ALL_PAGE_MASK            0x0000FFFFUL

/* BSY | CFGBSY */
#define FLASH_OPTION_BUSY_MASK     0x00050000UL
#define FLASH_OPTION_ERROR_MASK    0x000083FAUL
#define FLASH_OPTION_CLEAR_MASK    0x000083FBUL

/// Bit fields for the CSW register
#define AP_CSW_DEFAULT          0x23000012L

#define DP_SELECT_AHB_AP        0
void Send_One_Clk(void);
void InitIO(void);
void SetEasyFPGAPinDefine(void);

void Send_8Bit_DataCmd(uChar TempData);
void Check_Parity_Bit(uShort Parity_Lo, uShort Parity_Hi);
void Read_32Bit_Data(void);
void Send_32Bit_Data(uShort DatLo, uShort DatHi);
uShort Receive_ACK(void);
uChar ChkAllSckRead_32Bit_Data_Mask(uInt32 data, uInt32 mask);

void SWD_Write(uShort cmd, uInt32 data);
void SWD_Read(uShort cmd);
uChar SWD_Verify(uChar cmd, uInt32 data, uInt32 mask);
uChar SWDIDcode_Verify(uInt32 data);

void Write_AHB(uInt32 addr, uInt32 data);
void Read_AHB(uInt32 addr);
void Write_AHB_S(uInt32 addr, uInt32 data);
void Read_AHB_S(uInt32 addr);
uChar ChkAllSckRead_32Bit_Data(uShort DatLo, uShort DatHi);
uChar ChkAllSckRead_AHB(uInt32 addr, uShort DatLo, uShort DatHi);
uChar ChkAllSckRead_AHB_S(uInt32 addr, uShort DatLo, uShort DatHi);
uChar ChkAllSckRead_AHB_Mask(uInt32 addr, uInt32 data, uInt32 mask);
uChar swd_writeAPResource(DWORD targetAddr, DWORD wData);
DWORD swd_readAPResource(DWORD targetAddr);

void Swd_Bus_Reset(void);
void JTAG_to_SWD(void);
uChar connecting(void);
uChar connectDAP(void);

void GetDeviceParameterInfo(void);
void Get1PageDataChkBlank(void);
static void SelectActiveSockets(void);
static uChar CheckFlashStatusAll(uInt32 addr, uInt32 expected, uInt32 mask, uChar errorCode);
static uChar WaitFlashStatusAll(uInt32 expected, uInt32 mask, uInt32 waitLimit, uChar errorCode);
static uChar VerifyOptionRegistersAll(
	uInt32 optr, uInt32 wrp1ar, uInt32 wrp1br, uChar errorCode);
static uInt32 GetSpecialBitDWord(uShort offset);
static void SetSpecialBitDWord(uShort offset, uInt32 value);
static uChar ValidateProtectConfig(uInt32 optr, uInt32 wrp1ar, uInt32 wrp1br,
	uInt32 *wrp1ar_reg, uInt32 *wrp1br_reg);

uChar PowerOnFun(void);
uChar PowerOffFun(void);
uChar ReadFun(void);
uChar ProgramFun(void);
uChar VerifyFun(void);
uChar EraseFun(void);
uChar BlankCheckFun(void);
uChar IDCheckFun(void);
uChar SecureFun(void);
uChar CheckSumFun(void);
uChar ProtectFun(void);
uChar UnProtectFun(void);

void DevSNProgAndVerify(void);

/*******************************************************************\
Each Pin Define: Bit15-12 for Pin Default Direction;
					Bit11-8 for Pin Default Status When Disable Sck (Not NG Sck)
		   Bit7-0 for A-Sck Pin Number:1--48, FPGA will offset to B,C,D
\*******************************************************************/
void SetEasyFPGAPinDefine(void)
{
	uChar TempPmpIndex;

	TempPmpIndex = DrvElement_Table[Project->fwAlgoICIndex][PmpIdxOffset];

	VCCPinNum = (U8)DmmIOPortDefine[TempPmpIndex][0];  /* get each pin number */
	GNDPinNum = (U8)DmmIOPortDefine[TempPmpIndex][1];
	SWDPinNum = (U8)DmmIOPortDefine[TempPmpIndex][2];
	SCKPinNum = (U8)DmmIOPortDefine[TempPmpIndex][3];
	RSTPinNum = (U8)DmmIOPortDefine[TempPmpIndex][4];
	TMSPinNum = (U8)DmmIOPortDefine[TempPmpIndex][5];
	TDIPinNum = (U8)DmmIOPortDefine[TempPmpIndex][6];
	JCOMPinNum = (U8)DmmIOPortDefine[TempPmpIndex][7];

	/*** set Easy FPGA each pin number ***/
	PtrData(SetVCCPinNum) = SetPinInPut | SetPinDisLo | VCCPinNum; /* set pin default DIR + Disable satus +Pin number */
	Delay_10nsX(3);
	PtrData(SetGNDPinNum) = SetPinInPut | SetPinDisLo | GNDPinNum; /* set pin default DIR + Disable satus +Pin number */
	Delay_10nsX(3);

	PtrData(SetSWDPinNum) = SetPinOutPut | SetPinDisHi | SWDPinNum; /* set pin default DIR + Disable satus +Pin number */
	Delay_10nsX(3);
	PtrData(SetSCKPinNum) = SetPinOutPut | SetPinDisHi | SCKPinNum;
	Delay_10nsX(3);
	PtrData(SetRSTPinNum) = SetPinOutPut | SetPinDisHi | RSTPinNum; /* some device CANNOT reset when in disable status, or will exit program mode! */
	Delay_10nsX(3);

	PtrData(SetTMSPinNum) = SetPinInPut | SetPinDisLo | TMSPinNum; /* No use pin NEED set to input dir!or NOT define here */
	Delay_10nsX(3);
	PtrData(SetTDIPinNum) = SetPinInPut | SetPinDisLo | TDIPinNum; /* No use pin NEED set to input dir!or NOT define here */
	Delay_10nsX(3);
	PtrData(SetJCOMPinNum) = SetPinInPut | SetPinDisLo | JCOMPinNum; /* No use pin NEED set to input dir!or NOT define here */
	Delay_10nsX(3);
}

/******************************************************/
void InitIO(void)
{
	PtrData(OneDataCmd) = TTL_HI;
	PtrData(SetSCKPinCmd) = TTL_HI;
	PtrData(SetRSTPinCmd) = TTL_HI;
}

/******************************************************/
void Send_One_Clk(void)
{
	Delay_10nsX(20);
	PtrData(SetSCKPinCmd) = 1;			//! TCK -> high
	Delay_10nsX(20);
	PtrData(SetSCKPinCmd) = 0;			//! TCK -> low
	Delay_10nsX(20);
}


/******************************************************/
void Send_8Bit_DataCmd(uChar TempData)
{
	uChar BitCnt;

	for (BitCnt = 8; BitCnt > 0; BitCnt--)
	{ //! LSW bit shift 'in' first
		if (TempData & READ_BIT_MASK)
			PtrData(OneDataCmd) = TTL_HI; //! a bit shift 'in'
		else
			PtrData(OneDataCmd) = TTL_LO; //! a bit shift 'in'
		TempData >>= 1;
		Send_One_Clk();
	}
}

/******************************************************/
void Check_Parity_Bit(uShort Parity_Lo, uShort Parity_Hi)
{ /* OOB parity check */
	uShort BitCnt;

	Parity_Data = 0;
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{
		Parity_Data += (Parity_Lo & BIT_MASK);
		Parity_Lo >>= 1;
	}
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{
		Parity_Data += (Parity_Hi & BIT_MASK);
		Parity_Hi >>= 1;
	}
	if (Parity_Data & BIT_MASK)
		Parity_Data = TTL_HI;
	else
		Parity_Data = TTL_LO;
}

/******************************************************/
void Read_32Bit_Data(void)
{
	uShort BitCnt;

	BitCnt = PtrData(OneDataCmd); /* change to read status before read data */
	Delay_10nsX(10);
	OutDataLo = 0;
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{
		OutDataLo >>= 1; // !LSW bit shift 'OUT' first
		if (PtrData(OneDataCmd) & READ_BIT_MASK)
			OutDataLo |= 0x8000; /* Shift sampled SWD bit into bit 15. */
		Send_One_Clk();
	}

	OutDataHi = 0;
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{
		OutDataHi >>= 1; // !LSW bit shift 'OUT' first
		if (PtrData(OneDataCmd) & READ_BIT_MASK)
			OutDataHi |= 0x8000; /* Shift sampled SWD bit into bit 15. */
		Send_One_Clk();
	}
	Send_One_Clk();	/* Get parity bit */
	Send_One_Clk();	/* Get Trm bit */
}

/******************************************************/
uChar ChkAllSckRead_32Bit_Data(uShort DatLo, uShort DatHi)
{ /* send MSB firstly */
	uShort BitCnt;
	uChar sckStatusLo, sckStatusHi;
	uChar sckStatus;

	PtrData(GDevChkDataLoInc0) = TTL_HI; /* change to read status before read data */
	Delay_10nsX(10);

	PtrData(GSetAllMaskDataLo) = ALL_BITS_16;  /* send byte checking mask data to FPGA */
	Delay_10nsX(10);
	PtrData(GClrPageChkStatus) = 0;
	Delay_10nsX(10);
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{ /* read LSB firstly to FPGA then checking to fee-back data */
		PtrData(GDevChkDataLoInc0) = DatLo;
		Send_One_Clk();
	}
	sckStatusLo = PtrData(GChkAllSckStatus); /* get fpga checking all socket result */

	PtrData(GClrPageChkStatus) = 0;
	Delay_10nsX(10);
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{ /* read LSB firstly to FPGA then checking to fee-back data */
		PtrData(GDevChkDataLoInc0) = DatHi;
		Send_One_Clk();
	}
	Send_One_Clk();
	Send_One_Clk();
	sckStatusHi = PtrData(GChkAllSckStatus); /* get fpga checking all socket result */
	sckStatus = sckStatusLo | sckStatusHi;
	return sckStatus;
}

/******************************************************/
void Send_32Bit_Data(uShort DatLo, uShort DatHi)
{
	uShort BitCnt;

	Check_Parity_Bit(DatLo, DatHi);

	PtrData(GDevChkDataLoInc0) = TTL_HI; /* change to read status before read data */
	Delay_10nsX(10);
	Send_One_Clk(); /* Get Trm */

	for (BitCnt = 16; BitCnt > 0; BitCnt--) /* send lo 16 bit */
	{
		if (DatLo & BIT_MASK)
			PtrData(OneDataCmd) = TTL_HI; //! a bit shift 'in'
		else
			PtrData(OneDataCmd) = TTL_LO; //! a bit shift 'in'
		DatLo >>= 1;
		Send_One_Clk();
	}

	for (BitCnt = 16; BitCnt > 0; BitCnt--) /* send hi 16 bit */
	{
		if (DatHi & BIT_MASK)
			PtrData(OneDataCmd) = TTL_HI; //! a bit shift 'in'
		else
			PtrData(OneDataCmd) = TTL_LO; //! a bit shift 'in'
		DatHi >>= 1;
		Send_One_Clk();
	}

	PtrData(OneDataCmd) = Parity_Data; /* send parity bit */
	Send_One_Clk();
}

/******************************************************/
uShort Receive_ACK(void)
{
	uChar TempData, BitCnt;

	PtrData(GDevChkDataLoInc0) = TTL_HI; /* change to read status before read data */
	Delay_10nsX(20);
	Send_One_Clk();
	
	PtrData(GClrPageChkStatus) = 0;
	Delay_10nsX(20);
	PtrData(GSetAllMaskDataLo) = 0xE0; /* FPGA mask for the three SWD ACK bits. */
	Delay_10nsX(20);

	TempData = 0;
	for (BitCnt = 3; BitCnt > 0; BitCnt--) /* 3 ACK bits */
	{ //! LSW bit shift 'in' first
		PtrData(GDevChkDataLoInc0) = TTL_HI;
		Send_One_Clk();
	}
	TempData = PtrData(GChkAllSckStatus); //! LSW bit shift 'out' first
	return (TempData);
}

/**********************************************************\
		Check data with enable bits
		Data: 32-bit data
		Mask: 32-bit mask
		Return: FAIL/SUCCESS and socket status
\**********************************************************/
uChar ChkAllSckRead_32Bit_Data_Mask(uInt32 data, uInt32  mask)
{
	uShort mask_hi;
	uShort mask_lo;
	uShort data_lo;
	uShort data_hi;
	uShort BitCnt;
	uChar SckStatusLo, SckStatusHi;
	uChar sckStatus;
	data_lo = data;
	data_hi = data >> 16;
	mask_lo = mask;
	mask_hi = mask >> 16;


	PtrData(GDevChkDataLoInc0) = TTL_HI; /* change to read status before read data */
	Delay_10nsX(10);
	//lsb
	PtrData(GSetAllMaskDataLo) = mask_lo;  /* send byte checking mask data to FPGA */
	Delay_10nsX(10);
	PtrData(GClrPageChkStatus) = 0;
	Delay_10nsX(10);
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{ /* read LSB firstly to FPGA then checking to fee-back data */
		PtrData(GDevChkDataLoInc0) = data_lo;
		Send_One_Clk();
	}
	SckStatusLo = PtrData(GChkAllSckStatus); /* get fpga checking all socket result */

	//hsb
	PtrData(GSetAllMaskDataLo) = mask_hi;  /* send byte checking mask data to FPGA */
	Delay_10nsX(10);
	PtrData(GClrPageChkStatus) = 0;
	Delay_10nsX(10);
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{ /* read LSB firstly to FPGA then checking to fee-back data */
		PtrData(GDevChkDataLoInc0) = data_hi;
		Send_One_Clk();
	}
	Send_One_Clk();
	Send_One_Clk();
	SckStatusHi = PtrData(GChkAllSckStatus); /* get fpga checking all socket result */

	sckStatus = (SckStatusHi | SckStatusLo);

	return sckStatus;
}


/******************************************************/
void SWD_Write(uShort cmd, uInt32 data)
{
	Send_8Bit_DataCmd(cmd);
	Receive_ACK();
	Send_32Bit_Data(LSW(data), MSW(data));
	Send_8Bit_DataCmd(IDLE_BYTE);
}

/******************************************************/
void SWD_Read(uShort cmd)
{
	Send_8Bit_DataCmd(cmd);
	Receive_ACK();
	Read_32Bit_Data();
	Send_8Bit_DataCmd(IDLE_BYTE);
	SWD_Data = (((uInt32)OutDataHi) << 16) | ((uInt32)OutDataLo);
}

/******************************************************/
uChar SWD_Verify(uChar cmd, uInt32 data, uInt32 mask)
{
	uChar sckStatus = 0;
	Send_8Bit_DataCmd(cmd);
	Receive_ACK();
	sckStatus = ChkAllSckRead_32Bit_Data_Mask(data, mask);
	Send_8Bit_DataCmd(IDLE_BYTE);

	return sckStatus;
}
/******************************************************/
uChar SWDIDcode_Verify(uInt32 data)
{
	return SWD_Verify(DPACC_IDCODE_RD, data, ALL_BITS_32);
}

/******************************************************/
void Read_AHB(uInt32 addr)
{
	SWD_Write(APACC_TAR_WR, addr);
	SWD_Read(APACC_DRW_RD);
}

/******************************************************/
uChar swd_writeAPResource(DWORD targetAddr, DWORD wData)
{
	SWD_Write(APACC_TAR_WR, targetAddr);
	SWD_Write(APACC_DRW_WR, wData);

	return SUCCESS;
}

/******************************************************/
DWORD swd_readAPResource(DWORD targetAddr)
{
	SWD_Write(APACC_TAR_WR, targetAddr);

	SWD_Read(APACC_DRW_RD);

	SWD_Read(DPACC_READBUFF_RD);

	return SWD_Data;
}

/******************************************************/
void Write_AHB(uInt32 addr, uInt32 data)
{
	SWD_Write(APACC_TAR_WR, addr);
	SWD_Write(APACC_DRW_WR, data);
}

/******************************************************/
void Write_AHB_S(uInt32 addr, uInt32 data)
{
	SWD_Write(DPACC_SELECT_WR, DP_SELECT_AHB_AP);

	Send_8Bit_DataCmd(APACC_TAR_WR); /* write TAR */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Send_32Bit_Data(LSW(addr), MSW(addr));
	Send_8Bit_DataCmd(IDLE_BYTE);

	SWD_Write(DPACC_SELECT_WR, DP_SELECT_AHB_AP);

	Send_8Bit_DataCmd(APACC_DRW_WR); /* write DRW */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Send_32Bit_Data(LSW(data), MSW(data));
	Send_8Bit_DataCmd(IDLE_BYTE);
	Delay_1us(5);
}

/******************************************************/
void Read_AHB_S(uInt32 addr)
{
	Send_8Bit_DataCmd(APACC_TAR_WR); /* write TAR */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Send_32Bit_Data(LSW(addr), MSW(addr));
	Send_8Bit_DataCmd(IDLE_BYTE);

	Send_8Bit_DataCmd(APACC_DRW_RD); /* read DRW dummy */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Read_32Bit_Data(); /* dummy read data */
	Send_8Bit_DataCmd(IDLE_BYTE);

	Send_8Bit_DataCmd(DPACC_READBUFF_RD); /* read rdbuf */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Read_32Bit_Data(); /* realy read true data */
	Send_8Bit_DataCmd(IDLE_BYTE);
}

/******************************************************/
uChar ChkAllSckRead_AHB(uInt32 addr, uShort DatLo, uShort DatHi)
{
	uShort SckStatus;

	SWD_Write(DPACC_SELECT_WR, DP_SELECT_AHB_AP);

	Send_8Bit_DataCmd(APACC_TAR_WR); /* write TAR */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Send_32Bit_Data(LSW(addr), MSW(addr));
	Send_8Bit_DataCmd(IDLE_BYTE);

	SWD_Write(DPACC_SELECT_WR, DP_SELECT_AHB_AP);

	Send_8Bit_DataCmd(APACC_DRW_RD); /* read DRW dummy */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	ChkAllSckRead_32Bit_Data(DatLo, DatHi); /* dummy read data */
	Send_8Bit_DataCmd(IDLE_BYTE);

	Send_8Bit_DataCmd(DPACC_READBUFF_RD); /* read rdbuf */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	SckStatus = ChkAllSckRead_32Bit_Data(DatLo, DatHi); /* realy read true data */
	Send_8Bit_DataCmd(IDLE_BYTE);

	return SckStatus;
}

/******************************************************/
uChar ChkAllSckRead_AHB_Mask(uInt32 addr, uInt32 data, uInt32 mask)
{
	SWD_Write(DPACC_SELECT_WR, DP_SELECT_AHB_AP);
	SWD_Write(APACC_TAR_WR, addr);
	SWD_Read(APACC_DRW_RD); /* dummy AP read */

	return SWD_Verify(DPACC_READBUFF_RD, data, mask);
}

/******************************************************/
uChar ChkAllSckRead_AHB_S(uInt32 addr, uShort DatLo, uShort DatHi)
{
	uShort SckStatus;

	Send_8Bit_DataCmd(APACC_TAR_WR); /* write TAR */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Send_32Bit_Data(LSW(addr), MSW(addr));
	Send_8Bit_DataCmd(IDLE_BYTE);

	Send_8Bit_DataCmd(APACC_DRW_RD); /* read DRW dummy */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	ChkAllSckRead_32Bit_Data(DatLo, DatHi); /* dummy read data */
	Send_8Bit_DataCmd(IDLE_BYTE);

	Send_8Bit_DataCmd(DPACC_READBUFF_RD); /* read rdbuf */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	SckStatus = ChkAllSckRead_32Bit_Data(DatLo, DatHi); /* realy read true data */
	Send_8Bit_DataCmd(IDLE_BYTE);

	return SckStatus;
}


/******************************************************/
void Swd_Bus_Reset(void)
{
	uShort TempCnt;

	PtrData(OneDataCmd) = TTL_HI;
	PtrData(SetSCKPinCmd) = TTL_LO;
	Delay_10nsX(10);
	for (TempCnt = 0; TempCnt < 0x37; TempCnt++) /* At least 55 SWCLK cycles. */
	{
		PtrData(SetSCKPinCmd) = TTL_HI;
		Delay_10nsX(10);
		PtrData(SetSCKPinCmd) = TTL_LO;
		Delay_10nsX(10);
	}
}

/******************************************************/
void JTAG_to_SWD(void)
{
	Swd_Bus_Reset();
	Send_8Bit_DataCmd(0x9E); /* JTAG-to-SWD selection sequence, byte 0. */
	Send_8Bit_DataCmd(0xE7); /* JTAG-to-SWD selection sequence, byte 1. */
	Send_8Bit_DataCmd(0xAA); /* Trailing selection/idle byte. */
	Send_8Bit_DataCmd(IDLE_BYTE);
	Swd_Bus_Reset();
	Send_8Bit_DataCmd(IDLE_BYTE);
}

/******************************************************/
uChar connecting(void)
{
    uChar sckStatus;
    JTAG_to_SWD();
	JTAG_to_SWD();

	sckStatus = SWD_Verify(DPACC_IDCODE_RD, DPIDR, ALL_BITS_32);
	if (GangCheckIfAllSocketError(sckStatus & STATUS_LOW_NIBBLE))
	{
		return FAIL;
	}

    SWD_Write(DPACC_ABORT_WR, 0x0000001EUL); /* Clear sticky DP errors. */
    SWD_Write(DPACC_CTRLSTAT_WR, 0x50000000UL); /* Request debug and system power-up. */
    Delay_1ms(1);

	sckStatus = SWD_Verify(DPACC_DP_CTRLSTAT_RD,0xF0000000UL, 0xF0000000UL);
	if (GangCheckIfAllSocketError(sckStatus & STATUS_LOW_NIBBLE))
	{
		return FAIL;
	}
    SWD_Write(DPACC_CTRLSTAT_WR, 0x50000F00UL); /* Keep power-up requests and enable all lanes. */
    SWD_Write(DPACC_SELECT_WR, DP_SELECT_AHB_AP);/* DPACC_SELECT_WR = 0xB1，DP_SELECT_AHB_AP = 0x00000000 */

    return SUCCESS;
}

/******************************************************/
uChar connectDAP(void)
{
	if (connecting() != SUCCESS)
	{
		return FAIL;
	}

	SWD_Write(DPACC_SELECT_WR, DP_SELECT_AHB_AP);/* DPACC_SELECT_WR = 0xB1，DP_SELECT_AHB_AP = 0x00000000 */
	SWD_Write(APACC_CSW_WR, AP_CSW_DEFAULT);/* APACC_CSW_WR = 0xA3，AP_CSW_DEFAULT = 0x23000012 */

	swd_writeAPResource(DHCSR_ADDR, DHCSR_HALT_CMD);/* DHCSR_ADDR = 0xE000EDF0，DHCSR_HALT_CMD = 0xA05F0003 */
	Delay_1ms(1);
	swd_writeAPResource(AIRCR_ADDR, AIRCR_SYSRESETREQ);
	Delay_1ms(1);

	return SUCCESS;
}

/******************************************************/
uChar GetDrvElementLen(void)
{
	DevDrvVersion = DevDRV_Version; /* get driver version */
	GangType = SystemGang;
	return (uChar)(sizeof(DrvElement_Table) / 5);
}

/******************************************************/
void GetDeviceParameterInfo(void)
{
	ProgPageSize = ChipPageSizeTable[fwAlgoIndex];
}

/******************************************************/
void Get1PageDataChkBlank(void)
{
	uShort BufferCnt;

	BlankPageFlag = FLASH_BLANK_WORD;
	for (BufferCnt = 0; BufferCnt < ProgPageSize; BufferCnt++)
	{ /* divide by 2 for byte data width */
		PageDataBuf[BufferCnt] = CFCardReadWordData(); /* Get low word */
		PageDataBuf[BufferCnt] |= (CFCardReadWordData() << 16); /* Get hi word */
		BlankPageFlag &= PageDataBuf[BufferCnt];

	}
	if (BlankPageFlag != FLASH_BLANK_WORD)
		BlankPageFlag = 0;
}

/******************************************************/
static void SelectActiveSockets(void)
{
	/*
	 * Keep the FPGA comparison mask in step with the SWD enable mask.
	 * Otherwise a powered-down socket can remain in GChkAllSckStatus.
	 */
	PtrData(GSetAllErrSckMask) = (uShort)GangErrSckMask;
	Delay_10nsX(3);
	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask);
	Delay_10nsX(3);
}

/******************************************************/
static uChar CheckFlashStatusAll(uInt32 addr, uInt32 expected, uInt32 mask, uChar errorCode)
{
	uChar sckStatus;

	sckStatus = ChkAllSckRead_AHB_Mask(addr, expected, mask);
	if (sckStatus)
	{
		if (GangCheckIfAllSocketError(sckStatus & STATUS_LOW_NIBBLE))
		{
			SysStatusCode = errorCode;
			return FAIL;
		}
		SelectActiveSockets();
	}

	return SUCCESS;
}

/******************************************************/
static uChar VerifyOptionRegistersAll(
	uInt32 optr, uInt32 wrp1ar, uInt32 wrp1br, uChar errorCode)
{

	if (CheckFlashStatusAll(
			FLASH_OPTR, optr, ALL_BITS_32, errorCode) != SUCCESS)
	{
		return FAIL;
	}

	if (CheckFlashStatusAll(
			FLASH_WRP1AR, wrp1ar, WRP_FIELDS_MASK, errorCode) != SUCCESS)
	{
		return FAIL;
	}

	if (CheckFlashStatusAll(
			FLASH_WRP1BR, wrp1br, WRP_FIELDS_MASK, errorCode) != SUCCESS)
	{
		return FAIL;
	}

	return SUCCESS;
}

/******************************************************/
static uChar WaitFlashStatusAll(uInt32 expected,uInt32 mask,uInt32 waitLimit,uChar errorCode)
{
	uChar sckStatus;
	sckStatus = 0;

	while (waitLimit > 0)
	{
		sckStatus = ChkAllSckRead_AHB_Mask(FLASH_SR,expected,mask);
		if (sckStatus == 0)
		{
			return SUCCESS;
		}
		if ((waitLimit & 0x00FFUL) == 0)
		{
			HandleEvent_EP0();
		}

		waitLimit--;
	}
	if (GangCheckIfAllSocketError(sckStatus & STATUS_LOW_NIBBLE))
	{
		SysStatusCode = errorCode;
		return FAIL;
	}
	SelectActiveSockets();

	return SUCCESS;
}

/******************************************************/
static uInt32 GetSpecialBitDWord(uShort offset)
{
	uInt32 value;

	value = ((uInt32)SpecialBitBuf(offset)) << 24;
	value |= ((uInt32)SpecialBitBuf(offset + 1)) << 16;
	value |= ((uInt32)SpecialBitBuf(offset + 2)) << 8;
	value |= (uInt32)SpecialBitBuf(offset + 3);

	return value;
}

/******************************************************/
static void SetSpecialBitDWord(uShort offset, uInt32 value)
{
	SpecialBitBuf(offset) = (uChar)(value >> 24);
	SpecialBitBuf(offset + 1) = (uChar)(value >> 16);
	SpecialBitBuf(offset + 2) = (uChar)(value >> 8);
	SpecialBitBuf(offset + 3) = (uChar)value;
}

/******************************************************/
static uChar ValidateProtectConfig(uInt32 optr,uInt32 wrp1ar,uInt32 wrp1br,uInt32 *wrp1ar_reg,uInt32 *wrp1br_reg)
{
	uInt32 wrp1ar_start;
	uInt32 wrp1ar_end;
	uInt32 wrp1ar_pages;
	uInt32 wrp1br_start;
	uInt32 wrp1br_end;
	uInt32 wrp1br_pages;

	if ((optr & OPTR_OTHER_MASK) != OPTR_DEFAULT_HIGH)
	{
		SysStatusCode = E_ProtectFail;
		return FAIL;
	}
	if ((optr & PROTECT_RDP_MASK) == PROTECT_RDP_LEVEL2)
	{
		SysStatusCode = E_ProtectFail;
		return FAIL;
	}
	*wrp1ar_reg = wrp1ar & WRP_FIELDS_MASK;
	wrp1ar_start = *wrp1ar_reg & PROTECT_WRP_START_MASK;
	wrp1ar_end = (*wrp1ar_reg >> PROTECT_WRP_END_SHIFT) & PROTECT_WRP_START_MASK;

	if ((wrp1ar_start == PROTECT_WRP_DISABLED_START) && (wrp1ar_end == 0))
	{
		wrp1ar_pages = 0;
	}
	else
	{
		if ((wrp1ar_start > wrp1ar_end) || (wrp1ar_end >= TOTAL_PAGE))
		{
			SysStatusCode = E_ProtectFail;
			return FAIL;
		}

		wrp1ar_pages =((1UL << ((wrp1ar_end - wrp1ar_start) + 1)) - 1UL) << wrp1ar_start;
		wrp1ar_pages &= PROTECT_ALL_PAGE_MASK;
	}

	*wrp1br_reg = wrp1br & WRP_FIELDS_MASK;
	wrp1br_start = *wrp1br_reg & PROTECT_WRP_START_MASK;
	wrp1br_end = (*wrp1br_reg >> PROTECT_WRP_END_SHIFT) & PROTECT_WRP_START_MASK;

	if ((wrp1br_start == PROTECT_WRP_DISABLED_START) && (wrp1br_end == 0))
	{
		wrp1br_pages = 0;
	}
	else
	{
		if ((wrp1br_start > wrp1br_end) || (wrp1br_end >= TOTAL_PAGE))
		{
			SysStatusCode = E_ProtectFail;
			return FAIL;
		}

		wrp1br_pages =((1UL << ((wrp1br_end - wrp1br_start) + 1)) - 1UL)<< wrp1br_start;
		wrp1br_pages &= PROTECT_ALL_PAGE_MASK;
	}

	return SUCCESS;
}


/**********************************************************\
		PowerOnFun -- Power on + SWD connect
\**********************************************************/
uChar PowerOnFun(void)
{
	Set_VccDAC(); /* setting DAC array to rise VCC/VPP/VPE/VPS/VPIO volts level */
	Set_VihDAC();
	Set_VppDAC();
	Set_VpioDAC();
	SetEasyFPGAPinDefine(); /* only for use easy fpga file!! other need mask!! */
	Delay_1ms(50); /* delay for stable volte */

	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask); /* re-enable all pass socket */
	Delay_10nsX(3);

	InitIO();
	GangRiseAllPassSocketVccIccPin();/* open VCC/VPP/VPE/VPS/VPIO pin driving */
	GangRiseAllPassSocketVppIppPin();/* open VCC/VPP/VPE/VPS/VPIO pin driving */
	Delay_1ms(100);
	GetDeviceParameterInfo();/* open VCC/VPP/VPE/VPS/VPIO pin driving */
	if (connectDAP() != SUCCESS)
{
	return FAIL;
}
	return SUCCESS;
}

/**********************************************************\
		PowerOffFun -- Power off
\**********************************************************/
uChar PowerOffFun(void)
{
	PtrData(OneDataCmd) = TTL_LO;
	PtrData(SetSCKPinCmd) = TTL_LO;
	PtrData(SetRSTPinCmd) = TTL_LO;
	Delay_1ms(10);

	Fall_VpioIpioAll();
	Fall_VppIppAll();
	Fall_VihAll();
	Fall_VccAll();
	Delay_1ms(50); /* delay for stable volte */
	return SUCCESS;
}


/**********************************************************\
		ReadFun -- Read Flash via AHB-AP
\**********************************************************/
uChar ReadFun(void)
{
	uInt32 dev_addr;
	uInt32 read_addr;
	uInt32 readData;
	uChar sckStatus=0;

	if (IfSpecialBitBlk())
	{
		SelectActiveSockets();

		readData = swd_readAPResource(FLASH_OPTR);
		SetSpecialBitDWord(0x00, readData);

		sckStatus = ChkAllSckRead_AHB_Mask(FLASH_OPTR, readData, ALL_BITS_32);
		if (sckStatus & STATUS_LOW_NIBBLE)
		{
			SysStatusCode = E_ReadAHBFail;
			return FAIL;
		}

		readData = swd_readAPResource(FLASH_WRP1AR);
		SetSpecialBitDWord(0x08, readData);

		sckStatus = ChkAllSckRead_AHB_Mask(FLASH_WRP1AR, readData, ALL_BITS_32);
		if (sckStatus & STATUS_LOW_NIBBLE)
		{
			SysStatusCode = E_ReadAHBFail;
			return FAIL;
		}

		readData = swd_readAPResource(FLASH_WRP1BR);
		SetSpecialBitDWord(0x0C, readData);

		sckStatus = ChkAllSckRead_AHB_Mask(FLASH_WRP1BR, readData, ALL_BITS_32);
		if (sckStatus & STATUS_LOW_NIBBLE)
		{
			SysStatusCode = E_ReadAHBFail;
			return FAIL;
		}

		return SUCCESS;
	}

	sckStatus = SWDIDcode_Verify(DPIDR);


	if (GangCheckIfAllSocketError(sckStatus & STATUS_LOW_NIBBLE))
	{
		SysStatusCode = E_ReadIDVerifyFail;
		return FAIL;
	}


	dev_addr = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();

	while (DevCurrBlkLen.all)
	{
		if (dev_addr < FALSH_APP_BASE)
		{
			read_addr = dev_addr + FALSH_APP_BASE;
		}
		else
		{
			read_addr = dev_addr;
		}

		readData = swd_readAPResource(read_addr);

		ReadDataBuf[ReadBufCnt / 2 + 0] = (uShort)(readData & ALL_BITS_16);
		ReadDataBuf[ReadBufCnt / 2 + 1] = (uShort)((readData >> 16) & ALL_BITS_16);

		ReadBufCnt += 4;
		DevCurrBlkLen.all -= 4;

		if (SaveChkBlkLenEnd())
		{
			return SUCCESS;
		}

		dev_addr += 4;

		if (!(dev_addr & USB_EVENT_MASK)) /* Service USB at each 64 KiB address boundary. */
		{
			HandleEvent_EP0();
			Delay_1ms(10);
		}
	}

	return SUCCESS;
}


/**********************************************************\
		ProgramFun -- Direct STM32G0 FLASH register 64-bit double-word program via SWD AHB-AP
\**********************************************************/
uChar ProgramFun(void)
{
	uInt32 dev_addr, chunk_start_addr, write_addr, data_lo, data_hi;
	uShort buf_index, prog_index, buf_word_cnt;
	uChar sckStatus, null_flag;
	static uInt32 pMem[4096];

	if (IfSpecialBitBlk()) 
	{
		return SUCCESS;
	}

	PowerOffFun();
	Delay_1Second(1);

	if (PowerOnFun() != SUCCESS)
	{
		SysStatusCode = E_ProgramFail;
		return FAIL;
	}

	HandleEvent_EP0();
	Delay_1ms(10);

	sckStatus = SWDIDcode_Verify(DPIDR);
	if (GangCheckIfAllSocketError(sckStatus & STATUS_LOW_NIBBLE))
	{
		SysStatusCode = E_ProgramFail;
		return FAIL;
	}

	dev_addr = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();
	BlockEndFlag = 0;

	if (WaitFlashStatusAll(0, FLASH_SR_BSY, FLASH_WAIT_LIMIT, E_ProgramFail) != SUCCESS)
	{
		return FAIL;
	}

	swd_writeAPResource(FLASH_KEYR, FLASH_KEY1);
	swd_writeAPResource(FLASH_KEYR, FLASH_KEY2);

	if (CheckFlashStatusAll(FLASH_CR, 0, FLASH_CR_LOCK, E_ProgramFail) != SUCCESS)
	{
		return FAIL;
	}

	swd_writeAPResource(FLASH_CR, FLASH_CR_IDLE);
	swd_writeAPResource(FLASH_SR, FLASH_CLEAR_MASK);

	while (DevCurrBlkLen.all)
	{
		chunk_start_addr = dev_addr;
		buf_word_cnt = 0;
		null_flag = 0;

		for (buf_index = 0; buf_index < (16384 / 4); buf_index++)
		{
			data_lo = CFCardReadWordData();
			data_lo |= (uInt32)CFCardReadWordData() << 16;
			pMem[buf_index] = data_lo;
			if (data_lo != FLASH_BLANK_WORD) null_flag = 1;
			DramBufCnt -= 4;
			DevCurrBlkLen.all -= 4;
			buf_word_cnt++;
			BlockEndFlag = GetChkBlkLenEnd();
			if (BlockEndFlag) break;
		}

		if (null_flag)
		{
			for (prog_index = 0; prog_index < buf_word_cnt; prog_index += 2)
			{
				data_lo = pMem[prog_index];
				data_hi = ((prog_index + 1) < buf_word_cnt) ? pMem[prog_index + 1] : FLASH_BLANK_WORD;
				if ((data_lo == FLASH_BLANK_WORD) && (data_hi == FLASH_BLANK_WORD)) continue;

				if (chunk_start_addr < FALSH_APP_BASE)
					write_addr = chunk_start_addr + FALSH_APP_BASE + ((uInt32)prog_index * 4);
				else
					write_addr = chunk_start_addr + ((uInt32)prog_index * 4);

				if (write_addr & (8 - 1))
				{
					swd_writeAPResource(FLASH_CR, FLASH_CR_LOCK);
					SysStatusCode = E_ProgramFail;
					return FAIL;
				}

				if (WaitFlashStatusAll(0, FLASH_SR_BSY, FLASH_WAIT_LIMIT, E_ProgramFail) != SUCCESS)
				{
					swd_writeAPResource(FLASH_CR, FLASH_CR_LOCK);
					return FAIL;
				}

				swd_writeAPResource(FLASH_SR, FLASH_CLEAR_MASK);
				swd_writeAPResource(FLASH_CR, 0x00000001UL); /* PG: enable double-word programming. */
				swd_writeAPResource(write_addr, data_lo);
				swd_writeAPResource(write_addr + 4, data_hi);

				if (WaitFlashStatusAll(0, FLASH_SR_BSY, FLASH_WAIT_LIMIT, E_ProgramFail) != SUCCESS)
				{
					swd_writeAPResource(FLASH_CR, FLASH_CR_LOCK);
					return FAIL;
				}

				if (CheckFlashStatusAll(FLASH_SR, 0, FLASH_ERROR_MASK, E_ProgramFail) != SUCCESS)
				{
					swd_writeAPResource(FLASH_SR, FLASH_CLEAR_MASK);
					swd_writeAPResource(FLASH_CR, FLASH_CR_LOCK);
					return FAIL;
				}

				swd_writeAPResource(FLASH_SR, FLASH_SR_EOP);
				swd_writeAPResource(FLASH_CR, FLASH_CR_IDLE);
			}
		}

		dev_addr += ((uInt32)buf_word_cnt * 4);
		HandleEvent_EP0();
		Delay_1ms(10);
		if (BlockEndFlag) break;
	}

	swd_writeAPResource(FLASH_CR, FLASH_CR_LOCK);
	return SUCCESS;
}
/**********************************************************\
		VerifyFun -- Verify Flash data
\**********************************************************/
uChar VerifyFun(void)
{
	uInt32 dev_addr;
	uInt32 read_addr;
	uInt32 USBData;
	uChar verifyStatus;

	if (IfSpecialBitBlk())
	{
		return SUCCESS;
	}

	SelectActiveSockets();

	dev_addr = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();

	while (DevCurrBlkLen.all)
	{
		USBData = CFCardReadWordData();
		USBData |= (uInt32)CFCardReadWordData() << 16;

		if (dev_addr < FALSH_APP_BASE)
		{
			read_addr = dev_addr + FALSH_APP_BASE;
		}
		else
		{
			read_addr = dev_addr;
		}

		Read_AHB(read_addr);

		verifyStatus = SWD_Verify(DPACC_READBUFF_RD, USBData, ALL_BITS_32);

		if (verifyStatus)
		{
			if (GangCheckIfAllSocketError(
				verifyStatus & STATUS_LOW_NIBBLE))
			{
				SysStatusCode = E_VerifyFail;
				return FAIL;
			}

			SelectActiveSockets();
		}

		DramBufCnt -= 4;
		DevCurrBlkLen.all -= 4;
		dev_addr += 4;

		if (GetChkBlkLenEnd())
		{
			return SUCCESS;
		}

		if (!(DevCurrBlkLen.all & USB_EVENT_MASK)) /* Service USB every 64 KiB remaining. */
		{
			HandleEvent_EP0();
			Delay_1ms(10);
		}
	}

	return SUCCESS;
}
/**********************************************************\
		EraseFun -- Direct STM32G0 FLASH register page erase via SWD AHB-AP
\**********************************************************/
uChar EraseFun(void)
{
	uInt32 page;
	uInt32 cr;

	if (!IfFirstTimeEntry())
		return SUCCESS;

	PowerOffFun();
	Delay_1ms(50);
	if (PowerOnFun() != SUCCESS)
	{
		SysStatusCode = E_EraseFail;
		return FAIL;
	}

	if (WaitFlashStatusAll(0, FLASH_SR_BSY, FLASH_WAIT_LIMIT, E_EraseFail) != SUCCESS)
	{
		return FAIL;
	}

	swd_writeAPResource(FLASH_KEYR, FLASH_KEY1);
	swd_writeAPResource(FLASH_KEYR, FLASH_KEY2);

	if (CheckFlashStatusAll(FLASH_CR, 0, FLASH_CR_LOCK, E_EraseFail) != SUCCESS)
	{
		return FAIL;
	}

	swd_writeAPResource(FLASH_SR, FLASH_CLEAR_MASK);

	for (page = 0; page < TOTAL_PAGE; page++)
	{
		if (WaitFlashStatusAll(0, FLASH_SR_BSY, FLASH_WAIT_LIMIT, E_EraseFail) != SUCCESS)
		{
			swd_writeAPResource(FLASH_CR, FLASH_CR_LOCK);
			return FAIL;
		}

		swd_writeAPResource(FLASH_SR, FLASH_CLEAR_MASK);

		cr = 0x00000002UL | (page << 3); /* PER plus page number in PNB[7:3]. */
		swd_writeAPResource(FLASH_CR, cr);
		cr |= 0x00010000UL; /* STRT: start page erase. */
		swd_writeAPResource(FLASH_CR, cr);
		Delay_1ms(0x32); /* 50 ms page-erase settling delay. */

		if (WaitFlashStatusAll(0, FLASH_SR_BSY, FLASH_WAIT_LIMIT, E_EraseFail) != SUCCESS)
		{
			swd_writeAPResource(FLASH_CR, FLASH_CR_LOCK);
			return FAIL;
		}

		if (CheckFlashStatusAll(FLASH_SR, 0, FLASH_ERROR_MASK, E_EraseFail) != SUCCESS)
		{
			swd_writeAPResource(FLASH_SR, FLASH_CLEAR_MASK);
			swd_writeAPResource(FLASH_CR, FLASH_CR_LOCK);
			return FAIL;
		}

		swd_writeAPResource(FLASH_SR, FLASH_SR_EOP);
		swd_writeAPResource(FLASH_CR, FLASH_CR_IDLE);
	}

	swd_writeAPResource(FLASH_CR, FLASH_CR_LOCK);
	return SUCCESS;
}

/**********************************************************\
		BlankCheckFun -- Blank check (all 0xFF)
\**********************************************************/
uChar BlankCheckFun(void)
{
	uInt32 dev_addr;
	uInt32 read_addr;
	uChar sckStatus;

	if (IfSpecialBitBlk())
	{
		return SUCCESS;
	}

	dev_addr = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();

	while (DevCurrBlkLen.all)
	{
		if (dev_addr < FALSH_APP_BASE)
		{
			read_addr = dev_addr + FALSH_APP_BASE;
		}
		else
		{
			read_addr = dev_addr;
		}

		Read_AHB(read_addr);

		sckStatus = SWD_Verify(DPACC_READBUFF_RD, FLASH_BLANK_WORD, ALL_BITS_32);

		if (GangCheckIfAllSocketError(sckStatus))
		{
			SysStatusCode = E_BlankFail;
			return FAIL;
		}

		DramBufCnt -= 4;
		DevCurrBlkLen.all -= 4;
		dev_addr += 4;

		if (SaveChkBlkLenEnd())
		{
			return SUCCESS;
		}

		if (!(DevCurrBlkLen.all & USB_EVENT_MASK)) /* Service USB every 64 KiB remaining. */
		{
			HandleEvent_EP0();
			Delay_1ms(10);
		}
	}

	return SUCCESS;
}

/**********************************************************\
		IDCheckFun -- Verify DPIDR
\**********************************************************/
uChar IDCheckFun(void)
{

	uChar sckStatus = 0x00;
	DWORD g_DeviceID;

	g_DeviceID = ((uInt32)DeviceIDLo()) | (((uInt32)DeviceIDHi()) << 16);

	JTAG_to_SWD();
	sckStatus = SWDIDcode_Verify(g_DeviceID);
	Delay_1ms(5);
	if (GangCheckIfAllSocketError(sckStatus&0xF))
	{ 
		SysStatusCode = E_AutoIDFail; /* sending error type code */
		return FAIL;
	}

	return SUCCESS;
}

/**********************************************************\
		SecureFun
\**********************************************************/
uChar SecureFun(void)
{
	uChar sckStatus = 0;

	if (GangCheckIfAllSocketError(sckStatus))
	{ /* if all socket error then cancle function */
		SysStatusCode = E_SecureFail; /* sending error type code */
		return FAIL;
	}
	return SUCCESS;
}

/**********************************************************\
	ProtectFun -- Program OPTR, WRP1AR and WRP1BR from .spc
\**********************************************************/
uChar ProtectFun(void)
{
	uInt32 optr;
	uInt32 wrp1ar;
	uInt32 wrp1br;
	uInt32 wrp1ar_reg;
	uInt32 wrp1br_reg;

	optr = GetSpecialBitDWord(0x00);
	wrp1ar = GetSpecialBitDWord(0x08);
	wrp1br = GetSpecialBitDWord(0x0C);

	if (ValidateProtectConfig(optr, wrp1ar, wrp1br,&wrp1ar_reg, &wrp1br_reg) != SUCCESS)
	{
		return FAIL;
	}

	PowerOffFun();
	Delay_1ms(50);
	if (PowerOnFun() != SUCCESS)
	{
		SysStatusCode = E_ProtectFail;
		return FAIL;
	}

	if (WaitFlashStatusAll(0, FLASH_OPTION_BUSY_MASK,FLASH_WAIT_LIMIT, E_ProtectFail) != SUCCESS)
	{
		return FAIL;
	}

	swd_writeAPResource(FLASH_KEYR, FLASH_KEY1);
	swd_writeAPResource(FLASH_KEYR, FLASH_KEY2);
	if (CheckFlashStatusAll(FLASH_CR, 0, FLASH_CR_LOCK, E_ProtectFail) != SUCCESS)
	{
		return FAIL;
	}

	swd_writeAPResource(FLASH_OPTKEYR, FLASH_OPTKEY1);
	swd_writeAPResource(FLASH_OPTKEYR, FLASH_OPTKEY2);
	if (CheckFlashStatusAll(FLASH_CR, 0, FLASH_CR_LOCK | FLASH_CR_OPTLOCK,E_ProtectFail) != SUCCESS)
	{
		swd_writeAPResource(FLASH_CR, FLASH_CR_LOCK);
		return FAIL;
	}

	swd_writeAPResource(FLASH_SR, FLASH_OPTION_CLEAR_MASK);
	swd_writeAPResource(FLASH_WRP1AR, wrp1ar_reg);
	swd_writeAPResource(FLASH_WRP1BR, wrp1br_reg);
	swd_writeAPResource(FLASH_OPTR, optr);

	if (WaitFlashStatusAll(0, FLASH_OPTION_BUSY_MASK,FLASH_WAIT_LIMIT, E_ProtectFail) != SUCCESS)
	{
		swd_writeAPResource(FLASH_CR, FLASH_CR_OPTLOCK | FLASH_CR_LOCK);
		return FAIL;
	}

	swd_writeAPResource(FLASH_CR, 0x00020000UL); /* OPTSTRT: program Option Bytes. */
	Delay_1ms(50);

	if (WaitFlashStatusAll(0, FLASH_OPTION_BUSY_MASK,FLASH_WAIT_LIMIT, E_ProtectFail) != SUCCESS)
	{
		swd_writeAPResource(FLASH_CR, FLASH_CR_OPTLOCK | FLASH_CR_LOCK);
		return FAIL;
	}

	if (CheckFlashStatusAll(FLASH_SR, 0,FLASH_OPTION_ERROR_MASK, E_ProtectFail) != SUCCESS)
	{
		swd_writeAPResource(FLASH_SR, FLASH_OPTION_CLEAR_MASK);
		swd_writeAPResource(FLASH_CR, FLASH_CR_OPTLOCK | FLASH_CR_LOCK);
		return FAIL;
	}

	swd_writeAPResource(FLASH_SR, FLASH_SR_EOP);
	swd_writeAPResource(FLASH_CR, 0x08000000UL); /* OBL_LAUNCH: reload and reset. */
	Delay_1ms(50);

	PowerOffFun();
	Delay_1ms(50);
	if ((optr & PROTECT_RDP_MASK) != 0x000000AAUL)
	{
		return SUCCESS;
	}

	if (PowerOnFun() != SUCCESS)
	{
		SysStatusCode = E_ProtectFail;
		return FAIL;
	}

	if (VerifyOptionRegistersAll(optr, wrp1ar_reg, wrp1br_reg,E_ProtectFail) != SUCCESS)
	{
		return FAIL;
	}

	return SUCCESS;
}

/**********************************************************\
	UnProtectFun -- Restore RDP Level 0 and disable both WRP areas
\**********************************************************/
uChar UnProtectFun(void)
{
	uInt32 optr;
	uInt32 wrp1ar;
	uInt32 wrp1br;

	optr = 0xDFFFE1AAUL;   /* RDP Level 0 with default user options. */
	wrp1ar = 0x0000001FUL; /* Disable WRP area A. */
	wrp1br = 0x0000001FUL; /* Disable WRP area B. */

	PowerOffFun();
	Delay_1ms(50);
	if (PowerOnFun() != SUCCESS)
	{
		SysStatusCode = E_UnprotectFail;
		return FAIL;
	}

	if (WaitFlashStatusAll(0, FLASH_OPTION_BUSY_MASK,FLASH_WAIT_LIMIT, E_UnprotectFail) != SUCCESS)
	{
		return FAIL;
	}

	swd_writeAPResource(FLASH_KEYR, FLASH_KEY1);
	swd_writeAPResource(FLASH_KEYR, FLASH_KEY2);
	if (CheckFlashStatusAll(FLASH_CR, 0, FLASH_CR_LOCK, E_UnprotectFail) != SUCCESS)
	{
		return FAIL;
	}

	swd_writeAPResource(FLASH_OPTKEYR, FLASH_OPTKEY1);
	swd_writeAPResource(FLASH_OPTKEYR, FLASH_OPTKEY2);
	if (CheckFlashStatusAll(FLASH_CR, 0, FLASH_CR_LOCK | FLASH_CR_OPTLOCK,E_UnprotectFail) != SUCCESS)
	{
		swd_writeAPResource(FLASH_CR, FLASH_CR_LOCK);
		return FAIL;
	}

	swd_writeAPResource(FLASH_SR, FLASH_OPTION_CLEAR_MASK);
	swd_writeAPResource(FLASH_WRP1AR, wrp1ar);
	swd_writeAPResource(FLASH_WRP1BR, wrp1br);
	swd_writeAPResource(FLASH_OPTR, optr);

	if (WaitFlashStatusAll(0, FLASH_OPTION_BUSY_MASK,FLASH_WAIT_LIMIT, E_UnprotectFail) != SUCCESS)
	{
		swd_writeAPResource(FLASH_CR, FLASH_CR_OPTLOCK | FLASH_CR_LOCK);
		return FAIL;
	}

	swd_writeAPResource(FLASH_CR, 0x00020000UL); /* OPTSTRT: program Option Bytes. */
	Delay_1ms(50);

	if (WaitFlashStatusAll(0, FLASH_OPTION_BUSY_MASK,FLASH_WAIT_LIMIT, E_UnprotectFail) != SUCCESS)
	{
		swd_writeAPResource(FLASH_CR, FLASH_CR_OPTLOCK | FLASH_CR_LOCK);
		return FAIL;
	}

	if (CheckFlashStatusAll(FLASH_SR, 0,FLASH_OPTION_ERROR_MASK, E_UnprotectFail) != SUCCESS)
	{
		swd_writeAPResource(FLASH_SR, FLASH_OPTION_CLEAR_MASK);
		swd_writeAPResource(FLASH_CR, FLASH_CR_OPTLOCK | FLASH_CR_LOCK);
		return FAIL;
	}

	swd_writeAPResource(FLASH_SR, FLASH_SR_EOP);
	swd_writeAPResource(FLASH_CR, 0x08000000UL); /* OBL_LAUNCH: reload and reset. */
	Delay_1ms(50);

	PowerOffFun();
	Delay_1ms(50);
	if (PowerOnFun() != SUCCESS)
	{
		SysStatusCode = E_UnprotectFail;
		return FAIL;
	}

	if (VerifyOptionRegistersAll(optr, wrp1ar, wrp1br,E_UnprotectFail) != SUCCESS)
	{
		return FAIL;
	}

	return SUCCESS;
}
/**********************************************************\
		CheckSumFun -- Checksum calculation
\**********************************************************/
uChar CheckSumFun(void)
{
	uShort TempData;
	uInt32 ChecksumAll;
	ChecksumAll = (StdChecksumHi << 16) + StdChecksumLo; /* get hi-lo 32 bit checksum data */
	while (1)
	{
		TempData = CFCardReadWordData();
		ChecksumAll += LSB(TempData);
		DramBufCnt--;	
		DevCurrBlkLen.all--;
		if (GetChkBlkLenEnd()) /* checking block length end */
		{
			StdChecksumLo = LSW(ChecksumAll);
			StdChecksumHi = MSW(ChecksumAll);
			return SUCCESS;
		}

		ChecksumAll += MSB(TempData);
		DramBufCnt--;
		DevCurrBlkLen.all--;
		if (GetChkBlkLenEnd()) /* checking block length end */
		{
			StdChecksumLo = LSW(ChecksumAll);
			StdChecksumHi = MSW(ChecksumAll);
			return SUCCESS;
		}
	}
}


/******************************************************/
void DevSNProgAndVerify(void)
{
}
