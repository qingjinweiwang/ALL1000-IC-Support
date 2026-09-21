/******************************************************************************\
	Programmer Driver Total Setting Here
		CAT25256VI-GT3 SPI EEPROM (256Kb)
\*******************************************************************************/
#ifndef ProgramDrv_H_
#define ProgramDrv_H_

#include "h\Type.h"
#include "h\FirmWare.h"
#include "h\BlockSectorDef.h"
#include "h\Subroutine.h"
#include "h\PinDef.h"


/********************* total 13 algorithm subroutine entance define *********************/
extern uChar PowerOnFun(void) ;
extern uChar PowerOffFun(void) ;
extern uChar ReadFun(void) ;
extern uChar ProgramFun(void) ;
extern uChar VerifyFun(void) ;
extern uChar EraseFun(void) ;
extern uChar BlankCheckFun(void) ;
extern uChar IDCheckFun(void) ;
extern uChar CheckSumFun(void) ;
extern uChar ProtectFun(void) ;
extern uChar UnProtectFun(void) ;



/******************************************************************************\
	Algorithm Macro Define Table Here

\******************************************************************************/
/* Algorithm Index Code Constant Define, You Can Change Name by Yourself */
#define	AlgoIdx_CAT25256			0 /* max define 255 */

/* Block/Sector Index Code Constant Define, You Can Change Name by Yourself */
#define BlkIdx_CAT25256VI_GT3		0 /* max define 255 */



/* Specifications Index Code Constant Define, You Can Change Name by Yourself */
#define SpcIdx_3V3			0/* max define 255 */
#define SpcIdx_1V8          1/* max define 255 */
#define SpcIdx_3V0			2/* max define 255 */
#define SpcIdx_3V5			3/* max define 255 */


/* CAT25256 configurable status bits: BP0, BP1, and WPEN. */
#define CATStatusConfigMask		0x008C

/* Pin Mapping Index Code Constant Define, You Can Change Name by Yourself */
#define PmpIdx_H3V3SO8UG4A		0/* max define 255 */
#define PmpIdx_H3V3SO16UG4A		1/* max define 255 */
#define PmpIdx_H3V3TFBGA24SG4A	2/* max define 255 */


/* Version Number of Driver Master (Pseudo!), You CANNOT Change the Macro Name */
#define	DrvMST_Version		0xFFFF	/* Version: Pseudo! */


