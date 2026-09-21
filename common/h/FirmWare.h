
/*******************************************************************\
	For system hardware & softwarre subroute setting
\*******************************************************************/

#ifndef FIRMWARE_H_
#define  FIRMWARE_H_

#include "Type.h"


/*************************************************************************\
	Fireware System Using Define

\*************************************************************************/
extern uChar fwDrvMSTVer ;
extern uChar fwDevDRVVer ;
extern const uChar DrvElement_Table[ ][5] ;/* FirmWare Algo Table Define */

extern uShort SysStatusCode ; /* current System working status */
extern uChar SysStatus ; /* current firmware working status */
#define	SysIdle				0
#define	SysBusyInitial		1
#define	SysBusyPCCom		2
#define	SysBusyFWRun		3
#define	SysBusyGandCom	4	
#define	SysError			5
#define	SysLock				6


/********* DevAlgoIdx_Table Element Offset Macro Define ******************/
#define	AlgoIdxOffset		0
#define	BlkIdxOffset			1
#define	SpcIdxOffset		2
#define	PmpIdxOffset		3
#define	AlgoICIdxOffset		4/* can use for debug when load driver from PC */


/**************************************************************************\
	Algo IC Index Mark for Database Record Setting

\**************************************************************************/
extern uChar fwAlgoIndex ;
extern uChar fwBlkIndex ;
extern uChar fwSpcIndex ;
extern uChar fwPmpIndex ;
extern uChar ( * const DevAlgoIdx_Table[ ][13] )( void ) ;

/**************** Algorithm Subroutine Offset ****************/
#define	AlgoPwrOn			0
#define	AlgoPwrOff			1
#define	AlgoRead			2
#define	AlgoProgram		3
#define	AlgoVerify			4
#define	AlgoErase			5
#define	AlgoBlkChk			6
#define	AlgoIlgChk			7
#define	AlgoIDChk			8
#define	AlgoSecure			9
#define	AlgoChecksum		10
#define	AlgoProt			11
#define	AlgoUnpt			12
#define	AlgoTest			13/* For Functional Test */
#define	AlgoPreLoad			14/* For PLD device PreLoad */
#define	AlgoDriIni			15/* For driver initial before power on */

/**************************************************************/
#define	AlgoIC_0x00			0/* max define 255 */
#define	AlgoIC_0x01			1
#define	AlgoIC_0x02			2
#define	AlgoIC_0x03			3
#define	AlgoIC_0x04			4
#define	AlgoIC_0x05			5
#define	AlgoIC_0x06			6
#define	AlgoIC_0x07			7
#define	AlgoIC_0x08			8
#define	AlgoIC_0x09			9
#define	AlgoIC_0x0A		10
#define	AlgoIC_0x0B			11
#define	AlgoIC_0x0C			12
#define	AlgoIC_0x0D		13
#define	AlgoIC_0x0E			14
#define	AlgoIC_0x0F			15

#define	AlgoIC_0x10			16
#define	AlgoIC_0x11			17
#define	AlgoIC_0x12			18
#define	AlgoIC_0x13			19
#define	AlgoIC_0x14			20
#define	AlgoIC_0x15			21
#define	AlgoIC_0x16			22
#define	AlgoIC_0x17			23
#define	AlgoIC_0x18			24
#define	AlgoIC_0x19			25
#define	AlgoIC_0x1A		26
#define	AlgoIC_0x1B			27
#define	AlgoIC_0x1C			28
#define	AlgoIC_0x1D		29
#define	AlgoIC_0x1E			30
#define	AlgoIC_0x1F			31

#define	AlgoIC_0x20			32
#define	AlgoIC_0x21			33
#define	AlgoIC_0x22			34
#define	AlgoIC_0x23			35
#define	AlgoIC_0x24			36
#define	AlgoIC_0x25			37
#define	AlgoIC_0x26			38
#define	AlgoIC_0x27			39
#define	AlgoIC_0x28			40
#define	AlgoIC_0x29			41
#define	AlgoIC_0x2A		42
#define	AlgoIC_0x2B			43
#define	AlgoIC_0x2C			44
#define	AlgoIC_0x2D		45
#define	AlgoIC_0x2E			46
#define	AlgoIC_0x2F			47

