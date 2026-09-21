

#ifndef Subroutine_H_
#define Subroutine_H_

#include "Type.h"
#include "2450addr.h"

//////////////////////////////////////////////////////////////////
#define	EngineerDebugMachine		1 /* for Engineer debug, when release NEED Disable! */
//////////////////////////////////////////////////////////////////
#define	CFMulSect				1
#define	FW_LIB_VERSION		(0x04)


/*********************************************************************/
/*********************************************************************/
#define MSB(word)		(uChar)(((uShort)(word) >> 8) & 0xFF)
#define LSB(word)		(uChar)((uShort)(word) & 0xFF)
#define MSW(int_data)	(uShort)(((uInt32)(int_data) >> 16) & 0xFFFF)
#define LSW(int_data)		(uShort)((uInt32)(int_data) & 0xFFFF)


/*********************************************************************/
/**************** CPLD Control Port Decodde *************************/
/*********************************************************************/
#define	DIS_SEL_PORT		00 /* GPH12~GPH8 for decode port, GPH7~GPH2 decode parameter */
#define	RD_MB_SC			(0x01<<8)/* 5Sel+RST/CS/CLK/SDI/WE/CS2+SDO */
#define	RD_DIP48_SC		(0x02<<8)
#define	RD_CSKB_SC			(0x03<<8)
#define	RD_MSKB_SC			(0x04<<8)
#define	RD_CSKB_EN			(0x05<<8)
#define	WE_SKB_LED		(0x06<<8)
#define	WE_VSS_PIN			(0x07<<8)
#define	WE_VCC_PIN			(0x08<<8)
#define	WE_VPP_PIN			(0x09<<8)
#define	WE_VIO_PIN			(0x0A<<8)
#define	WE_DAC				(0x0B<<8)
#define	WE_SYS_LED		(0x0C<<8)
#define	WE_BUZZER			(0x0D<<8)
#define	WE_LCD				(0x0E<<8)
#define	SEL_SPI_PORT		(0x0F<<8)
#define	WE_NAND_PORT		(0x10<<8)
#define	WE_EMMC_PORT		(0x11<<8)
#define	WE_FPGA_CFG		(0x12<<8)
#define	WE_CF_PWR_PIN		(0x13<<8)
#define	RD_CF_DET_PIN		(0x14<<8)
#define	WE_ATE				(0x15<<8)
#define	RD_GANG			(0x16<<8)
#define	WE_CF_RST_PIN		(0x17<<8)
#define	WE_CF_CS2_PIN		(0x18<<8)
#define	WE_TIME_PIN		(0x19<<8)

/***** 0x1B ~ Max 1F for reserve *****/



/*********************************************************************/
/**************** CPLD Control Port Define *************************/
/*********************************************************************/
#define	S_CS2		(1<<2) /* GPHxx */
#define	S_WE		(1<<3)
#define	S_SDI		(1<<4)
#define	S_CLK		(1<<5)
#define	S_CS		(1<<6)
#define	S_RST		(1<<7)
#define	S_SEL0		(1<<8)
#define	S_SEL1		(1<<9)
#define	S_SEL2		(1<<10)
#define	S_SEL3		(1<<11)
#define	S_SEL4		(1<<12)
#define	S_SDO		(1<<4) /* GPE4 */

#define	AllSelPort	(S_SEL4|S_SEL3|S_SEL2|S_SEL1|S_SEL0)
#define	AllDataPort	(S_RST|S_CS|S_CLK|S_SDI|S_WE|S_CS2)

#define	SendCS2_Hi()		(rGPHDAT |= S_CS2)
#define	SendCS2_Lo()		(rGPHDAT &= (~S_CS2))
#define	SendWE_Hi()			(rGPHDAT |= S_WE)
#define	SendWE_Lo()		(rGPHDAT &= (~S_WE))
#define	SendSDI_Hi()		(rGPHDAT |= S_SDI)
#define	SendSDI_Lo()		(rGPHDAT &= (~S_SDI))
#define	SendCLK_Hi()		(rGPHDAT |= S_CLK)
#define	SendCLK_Lo()		(rGPHDAT &= (~S_CLK))
#define	SendCS_Hi()			(rGPHDAT |= S_CS)
#define	SendCS_Lo()			(rGPHDAT &= (~S_CS))
#define	SendRST_Hi()		(rGPHDAT |= S_RST)
#define	SendRST_Lo()		(rGPHDAT &= (~S_RST))
#define	SendSEL0_Hi()		(rGPHDAT |= S_SEL0)
#define	SendSEL0_Lo()		(rGPHDAT &= (~S_SEL0))
#define	SendSEL1_Hi()		(rGPHDAT |= S_SEL1)
#define	SendSEL1_Lo()		(rGPHDAT &= (~S_SEL1))
#define	SendSEL2_Hi()		(rGPHDAT |= S_SEL2)
#define	SendSEL2_Lo()		(rGPHDAT &= (~S_SEL2))
#define	SendSEL3_Hi()		(rGPHDAT |= S_SEL3)
#define	SendSEL3_Lo()		(rGPHDAT &= (~S_SEL3))
#define	SendSEL4_Hi()		(rGPHDAT |= S_SEL4)
#define	SendSEL4_Lo()		(rGPHDAT &= (~S_SEL4))