/* Version Number of Device Driver, You CANNOT Change the Macro Name */
#define	DevDRV_Version		0x3132	/* Version: 1.2, Use Ascii code */
/*
*	Version: 1.2
*	CAT25256VI-GT3 SPI EEPROM driver
*	256Kb (32KB), 64-byte page, 4MHz FPGA SCK @ 3.3V
*	Single algorithm group, 16-bit address, no erase command, no JEDEC ID
*/

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
const uShort Block_CAT25256VI_GT3[] =//256Kb SPI EEPROM, 32K bytes, 64-byte page
{
    /*MainByteSumCal|MainByte8Width,*/
	MainByte8Width,
	0x0000,0x0000,	/* RAM Base Address */
	0x0000,0x0000, 0x0000,0x7FFF, 	/* Chip Start Address, Chip End Address */

	0x0000,0x00FF,0x00FF,
	0x0000,0x0000,
	0x0000,0x0000, 0x0000,0x7FFF,
	Noun_Block0,

//Status Register
	0x03FF,0x00FF,0x0000,
	0x0000,0x0000,
	0x0000,0x0000, 0x0000,0x0000,
	Noun_Block5,

	0x02FF,0x00FF,0x0000, /* No use block, just for spec setting */
	0x0000,0x0003,
	0x0000,0x0003, 0x0000,0x0003,
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
const uShort Spec_3V3[ ] =
{ /* Typical   ParaType  Scale          MinVal     MaxVal      ParaName */
	330,		VCCR,	x10mV,		270,		360,		NounVCCR,
	330,		VCCP,	x10mV,		270,		360,		NounVCCP,
	330,		VCCE,	x10mV,		270,		360,		NounVCCE,
	360,		VCCH,	x10mV,		270,		360,		NounVCCH,
	270,		VCCL,	x10mV,		270,		360,		NounVCCL,
	350,		VIH,	x10mV,		270,		360,		NounVIH,	/* VIH Volts MUST be same with FPGA Pin Type Setting!! */
	133,		RSCPD,	x15nS,		97,			160,	NounRSCPD, /* 4MHz SCK: 8bit ~= 2.00us, x15nS value ~= 133 */
	133,		WSCPD,	x15nS,		97,			160,	NounWSCPD, /* 4MHz SCK: 8bit ~= 2.00us, x15nS value ~= 133 */
};

const uShort Spec_1V8[ ] =
{ /* Typical   ParaType  Scale          MinVal     MaxVal      ParaName */
	180,		VCCR,	x10mV,		165,		195,		NounVCCR,
	180,		VCCP,	x10mV,		165,		195,		NounVCCP,
	180,		VCCE,	x10mV,		165,		195,		NounVCCE,
	195,		VCCH,	x10mV,		165,		195,		NounVCCH,
	165,		VCCL,	x10mV,		165,		195,		NounVCCL,
	190,		VIH,	x10mV,		165,		195,		NounVIH,	/* VIH Volts MUST be same with FPGA Pin Type Setting!! */
	133,		RSCPD,	x15nS,		97,			160,		NounRSCPD, /* 4MHz SCK: 8bit ~= 2.00us, x15nS value ~= 133 */
	133,		WSCPD,	x15nS,		97,			160,		NounWSCPD, /* 4MHz SCK: 8bit ~= 2.00us, x15nS value ~= 133 */
};

const uShort Spec_3V0[] =
{ /* Typical   ParaType  Scale          MinVal     MaxVal      ParaName */
	300,		VCCR,	x10mV,		270,		330,		NounVCCR,
	300,		VCCP,	x10mV,		270,		330,		NounVCCP,
	300,		VCCE,	x10mV,		270,		330,		NounVCCE,
	330,		VCCH,	x10mV,		270,		330,		NounVCCH,
	270,		VCCL,	x10mV,		270,		330,		NounVCCL,
	310,		VIH,	x10mV,		270,		360,		NounVIH,	/* VIH Volts MUST be same with FPGA Pin Type Setting!! */
	133,		RSCPD,	x15nS,		97,			160,		NounRSCPD, /* 4MHz SCK: 8bit ~= 2.00us, x15nS value ~= 133 */
	133,		WSCPD,	x15nS,		97,			160,		NounWSCPD, /* 4MHz SCK: 8bit ~= 2.00us, x15nS value ~= 133 */
};

const uShort Spec_3V5[] =
{ /* Typical   ParaType  Scale          MinVal     MaxVal      ParaName */
	350,		VCCR,	x10mV,		270,		360,		NounVCCR,
	350,		VCCP,	x10mV,		270,		360,		NounVCCP,
	350,		VCCE,	x10mV,		270,		360,		NounVCCE,
	350,		VCCH,	x10mV,		270,		360,		NounVCCH,
	350,		VCCL,	x10mV,		270,		360,		NounVCCL,
	350,		VIH,	x10mV,		270,		360,		NounVIH,	/* VIH Volts MUST be same with FPGA Pin Type Setting!! */
	133,		RSCPD,	x15nS,		97,			160,		NounRSCPD, /* 4MHz SCK: 8bit ~= 2.00us, x15nS value ~= 133 */
	133,		WSCPD,	x15nS,		97,			160,		NounWSCPD, /* 4MHz SCK: 8bit ~= 2.00us, x15nS value ~= 133 */
};


/***************************************************************************\
	Pin Map Table for Definition
	Notes: CAN NOT change Terminal Table sequence, if donot have this type
			pin, then skit this table directly is OK !!!
\***************************************************************************/
const uShort Pmp_H3V3SO8UG4A[ ] =
{/* for CAT25256 SPI EEPROM */
/* Socket A */
	V28,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G24,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
/* Socket B */
	V76,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G72,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
/* Socket C */
	V124,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G120,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
/* Socket D */
	V172,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G168,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
} ;


const uShort Pmp_H3V3SO16UG4A[ ] =
{/* for CAT25256 SPI EEPROM */
/* Socket A */
	V18,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G26,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
/* Socket B */
	V66,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G74,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
/* Socket C */
	V114,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G122,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
/* Socket D */
	V162,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G170,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
} ;


const uShort Pmp_H3V3TFBGA24SG4A[ ] =
{/* for CAT25256 SPI EEPROM */
/* Socket A */
	V19,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G17,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
/* Socket B */
	V69,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G67,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
/* Socket C */
	V93,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G95,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
/* Socket D */
	V121,
	VCCTbl_Terminal , /* 1#: Pin VCC Setting Table Address */
	VPPESTbl_Terminal , /*2#: Pin VPP/VPE/VPS Setting Table Address */
	VPIOTbl_Terminal , /* 3#: Pin VPIO(format: X19) Setting Table Address */
	G123,
	GNDTbl_Terminal , /* 4#: Pin GND Setting Table Address */
	NUNCTbl_Terminal , /* 5#: Pin No Use Setting Table Address */
} ;


/************************************************************************************************\

\************************************************************************************************/
/* Driver Element Combination Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by same format                        */
const uChar DrvElement_Table[ ][5] =
{
	{ AlgoIdx_CAT25256, BlkIdx_CAT25256VI_GT3, SpcIdx_3V3,  PmpIdx_H3V3SO8UG4A, AlgoIC_0x00 },//ON Semiconductor CAT25256VI-GT3 SOP8


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
	{	PowerOnFun,								PowerOffFun,
		ReadFun,			ProgramFun,			VerifyFun,
		EraseFun,			BlankCheckFun,		UselessAlgoFun,
		IDCheckFun,			UselessAlgoFun,		CheckSumFun,
		ProtectFun,								UnProtectFun,
	},
};



/* Device Block/Sector	Index Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by using same format               */
const uInt32 DevBlockIdx_Table[ ] =
{
	(uInt32)Block_CAT25256VI_GT3,sizeof(Block_CAT25256VI_GT3),	//0

	/* here adding more... */
};


/* Device Spec Index Table, You CANNOT Change the Table Name            *\
\* But you can add more chip element here by using same format              */
const uInt32 DevSpecIdx_Table[ ] =
{/* Specification Setting Table Address */
	(uInt32)Spec_3V3, sizeof(Spec_3V3),		//0
	(uInt32)Spec_1V8, sizeof(Spec_1V8),		//1
	(uInt32)Spec_3V0,sizeof(Spec_3V0),		//2
	(uInt32)Spec_3V5,sizeof(Spec_3V5),		//3

	/* here adding more... */
};


/* Device Pin Mapping Index Table, You CANNOT Change the Table Name   *\
\* But you can add more chip element here by using same format              */
const uInt32 DevPmpIdx_Table[ ] =
{	/* User CAN NOT change the pin table type sequency */
	(uInt32)Pmp_H3V3SO8UG4A, sizeof(Pmp_H3V3SO8UG4A),			//0
	(uInt32)Pmp_H3V3SO16UG4A, sizeof(Pmp_H3V3SO16UG4A),			//1
	(uInt32)Pmp_H3V3TFBGA24SG4A, sizeof(Pmp_H3V3TFBGA24SG4A),	//2
	/* here adding more... */
};



/*********** Device Parameter Table ***************\
\***** Table Sequence by Block/Sector Index Code Constant Define *****/
const uShort ChipPageSizeTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
	64,//0 CAT25256 page write size (64 bytes)
	/* here adding more... */
} ;


const uChar ChipSPIMaxClkTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
  /* 4MHz FPGA SCK, within CAT25256 max 5MHz at 3.3V */
    4,//0
	/* here adding more... */
} ;

/***************************************************************/
/******below are register MASK Table, no register set 0x0000****/
/***************************************************************/
const uShort ProtectRegisterTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
  /* Status Register: BP0(0x04) | BP1(0x08) | WPEN(0x80). */
	CATStatusConfigMask,//0

	/* here adding more... */
} ;

const uShort ProtectRegisterDefaultTable[] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
  /* Status Register default: all protection bits clear */
	0x0000,//0

	/* here adding more... */
};

#endif
