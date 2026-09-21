/******************************************************************************\
	Programmer Driver Total Setting Here

\*******************************************************************************/
#ifndef ProgramDrv_H_
#define ProgramDrv_H_

#include "h\Type.h"
#include "h\FirmWare.h"
#include "h\BlockSectorDef.h"
#include "h\Subroutine.h"
#include "h\PinDef.h"
#include "h\Hspi.h"
#include "h\SimPinDef.h"


/********************* total 13 algorithm subroutine entance define *********************/
extern uChar PowerOnFun(void);
extern uChar PowerOffFun(void);
extern uChar ReadFun(void);
extern uChar ProgramFun(void);
extern uChar VerifyFun(void);
extern uChar EraseFun(void);
extern uChar BlankCheckFun(void);
extern uChar IDCheckFun(void);
extern uChar CheckSumFun(void);
extern uChar ProtectFun(void);
extern uChar UnProtectFun(void);



/******************************************************************************\
	Algorithm Macro Define Table Here

\******************************************************************************/
/* Algorithm Index Code Constant Define, You Can Change Name by Yourself */
#define AlgoIdx_BY25Q			0/* max define 255 */


/* Block/Sector Index Code Constant Define, You Can Change Name by Yourself */
#define BlkIdx_BY25Q16			0
#define BlkIdx_BY25Q32			1
#define BlkIdx_BY25Q64			2
#define BlkIdx_BY25Q128			3


/* Specifications Index Code Constant Define, You Can Change Name by Yourself */
#define	SpcIdx_LV33			0/* max define 255 */
#define	SpcIdx_LV18			1


/* Pin Mapping Index Code Constant Define, You Can Change Name by Yourself */
#define PmpIdx_H3V3SO8UG4A	        0/* max define 255 */
#define PmpIdx_H3V3SO16UG4A	        1
#define PmpIdx_H3V3TFBGA24SG4A      2 /* use SO8 same PinDriver */


/* Version Number of Driver Master (Pseudo!), You CANNOT Change the Macro Name */
#define	DrvMST_Version		0xFFFF	/* Version: Pseudo! */


/* Version Number of Device Driver, You CANNOT Change the Macro Name */
#define	DevDRV_Version		0x3130	/* Version: 1.1, Use Ascii code */


/* Gang Type for system, You CANNOT Change the Macro Name */
#define	SystemGang			Gang1Type /* Gang2Type, Gang4Type or Gang8Type */


/******************************************************************************
 ***	          <<<  The Device Block/Sector Table  >>>		    ***
 ******************************************************************************/
 /* FORMAT:
	1) Main Structure:

	 Device_Option(1W),
	 RAM_Start_Address(2W),  	! byte memory addressing
	 Chip_Start_Address(2W), Chip_End_Address(2W),
	 [Property_Of_Block_1(10W)],
	 [Property_Of_Block_2(10W)],
	 .
	 .
	 [Property_Of_Block_N(10W)];

	2) Property Of Block:

	 Blk_Attribute(1W),	Blk_SumMask(1W),	Blk_Virgin(1W),
	 Blk_RAMstart_Address(2W),	! byte memory addressing
	 Blk_Devstart_Address(2W),	Blk_Devend_Address(2W),
	 Blk_NounsCode(1W),

	3) The Device_Option Define:

	 bit: 1 1 1 1  1 1
		   5 4 3 2  1 0 9 8  7 6 5 4 3 2 1 0
		|  | | | | | | |  |  | | | | | | |
		|  | | | | | | |  |  | | | | | | +-------- 1: Byte Data Width
		|  | | | | | | |  |  | | | | | +----------1: Word Data Width
		   |  | | | | | | |  |  | | | | +----------- 1: Double Word Data Width
		|  | | | | | | |  |  | | | +-------------1: Long Word Data Width
		|  | | | | | | |  |  | | +---------------1: Device  Option Reserve Bit 4
		|  | | | | | | |  |  | +----------------1: Device  Option Reserve Bit 5
		|  | | | | | | |  |  +------------------1: Device  Option Reserve Bit 6
		|  | | | | | | |  +--------------------1: Double Cycle On
		|  | | | | | | +----------------------1: Byte Data Sum
		|  | | | | | +------------------------1: Word Data Sum
		|  | | | | +--------------------------1: CRC-16Bit Sum
		|  | | | +---------------------------1: CRC-32Bit Sum
		|  | | +-----------------------------1: Device  Option Reserve Bit 12
		|  | +-------------------------------1: Verify Error List On
		|  +--------------------------------1: Program CheckSum Compair On
		+---------------------------------- 1: Sector Erase On


	4) The Blk_Attribute Define:

	   bit: 1 1 1 1  1 1
			 5 4 3 2  1 0 9 8  7  6 5 4 3 2 1 0
			 |  | | | | | | |  |  | | | | | | |
			 |  | | | | | | |  |  | | | | | | +-------- 1: Checksum-Off
			 |  | | | | | | |  |  | | | | | +---------- 0: DRAM, 1: SRAM
			 |  | | | | | | |  |  | | | | +----------- 1: Program-Off
			 |  | | | | | | |  |  | | | +------------- 1: Sercure-Off
			 |  | | | | | | |  |  | | +--------------- 1: Verify-Off
			 |  | | | | | | |  |  | +----------------- 1: Erase-Off
			 |  | | | | | | |  |  +------------------ 1: Check-Off
			 |  | | | | | | |  +-------------------- 1: DualLoop-On
			 |  | | | | | |+----------------------- 1: Read-Off
			 |  | | | | | +------------------------ 1: Special Array
			 |  | | | | +-------------------------- 1: Block Attribute Reserve Bit 10
			 |  | | | +--------------------------- 1:Block Attribute Reserve Bit 11
			 |  | | +----------------------------- 1:Block Attribute Reserve Bit 12
			 |  | +------------------------------- 1:Block Attribute Reserve Bit 13
			 |  +--------------------------------- 1:Block Attribute Reserve Bit 14
			 +----------------------------------- 1:Block Attribute Reserve Bit 15

 ------------------------------------------------------------------------------*/
