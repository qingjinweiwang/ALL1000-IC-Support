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


/********************* total 13 algorithm subroutine entance define *********************/
extern uChar PowerOnFun(void);
extern uChar PowerOffFun(void);
extern uChar ReadFun(void);
extern uChar ProgramFun(void);
extern uChar VerifyFun(void);
extern uChar EraseFun(void);
extern uChar BlankCheckFun(void);
extern uChar IDCheckFun(void);
extern uChar SecureFun(void);
extern uChar CheckSumFun(void);



/******************************************************************************\
	Algorithm Macro Define Table Here

\******************************************************************************/
/* Algorithm Index Code Constant Define, You Can Change Name by Yourself */
#define	AlgoIdx_SWD			0/* max define 255 */


/* Block/Sector Index Code Constant Define, You Can Change Name by Yourself */
#define	BlkIdx_SK32K324_8M			0/* max define 255 */


/* Specifications Index Code Constant Define, You Can Change Name by Yourself */
#define	SpcIdx_LV33		0/* max define 255 */


/* Pin Mapping Index Code Constant Define, You Can Change Name by Yourself */
#define	PmpIdx_P3V3SWD_Easyfpga		0/* max define 255 */


/* Version Number of Driver Master (Pseudo!), You CANNOT Change the Macro Name */
#define	DrvMST_Version		0xFFFF	/* Version: Pseudo! */


/* Version Number of Device Driver, You CANNOT Change the Macro Name */
#define	DevDRV_Version		0x3130	/* Version: 1.1, Use Ascii code */
/**********************************************************************

Version: 1.0	NXP

Version: 1.1	NXP 优化烧录时间，增加USB握手，T卡更新

***********************************************************************/

/* Gang Type for system, You CANNOT Change the Macro Name */
#define	SystemGang			Gang4Type /* Gang2Type, Gang4Type or Gang8Type */


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
const uShort Block_SK32K324_8M[] =
{
	//	MainByteSumCal|MainByte8Width,
	MainByte8Width,
	0x0000,0x0000,	/* RAM Base Address */
	0x0000,0x0000, 0x0041,0xFFFF, 	/* Chip Start Address, Chip End Address */

	0x0000,	0x00FF,	0x00FF, /* PM */
	0x0000,0x0000,
	0x0000,0x0000, 0x0041,0xFFFF,
	Noun_Block0,

	0x03FF,0x00FF,0x00FF,	 /* Special bit in SRAM */
	0x0000,0x0000,
	0x0000,0x0000, 0x0000,0x001F,
	Noun_BlkPtStatus,
};



/******************************************************************************
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
	330,		VCCS,	x10mV,		300,		360,		NounVCCS,
	330,		VPPP,   x10mV,		330,		330,		NounVPPP,
	360,		VCCH,	x10mV,		300,		360,		NounVCCH,
	300,		VCCL,	x10mV,		300,		360,		NounVCCL,
	340,		VIH,	x10mV,		300,		360,		NounVIH,	/* VIH Volts MUST be same with FPGA Pin Type Setting!! */
	10,		RSCPD,	x15nS,		4,		32,		NounRSCPD, /* Read FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
	8,		WSCPD,	x15nS,		4,		32,		NounWSCPD, /* Write FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
};



/***************************************************************************\
	Pin Map Table for Definition
	Notes: CAN NOT change Terminal Table sequence, if donot have this type
			pin, then skit this table directly is OK !!!
\***************************************************************************/
const uShort Pmp_P3V3SWD_Easyfpag[] =
{/* for ALL1000-ISP-sck */
	/* Socket A */
	V47,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	P01,
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G48,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
	/* Socket B */
	V95,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	P09,
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G96,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
	/* Socket C */
	V143,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	P17,
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G144,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
	/* Socket D */
	V191,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	P25,
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G192,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
};



/************************************************************************************************\

\************************************************************************************************/
/* Driver Element Combination Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by same format                        */
const uChar DrvElement_Table[][5] =
{
	{ AlgoIdx_SWD, BlkIdx_SK32K324_8M, SpcIdx_LV33, PmpIdx_P3V3SWD_Easyfpga, AlgoIC_0x00 }, /* A31G316RLN(LQFP64) */

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
	{	PowerOnFun,						PowerOffFun,
		ReadFun,		ProgramFun,		VerifyFun,
		EraseFun,		BlankCheckFun,		UselessAlgoFun,
		IDCheckFun,		SecureFun,			CheckSumFun,
		UselessAlgoFun,						UselessAlgoFun,
	},
};


/* Device Block/Sector	Index Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by using same format               */
const uInt32 DevBlockIdx_Table[] =
{
	(uInt32)Block_SK32K324_8M, sizeof(Block_SK32K324_8M),
	/* here adding more... */
};


/* Device Spec Index Table, You CANNOT Change the Table Name            *\
\* But you can add more chip element here by using same format              */
const uInt32 DevSpecIdx_Table[] =
{
	(uInt32)Spec_LV33, sizeof(Spec_LV33), /* Specification Setting Table Address */

	/* here adding more... */
};


/* Device Pin Mapping Index Table, You CANNOT Change the Table Name   *\
\* But you can add more chip element here by using same format              */
const uInt32 DevPmpIdx_Table[] =
{	/* User CAN NOT change the pin table type sequency */
	(uInt32)Pmp_P3V3SWD_Easyfpag, sizeof(Pmp_P3V3SWD_Easyfpag),
	/* here adding more... */
};

/***** CAN NOT Change the Table Name *****/
const uShort DmmIOPortDefine[][8] =
{ /* Table Sequence by PinMap Index Code Constant Define */
/*	 VCC1  GND1 SWD  SCK  RST  TMS  TDI   JCOM, CAN NOT modify Sequence */
	{V47,  G48, D12, D35, D29, D21, D22,  D23}, /* 24# pinout, RST=WP, S=A0, TDI=A1, JCOM=A2 */

	/* here adding more... */
};

/*********** Device Sector Protect Address Main Table ***************\
\***** Table Sequence by Block/Sector Index Code Constant Define *****/
const uShort ChipPageSizeTable[] =
{ /* Table Sequence by AlgoIdx Code Constant Define */
	0,
	/* here adding more... */
};

#endif



