/*
V1.1 2026-09-02

*/
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
extern uChar PowerOnFun(void) ;
extern uChar PowerOffFun(void) ;
extern uChar ReadFun(void) ;
extern uChar ProgramFun(void) ;
extern uChar VerifyFun(void) ;
extern uChar EraseFun(void) ;
extern uChar BlankCheckFun(void) ;
extern uChar IDCheckFun(void) ;
extern uChar SecureFun(void) ;
extern uChar CheckSumFun(void) ;
extern uChar ProtectFun(void) ;
extern uChar UnProtectFun(void) ;



/******************************************************************************\
	Algorithm Macro Define Table Here

\******************************************************************************/
/* Algorithm Index Code Constant Define, You Can Change Name by Yourself */
#define AlgoIdx_N25Q256A          0
/* Block/Sector Index Code Constant Define, You Can Change Name by Yourself */
#define BlkIdx_N25Q256Ax3ESFxxx   0
/* Specifications Index Code Constant Define, You Can Change Name by Yourself */
#define SpcIdx_3V3			    0/* max define 255 */

/* Pin Mapping Index Code Constant Define, You Can Change Name by Yourself */
#define PmpIdx_H3V3SO16UG4A3		0/* max define 255 */

/* Version Number of Driver Master (Pseudo!), You CANNOT Change the Macro Name */
#define	DrvMST_Version		0xFFFF	/* Version: Pseudo! */

/* Version Number of Device Driver, You CANNOT Change the Macro Name */
#define	DevDRV_Version		0x3131	/* Version: 1.1, Use Ascii code */

/* Gang Type for system, You CANNOT Change the Macro Name */
#define	SystemGang			Gang4Type /* Gang2Type, Gang4Type or Gang8Type */
/* N25Q256A JEDEC ID returned by instruction 9Fh. */
#define N25Q256A_MANUFACTURER_ID   0x20
#define N25Q256A_MEMORY_TYPE       0xBA
#define N25Q256A_CAPACITY_ID       0x19
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

