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
extern uChar PowerOnFun(void) ;
extern uChar PowerOffFun(void) ;
extern uChar ReadFun(void) ;
extern uChar ReadFun_4Byte(void) ;
extern uChar ProgramFun(void) ;
extern uChar ProgramFun_4Byte(void) ;
extern uChar VerifyFun(void) ;
extern uChar VerifyFun_4Byte(void) ;
extern uChar EraseFun(void) ;
extern uChar BlankCheckFun(void) ;
extern uChar BlankCheckFun_4Byte(void) ;
extern uChar IDCheckFun(void) ;
extern uChar CheckSumFun(void) ;
extern uChar ProtectFun(void) ;
extern uChar UnProtectFun(void) ;



/******************************************************************************\
	Algorithm Macro Define Table Here

\******************************************************************************/
/* Algorithm Index Code Constant Define, You Can Change Name by Yourself */
#define AlgoIdx_PY25Q		             0/* max define 255 */


/* Block/Sector Index Code Constant Define, You Can Change Name by Yourself */
#define BlkIdx_PY25Q32LB		0
#define BlkIdx_PY25Q64LB		1
#define BlkIdx_PY25Q64HA		2
#define BlkIdx_PY25Q40HB		3
#define BlkIdx_P25Q40SH		      4
#define BlkIdx_XM25QU20BU		5
#define BlkIdx_HS25Q64D		       6


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

const uShort Block_PY25Q32LB[] = 
{
//	MainByteSumCal|MainByte8Width,
	MainByte8Width,
	0x0000,0x0000,	/* RAM Base Address */
	0x0000,0x0000, 0x003F,0xFFFF, 	/* Chip Start Address, Chip End Address */
	
	0x0000,0x00FF,0x00FF,	
	0x0000,0x0000,
	0x0000,0x0000, 0x003F,0xFFFF, 
	Noun_Block0,
#if 0
	0x8028,0x00FF,0x00FF, 
	0x0200,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,// 4K-bit secured OTP 
	Noun_Block1,
#endif
	0x0223,0x00FF,0x0000,
	0x0000,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,
	Noun_BlkPtStatus,
};