#define	DisSelAllPort()		(rGPHDAT &= (~(AllDataPort|AllSelPort))) /* all port be low status!! */
#define	DisSelPort()			(rGPHDAT &= (~(AllSelPort)))
#define	SendSelPort(_Sel_)	(rGPHDAT |= _Sel_)

#define	ReadSDOData()		(rGPEDAT & S_SDO)


///////////////////////////////////////////////////////
#define	Delay1usCnt			100 /* 400M system clock, 100 scale for MMU */
#define	DecodeTime			(1)
#define	RegDecodeTime		(1)
#define	DecodeDelay(_time_)	(Delay_1us(_time_))






/**********************************************************************/
/******* CPU Control Port Setting Decode  **********************************/
/**********************************************************************/
/******* FPGA cfg control *************/
#define	ReadCFGINIT()			ReadSDOData()
#define	ReadCFGDone()			ReadSDOData()
#define	SetReadCFGDone()		SendSDI_Hi() /* SDI hi to read cfg-done pin */
#define	SetReadCFGINIT()		SendSDI_Lo() /* PD186 */

#define	SendCFGCS2_Hi()		SendCS2_Hi() /* 2# fpga */
#define	SendCFGCS2_Lo()		SendCS2_Lo()
#define	SendCFGCS_Hi()			SendCS_Hi()
#define	SendCFGCS_Lo()			SendCS_Lo()
#define	SendCFGRDWR_Hi()		SendWE_Hi() /* PD188 */
#define	SendCFGRDWR_Lo()		SendWE_Lo()
#define	SendCFGPROG_Hi()		SendRST_Hi()
#define	SendCFGPROG_Lo()		SendRST_Lo()
#define	SendCFGCCLK_Hi()		SendCLK_Hi()
#define	SendCFGCCLK_Lo()		SendCLK_Lo()



/******* socket board PCC & ID checking control *************/
#define	SOCKET_A		0x00 /* B2~0 = S_RST,S_CS,S_CLK to select port */
#define	SOCKET_B		0x01
#define	SOCKET_C		0x02
#define	SOCKET_D		0x03
#define	SOCKET_E		0x04 /* socket E,F,G,H reserve for feature using */
#define	SOCKET_F		0x05
#define	SOCKET_G		0x06
#define	SOCKET_H		0x07

/****** socket borad led control **************/
#define	SKBLED_STR_Hi()			SendCS_Hi()
#define	SKBLED_STR_Lo()		SendCS_Lo()
#define	SKBLED_CLK_Hi()			SendCLK_Hi()
#define	SKBLED_CLK_Lo()		SendCLK_Lo()
#define	SKBLED_SDA_Hi()		SendSDI_Hi()
#define	SKBLED_SDA_Lo()		SendSDI_Lo()


/******** Below for XP8 Programmer ***********************/
/******* read & write driving board checking control *************/
#define	VSS_STR_Hi()			SendCS_Hi()
#define	VSS_STR_Lo()			SendCS_Lo()
#define	VSS_CLK_Hi()			SendCLK_Hi()
#define	VSS_CLK_Lo()			SendCLK_Lo()
#define	VSS_SDA_Hi()			SendSDI_Hi()
#define	VSS_SDA_Lo()			SendSDI_Lo()

#define	VCC_OE_Hi()				SendWE_Hi()
#define	VCC_OE_Lo()				SendWE_Lo()
#define	VCC_STR_Hi()			SendCS_Hi()
#define	VCC_STR_Lo()			SendCS_Lo()
#define	VCC_CLK_Hi()			SendCLK_Hi()
#define	VCC_CLK_Lo()			SendCLK_Lo()
#define	VCC_SDA_Hi()			SendSDI_Hi()
#define	VCC_SDA_Lo()			SendSDI_Lo()

#define	VPP_OE_Hi()				SendWE_Hi()
#define	VPP_OE_Lo()				SendWE_Lo()
#define	VPP_STR_Hi()			SendCS_Hi()
#define	VPP_STR_Lo()			SendCS_Lo()
#define	VPP_CLK_Hi()			SendCLK_Hi()
#define	VPP_CLK_Lo()			SendCLK_Lo()
#define	VPP_SDA_Hi()			SendSDI_Hi()
#define	VPP_SDA_Lo()			SendSDI_Lo()

#define	VIO_OE_Hi()				SendWE_Hi()
#define	VIO_OE_Lo()				SendWE_Lo()
#define	VIO_STR_Hi()			SendCS_Hi()
#define	VIO_STR_Lo()			SendCS_Lo()
#define	VIO_CLK_Hi()			SendCLK_Hi()
#define	VIO_CLK_Lo()			SendCLK_Lo()
#define	VIO_SDA_Hi()			SendSDI_Hi()
#define	VIO_SDA_Lo()			SendSDI_Lo()
/*********************************************************/
/*********************************************************/


/******** Below for XP1 Programmer ***********************/
/******* read & write driving board checking control *************/
#define	VSS_STR1_Hi()			(rGPBDAT |= (1<<9)) /* GPB9 */
#define	VSS_STR1_Lo()			(rGPBDAT &= (~(1<<9)))
#define	VSS_CLK1_Hi()			(rGPBDAT |= (1<<10)) /* GPB10 */
#define	VSS_CLK1_Lo()			(rGPBDAT &= (~(1<<10)))
#define	VSS_SDA1_Hi()			(rGPEDAT |= (1<<3)) /* GPE3 */
#define	VSS_SDA1_Lo()			(rGPEDAT &= (~(1<<3)))