#define	AlgoIC_0x30			48
#define	AlgoIC_0x31			49
#define	AlgoIC_0x32			50
#define	AlgoIC_0x33			51
#define	AlgoIC_0x34			52
#define	AlgoIC_0x35			53
#define	AlgoIC_0x36			54
#define	AlgoIC_0x37			55
#define	AlgoIC_0x38			56
#define	AlgoIC_0x39			57
#define	AlgoIC_0x3A		58
#define	AlgoIC_0x3B			59
#define	AlgoIC_0x3C			60
#define	AlgoIC_0x3D		61
#define	AlgoIC_0x3E			62
#define	AlgoIC_0x3F			63

#define	AlgoIC_0x40			64
#define	AlgoIC_0x41			65
#define	AlgoIC_0x42			66
#define	AlgoIC_0x43			67
#define	AlgoIC_0x44			68
#define	AlgoIC_0x45			69
#define	AlgoIC_0x46			70
#define	AlgoIC_0x47			71
#define	AlgoIC_0x48			72
#define	AlgoIC_0x49			73
#define	AlgoIC_0x4A		74
#define	AlgoIC_0x4B			75
#define	AlgoIC_0x4C			76
#define	AlgoIC_0x4D		77
#define	AlgoIC_0x4E			78
#define	AlgoIC_0x4F			79

#define	AlgoIC_0x50			80
#define	AlgoIC_0x51			81
#define	AlgoIC_0x52			82
#define	AlgoIC_0x53			83
#define	AlgoIC_0x54			84
#define	AlgoIC_0x55			85
#define	AlgoIC_0x56			86
#define	AlgoIC_0x57			87
#define	AlgoIC_0x58			88
#define	AlgoIC_0x59			89
#define	AlgoIC_0x5A		90
#define	AlgoIC_0x5B			91
#define	AlgoIC_0x5C			92
#define	AlgoIC_0x5D		93
#define	AlgoIC_0x5E			94
#define	AlgoIC_0x5F			95

#define	AlgoIC_0x60			96
#define	AlgoIC_0x61			97
#define	AlgoIC_0x62			98
#define	AlgoIC_0x63			99
#define	AlgoIC_0x64			100
#define	AlgoIC_0x65			101
#define	AlgoIC_0x66			102
#define	AlgoIC_0x67			103
#define	AlgoIC_0x68			104
#define	AlgoIC_0x69			105
#define	AlgoIC_0x6A		106
#define	AlgoIC_0x6B			107
#define	AlgoIC_0x6C			108
#define	AlgoIC_0x6D		109
#define	AlgoIC_0x6E			110
#define	AlgoIC_0x6F			111

#define	AlgoIC_0x70			112
#define	AlgoIC_0x71			113
#define	AlgoIC_0x72			114
#define	AlgoIC_0x73			115
#define	AlgoIC_0x74			116
#define	AlgoIC_0x75			117
#define	AlgoIC_0x76			118
#define	AlgoIC_0x77			119
#define	AlgoIC_0x78			120
#define	AlgoIC_0x79			121
#define	AlgoIC_0x7A		122
#define	AlgoIC_0x7B			123
#define	AlgoIC_0x7C			124
#define	AlgoIC_0x7D		125
#define	AlgoIC_0x7E			126
#define	AlgoIC_0x7F			127

#define	AlgoIC_0x80			128
#define	AlgoIC_0x81			129
#define	AlgoIC_0x82			130
#define	AlgoIC_0x83			131
#define	AlgoIC_0x84			132
#define	AlgoIC_0x85			133
#define	AlgoIC_0x86			134
#define	AlgoIC_0x87			135
#define	AlgoIC_0x88			136
#define	AlgoIC_0x89			137
#define	AlgoIC_0x8A		138
#define	AlgoIC_0x8B			139
#define	AlgoIC_0x8C			140
#define	AlgoIC_0x8D		141
#define	AlgoIC_0x8E			142
#define	AlgoIC_0x8F			143

#define	AlgoIC_0x90			144
#define	AlgoIC_0x91			145
#define	AlgoIC_0x92			146
#define	AlgoIC_0x93			147
#define	AlgoIC_0x94			148
#define	AlgoIC_0x95			149
#define	AlgoIC_0x96			150
#define	AlgoIC_0x97			151
#define	AlgoIC_0x98			152
#define	AlgoIC_0x99			153
#define	AlgoIC_0x9A		154
#define	AlgoIC_0x9B			155
#define	AlgoIC_0x9C			156
#define	AlgoIC_0x9D		157
#define	AlgoIC_0x9E			158
#define	AlgoIC_0x9F			159

