/*
V1.1 2026-09-08
1. First version for MICRON M25PE40 T9HX only (Datasheet Rev.D, 01/2018).
2. Modify from previous SPI NOR Flash driver template.
3. Main Flash size is 512KB, address range 000000h-07FFFFh, 3-byte address only.
4. JEDEC ID (9Fh) is 20h-80h-13h.
5. Active flow uses JEDEC ID (9Fh), Read Status Register (05h), Page Program
   (02h), Page Write (0Ah), and Bulk Erase (C7h).
6. T9HX-only features: Status Register BP2-BP0/SRWD/W# protection and Bulk Erase.
7. Target package is MP: VFQFPN8, 6mm x 5mm.
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
extern uChar CheckSumFun(void) ;
extern uChar ProtectFun(void) ;
extern uChar UnProtectFun(void) ;



/******************************************************************************\
	Algorithm Macro Define Table Here

\******************************************************************************/
/* Algorithm Index Code Constant Define, You Can Change Name by Yourself */
#define AlgoIdx_M25PE40        0
/* Block/Sector Index Code Constant Define, You Can Change Name by Yourself */ 
#define BlkIdx_M25PE40VMPxxx	    0
/* Specifications Index Code Constant Define, You Can Change Name by Yourself */
#define SpcIdx_3V3			    0/* max define 255 */

/* Pin Mapping Index Code Constant Define, You Can Change Name by Yourself */
#define PmpIdx_H3V3VFQFPN8UG4A	0/* max define 255 */

/* Version Number of Driver Master (Pseudo!), You CANNOT Change the Macro Name */
#define	DrvMST_Version		0xFFFF	/* Version: Pseudo! */

/* Version Number of Device Driver, You CANNOT Change the Macro Name */
#define DevDRV_Version         0x3132  /* Version: 1.2, Use Ascii code */

/* Gang Type for system, You CANNOT Change the Macro Name */
#define	SystemGang			Gang4Type /* Gang2Type, Gang4Type or Gang8Type */
/* M25PE40 JEDEC ID returned by instruction 9Fh.
 * 注：IDCheckFun 实际比较的是上位机 Project 下发的 ManufactID/DeviceIDLo,
 * 以下宏为期望值的工程内记录,上位机 Auto ID 必须配置为 20h-80h-13h 与之相符 */
#define M25PE40_MANUFACTURER_ID  0x20
#define M25PE40_MEMORY_TYPE      0x80
#define M25PE40_CAPACITY_ID      0x13
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

const uShort Block_M25PE40VMPxxx[] = 
{
//	MainByteSumCal|MainByte8Width,
	MainByte8Width,
	0x0000,0x0000,	/* RAM Base Address */
	0x0000,0x0000, 0x0007,0xFFFF, 	/* Chip Start Address, Chip End Address */
	
	0x0000,0x00FF,0x00FF,	
	0x0000,0x0000,
	0x0000,0x0000, 0x0007,0xFFFF, 
	Noun_Block0,

	/************* Status Register *************/
	0x0223,0x00FF,0x0000,
	0x0000,0x0000,
	0x0000,0x0000, 0x0000,0x0000,
	Noun_Block1,

	/************* Driver operation control *************/
	0x02FF,0x00FF,0x0000,
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
	320,		VCCR,	x10mV,		270,		360,		NounVCCR,
	320,		VCCP,	x10mV,		270,		360,		NounVCCP,
	320,		VCCE,	x10mV,		270,		360,		NounVCCE,
	320,		VCCH,	x10mV,		270,		360,		NounVCCH,
	320,		VCCL,	x10mV,		270,		360,		NounVCCL,
	320,		VIH,		x10mV,		270,		360,		NounVIH,	/* VIH Volts MUST be same with FPGA Pin Type Setting!! */
	10,		RSCPD,	x15nS,		4,		32,		NounRSCPD, /* Read FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
	8,		WSCPD,	x15nS,		4,		32,		NounWSCPD, /* Write FPGA, CorePara>=8, NorFlash>=10 for SCLK=33M! */
};



/***************************************************************************\
	Pin Map Table for Definition
	Notes: CAN NOT change Terminal Table sequence, if donot have this type
			pin, then skit this table directly is OK !!!
\***************************************************************************/


const uShort Pmp_H3V3VFQFPN8UG4A[ ] =
{/* M25PE40 MP package: VFQFPN8, 6mm x 5mm */
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

/************************************************************************************************\

\************************************************************************************************/
/* Driver Element Combination Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by same format                        */
const uChar DrvElement_Table[ ][5] = 
{
///////// 3.3V ///////////////////////
	{ AlgoIdx_M25PE40, BlkIdx_M25PE40VMPxxx, SpcIdx_3V3, PmpIdx_H3V3VFQFPN8UG4A, AlgoIC_0x00}, /* M25PE40-VMPxxx, VFQFPN8 */
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
	{	PowerOnFun,							   PowerOffFun,
		ReadFun,			ProgramFun,		   VerifyFun,
		EraseFun,		BlankCheckFun,		   UselessAlgoFun,
		IDCheckFun,		UselessAlgoFun,	       CheckSumFun,
		ProtectFun,							   UnProtectFun,
	},
};



/* Device Block/Sector	Index Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by using same format               */
/* These firmware ABI tables store ROM addresses in 32-bit integer slots. */
const uInt32 DevBlockIdx_Table[ ] = 
{
	(uInt32)Block_M25PE40VMPxxx, sizeof(Block_M25PE40VMPxxx),
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
	(uInt32)Pmp_H3V3VFQFPN8UG4A, sizeof(Pmp_H3V3VFQFPN8UG4A),
	/* here adding more... */
};

/*********** Device Sector Protect Address Main Table ***************\
\***** Table Sequence by Block/Sector Index Code Constant Define *****/
const uShort ChipPageSizeTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
	256, 
	/* here adding more... */
} ;


const uShort ChipEraseTimeTable[ ] =
{
    11,  /* tBE max 10s; one-second driver polling margin */
};


const uChar ChipSPIMaxClkTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
   11,   /* FPGA SPI Delay Time = (1/xxM)*8Bit/10ns/2, conservative bring-up value */
	/* here adding more... */
} ;


const uShort ProtectRegisterTable[ ] =
{
    /*
     * M25PE40 has one 8-bit Status Register.
     *
     * bit7 SRWD = Status Register Write Disable
     * bit6       = 0
     * bit5       = 0
     * bit4 BP2   = 0x10
     * bit3 BP1   = 0x08
     * bit2 BP0   = 0x04
     * bit1 WEL   = Read Only
     * bit0 WIP   = Read Only
     *
     * Only BP2-BP0 are opened for software protect setting.
     * SRWD is not opened to avoid entering Hardware Protected Mode
     * when W# is LOW, which could prevent UnProtect from changing BP bits.
     */
    0x001C,
};

const uChar ProtectRegisterDefaultTable[ ] =
{
    0x00,       /* SR low byte：无区域保护 */
};

#endif