#define	VCC_OE1_Hi()			(rGPBDAT |= (1<<1)) /* GPB1 */
#define	VCC_OE1_Lo()			(rGPBDAT &= (~(1<<1)))
#define	VCC_STR1_Hi()			(rGPBDAT |= (1<<2)) /* GPB2 */
#define	VCC_STR1_Lo()			(rGPBDAT &= (~(1<<2)))
#define	VCC_CLK1_Hi()			(rGPBDAT |= (1<<3)) /* GPB3 */
#define	VCC_CLK1_Lo()			(rGPBDAT &= (~(1<<3)))
#define	VCC_SDA1_Hi()			(rGPEDAT |= (1<<3)) /* GPE3 */
#define	VCC_SDA1_Lo()			(rGPEDAT &= (~(1<<3)))

#define	VPP_OE1_Hi()			(rGPBDAT |= (1<<4)) /* GPB4 */
#define	VPP_OE1_Lo()			(rGPBDAT &= (~(1<<4)))
#define	VPP_STR1_Hi()			(rGPBDAT |= (1<<5)) /* GPB5 */
#define	VPP_STR1_Lo()			(rGPBDAT &= (~(1<<5)))
#define	VPP_CLK1_Hi()			(rGPBDAT |= (1<<6)) /* GPB6 */
#define	VPP_CLK1_Lo()			(rGPBDAT &= (~(1<<6)))
#define	VPP_SDA1_Hi()			(rGPEDAT |= (1<<3)) /* GPE3 */
#define	VPP_SDA1_Lo()			(rGPEDAT &= (~(1<<3)))

#define	VIO_OE1_Hi()			(rGPEDAT |= (1<<0)) /* GPE0 */
#define	VIO_OE1_Lo()			(rGPEDAT &= (~(1<<0)))
#define	VIO_STR1_Hi()			(rGPEDAT |= (1<<1)) /* GPE1 */
#define	VIO_STR1_Lo()			(rGPEDAT &= (~(1<<1)))
#define	VIO_CLK1_Hi()			(rGPEDAT |= (1<<2)) /* GPE2 */
#define	VIO_CLK1_Lo()			(rGPEDAT &= (~(1<<2)))
#define	VIO_SDA1_Hi()			(rGPEDAT |= (1<<3)) /* GPE3 */
#define	VIO_SDA1_Lo()			(rGPEDAT &= (~(1<<3)))
/*********************************************************/
/*********************************************************/


/********** 6 keys input Contro ********************/
#define	Read_KEY1_ENTER()		(rGPFDAT & (1<<5))
#define	Read_KEY2_EXIT()		(rGPFDAT & (1<<6))
#define	Read_KEY5_NEXT()		(rGPFDAT & (1<<7))
#define	Read_KEY6_PREV()		(rGPGDAT & (1<<0))
#define	Read_KEY3_INC()		(rGPGDAT & (1<<1))
#define	Read_KEY4_DEC()		(rGPGDAT & (1<<2))

/*Standard Key Definitions: */
#define	KEY0_NONE		0x00
#define	KEY1_ENTER		0x01
#define	KEY2_EXIT		0x02
#define	KEY3_INC		0x03
#define	KEY4_DEC		0x04
#define	KEY5_NEXT		0x05
#define	KEY6_PREV		0x06
#define	KEY7_Resv		0x07
#define	KEY8_Resv		0x08


/******* LED status Control **********/
#define	RedLedOn()			SendRST_Lo() /* 1=off, 0=on */
#define	RedLedOff()			SendRST_Hi()
#define	YellowLedOn()		SendCS_Lo()	
#define	YellowLedOff()		SendCS_Hi()
#define	GreenLedOn()		SendCLK_Lo()
#define	GreenLedOff()		SendCLK_Hi()


/****** Beep Sound Control ***********/
#define	BeepOn()			SendRST_Hi() /* 1=on, 0=off */
#define	BeepOff()			SendRST_Lo()


/****** Main board LCD Control ***************/
#define	LCDSCK_Hi()			SendCLK_Hi()
#define	LCDSCK_Lo()			SendCLK_Lo()
#define	LCDRST_Hi()			SendRST_Hi()
#define	LCDRST_Lo()			SendRST_Lo()
#define	LCDSDI_Hi()			SendSDI_Hi()
#define	LCDSDI_Lo()			SendSDI_Lo()
#define	LCDCS_Hi()			SendCS_Hi()
#define	LCDCS_Lo()			SendCS_Lo()
#define	LCDA0_Hi()			SendWE_Hi()
#define	LCDA0_Lo()			SendWE_Lo() /* 0=cmd, 1=data */


/****** eMMC & Nand Buffer Control ***************/
#define	Dis_SelSPI			0
#define	SPI25_SO8			1
#define	SPI45_SO8			2
#define	SPI25_SO16			3

#define	Sel_SPI25_SO8()		(rGPHDAT |= (S_CS|S_CLK)) /* S_RST=0 Enable */
#define	Sel_SPI45_SO8()		(rGPHDAT |= (S_RST|S_CLK)) /* S_CS=0 Enable */
#define	Sel_SPI25_SO16()	(rGPHDAT |= (S_RST|S_CS)) /* S_CLK=0 Enable */
#define	Dis_Sel_SPI()		(rGPHDAT |= (S_RST|S_CLK|S_CS)) /* all=1 Disable all */