#define	AlgoIC_0xA0		160
#define	AlgoIC_0xA1		161
#define	AlgoIC_0xA2		162
#define	AlgoIC_0xA3		163
#define	AlgoIC_0xA4		164
#define	AlgoIC_0xA5		165
#define	AlgoIC_0xA6		166
#define	AlgoIC_0xA7		167
#define	AlgoIC_0xA8		168
#define	AlgoIC_0xA9		169
#define	AlgoIC_0xAA		170
#define	AlgoIC_0xAB		171
#define	AlgoIC_0xAC		172
#define	AlgoIC_0xAD		173
#define	AlgoIC_0xAE		174
#define	AlgoIC_0xAF		175

#define	AlgoIC_0xB0			176
#define	AlgoIC_0xB1			177
#define	AlgoIC_0xB2			178
#define	AlgoIC_0xB3			179
#define	AlgoIC_0xB4			180
#define	AlgoIC_0xB5			181
#define	AlgoIC_0xB6			182
#define	AlgoIC_0xB7			183
#define	AlgoIC_0xB8			184
#define	AlgoIC_0xB9			185
#define	AlgoIC_0xBA		186
#define	AlgoIC_0xBB			187
#define	AlgoIC_0xBC			188
#define	AlgoIC_0xBD		189
#define	AlgoIC_0xBE			190
#define	AlgoIC_0xBF			191

#define	AlgoIC_0xC0			192
#define	AlgoIC_0xC1			193
#define	AlgoIC_0xC2			194
#define	AlgoIC_0xC3			195
#define	AlgoIC_0xC4			196
#define	AlgoIC_0xC5			197
#define	AlgoIC_0xC6			198
#define	AlgoIC_0xC7			199
#define	AlgoIC_0xC8			200
#define	AlgoIC_0xC9			201
#define	AlgoIC_0xCA		202
#define	AlgoIC_0xCB			203
#define	AlgoIC_0xCC			204
#define	AlgoIC_0xCD		205
#define	AlgoIC_0xCE			206
#define	AlgoIC_0xCF			207

#define	AlgoIC_0xD0		208
#define	AlgoIC_0xD1		209
#define	AlgoIC_0xD2		210
#define	AlgoIC_0xD3		211
#define	AlgoIC_0xD4		212
#define	AlgoIC_0xD5		213
#define	AlgoIC_0xD6		214
#define	AlgoIC_0xD7		215
#define	AlgoIC_0xD8		216
#define	AlgoIC_0xD9		217
#define	AlgoIC_0xDA		218
#define	AlgoIC_0xDB		219
#define	AlgoIC_0xDC		220
#define	AlgoIC_0xDD		221
#define	AlgoIC_0xDE		222
#define	AlgoIC_0xDF		223

#define	AlgoIC_0xE0			224
#define	AlgoIC_0xE1			225
#define	AlgoIC_0xE2			226
#define	AlgoIC_0xE3			227
#define	AlgoIC_0xE4			228
#define	AlgoIC_0xE5			229
#define	AlgoIC_0xE6			230
#define	AlgoIC_0xE7			231
#define	AlgoIC_0xE8			232
#define	AlgoIC_0xE9			233
#define	AlgoIC_0xEA		234
#define	AlgoIC_0xEB			235
#define	AlgoIC_0xEC			236
#define	AlgoIC_0xED		237
#define	AlgoIC_0xEE			238
#define	AlgoIC_0xEF			239

#define	AlgoIC_0xF0			240
#define	AlgoIC_0xF1			241
#define	AlgoIC_0xF2			242
#define	AlgoIC_0xF3			243
#define	AlgoIC_0xF4			244
#define	AlgoIC_0xF5			245
#define	AlgoIC_0xF6			246
#define	AlgoIC_0xF7			246
#define	AlgoIC_0xF8			248
#define	AlgoIC_0xF9			249
#define	AlgoIC_0xFA		250
#define	AlgoIC_0xFB			251
#define	AlgoIC_0xFC			252
#define	AlgoIC_0xFD		253
#define	AlgoIC_0xFE			254
#define	AlgoIC_0xFF			255



