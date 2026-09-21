/*
V1.0 2026-08-12
1. First version for TSINGTENG TH25Q-80HB (Datasheet V1.6, 2022-03-26).
2. Main Flash size is 1 MByte, address range 000000h-0FFFFFh, 3-byte address only.
3. Three 1K-Byte user Security Registers (device addr 1000h/2000h/3000h).
4. JEDEC ID (9Fh) is CDh-60h-14h.
5. Single 16-bit Status Register: low byte read by 05h (SRP0,BP4-BP0,WEL,WIP),
   high byte read by 35h (SUS1,CMP,LB3-LB1,SUS2,QE,SRP1).
6. Protection: BP4-BP0 in SR low byte, CMP in SR high byte (bit6).
   WRSR(01h) must write both bytes in one command; a 1-byte write clears CMP/QE.
   LB1-LB3 are OTP bits and are never written by this driver.
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
#define AlgoIdx_TH25Q80HB         0
/* Block/Sector Index Code Constant Define, You Can Change Name by Yourself */ 
#define BlkIdx_TH25Q80HBMSDx	    0
/* Specifications Index Code Constant Define, You Can Change Name by Yourself */
#define SpcIdx_3V3			    0/* max define 255 */

/* Pin Mapping Index Code Constant Define, You Can Change Name by Yourself */
#define PmpIdx_H3V3SO8UG4A		0/* max define 255 */

/* Version Number of Driver Master (Pseudo!), You CANNOT Change the Macro Name */
#define	DrvMST_Version		0xFFFF	/* Version: Pseudo! */

/* Version Number of Device Driver, You CANNOT Change the Macro Name */
#define	DevDRV_Version		0x3130	/* Version: 1.0, Use Ascii code */

/* Gang Type for system, You CANNOT Change the Macro Name */
#define	SystemGang			Gang4Type /* Gang2Type, Gang4Type or Gang8Type */
/* TH25Q-80HB JEDEC ID returned by instruction 9Fh.
 * 注：IDCheckFun 实际比较的是上位机 Project 下发的 ManufactID/DeviceIDLo,
 * 以下宏为期望值的工程内记录,上位机 Auto ID 必须配置为 CDh-60h-14h 与之相符 */
#define TH25Q80HB_MANUFACTURER_ID  0xCD
#define TH25Q80HB_MEMORY_TYPE      0x60
#define TH25Q80HB_CAPACITY_ID      0x14
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