/****** eMMC & Nand Buffer Control ***************/
#define	EMMC_EN()			SendCS_Lo() /* 0=enable */
#define	EMMC_DisEN()		SendCS_Hi()
#define	NAND_EN()			SendCS_Lo()
#define	NAND_DisEN()		SendCS_Hi()


/****** DAC power Control ***************/
#define	DAC_RD_SDI()		SendWE_Hi()
#define	DAC_WE_SDI()		SendWE_Lo()
#define	DAC_SEL_ALT()		SendCS_Hi()
#define	DAC_SEL_SDI()		SendCS_Lo()
#define	DAC_SCK_Hi()		SendCLK_Hi()
#define	DAC_SCK_Lo()		SendCLK_Lo()
#define	DAC_SDI_Hi()		SendSDI_Hi()
#define	DAC_SDI_Lo()		SendSDI_Lo()


/****** Timer Port Control ***************/
#define	TIME_RD_DAT()		SendWE_Hi()
#define	TIME_WE_DAT()		SendWE_Lo()
#define	TIME_CLK_Hi()		SendCLK_Hi()
#define	TIME_CLK_Lo()		SendCLK_Lo()
#define	TIME_DAT_Hi()		SendSDI_Hi()
#define	TIME_DAT_Lo()		SendSDI_Lo()


/****** ATE single Control ***************/
#define	ATE_OK_Hi()			SendRST_Hi() /* 1=Enable */
#define	ATE_OK_Lo()			SendRST_Lo()
#define	ATE_NG_Hi()			SendCS_Hi()
#define	ATE_NG_Lo()		SendCS_Lo()
#define	ATE_BY_Hi()			SendCLK_Hi()
#define	ATE_BY_Lo()			SendCLK_Lo()


///////////////////////////////////////////////////////////////
/* DAC command */
#define	DACVihAddr			0x60 /* Vih */
#define	DACVccAddr			0x62 /* Vcc */
#define	DACVppAddr			0x64 /* Vpp */
#define	DACVioAddr			0x66 /* Vio */

#define	DACRegVOUT_L		0x00 /* DAC reg, 10mV step */
#define	DACRegVOUT_H		0x01 /* DAC reg */
#define	DACRegVOUT_Go		0x02 /* DAC reg */
#define	DACRegIOUT_LIM	0x03 /* DAC reg, 50mA step */
#define	DACRegCTRL1		0x04 /* DAC reg */
#define	DACRegCTRL2		0x05 /* DAC reg */
#define	DACRegStatus		0x09 /* chk dac status over ? */
#define	DACRegIntp			0x0A
#define	DACRegMask			0x0B /* ALT mask */

#define	DACGoBitStatus		0x01 /* for chk DAC finish */
#define	DACI2C_Delay		5 /* 3xus for <500K */


/******** 10mV per scale **************************************/
#define	MaxIccDAC			(1000/50) /* 1000mA, 50mA per scale */
#define	MaxVihDAC			380 /* 3.8V *//* 10mV per scale */
#define	MinVihDAC			120 /* 1.0V */
#define	IdleVihDAC			180 /* MUST >=1.8V to ensure fpga can work */
//#define	VihDACDiode		30 /* decrease for 0.55V for diode */
#define	MaxVccDAC			600 /* Max 6V */
#define	IdleVccDAC			100 /* 1V for idle initial */
#define	VccDACDiode		30 /* increase 0.3V for diode */
#define	MaxVppDAC			1500 /* Max 15V */
#define	MaxVpeDAC			1500 /* Max 15V */
#define	MaxVpsDAC			1500 /* Max 15V */
#define	IdleVppesDAC		100 /* 1V for idle initial */
#define	VppesDACDiode		30 /* increase 0.3V for diode */
#define	MaxVpioDAC			600 /* Max 6V */
#define	IdleVpioDAC			100 /* 1V for idle initial */
#define	VpioDACDiode		30 /* increase 0.3V for diode */

#define	MaxPCCVolt			240 /* Pin Connect Check use 2.4V for protect */
#define	PCCShortVolt		190


///////////////////////////////////////////////////////////
/* Pin Connect Check */
#define	PCCAllPinPass		0x00
#define	PCCVccPinFail		0x01
#define	PCCGndPinFail		0x02
#define	PCCDioPinFail		0x04
#define	PCCAllPinFail		0xFF
#define	PCCMinPassCnt		6 /* these two macro define for stable pcc checking */
#define	PCCMinFailCnt		6 /* about 70ms per time & (PCCMinPassCnt+PCCMinFailCnt) <=0xFF) */
#define	PCCGangDecCnt		5 /* Gang socket check less times */


/************ Checking Gang ID Staus ************/
#define	GangPort0			(0x00) /* b1~0 for S_CS,S_CLK to select port */
#define	GangPort1			(0x01)
#define	GangPort2			(0x02)