/*********************************************************************\
	Block/Sector Name Define

\*********************************************************************/
extern const uInt32 DevBlockIdx_Table[ ] ;



/****************************************************************************\
	Block/Sector Property Setting Define
	User CANNOT Change these Definition Here!!!
\****************************************************************************/
/************** The Device_Option Define ******************************\
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

\*********************************************************************/
#define	MainByte8Width				0x0001/* You can not change Definition Here */
#define	MainWord16Width			0x0002
#define	MainDouble32Width			0x0004
#define	MainLong64Width			0x0008
#define	MainDevOpt_Res4			0x0010
#define	MainDevOpt_Res5			0x0020
#define	MainDevOpt_Res6			0x0040
#define	MainDoubleCycle				0x0080
#define	MainByteSumCal				0x0100
#define	MainWordSumCal			0x0200
#define	MainCRC16SumCal			0x0400
#define	MainCRC32SumCal			0x0800
#define	MainDevOpt_Res12			0x1000
#define	MainVerifyErrList			0x2000
#define	MainPrgChkSumCmp			0x4000
#define	MainSectorEraseOn			0x8000


typedef union
{
	uShort MainDevOptionWord ;
	struct
	{
		uShort bByte8Width		: 1 ;	
		uShort bWord16Width		: 1 ;
		uShort bDouble32Width	: 1 ;
		uShort bLong64Width		: 1 ;
		uShort bDevOpt_Res4		: 1 ;
		uShort bDevOpt_Res5		: 1 ;
		uShort bDevOpt_Res6		: 1 ;
		uShort bDoubleCycle		: 1 ;
		uShort bByteSumCal		: 1 ;
		uShort bWordSumCal		: 1 ;
		uShort bCRC16SumCal		: 1 ;
		uShort bCRC32SumCal		: 1 ;
		uShort bDevOpt_Res12	: 1 ;
		uShort bVerifyErrList		: 1 ;
		uShort bPrgChkSumCmp	: 1 ;
		uShort bSectorEraseOn	: 1 ;
	} Bit ;
}stMainDevOption ;


/*************** The Blk_Attribute Define:******************************\
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

\******************************************************************/
#define	BlkCheckSumOff			0x0001
#define	BlkRamBufData			0x0002
#define	BlkProgramOff			0x0004
#define	BlkSecureOff			0x0008
#define	BlkVerifyOff				0x0010
#define	BlkEraseOff				0x0020
#define	BlkCheckOff				0x0040
#define	BlkDualLoopOn			0x0080
#define	BlkReadOff				0x0100
#define	BlkSpecialArray			0x0200
#define	BlkAttri_Res10			0x0400
#define	BlkAttri_Res11			0x0800
#define	BlkAttri_Res12			0x1000
#define	BlkAttri_Res13			0x2000
#define	BlkAttri_Res14			0x4000
#define	BlkAttri_Res15			0x8000

typedef union
{
	uShort BlkAttriWord ;
	struct
	{
		uShort bCheckSumOff		: 1 ;
		uShort bRamBufData		: 1 ;
		uShort bProgramOff		: 1 ;
		uShort bSecureOff		: 1 ;
		uShort bVerifyOff			: 1 ;
		uShort bEraseOff			: 1 ;
		uShort bCheckOff			: 1 ;
		uShort bDualLoopOn		: 1 ;
		uShort bReadOff			: 1 ;
		uShort bSpecialBit		: 1 ;
		uShort bBlkAttri_Res10	: 1 ;	
		uShort bBlkAttri_Res11	: 1 ;
		uShort bBlkAttri_Res12	: 1 ;
		uShort bBlkAttri_Res13	: 1 ;
		uShort bBlkAttri_Res14	: 1 ;
		uShort bBlkAttri_Res15	: 1 ;
	} Bit ;
}stBlkAttri ;


/******************************************************************************
 ***	          <<<  The Device Block/Sector Table  >>>		    ***
 ******************************************************************************/
/* FORMAT:
	Property Of Block:
	Blk_Attribute(1W),	Blk_SumMask(1W),	Blk_Virgin(1W),
	Blk_RAMstart_Address(2W),	! byte memory addressing
	Blk_Devstart_Address(2W),	Blk_Devend_Address(2W),
	Blk_NounsCode(1W),
------------------------------------------------------------------------------*/
#define	BlkPropertyLen		10