const uShort Block_TH25Q80HBMSDx[] = 
{
    /*
     * Main Flash 1MB
     * Security Register 1  1KB
     * Security Register 2  1KB
     * Security Register 3  1KB
     */
    MainByte8Width,
    0x0000,0x0000,                         /* RAM Base Address */

    /*
     * 上位机虚拟数据范围
     *
     * Main Flash: 00000000h - 000FFFFFh
     * Security 1: 00100000h - 001003FFh
     * Security 2: 00100400h - 001007FFh
     * Security 3: 00100800h - 00100BFFh
     */
    0x0000,0x0000, 0x0010,0x0BFF,         /* Virtual data range: 00000000h-00100BFFh */

    /**************** Main Flash ****************/
    0x0000,0x00FF,0x00FF,                 /* Main Flash, 1 MByte */
    0x0000,0x0000,                         /* RAM start: 00000000h */
    0x0000,0x0000,                         /* Device start: 000000h */
    0x000F,0xFFFF,                         /* Device end:   0FFFFFh */
    Noun_Block0,

    /************* Security Register 1 *************/
    0x8008,0x00FF,0x00FF,                 /* Security Register 1, 1 KByte */
    0x0010,0x0000,                         /* RAM start: 00100000h */
    0x0000,0x1000,                         /* Device start: 001000h */
    0x0000,0x13FF,                         /* Device end:   0013FFh */
    Noun_Block1,

    /************* Security Register 2 *************/
    0x8008,0x00FF,0x00FF,                 /* Security Register 2, 1 KByte */
    0x0010,0x0400,                         /* RAM start: 00100400h */
    0x0000,0x2000,                         /* Device start: 002000h */
    0x0000,0x23FF,                         /* Device end:   0023FFh */
    Noun_Block2,

    /************* Security Register 3 *************/
    0x8008,0x00FF,0x00FF,                 /* Security Register 3, 1 KByte */
    0x0010,0x0800,                         /* RAM start: 00100800h */
    0x0000,0x3000,                         /* Device start: 003000h */
    0x0000,0x33FF,                         /* Device end:   0033FFh */
    Noun_Block3,

    /************* Status Register low(05h) and high(35h) byte *************/
    0x0223,0x00FF,0x0000,                 /* Status Register S7-S0 and S15-S8 */
    0x0000,0x0000,                         /* SRAM offset 0 */
    0x0000,0x0000,
    0x0000,0x0001,                         /* 两个状态字节 */
    Noun_Block4,

    /************* Driver operation control *************/
    0x03FF,0x00FF,0x0000,                 /* Operation control byte for the C driver */
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
	330,		VCCR,	x10mV,		230,		360,		NounVCCR, 
	330,		VCCP,	x10mV,		230,		360,		NounVCCP,
	330,		VCCE,	x10mV,		230,		360,		NounVCCE,
	330,		VCCH,	x10mV,		230,		360,		NounVCCH,
	330,		VCCL,	x10mV,		230,		360,		NounVCCL,
	330,		VIH,		x10mV,		230,		360,		NounVIH,	/* VIH Volts MUST be same with FPGA Pin Type Setting!! */
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
	{ AlgoIdx_TH25Q80HB, BlkIdx_TH25Q80HBMSDx, SpcIdx_3V3, PmpIdx_H3V3SO8UG4A, AlgoIC_0x00}, /* TH25Q-80HB-MSDx */
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
		IDCheckFun,			UselessAlgoFun,		CheckSumFun,
		ProtectFun,								UnProtectFun,
	},
};



/* Device Block/Sector	Index Table, You CANNOT Change the Table Name *\
\* But you can add more chip element here by using same format               */
const uInt32 DevBlockIdx_Table[ ] = 
{
	(uInt32)Block_TH25Q80HBMSDx, sizeof(Block_TH25Q80HBMSDx),
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
	(uInt32)Pmp_H3V3SO8UG4A, sizeof(Pmp_H3V3SO8UG4A),
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
{ /* Table Sequence by Block/Sector Index Code Constant Define */
	1,   /* TH25Q-80HB tCE: typ 5.2ms, max 7.8ms; timeout = value x 100 x 10ms = 1s */
} ;


const uChar ChipSPIMaxClkTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
    10,   /* FPGA SPI Delay Time = (1/xxM)*8Bit/10ns/2, conservative bring-up value */
	/* here adding more... */
} ;


const uShort ProtectRegisterTable[ ] =
{
    /*
     * 单一 16 位 Status Register(05h 读低字节, 35h 读高字节)。
     *
     * High byte SR high(S15-S8) mask:
     *   0x40 = 只开放 CMP(S14)
     *   QE/LB1-LB3/SRP1 不开放(LBx 为 OTP 位, 禁止写入)
     *
     * Low byte SR low(S7-S0) mask:
     *   bit6 BP4  = 0x40
     *   bit5 BP3  = 0x20
     *   bit4 BP2  = 0x10
     *   bit3 BP1  = 0x08
     *   bit2 BP0  = 0x04
     *
     * 不开放 SRP0、SRP1，保证保护可解除。
     */
    0x407C,
};

const uChar ProtectRegister0DefaultTable[ ] =
{
    0x00,       /* SR low byte：无区域保护 */
};

const uChar ProtectRegister1DefaultTable[ ] =
{
    0x00,       /* SR high byte：CMP=0 */
};


#endif