#if 1
const uShort Block_BY25Q16[] =
{
	//	MainByteSumCal|MainByte8Width,
		MainByte8Width,
		0x0000,0x0000,	/* RAM Base Address */
		0x0000,0x0000, 0x0007,0xFFFF, 	/* Chip Start Address, Chip End Address */

		0x0000,0x00FF,0x00FF,
		0x0000,0x0000,
		0x0000,0x0000, 0x0007,0xFFFF,
		Noun_Block0,

		 0x8028,0x00FF,0x00FF, 
		 0x0008,0x0000,
		 0x0000,0x0000, 0x0000,0x00FF, 
		 Noun_Block1,

         0x8028,0x00FF,0x00FF, 
		 0x0008,0x0100,
		 0x0000,0x0100, 0x0000,0x01FF,
		 Noun_Block2,

         0x8028,0x00FF,0x00FF, 
		 0x0008,0x0200,
		 0x0000,0x0200, 0x0000,0x02FF,
		 Noun_Block3,

		 0x8028,0x00FF,0x00FF,
		 0x0008,0x0300,
		 0x0000,0x0300, 0x0000,0x03FF,
		 Noun_Block4,
		0x0203,0x00FF,0x0000,
		0x0000,0x0000,
		0x0000,0x0000, 0x0000,0x0002,
		Noun_BlkPtStatus,
};
#endif
const uShort Block_BY25Q32[] =
{
	//	MainByteSumCal|MainByte8Width,
		MainByte8Width,
		0x0000,0x0000,	/* RAM Base Address */
		0x0000,0x0000, 0x003F,0xFFFF, 	/* Chip Start Address, Chip End Address */

		0x0000,0x00FF,0x00FF,
		0x0000,0x0000,
		0x0000,0x0000, 0x003F,0xFFFF,
		Noun_Block0,

		// 0x8028,0x00FF,0x00FF, 
		// 0x0100,0x0000,
		// 0x0000,0x0000, 0x0000,0x01FF,// 4K-bit secured OTP 
		// Noun_Block1,

		0x0203,0x00FF,0x0000,
		0x0000,0x0000,
		0x0000,0x0000, 0x0000,0x0006,
		Noun_BlkPtStatus,
};


const uShort Block_BY25Q64[] =
{
	//	MainByteSumCal|MainByte8Width,
		MainByte8Width,
		0x0000,0x0000,	/* RAM Base Address */
		0x0000,0x0000, 0x007F,0xFFFF, 	/* Chip Start Address, Chip End Address */

		0x0000,0x00FF,0x00FF,
		0x0000,0x0000,
		0x0000,0x0000, 0x007F,0xFFFF,
		Noun_Block0,

		// 0x8028,0x00FF,0x00FF, 
		// 0x0100,0x0000,
		// 0x0000,0x0000, 0x0000,0x01FF,// 4K-bit secured OTP 
		// Noun_Block1,

		0x0203,0x00FF,0x0000,
		0x0000,0x0000,
		0x0000,0x0000, 0x0000,0x0006,
		Noun_BlkPtStatus,
};