typedef union
{
	uShort			BlkPtyArrayWord[BlkPropertyLen] ;
	struct
	{	/* struch for device block property element definition	, total 10 words			*/
		stBlkAttri		BlkAttri ;
		uShort		BlkSumMask ;
		uShort		BlkVirgin ;
		uShort		BlkRamStartAddrHi ;
		uShort		BlkRamStartAddrLo ;
		uShort		BlkChipStartAddrHi ;
		uShort		BlkChipStartAddrLo ;
		uShort		BlkChipEndAddrHi ;
		uShort		BlkChipEndAddrLo ;
		uShort		BlkNounsCode ;
	}Word ;
} stDevBlkPty ;

extern stDevBlkPty fwDevBlkPty ;


/* FORMAT:
	Main Structure:
	Device_Option(1W),
	RAM_Start_Address(2W),  	! byte memory addressing
	Chip_Start_Address(2W), Chip_End_Address(2W),
	[Property_Of_Block_1(10W)],
	[Property_Of_Block_2(10W)],
	.
	.
	[Property_Of_Block_N(10W)];
------------------------------------------------------------------------------*/
#define	DevOptionLen			7

typedef union
{
	uShort				MainPtyArrayWord[DevOptionLen+3] ;
	struct
	{	/* struch for device block table element definition, total 8 words				*/
		stMainDevOption	MainDevOption ;
		uShort			MainRamStartAddrHi;
		uShort			MainRamStartAddrLo ;
		uShort			MainChipStartAddrHi ;
		uShort			MainChipStartAddrLo ;
		uShort			MainChipEndAddrHi ;
		uShort			MainChipEndAddrLo ;
		uShort			SRAMSpcBitBlkNum ;/* Device Block/Sector Number for SRAM special bit setting */
		uShort			MainBlkAmount ;
		uShort			CurrBlkNum ;/* Device Block/Sector Number of current proceeding block */
	}Word ;
} stDevMainPty;

extern stDevMainPty fwDevMainPty ;
#define	IfSectorEraseOn()	(fwDevMainPty.Word.MainDevOption.Bit.bSectorEraseOn)
#define	IfSpecialBitBlk() 		(DevBlkAttri.Bit.bSpecialBit)
#define	IfSRAMData()		(DevBlkAttri.Bit.bRamBufData)
#define	GetCurrBlkNum()		(fwDevMainPty.Word.CurrBlkNum)
#define	GetBlkChipStartAddrHi()		(fwDevBlkPty.Word.BlkChipStartAddrHi)
#define	GetBlkChipStartAddrLo()		(fwDevBlkPty.Word.BlkChipStartAddrLo)
#define	GetBlkRamStartAddrHi()		(fwDevBlkPty.Word.BlkRamStartAddrHi)
#define	GetBlkRamStartAddrLo()		(fwDevBlkPty.Word.BlkRamStartAddrLo)


/************* Chip Specification Macro Definition ******************************\
	Property of Parameters:
	ParaValue(1W), RegOfs(1W), ParaUnit(1W), MinVal(1W), MaxVal(1W), NameIdx(1W),
\************************************************************************/
#define	MaxSpecCnt				24
#define	SpecArrayLen			6
#define	ParaValueOffset			0
#define	ParaTypeOffset			1
#define	ParaMinValueOffset		3
#define	ParaMaxValueOffset		4

/******* Regiser for Offset Definition **************/
#define	VCCR			0
#define	VCCP			1	
#define	VCCE			2
#define	VCCS			3
#define	VCCIO			4
#define	VCCH			5
#define	VCCL			6
#define	VIH				7
#define	VOH				8
#define	VOL				9
#define	VPPP			10
#define	VPPE			11
#define	VPPS			12
#define	VPPIO			13
#if 0
#define	IPPP				14
#define	IPPE				15
#define	IPPS				16
#endif
#define	RSCPD			17/* Read System CPU Period Delay, period = 60ns+(n-4)*15ns */
#define	WSCPD			18/* Write System CPU Period Delay, period = 60ns+(n-4)*15ns */
#define	VrInsert			19/* Vr voltage setting when insertion check */
#define	VCInsert		20/* Vc voltage setting when insertion check */
#define	VCrInsert		21/* Vcr voltage setting when insertion check */
#define	VhPinCChk		22/* VhPCC voltage setting when Pin Connect Check for Vcc */
#define	VccPinCChk		VhPinCChk/* VhPCC voltage setting when Pin Connect Check for Vcc */
#define	VlPinCChk		23/* VlPCC voltage setting when Pin Connect Check for Gnd */
#define	GndPinCChk		VlPinCChk/* VlPCC voltage setting when Pin Connect Check for Gnd */