/**********************************************************************/
/***** Each Bank Offset Address ******/
/**********************************************************************/
#define	NorFlashOffset		0x00000000 // Bank0, bank0~5 each bank only have 64M Bytes, not 128M Bytes
#define	SystemOffset		0x08000000 // Bank1
#define	CFCardOffset		0x10000000 // Bank2
#define	FPGAOffset			0x18000000 // Bank3
#define	EBI0Offset			0x20000000 // Bank4
#define	EBI1Offset			0x28000000 // Bank5
#define	Sdram0Offset		0x30000000 // Bank6, bank6~7 each bank have 128M Bytes
#define	Sdram1Offset		0x38000000 // Bank7
#define	CpuSramOffset		0x40000000


/**********************************************************************/
/***** CPLD address & data decode setting for FPGA  ******/
/***** All Address OFFSET BY 2 for word width!!!! ********/
/**********************************************************************/
/* Port 0x0000 ~ 0x0FFF for Device Data setting decode                                         */
/**********************************************************************/
/********* Nor(ADM) Flash *********************/
#define	DevDataLoInc0			(FPGAOffset|0x0000<<1)
#define	DevDataLoInc1			(FPGAOffset|0x0100<<1)
#define	DevDataLoDec1			(FPGAOffset|0x0200<<1)
#define	DevCmdxAddr55			(FPGAOffset|0x0500<<1)
#define	DevAddrDataCmd		(FPGAOffset|0x0600<<1) // for ADM nor flash */
#define	DevCmdxAddrAA			(FPGAOffset|0x0A00<<1)
#define	GDevChkDataLoInc0		(FPGAOffset|0x0E00<<1)
#define	GDevChkDataLoInc1		(FPGAOffset|0x0F00<<1)

/***************** Nand Flash **********************/
#define	DevDataCmd				(FPGAOffset|0x0000<<1)
#define	DevCtrlCmd				(FPGAOffset|0x0500<<1)
#define	DevAddrCmd			(FPGAOffset|0x0600<<1)

/***************** SPI Flash **********************/
#define	StartReadData			(FPGAOffset|0x0000<<1)
#define	OneDataCmd				(FPGAOffset|0x0100<<1) /* for SPI, Read Device Data from Fpga */



/********************************************************************/
/* Port 0x8000 ~ 0xFFFF for FPGA & Device Parameter decode                             */
/********************************************************************/
//#define	SetFpgaPara				(FPGAOffset|0x8000<<1)
#define	SetRSTPinCmd			(FPGAOffset|0x8000<<1) /* 0x8000 for pin control */
#define	SetVPPPinCmd			(FPGAOffset|0x8100<<1)
#define	SetADVPinCmd			(FPGAOffset|0x8200<<1)
#define	SetCSPinCmd			(FPGAOffset|0x8300<<1)
#define	SetCLEPinCmd			(FPGAOffset|0x8400<<1)
#define	SetALEPinCmd			(FPGAOffset|0x8500<<1)

#define	SetAddrLoWord			(FPGAOffset|0x9000<<1) /* for device parameter setting */
#define	SetAddrHiWord			(FPGAOffset|0x9100<<1)
#define	SetDevPageLth			(FPGAOffset|0x9200<<1)
#define	DevAddxAddr			(FPGAOffset|0x9300<<1)

#define	GSetAllErrSckMask		(FPGAOffset|0xA000<<1) /* set fpga all error or NoUse socket mask flag */
#define	GSetAllLoErrSckMask		(FPGAOffset|0xA000<<1) /* set fpga all error or NoUse socket mask flag */
#define	GSetAllMaskDataLo		(FPGAOffset|0xA100<<1) /* mask data check bits */
#define	GEnSelSckNum			(FPGAOffset|0xA200<<1) /*  1 = enable socket , 0 = disable socket */
#define	GChkAllSckStatus		(FPGAOffset|0xA300<<1) /* fpga checking data and fee-back result, 0=pass, 1=fail */
#define	GChkAllSckLoStatus		(FPGAOffset|0xA300<<1) /* fpga checking data and fee-back result, 0=pass, 1=fail */
#define	GSetPageChkCnt			(FPGAOffset|0xA400<<1)
#define	GClrPageChkStatus		(FPGAOffset|0xA500<<1) /* clear page check status */
#define	GClrPageChkLoStatus		(FPGAOffset|0xA500<<1) /* clear page check status */
#define	GGetSckxData			(FPGAOffset|0xA600<<1) /* get each socket data for check */
#define	GChkAllSckErrCnt		(FPGAOffset|0xA700<<1) /* check error data cnt per ECC group */
#define	GChkAllSckErrData		(FPGAOffset|0xA800<<1) /* check error data per ECC group */

#define	GSetAllHiErrSckMask		(FPGAOffset|0xA400<<1) /* set fpga all error or NoUse socket mask flag */
#define	GClrPageChkHiStatus		(FPGAOffset|0xA600<<1) /* clear page check status */
#define	GChkAllSckHiStatus		(FPGAOffset|0xA900<<1) /* fpga checking data and fee-back result, 0=pass, 1=fail */

#define	GEnSelGangBoard		(FPGAOffset|0xAF00<<1) /* 1 = enable FpgaBoard for 16 socket , 0 = disable FpgaBoard */
#define	EnAllBoardFlag			0xFF /* each bit status for each FpgaBoard */
#define	EnABoardFlag			0x01 /*  1 = enable FpgaBoard, 0 = disable FpgaBoard */
#define	EnBBoardFlag			0x02
#define	EnCBoardFlag			0x04
#define	EnDBoardFlag			0x08
#define	EnEBoardFlag			0x10
#define	EnFBoardFlag			0x20
#define	EnGBoardFlag			0x40
#define	EnHBoardFlag			0x80