const uShort Block_BY25Q128[] =
{
	//	MainByteSumCal|MainByte8Width,
		MainByte8Width,
		0x0000,0x0000,	/* RAM Base Address */
		0x0000,0x0000, 0x009F,0xFFFF, 	/* Chip Start Address, Chip End Address */

		0x0000,0x00FF,0x00FF,
		0x0000,0x0000,
		0x0000,0x0000, 0x009F,0xFFFF,
		Noun_Block0,

		// 0x8028,0x00FF,0x00FF, 
		// 0x0100,0x0000,
		// 0x0000,0x0000, 0x0000,0x01FF,// 4K-bit secured OTP 
		// Noun_Block1,

		0x0203,0x00FF,0x0000,
		0x0000,0x0000,
		0x0000,0x0000, 0x0000,0x0006,
		Noun_BlkPtStatus,
};


/*****************************************************************************
 ***		  <<<  The Device Specifications Table	>>>		    ***
 ******************************************************************************
{! ATTENTION:
	This table format is absolute! All Driver Master and Device Driver must
   follow it, That PC and System Firmware can be get right information.

 ? FORMAT:
   1) Main Structure:

	Amount_Of_Parameters(1W),
	[Property_Of_Para_1(6W)],
	[Property_Of_Para_2(6W)],
	.
	.
	[Property_Of_Para_N(6W)];

   2) Property of Parameters:

	ParaValue(1W), ParaType(1B), ParaUnit(1B), MinVal(1W), MaxVal(1W), NameIdx(1B),

 -----------------------------------------------------------------------------*/