/******* Register for Parameter Unit definition *************/
#define	x10mV			0
#if 0
#define	x500uA			1
#endif
#define	x15nS			2 /* read/write period delay period = 60ns+(n-4)*15ns */

/******* Regiser for Offset Definition **************/
#define	NounVCCR			0
#define	NounVCCP			1	
#define	NounVCCE			2
#define	NounVCCS			3
#define	NounVCCIO			4
#define	NounVCCH			5
#define	NounVCCL			6
#define	NounVIH			7
#define	NounVOH			8
#define	NounVOL			9
#define	NounVPPP			10
#define	NounVPPE			11
#define	NounVPPS			12
#define	NounVPPIO			13
#if 0
#define	NounIPPP				14
#define	NounIPPP				15
#define	NounIPPP				16
#endif
#define	NounRSCPD			17/* System CPU Period Delay */
#define	NounWSCPD			18/* System CPU Period Delay */
#define	NounVrInsert		19/* Vr voltage setting when insertion check */
#define	NounVCInsert		20/* Vc voltage setting when insertion check */
#define	NounVCrInsert		21/* Vcr voltage setting when insertion check */
#define	NounVhPinCChk		22/* VhPCC voltage setting when Pin Connect Check */
#define	NounVccPinCChk		NounVhPinCChk/* VhPCC voltage setting when Pin Connect Check */
#define	NounVlPinCChk		23/* VlPCC voltage setting when Pin Connect Check */
#define	NounGndPinCChk		NounVlPinCChk/* VlPCC voltage setting when Pin Connect Check */

typedef struct
{	/* struch for device block table element definition, total 8 words				*/
	uShort		ParaValue ;
#if 0
	uShort		ParaType ;
	uShort		ParaUnit ;
#endif
	uShort		ParaMinValue ;
	uShort		ParaMaxValue ;
#if 0
	uShort		ParaNameIdx ;
#endif
} stDevSpecTbl;

extern stDevSpecTbl fwDevSpecTbl[MaxSpecCnt] ;
extern const uInt32 DevSpecIdx_Table[ ] ;



/******************************************************************************
 ***		  <<<  The Device Pin Mapping Table  >>>		    ***
	IO Pin Table ,	GND Pin Table ,	NoUse Pin Table ,
 ******************************************************************************/
#define	DIOTbl_Terminal			0xFAFA/* DIO pin table end flag */
#define	VCCTbl_Terminal			0xFBFB/* Vcc pin table end flag */
#define	VPPESTbl_Terminal		0xFCFC/* Vpp,Vpe,Vps pin table end flag */
#define	VPIOTbl_Terminal		0xFDFD/* Vpio pin table end flag */
#define	GNDTbl_Terminal		0xFEFE/* GND pin table end flag */
#define	NUNCTbl_Terminal		0xFFFF/* No Use or No Connect pin table end flag */

extern const uInt32 DevPmpIdx_Table[ ] ;


/******************************************************************************
	FirmWare Global Variable definition

******************************************************************************/
typedef union
{
	uShort StatusFlagsWord;
	struct/* need recheck definition!!!--Maple-060809AM */
	{
		uShort bStatusFlags_Res0		: 1 ;	
		uShort bStatusFlags_Res1		: 1 ;
		uShort bDrv_Terminate		: 1 ;
		uShort bStatusFlags_Res3		: 1 ;
		uShort bStatusFlags_Res4		: 1 ;
		uShort bStatusFlags_Res5		: 1 ;
		uShort bBlkForce_End			: 1 ;
		uShort bIn_Second_Cycle		: 1 ;
		uShort bStatusFlags_Res8		: 1 ;
		uShort bStatusFlags_Res9		: 1 ;
		uShort bStatusFlags_Res10	: 1 ;
		uShort bStatusFlags_Res11	: 1 ;
		uShort bStatusFlags_Res12	: 1 ;
		uShort bStatusFlags_Res13	: 1 ;
		uShort bStatusFlags_Res14	: 1 ;
		uShort bFirstTime_Entry		: 1 ;
	} Bit ;
}stStatusFlags ;