//////////////////////////////////////////////////////////
#define	FPGAEnAllPin			(FPGAOffset|0xE000<<1)
#define	FPGAComChk5A5A		(FPGAOffset|0xE000<<1) /* check to confirm FPGA communication prot ok */
#define	FPGAComChkA5A5		(FPGAOffset|0xF000<<1) /* check to confirm FPGA communication prot ok */
#define	FPGADisAllPin			(FPGAOffset|0xF000<<1)



/*********************************************************/
/***** Port 0x7000~0x7FFF for DIO Mode Pin Setting Define  ******/
/***** DIO mode use Byte width FPGA setting !!!!              ******/
/********************************************************/
#define	SetClkPinFreq		(FPGAOffset|0x7000<<1) /* for setting FPGA output frequence divider */
#define	Freq0MHz			0 /* output clk = Max Freq=66MHz /2 /x */
#define	Freq3M3Hz			10
#define	Freq5M5Hz			6
#define	Freq6M6Hz			5
#define	Freq8M25Hz			4
#define	Freq11MHz			3
#define	Freq16M5Hz			2
#define	Freq33MHz			1 /* output clock max frequency be 66MHz */

///////////////////////////////////////////////////////////////////////////////////
/*****   address bit 7~0 for total 144 pin define   ********/
#define	VccPinMinNum		(FPGAOffset|0x7000<<1) /* 0x01 ~ 0x90 for Pin1~Pin144 control */
#define	VppPinMinNum		(FPGAOffset|0x7100<<1) /* 0x01 ~ 0x90 for Pin1~Pin144 control */
#define	VioPinMinNum		(FPGAOffset|0x7200<<1) /* 0x01 ~ 0x90 for Pin1~Pin144 control */
#define	DirPinMinNum		(FPGAOffset|0x7300<<1) /* 0x01 ~ 0x90 for Pin1~Pin144 control */
#define	ZPinMinNum			(FPGAOffset|0x7400<<1) /* 0x01 ~ 0x90 for Pin1~Pin144 control */
#define	ClkPinMinNum		(FPGAOffset|0x7500<<1) /* 0x01 ~ 0x90 for Pin1~Pin144 control */
#define	GndPinMinNum		(FPGAOffset|0x7600<<1) /* 0x01 ~ 0x90 for Pin1~Pin144 control */

#define	FPGACFGPort		(FPGAOffset|0x7FFF<<1) /* for FPGA CFG setting, CANNOT for other use!!! */




/**********************************************************************/
/*** 0x0000~0x00FF for CF Card Port Setting                                    ************/
/**********************************************************************/
#define	CFCardCtrlMinAddr	(CFCardOffset|0x0000<<1)
#define	CFCardMemMapPort	(CFCardOffset|0x0000<<1) /* A10 ~ A4 should be 0!! A3 ~ A0 be command reg addr */
#define	CFCardCtrlMaxAddr	(CFCardOffset|0x00FF<<1)





/*********************************************************/
/******* 29 serial flash CMD macro definition *******************/
/********************************************************/
#define	ChipErase29FCmdx10		0x10
#define	UlckBps29FCmdx20			0x20
#define	EntFastPrgModeCmdx20		0x20
#define	StErase29FCmdx30			0x30
#define	Lock29FCmdx40				0x40
#define	RdProtect29FCmdx40			0x40
#define	WrProtect29FCmdx60		0x60
#define	Erase29FCmdx80			0x80
#define	EntSecSiSt29FCmdx88		0x88
#define	ExtSecSiSt29FCmdx90		0x90
#define	EntRdID29FCmdx90			0x90
#define	DataProg29FCmdxA0			0xA0
#define	ExtRdID29FCmdxF0			0xF0
#define	Reset29FCmdxF0			0xF0


/******************************************************/
/******** 28 serial flash CMD macro definition ***************/
/******************************************************/
#define	BlkLock28FCmdx01			0x01
#define	ProgOne28FCmdx10			0x10
#define	BlkErase28FCmdx20			0x20
#define	EFABlkErase28FCmdx24		0x24
#define	BlkLockDn28FCmdx2F		0x2F
#define	ProgDoub28FCmdx30			0x30
#define	ProgOne28FCmdx40			0x40
#define	ProgQuad28FCmdx56			0x56
#define	EntBlkLock28FCmdx60		0x60
#define	EntEFABlkLock28FCmdx64	0x64
#define	StatusReg28FCmdx70		0x70
#define	EntEFAMode28FCmdx80		0x80
#define	EntRdID28FCmdx98			0x98
#define	PrtRegProg28FCmdxC0		0xC0
#define	Confirm28FCmdxD0			0xD0
#define	BlkUnLock28FCmdxD0		0xD0
#define	ReadFlash28FCmdxFF		0xFF


/******************************************************/
/********* 25 serial flash CMD macro definition **************/
/******************************************************/
#define	WriteEnableCmdx06		0x06
#define	WriteDisableCmdx04		0x04
#define	ReadStatusCmdx05		0x05
#define	WriteStatusCmdx01		0x01
#define	ReadDataCmdx03		0x03
#define	FastReadDataCmdx0B	0x0B
#define	PageProgCmdx02		0x02
#define	SectorEraseCmdx20		0x20
#define	BulkEraseCmdxD8		0xD8
#define	ChipEraseCmdxC7		0xC7
#define	PwrDownCmdxD9		0xD9
#define	ReadEleSignCmdxAB		0xAB
#define	ReadIDCmdx90			0x90