const uShort Block_N25Q256Ax3ESFxxx[] =
{
    /*
     * Main Flash 32MB
     */
    MainByte8Width,
    0x0000,0x0000,
    0x0000,0x0000, 0x0200,0x0041,
    /**************** Main Flash ****************/
    0x0000,0x00FF,0x00FF,                 /* Main Flash, 32 MByte */
    0x0000,0x0000,                         /* RAM start: 00000000h */
    0x0000,0x0000,                         /* Device start: 00000000h */
    0x01FF,0xFFFF,                         /* Device end:   01FFFFFFh */
    Noun_Block0,

    /************* OTP Array *************/
    0x8128,0x00FF,0x00FF,                 /* Diagnostic: OTP Read-Off; Program/Verify/BlankCheck/Checksum retained */
    0x0200,0x0000,                         /* DRAM start: 02000000h */
    0x0000,0x0000,
    0x0000,0x0040,                         /* OTP data 00h-3Fh + control byte 40h */
    Noun_Block1,

    /************* Status Register *************/
    0x023B,0x00FF,0x0000,                 /* Status Register S7-S0, Secure-Off */
    0x0000,0x0000,                         /* SRAM offset 0 */
    0x0000,0x0000,
    0x0000,0x0000,                         /* one status byte */
    Noun_Block2,

	/****************Non-Volatile Configuration Register ****************/
	0x0227,0x00FF,0xFFFF,
	0x0000,0x0001,         					/* SRAM offset 1: low byte */
	0x0000,0x0001,
	0x0000,0x0002,         					/* SRAM offset 2: high byte */
	Noun_Block3,

    /************* Driver operation control *************/
    0x02FF,0x00FF,0x0000,                 /* Operation control byte for the C driver */
    0x0000,0x0003,                         /* SRAM offset 3 */
    0x0000,0x0003,
    0x0000,0x0003,
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
	340,		VCCR,	x10mV,		300,		360,		NounVCCR,
	340,		VCCP,	x10mV,		300,		360,		NounVCCP,
	340,		VCCE,	x10mV,		300,		360,		NounVCCE,
	360,		VCCH,	x10mV,		300,		360,		NounVCCH,
	300,		VCCL,	x10mV,		300,		360,		NounVCCL,
	330,		VIH,		x10mV,		300,		360,		NounVIH,	/* VIH Volts MUST be same with FPGA Pin Type Setting!! */
	10,		RSCPD,	x15nS,		4,		32,		NounRSCPD, /* Read FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
	8,		WSCPD,	x15nS,		4,		32,		NounWSCPD, /* Write FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
};



/***************************************************************************\
	Pin Map Table for Definition
	Notes: CAN NOT change Terminal Table sequence, if donot have this type
			pin, then skit this table directly is OK !!!
\***************************************************************************/


const uShort Pmp_H3V3SO16UG4A3[ ] =
{/* for SPI25F */
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

/************************************************************************************************\

\************************************************************************************************/
/* Driver Element Combination Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by same format                        */
const uChar DrvElement_Table[ ][5] =
{
///////// 3.3V ///////////////////////
	{ AlgoIdx_N25Q256A, BlkIdx_N25Q256Ax3ESFxxx, SpcIdx_3V3, PmpIdx_H3V3SO16UG4A3, AlgoIC_0x00}, /* N25Q256Ax3ESFxxx */
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
	{	PowerOnFun,							    PowerOffFun,
		ReadFun,			ProgramFun,		    VerifyFun,
		EraseFun,			BlankCheckFun,		UselessAlgoFun,
		IDCheckFun,			SecureFun,			CheckSumFun,
		ProtectFun,								UnProtectFun,
	},
};



/* Device Block/Sector	Index Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by using same format               */
/* These firmware ABI tables store ROM addresses in 32-bit integer slots. */
#pragma diag_suppress 1296
const uInt32 DevBlockIdx_Table[ ] =
{
	(uInt32)Block_N25Q256Ax3ESFxxx, sizeof(Block_N25Q256Ax3ESFxxx),
	/* here adding more... */
};


/* Device Spec Index Table, You CANNOT Change the Table Name            *\
\* But you can add more chip element here by using same format              */
const uInt32 DevSpecIdx_Table[ ] =
{
	(uInt32)Spec_3V3, sizeof(Spec_3V3), /* Specification Setting Table Address */
	/* here adding more... */
};


/* Device Pin Mapping Index Table, You CANNOT Change the Table Name   *\
\* But you can add more chip element here by using same format              */
const uInt32 DevPmpIdx_Table[ ] =
{	/* User CAN NOT change the pin table type sequency */
	(uInt32)Pmp_H3V3SO16UG4A3, sizeof(Pmp_H3V3SO16UG4A3),
	/* here adding more... */
};
#pragma diag_default 1296



/*********** Device Sector Protect Address Main Table ***************\
\***** Table Sequence by Block/Sector Index Code Constant Define *****/
const uShort ChipPageSizeTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
	256,
	/* here adding more... */
} ;


const uShort ChipEraseTimeTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
	500, /* N25Q256A BULK ERASE maximum 480s; timeout = value x 100 x 10ms */
} ;


const uChar ChipSPIMaxClkTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
    22,   /* FPGA SPI Delay Time = (1/xxM)*8Bit/10ns/2, conservative bring-up value */
	/* here adding more... */
} ;


const uShort ProtectRegisterTable[ ] =
{
    /*
     * N25Q256A has one 8-bit Status Register.
     * Software protection uses BP3, TB and BP2-BP0 (bits 6:2).
     * SRWD is excluded so UnProtect can always update these bits.
     */
    0x007C,
};

const uChar ProtectRegisterDefaultTable[ ] =
{
    0x00,       /* no protected array region */
};


#endif