extern stStatusFlags fwStatusFlags;
#define	IfFirstTimeEntry()		(fwStatusFlags.Bit.bFirstTime_Entry)


/************ for Miscellaneous... *******************************/
#define	MaxDioLstCnt			0xFFFF
#define	MaxPinNumArray		24 	/* 24 chips control 8*24=192 pins */
#define	MaxVssNumArray		24 	/* 24 chips control 8*24=192 pins */
#define	MaxVccNumArray		24 	/* 24 chips control 8*24=192 pins */
#define	MaxVppNumArray		4 	/* 4 chips control 8*4=32 pins */
#define	MaxVioNumArray		4 	/* 4 chips control 8*4=32 pins */


/******** for XP1 programmer ************/
#define	MaxPinNumArray1		6 	/* 6 chips control 8*6=48 pins */
#define	MaxVssNumArray1		6 	/* 8 chips control 8*6=48 pins */
#define	MaxVccNumArray1		6 	/* 8 chips control 8*6=48 pins */
#define	MaxVppNumArray1		1 	/* 1 chips control 8*4=8 pins */
#define	MaxVioNumArray1		1 	/* 1 chips control 8*4=8 pins */


typedef union
{
	uChar	PinNumArray[MaxPinNumArray] ;
	struct 
	{
		uChar Pin8_1 ;
		uChar Pin16_9 ;
		uChar Pin24_17 ;
		uChar Pin32_25 ;
		uChar Pin40_33 ;
		uChar Pin48_41 ;
		uChar Pin56_49 ;
		uChar Pin64_57 ;
		uChar Pin72_65 ;
		uChar Pin80_73 ;
		uChar Pin88_81 ;
		uChar Pin96_89 ;
		uChar Pin104_97 ;
		uChar Pin112_105 ;
		uChar Pin120_113 ;
		uChar Pin128_121 ;
		uChar Pin136_129 ;
		uChar Pin144_137 ;
	}Byte ;
}stPINNUM ;


typedef union{
			INT32 all; /* little endie byte mode!!! */
			struct{
	#ifdef __BIG_ENDIAN
				uShort HiWord ;/* hi word data */
				uShort LoWord ;/* lo word data */
	#else
				uShort LoWord ;/* lo word data */
				uShort HiWord ;/* hi word data */
	#endif
				}Word;
			}stINT32;



/****************************************************************************/
extern uChar CheckSumMode ;

/***** below for Gang System Control *****/
extern uChar GangType ;
#define	Gang1Type				1 /* for DIO mode */
#define	Gang2Type				2
#define	Gang4Type				4
#define	Gang8Type				8
#define	Gang16Type				16
#define	Gang32Type				32
#define	Gang64Type				64


/****************************************/
/**** below for fpga socket control setting ******/
/****************************************/
#define	ChkAllSckErrFlag		0xFF /* each bit status for each socket */
#define	ChkSckAErrFlag			0x01
#define	ChkSckBErrFlag			0x02
#define	ChkSckCErrFlag			0x04
#define	ChkSckDErrFlag			0x08
#define	ChkSckEErrFlag			0x10
#define	ChkSckFErrFlag			0x20
#define	ChkSckGErrFlag			0x40
#define	ChkSckHErrFlag			0x80

#define	EnAllSckCtrlFlag			0xFFFF /* each bit status for each socket */
#define	EnSckACtrlFlag			0x01 /*  1 = enable socket, 0 = disable socket */
#define	EnSckBCtrlFlag			0x02
#define	EnSckCCtrlFlag			0x04
#define	EnSckDCtrlFlag			0x08
#define	EnSckECtrlFlag			0x10
#define	EnSckFCtrlFlag			0x20
#define	EnSckGCtrlFlag			0x40
#define	EnSckHCtrlFlag			0x80



/****************************************/
/**** below for socket parameter setting ******/
/****************************************/
extern uShort SkbLedStatusData ;
#define	MaxSocketNum		8
#define	MainSkbFF			0xFF
#define	SocketA				0
#define	SocketB				1
#define	SocketC				2
#define	SocketD				3
#define	SocketE				4
#define	SocketF				5
#define	SocketG				6
#define	SocketH				7