const uShort Spec_LV33[] =
{ /* Typical   ParaType  Scale          MinVal     MaxVal      ParaName */
	330,		VCCR,	x10mV,		300,		360,		NounVCCR,
	330,		VCCP,	x10mV,		300,		360,		NounVCCP,
	330,		VCCE,	x10mV,		300,		360,		NounVCCE,
	360,		VCCH,	x10mV,		300,		360,		NounVCCH,
	300,		VCCL,	x10mV,		300,		360,		NounVCCL,
	330,		VIH,		x10mV,		300,		360,		NounVIH,	/* VIH Volts MUST be same with FPGA Pin Type Setting!! */
	10,		RSCPD,	x15nS,		4,		32,		NounRSCPD, /* Read FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
	8,		WSCPD,	x15nS,		4,		32,		NounWSCPD, /* Write FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
};




const uShort Spec_LV18[] =
{ /* Typical   ParaType  Scale          MinVal     MaxVal      ParaName */
	180,		VCCR,	x10mV,		165,		195,		NounVCCR,
	180,		VCCP,	x10mV,		165,		195,		NounVCCP,
	180,		VCCE,	x10mV,		165,		195,		NounVCCE,
	180,		VCCH,	x10mV,		165,		195,		NounVCCH,
	180,		VCCL,	x10mV,		165,		195,		NounVCCL,
	220,		VIH,		x10mV,		165,		195,		NounVIH,	/* VIH Volts MUST be same with FPGA Pin Type Setting!! */
	10,		RSCPD,	x15nS,		4,		32,		NounRSCPD, /* Read FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
	8,		WSCPD,	x15nS,		4,		32,		NounWSCPD, /* Write FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
};



/***************************************************************************\
	Pin Map Table for Definition
	Notes: CAN NOT change Terminal Table sequence, if donot have this type
			pin, then skit this table directly is OK !!!
\***************************************************************************/
const uShort Pmp_H3V3SO8UG4A[] =
{/* for SPI25F */
/* Socket A */
	V28,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G24,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
};



const uShort Pmp_H3V3SO16UG4A[] =
{/* for SPI25F */
/* Socket A */
	V18, V30, /* VCC-VIO */
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G26,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
};



/************************************************************************************************\

\************************************************************************************************/
/* Driver Element Combination Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by same format                        */
const uChar DrvElement_Table[][5] =
{
	/********** below for 3V3 *************/
		{ AlgoIdx_BY25Q, BlkIdx_BY25Q16,  SpcIdx_LV33, PmpIdx_H3V3SO8UG4A, AlgoIC_0x00 }, /* BY25Q16AWSxx */
		{ AlgoIdx_BY25Q, BlkIdx_BY25Q32,  SpcIdx_LV33, PmpIdx_H3V3SO8UG4A, AlgoIC_0x01 }, /* BY25Q32ESxx */
		{ AlgoIdx_BY25Q, BlkIdx_BY25Q64,  SpcIdx_LV33, PmpIdx_H3V3SO8UG4A, AlgoIC_0x02 }, /* BY25Q64ESxx */
		{ AlgoIdx_BY25Q, BlkIdx_BY25Q128, SpcIdx_LV33, PmpIdx_H3V3SO8UG4A, AlgoIC_0x03 }, /* BY25Q128ESxx */
		//{ AlgoIdx_BY25Q, BlkIdx_BY25Q16, SpcIdx_LV33, PmpIdx_H3V3SO8UG4A, AlgoIC_0x04 },  /* BY25Q16BLSxx 2024/9/4*/
		/* here adding more... */
};


/******************************************************************************\
	Jumpper Table for Programmer total 13 tasks driver setting:
	Device Pin Mapping Index Table, You CANNOT Change the Element sequence !!

	POWER_ON_FUN							POWER_OFF_FUN
	READ_FUN			PROGRAM_FUN		VERIFY_FUN
	ERASE_FUN			BLANK_CHECK_FUN	ILLEGAL_CHECK_FUN
	ID_CHECK_FUN		SECURE_FUN			CHECKSUM_FUN
	PROTECT_FUN								UNPROTECT_FUN
\*******************************************************************************/

/* Device Algorithm Index Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by using same format          */

uChar(* const DevAlgoIdx_Table[][13])(void) =
{
	{	PowerOnFun,								PowerOffFun,
		ReadFun,			ProgramFun,			VerifyFun,
		EraseFun,			BlankCheckFun,			UselessAlgoFun,
		IDCheckFun,			UselessAlgoFun,			CheckSumFun,
		ProtectFun,									UnProtectFun,
	},
};



/* Device Block/Sector	Index Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by using same format               */
const uInt32 DevBlockIdx_Table[] =
{
	(uInt32)Block_BY25Q16, sizeof(Block_BY25Q16),
	(uInt32)Block_BY25Q32, sizeof(Block_BY25Q32),
	(uInt32)Block_BY25Q64, sizeof(Block_BY25Q64),
	(uInt32)Block_BY25Q128, sizeof(Block_BY25Q128),
	//(uInt32)Block_BY25Q16, sizeof(Block_BY25Q16B),
	/* here adding more... */
};


/* Device Spec Index Table, You CANNOT Change the Table Name            *\
\* But you can add more chip element here by using same format              */
const uInt32 DevSpecIdx_Table[] =
{
	(uInt32)Spec_LV33, sizeof(Spec_LV33), /* Specification Setting Table Address */
	(uInt32)Spec_LV18, sizeof(Spec_LV18), /* Specification Setting Table Address */

	/* here adding more... */
};

const uChar ChipSPIMaxClkTable[] =
{ /* Table Sequence by Spec Index Code Constant Define */
	HSPI_CLK_16M5,
	HSPI_CLK_16M5, /* 1.8V need use low speed clock to stable */

	/* here adding more... */
};


/* Device Pin Mapping Index Table, You CANNOT Change the Table Name   *\
\* But you can add more chip element here by using same format              */
const uInt32 DevPmpIdx_Table[] =
{
	(uInt32)Pmp_H3V3SO8UG4A, sizeof(Pmp_H3V3SO8UG4A),
	(uInt32)Pmp_H3V3SO16UG4A, sizeof(Pmp_H3V3SO16UG4A),
	(uInt32)Pmp_H3V3SO8UG4A, sizeof(Pmp_H3V3SO8UG4A), /* BGA24 use SO8 same PinDriver */

	/* here adding more... */
};

/***** CAN NOT Change the Table Name *****/
const uShort DmmIOPortDefine[][5] =
{ /* Table Sequence by PinMap Index Code Constant Define */
/*	  CS   WP   HOLD RST  CS2, CAN NOT modify Sequence */
	{D21, D23, D27,  D01, D20,}, /* 25# SO8, RST Dummy define for System Chk Error*/
	{D23, D25, D17,  D19, D22,}, /* 25# S16 */
	{D21, D23, D27,  D01, D20,}, /* 25# BGA24 use SO8 same PinDriver, RST Dummy define for System Chk Error*/

	/* here adding more... */
};

/***** CAN NOT Change the Table Name *****/
const uChar SpiPortTypeDefine[] =
{ /* Table Sequence by PinMap Index Code Constant Define */
	SPI25_SO8,
	SPI25_SO16,
	SPI25_SO8, /* BGA24 use SO8 same PinDriver */

	/* here adding more... */
};


/*********** Device Sector Protect Address Main Table ***************\
\***** Table Sequence by Block/Sector Index Code Constant Define *****/
const uShort ChipPageSizeTable[] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
	256, 256, 256, 256, 256, 256, 256, 256, 256,

	/* here adding more... */
};


const uShort ChipEraseTimeTable[] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
	128, 64, 32, 16, 8, 4, 4, 4, 4, /* Max time for second scale */

	/* here adding more... */
};


const uChar ProtectRegisterTable[] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
	0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC, 0xBC,

	/* here adding more... */
};


#endif