const uShort Block_PY25Q64LB[] = 
{
//	MainByteSumCal|MainByte8Width,
	MainByte8Width,
	0x0000,0x0000,	/* RAM Base Address */
	0x0000,0x0000, 0x007F,0xFFFF, 	/* Chip Start Address, Chip End Address */
	
	0x0000,0x00FF,0x00FF,	
	0x0000,0x0000,
	0x0000,0x0000, 0x007F,0xFFFF, 
	Noun_Block0,
#if 0
	0x8028,0x00FF,0x00FF, 
	0x0200,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,// 4K-bit secured OTP 
	Noun_Block1,
#endif
	0x0223,0x00FF,0x0000,
	0x0000,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,
	Noun_BlkPtStatus,
};
const uShort Block_PY25Q64HA[] = 
{
//	MainByteSumCal|MainByte8Width,
	MainByte8Width,
	0x0000,0x0000,	/* RAM Base Address */
	0x0000,0x0000, 0x007F,0xFFFF, 	/* Chip Start Address, Chip End Address */
	
	0x0000,0x00FF,0x00FF,	
	0x0000,0x0000,
	0x0000,0x0000, 0x007F,0xFFFF, 
	Noun_Block0,
#if 0
	0x8028,0x00FF,0x00FF, 
	0x0200,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,// 4K-bit secured OTP 
	Noun_Block1,
#endif
	0x0223,0x00FF,0x0000,
	0x0000,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,
	Noun_BlkPtStatus,
};
const uShort Block_PY25Q40HB[] = 
{
//	MainByteSumCal|MainByte8Width,
	MainByte8Width,
	0x0000,0x0000,	/* RAM Base Address */
	0x0000,0x0000, 0x0007,0xFFFF, 	/* Chip Start Address, Chip End Address */
	
	0x0000,0x00FF,0x00FF,	
	0x0000,0x0000,
	0x0000,0x0000, 0x0007,0xFFFF, 
	Noun_Block0,
#if 0
	0x8028,0x00FF,0x00FF, 
	0x0200,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,// 4K-bit secured OTP 
	Noun_Block1,
#endif
	0x0223,0x00FF,0x0000,
	0x0000,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,
	Noun_BlkPtStatus,
};
const uShort Block_P25Q40SH[] = 
{
//	MainByteSumCal|MainByte8Width,
	MainByte8Width,
	0x0000,0x0000,	/* RAM Base Address */
	0x0000,0x0000, 0x0007,0xFFFF, 	/* Chip Start Address, Chip End Address */
	
	0x0000,0x00FF,0x00FF,	
	0x0000,0x0000,
	0x0000,0x0000, 0x0007,0xFFFF, 
	Noun_Block0,
#if 0
	0x8028,0x00FF,0x00FF, 
	0x0200,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,// 4K-bit secured OTP 
	Noun_Block1,
#endif
	0x0223,0x00FF,0x0000,
	0x0000,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,
	Noun_BlkPtStatus,
};
const uShort Block_XM25QU20BU[] = 
{
//	MainByteSumCal|MainByte8Width,
	MainByte8Width,
	0x0000,0x0000,	/* RAM Base Address */
	0x0000,0x0000, 0x0003,0xFFFF, 	/* Chip Start Address, Chip End Address */
	
	0x0000,0x00FF,0x00FF,	
	0x0000,0x0000,
	0x0000,0x0000, 0x0003,0xFFFF, 
	Noun_Block0,
#if 0
	0x8028,0x00FF,0x00FF, 
	0x0200,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,// 4K-bit secured OTP 
	Noun_Block1,
#endif
	0x0223,0x00FF,0x0000,
	0x0000,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,
	Noun_BlkPtStatus,
};
const uShort Block_HS25Q64D[] = 
{
//	MainByteSumCal|MainByte8Width,
	MainByte8Width,
	0x0000,0x0000,	/* RAM Base Address */
	0x0000,0x0000, 0x007F,0xFFFF, 	/* Chip Start Address, Chip End Address */
	
	0x0000,0x00FF,0x00FF,	
	0x0000,0x0000,
	0x0000,0x0000, 0x007F,0xFFFF, 
	Noun_Block0,
#if 0
	0x8028,0x00FF,0x00FF, 
	0x0200,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,// 4K-bit secured OTP 
	Noun_Block1,
#endif
	0x0223,0x00FF,0x0000,
	0x0000,0x0000,
	0x0000,0x0000, 0x0000,0x01FF,
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
const uShort Spec_LV33[ ] =
{ /* Typical   ParaType  Scale          MinVal     MaxVal      ParaName */
	340,		VCCR,	x10mV,		300,		360,		NounVCCR, 
	340,		VCCP,	x10mV,		300,		360,		NounVCCP,
	340,		VCCE,	x10mV,		300,		360,		NounVCCE,
	360,		VCCH,	x10mV,		300,		360,		NounVCCH,
	300,		VCCL,	x10mV,		300,		360,		NounVCCL,
	330,		VIH,		x10mV,		300,		360,		NounVIH,	/* VIH Volts MUST be same with FPGA Pin Type Setting!! */
	10,		RSCPD,	x15nS,		4,		32,		NounRSCPD, /* Read FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
	8,		WSCPD,	x15nS,		4,		32,		NounWSCPD, /* Write FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
};



const uShort Spec_LV18[ ] =
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
const uShort Pmp_H3V3SO8UG4A[ ] =
{/* for SPI25F */
/* Socket A */
	V28,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G24,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
} ;



const uShort Pmp_H3V3SO16UG4A[ ] =
{/* for SPI25F */
/* Socket A */
	V18, V30, /* VCC-VIO */
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G26,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
} ;



/************************************************************************************************\

\************************************************************************************************/
/* Driver Element Combination Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by same format                        */
const uChar DrvElement_Table[ ][5] = 
{
/********** below for 18 *************/
{ AlgoIdx_PY25Q, BlkIdx_PY25Q32LB, SpcIdx_LV18, PmpIdx_H3V3SO8UG4A, AlgoIC_0x00 }, /* PY25Q32LB*/
{ AlgoIdx_PY25Q, BlkIdx_PY25Q64LB, SpcIdx_LV18, PmpIdx_H3V3SO8UG4A, AlgoIC_0x01 }, /* PY25Q64LB*/
{ AlgoIdx_PY25Q, BlkIdx_XM25QU20BU, SpcIdx_LV18, PmpIdx_H3V3SO8UG4A, AlgoIC_0x05 }, /* XM25QU20BU*/

	/********** below for 33 *************/
{ AlgoIdx_PY25Q, BlkIdx_PY25Q64HA, SpcIdx_LV33, PmpIdx_H3V3SO8UG4A, AlgoIC_0x02 }, /* PY25Q64HA*/
{ AlgoIdx_PY25Q, BlkIdx_PY25Q40HB, SpcIdx_LV33, PmpIdx_H3V3SO8UG4A, AlgoIC_0x03}, /* PY25Q40HB*GT25Q40C/
{ AlgoIdx_PY25Q, BlkIdx_P25Q40SH, SpcIdx_LV33, PmpIdx_H3V3SO8UG4A, AlgoIC_0x04}, /* P25Q40SH*/
{ AlgoIdx_PY25Q, BlkIdx_HS25Q64D, SpcIdx_LV33, PmpIdx_H3V3SO8UG4A, AlgoIC_0x06 }, /* PY25Q64HA*/

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

uChar ( *const DevAlgoIdx_Table[ ][13] )( void ) =
{
	{	PowerOnFun,							       		PowerOffFun,
		ReadFun_4Byte,			ProgramFun_4Byte,			VerifyFun_4Byte,
		EraseFun,			        BlankCheckFun_4Byte,		UselessAlgoFun,
		IDCheckFun,			        UselessAlgoFun,				CheckSumFun,
		ProtectFun,											UnProtectFun,
	},
	{	PowerOnFun,							       		PowerOffFun,
		ReadFun_4Byte,			ProgramFun_4Byte,			VerifyFun_4Byte,
		EraseFun,			        BlankCheckFun_4Byte,		UselessAlgoFun,
		IDCheckFun,			        UselessAlgoFun,				CheckSumFun,
		ProtectFun,											UnProtectFun,
	},
	{	PowerOnFun,							       		PowerOffFun,
		ReadFun_4Byte,			ProgramFun_4Byte,			VerifyFun_4Byte,
		EraseFun,			        BlankCheckFun_4Byte,		UselessAlgoFun,
		IDCheckFun,			        UselessAlgoFun,				CheckSumFun,
		ProtectFun,											UnProtectFun,
	},
	{	PowerOnFun,							       		PowerOffFun,
		ReadFun_4Byte,			ProgramFun_4Byte,			VerifyFun_4Byte,
		EraseFun,			        BlankCheckFun_4Byte,		UselessAlgoFun,
		IDCheckFun,			        UselessAlgoFun,				CheckSumFun,
		ProtectFun,											UnProtectFun,
	},
	{	PowerOnFun,							       		PowerOffFun,
		ReadFun_4Byte,			ProgramFun_4Byte,			VerifyFun_4Byte,
		EraseFun,			        BlankCheckFun_4Byte,		UselessAlgoFun,
		IDCheckFun,			        UselessAlgoFun,				CheckSumFun,
		ProtectFun,											UnProtectFun,
	},
	{	PowerOnFun,							       		PowerOffFun,
		ReadFun_4Byte,			ProgramFun_4Byte,			VerifyFun_4Byte,
		EraseFun,			        BlankCheckFun_4Byte,		UselessAlgoFun,
		IDCheckFun,			        UselessAlgoFun,				CheckSumFun,
		ProtectFun,											UnProtectFun,
	},
	{	PowerOnFun,							       		PowerOffFun,
		ReadFun_4Byte,			ProgramFun_4Byte,			VerifyFun_4Byte,
		EraseFun,			        BlankCheckFun_4Byte,		UselessAlgoFun,
		IDCheckFun,			        UselessAlgoFun,				CheckSumFun,
		ProtectFun,											UnProtectFun,
	},
	
};



/* Device Block/Sector	Index Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by using same format               */
const uInt32 DevBlockIdx_Table[ ] = 
{
	(uInt32)Block_PY25Q32LB, sizeof(Block_PY25Q32LB), 
	(uInt32)Block_PY25Q64LB, sizeof(Block_PY25Q64LB), 
	(uInt32)Block_PY25Q64HA, sizeof(Block_PY25Q64HA), 
	(uInt32)Block_PY25Q40HB, sizeof(Block_PY25Q40HB), 
	(uInt32)Block_P25Q40SH, sizeof(Block_P25Q40SH), 
	(uInt32)Block_XM25QU20BU, sizeof(Block_XM25QU20BU), 
	(uInt32)Block_HS25Q64D, sizeof(Block_HS25Q64D), 

	/* here adding more... */
};


/* Device Spec Index Table, You CANNOT Change the Table Name            *\
\* But you can add more chip element here by using same format              */
const uInt32 DevSpecIdx_Table[ ] = 
{
	(uInt32)Spec_LV33, sizeof(Spec_LV33), /* Specification Setting Table Address */
	(uInt32)Spec_LV18, sizeof(Spec_LV18), /* Specification Setting Table Address */
	
	/* here adding more... */
};

const uChar ChipSPIMaxClkTable[ ] =
{ /* Table Sequence by Spec Index Code Constant Define */
	HSPI_CLK_8M25,
	HSPI_CLK_5M5, /* 1.8V need use low speed clock                 to stable */

	/* here adding more... */
} ;


/* Device Pin Mapping Index Table, You CANNOT Change the Table Name   *\
\* But you can add more chip element here by using same format              */
const uInt32 DevPmpIdx_Table[ ] = 
{
	(uInt32)Pmp_H3V3SO8UG4A, sizeof(Pmp_H3V3SO8UG4A),
	(uInt32)Pmp_H3V3SO16UG4A, sizeof(Pmp_H3V3SO16UG4A),
	(uInt32)Pmp_H3V3SO8UG4A, sizeof(Pmp_H3V3SO8UG4A), /* BGA24 use SO8 same PinDriver */

	/* here adding more... */
};

/***** CAN NOT Change the Table Name *****/
const uShort DmmIOPortDefine[ ][5] =
{ /* Table Sequence by PinMap Index Code Constant Define */
/*	  CS   WP   HOLD RST  CS2, CAN NOT modify Sequence */
	{D21, D23, D27,  D01, D20,}, /* 25# SO8, RST Dummy define for System Chk Error*/
	{D23, D25, D17,  D19, D22,}, /* 25# S16 */
	{D21, D23, D27,  D01, D20,}, /* 25# BGA24 use SO8 same PinDriver, RST Dummy define for System Chk Error*/

	/* here adding more... */
};

/***** CAN NOT Change the Table Name *****/
const uChar SpiPortTypeDefine[ ] =
{ /* Table Sequence by PinMap Index Code Constant Define */
	SPI25_SO8,
	SPI25_SO16,
	SPI25_SO8, /* BGA24 use SO8 same PinDriver */

	/* here adding more... */
} ;


/*********** Device Sector Protect Address Main Table ***************\
\***** Table Sequence by Block/Sector Index Code Constant Define *****/
const uShort ChipPageSizeTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
	256, 256, 256, 256
	/* here adding more... */
} ;


const uShort ChipEraseTimeTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
	256, 256, 512, 1024, /* Max time for second scale */	/* here adding more... */

	/* here adding more... */
} ;


const uChar ProtectRegisterTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
    0xBC, 0xBC, 0xBC, 0xBC,

	/* here adding more... */
} ;


#endif



 