/****************************************************************************/
extern uShort DevDrvVersion ; /* use ASCII code, from 0x3130~0x3939-->1.0~9.9 */
extern uShort ManufactID ;
extern uShort ManufactErrID ;
extern uShort DeviceIDHi ;
extern uShort DeviceErrIDHi ;
extern uShort DeviceIDLo ;
extern uShort DeviceErrIDLo ;
extern uShort OperateVCC; /*	Operating VCC voltage */
extern uInt32 DioLstLoc[MaxSocketNum] ; /* Dio mode Pin table start address */
extern uShort DioLstCnt[MaxSocketNum]; /* Dio mode pin table counter for max Gang8 system */
extern uInt32 VccLstLoc[MaxSocketNum] ;/* save VCC pins list table address */
extern uChar VccLstCnt[MaxSocketNum] ;
extern stPINNUM VccPinNum ;
extern uInt32 VppesLstLoc[MaxSocketNum] ;/* save VPP, VPE, VPS pins list table address */
extern uChar VppesLstCnt[MaxSocketNum] ;
extern stPINNUM VppesPinNum ;
extern uInt32 VpioLstLoc[MaxSocketNum] ;/* save VPIO pins list table address */
extern uChar VpioLstCnt[MaxSocketNum] ;
extern stPINNUM VpioPinNum ;
extern uInt32 GndLstLoc[MaxSocketNum];/* save grand pins list table address */
extern uChar GndLstCnt[MaxSocketNum];/* save grand pins list table counter */
extern stPINNUM GndPinNum ;
extern uInt32 NUNCLstLoc[MaxSocketNum]; /* save no use pins list table address */
extern uChar NUNCLstCnt[MaxSocketNum] ; /* save no use pins list table counter */
extern stPINNUM NuNcPinNum ;
extern stPINNUM PCCErrPinNum ;
extern uShort ClockFreq ;
extern uChar PCCStatus[MaxSocketNum] ; /* if PCCStatus=0 be fail, =1 be pass */
extern uChar PCCCheckCnt ;
extern uChar ErrorCode ;

extern uShort RAMErrData;
extern uShort DevErrData;
extern uShort RAMErrDataAddrHi;
extern uShort RAMErrDataAddrLo;
extern uShort DevErrDataAddrHi;
extern uShort DevErrDataAddrLo;
extern stINT32 DevCurrBlkLen;
extern stBlkAttri DevBlkAttri ;
extern uShort DevBlkSumMask ;
extern uShort DevBlkVirgin ;

extern uShort StdChecksumHi ; /* true calculate hi-sum */
extern uShort StdChecksumLo ; /* true calculate lo-sum */
extern uShort PrgChecksumHi ; /* project setting hi-sum */
extern uShort PrgChecksumLo ; /* project setting lo-sum */


/******* RAM Buffer Data & Chip Data Address Pointer Value ********/
extern uShort SRAMPointer ; /* sram byte data pointer, address = 0~511, max 512 bytes */

extern uShort DramBufCnt ;/* double buffer length for processing, max 1024 bytes */
extern uShort ReadBufCnt ;/* double buffer length for processing, max 1024 bytes */

#define	MaxPktByteSize	512 /* for byte width length device */
#define	MaxPktWordSize	(MaxPktByteSize/2) /* for word width length device */

extern uChar EPxErrorStall ; /* setting flag to STALL End Point from USB */


/**********************************************************************/
#define	SysSNInfoMaxLen		(32*5)
#define	FWSNMaxLen			47 /* fromat: "ASAG2-WB5IT-DWP3I-AAKWY-TV836-XHDPN-YTV6A-AAAAB-X8" */
#define	FWGangIDNo			48 /* 48 for Programmer Type ID-See Below, 49 for Gang ID No. Max be 0-7 settings */
#define	WSYXP1Type			('2') /* can support Online/Offline Single/Gang mode, USE ARM XP1 System */
#define	WSYXP8Type			('X') /* can support Online/Offline Single/Gang mode, USE ARM XP8 System */

extern uShort ReadDataBuf[MaxPktWordSize*2] ; /* Double Buffer Mode, upload data to CPU */
extern uChar SNCodeBiosInfo[SysSNInfoMaxLen] ; /* for system serial number & gang information buffer */
extern uShort UpdateTimeFlag ;


/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/


#endif