#define	EntSecureOTPxB1		0xB1
#define	ExitSecureOTPxC1		0xC1
#define	ReadSecureLockx2B		0x2B
#define	WriteSecureLockx2F		0x2F
#define	EnSOOutputRYx70		0x70
#define	DisSOOutputRYx80		0x80


/**********************************************************/
/********* Nand flash CMD macro definition ******************/
/**********************************************************/
#define	ResetFlashCmdxFF			0xFF
#define	Read1FlashCmdx00			0x00
#define	Read2FlashCmdx30			0x30
#define	ReadIDCmdx90				0x90
#define	PageProgCmdx80			0x80
#define	PageProgCnfCmdx10			0x10
#define	CopyBackProgCmdx00		0x00
#define	CopyBackProgCnfCmdx8A		0x8A
#define	BlockEraseCmdx60			0x60
#define	BlockEraseCnfCmdxD0		0xD0
#define	ReadStatusCmdx70			0x70



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#define	PtrData(_DataAddr_)		*((volatile uShort *)(_DataAddr_))  /* for word width */
#define	PtrCode(_CodeAddr_)	*((volatile uShort *)(_CodeAddr_))
#define	PtrTbl(_TblAddr_,_Offset_)	*((volatile uShort *)(_TblAddr_)+_Offset_)
#define	PtrMcuReg(_RegAddr_)		(*(volatile unsigned *)(_RegAddr_))  /* for 32bit width */



/********************************************************/
/*************** Other Macro Definition *********************/
/********************************************************/
#define	nop()				__asm{NOP} /* one nop = 2.5ns */
#define	Timer0HiCnt			0x63 /* 10ms counter=(65536-40000)=25536 */
#define	Timer0LoCnt			0xC0
#define	Timer2HiCnt			0xFF /* timer2 use BandRate 100K for serial 0 */
#define	Timer2LoCnt			0xF1 /* BaudRate = CLOUT / (32 * (65536-ReloadCnt)) */
#define	Serial0Master		0xD8
#define	Serial0Slave			0xF0



/*************************************************************************\
	Local Varify Definition Here for FirmWare Driver Utility
\*************************************************************************/
extern uChar TaskStatus ;



/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/*                             Firmware Subroutine Driver Define                                                         */
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
extern void Busying_LED(void) ;
extern void Failed_LED(void) ;
extern void Passed_LED(void) ;
extern void Beep_On(void) ;
extern void Beep_Off(void) ;
extern void Normal_Beep_On(void) ;
extern void Error_Beep_On(void) ;
extern void Socket_A_Fail(void) ;
extern void Socket_A_Pass(void) ;
extern void Socket_A_Busy(void) ;
extern void Socket_B_Fail(void) ;
extern void Socket_B_Pass(void) ;
extern void Socket_B_Busy(void) ;
extern void Socket_C_Fail(void) ;
extern void Socket_C_Pass(void) ;
extern void Socket_C_Busy(void) ;
extern void Socket_D_Fail(void) ;
extern void Socket_D_Pass(void) ;
extern void Socket_D_Busy(void) ;
extern void Socket_E_Fail(void) ;
extern void Socket_E_Pass(void) ;
extern void Socket_E_Busy(void) ;
extern void Socket_F_Fail(void) ;
extern void Socket_F_Pass(void) ;
extern void Socket_F_Busy(void) ;
extern void Socket_G_Fail(void) ;
extern void Socket_G_Pass(void) ;
extern void Socket_G_Busy(void) ;
extern void Socket_H_Fail(void) ;
extern void Socket_H_Pass(void) ;
extern void Socket_H_Busy(void) ;


extern void SwitchPowerOn(void) ;
extern void SwitchPowerOff(void) ;
extern void SetVoltsToDAC(uChar I2CAddr, uChar RegAddr, uShort Para) ;
extern uChar CheckDACALT_RegStatus(uChar I2CAddr, uChar RegAddr, uChar SelPinOrReg) ; /* SelPinOrReg=1 chk ALT pin; =0 chk Reg */
extern uChar InitialDeviceInform(void) ;
extern uChar InitialPinMapTbl(void) ;
extern uChar InitialDACArray(void) ;
extern uChar UpdateSKBInformation(void) ;
extern uChar InitialDevBlkMainOption(void) ;
extern uChar InitialSpecificationTbl(void) ;
extern uChar CheckAllSKBIDCode(void) ;
extern uChar CheckDIPSKBIDCode(void) ;
extern void ClearAllSKBInformation(void) ;
extern void ClearDIPSKBInformation(void) ;
extern uChar AllChildSKBCheckStatus(void) ;
extern void AllSKBCheckStatus(void) ;

extern void Device_Select(void) ;
extern uChar FindSpecialBitBlk(void) ;
extern uChar GetChkBlkProperty(void) ;
extern void SetupSectorAddress(uShort SecAddrHiWord, uShort SecAddrLoWord) ;
extern uChar InitialChkAlgoICIndex(void) ;


extern void EnableSKBGndPinDrv(void) ;
extern void EnableSKBGndPinDrv_XP1(void) ;
extern void DisableSKBGndPinDrv_XP1(void) ;
extern void DisableAllPinDrv(void) ;
extern void EnableAllPinDrv(void) ;
extern void DisableSKBGndPinDrv(void) ;

extern void Delay_1Second(uInt32 TimeCnt) ;
extern void Delay_1ms(uInt32 TimeCnt);
extern void Delay_1us( uInt32 TimeCnt ) ;
extern void Delay_10nsX(uInt32 CntX10ns) ;

extern void DrvMaster_Initial(void) ;

extern void FPGAParaSetting(void) ;

extern void SetSysFailInforBuf(void) ;
extern void Send_Status_Code( void ) ;
extern void GetCalendarTimer(void) ;
extern void SaveCurrTimer(uChar * pTimer) ;
extern void CalculateAllProcessTime(void) ;
extern void SetCalendarTimer(uChar TimerAddr, uChar TimerData) ;

extern void ChangeVCCNumToDriverPin(uChar VccPin) ;
extern void ChangeVPPESNumToDriverPin(uChar VppesPin) ;
extern void ChangeVPIONumToDriverPin(uChar VpioPin) ;
extern void ChangeGNDNumToDriverPin(uChar GndPin) ;
extern void ChangeNuNcNumToDriverPin(uChar NuNcPin) ;
extern uChar ChangePCCErrPinNumToDriverPin(uChar PccPin) ;


/***** Enable pin drive to output volts *****/
extern void Send1WordToIOCPLDPortReg(uShort TempData) ;
extern void Rise_VccIccPin_XP1(void) ;
extern void Rise_VppIppPin_XP1(void) ;
extern void Rise_VpioIpioPin_XP1(void) ;
extern uChar ExchangeVpioPin(uChar TempPin) ;
extern void OutputPortPin(uInt32 PortPinLoc, uChar CtrlData) ; /* output control 8 pins */
extern void OutputPortPin_1IO(uInt32 PortPinAddr, uChar CtrlData) ;
extern uChar InPutPortPin(uInt32 PortPinLoc ) ; /* input control 8 pins */
extern uChar InPutPortPin_1IO(uInt32 PortPinAddr ) ;
extern void OutputPowerPortPin(uInt32 PortPinLoc, uChar CtrlData) ; /* output power control 1 pin!! */
extern void Rise_VccIccPin(void) ;
extern void Fall_VccIccPin(void) ;
extern void Rise_VppIppPin(void) ;
extern void Fall_VppIppPin(void) ;
extern void Rise_VpioIpioPin(void) ;
extern void Fall_VpioIpioPin(void) ;

/***** Set DAC device to output volts *****/
extern void Set_VccDAC(void) ;
extern void Set_VppDAC(void) ;
extern void Set_VpioDAC(void) ;
extern void Set_VihDAC(void) ;
extern void Initial_VihAll(void) ;

/***** Disable pin drive to output volts *****/
#define Fall_VccAll() 			Fall_VccIccAll()
extern void Fall_VccIccAll(void) ;
extern void Fall_VppIppAll(void) ;
extern void Fall_VpioIpioAll(void) ;
extern void Fall_VihAll(void) ;
extern void SettingDC20VPower(uChar PwrStatus) ;

extern void ProgCalcuChecksum( uShort SumData ) ;
extern uChar ChecksumCompare(void) ;
extern uChar SysStdChecksum(void) ; /* Device buffer MUST be 512 Bytes scale */
extern uChar UselessAlgoFun(void) ;
extern void CalErrRAMDevAddr(void) ;



/****************************************************************\
		Below subroutine function just for Gang System Setting
\****************************************************************/
extern void GangAllSocketInitial(void) ;
extern void GangSocketLEDBusyStatus(void) ;
extern void GangSocketLEDPassStatus(void) ;
extern void GangSocketLEDFailStatus(void) ;
extern void CSKBLEDChkFailStatus(void) ;
extern uChar GangCheckIfAllSocketError(uChar GSckChkStatus) ; /* Error Sck then power off VPP, VCC & VIO to protect device ! */
extern uChar GangCheckIfAll64SocketError(uInt32 GSckChkH32Status, uInt32 GSckChkL32Status) ; /* Error Sck then power off VPP, VCC & VIO to protect device ! */
extern void GangCheckAll64SckStatus(void) ; /* check 64 socket result */
extern void GangRiseAllPassSocketVccIccPin(void) ;
extern void GangRiseAllSocketVccIccPin(void) ;
extern void GangRiseAllPassSocketVppIppPin(void) ;
extern void GangRiseAllPassSocketVpioIpioPin(void) ;
extern void GangRiseAllSocketVpioIpioPin(void) ;
extern void GangFallAllErrorSocketVccIccPin(void) ;
extern void GangFallAllErrorSocketVppIppPin(void) ;
extern void GangFallAllErrorSocketVpioIpioPin(void) ;
extern void GangErrorSocketMsg(void) ;
extern void Update_PassFailCounter(void) ;
extern void SelectSPIPortType(uChar SpiType) ;



/****************************************************************\
		Firmware or System Subroutine Define in User's driver
\****************************************************************/
extern uChar GetDrvElementLen(void) ;
extern void DioPortListDefine(void) ;

#endif

