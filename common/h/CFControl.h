
/******************************************************************************
 ***                 Compact Flash Card Control Setting			    ***
 ******************************************************************************/

#ifndef CFControl_H_
#define CFControl_H_

#include "Type.h"
#include "Subroutine.h"
#include "Initial.h"


/*****************************************************************/
typedef union{
			uInt32 all; /* little endie byte mode!!! */
			struct{
	#ifdef __BIG_ENDIAN
				uChar Head ;/* hi-hi byte data */
				uChar BlockHi ;/* mi-hi byte data */
				uChar BlockMi ;/* mi-lo byte data */
				uChar BlockLo ;/* lo-lo byte data */
	#else
				uChar BlockLo ;/* lo-lo byte data */
				uChar BlockMi ;/* mi-lo byte data */
				uChar BlockHi ;/* mi-hi byte data */
				uChar Head ;/* hi-hi byte data */
	#endif
				}Byte;

			struct{
	#ifdef __BIG_ENDIAN
				uShort AddrHi ;
				uShort AddrLo ;
	#else
				uShort AddrLo ;
				uShort AddrHi ;
	#endif
				}Word;
			}stLBSAddr;


/********************************************************************/
#define	ProjectNameLen		16
#define	AttributeLen			4
#define	DriverStartAddrLen	4 /* CPU driver code in CF card start address, MAX 4G address */
#define	DriverCodeCntLen	4 /* CPU driver code length counter, max 64K bytes */
#define	ATRCmdBufAddrLen	4 /* CPU driver auto run command buffer in CF card start address */
#define	SRAMBufAddrLen		4 /* SRAM */
#define	FPGAStartAddrLen 	4 /* FPGA CFG code in CF card start address, MAX 4G address */
#define	FPGACodeCntLen		4 /* FFPGA CFG code length counter, max 4G */
#define	FileStartAddrLen		4 /* User file data start address, MAX 4G address */
#define	FileDataCntLen		4 /* User file data length, max 4G bytes */
#define	FileChkSumLen		4

#define	fwAlgoICIndexLen	1 /* Algo & Device Information */

#define	DeviceTypeLen		1
#define	MemoryType	('M')
#define	MPUType		('P')
#define	PLDType		('L')
#define	SimpleMemoryType	('E') /* for simple machine */
#define	SimpleMPUType		('U') /* for simple machine */
#define	SimplePLDType		('D') /* for simple machine */

#define	ManufactIDLen		2
#define	DeviceIDHiLen		2
#define	DeviceIDLoLen		2

#define	ChkSumTypeLen		1
#define	ByteSum			('B')
#define	WordSum			('W')
#define	CRC16Sum			('1')
#define	CRC32Sum			('3')

#define	MSKBIDCodeLen		4 /* main-skb */
#define	CSKBIDCodeLen		8 /* child-skb be same */
#define	SKBIDCodeHiHi	0
#define	SKBIDCodeMiHi	1
#define	SKBIDCodeMiLo	2
#define	SKBIDCodeLoLo	3

#define	ProjectReserve0		3

#define	DataBuf0StartAddrLen	4 /* User file data start address, MAX 4G address */
#define	DataBuf0CntLen			4 /* User file data length, max 16M bytes */
#define	DataBuf1StartAddrLen	4 /* User file data start address, MAX 4G address */
#define	DataBuf1CntLen			4 /* User file data length, max 16M bytes */

#define	PassCntLen			4
#define	FailCntLen			4

#define	ProjectReserve1		24

/********** device & project info msg ********/
#define	ManufNameLen		32
#define	DeviceNameLen		32
#define	PackageNameLen	16
#define	MSKBNameLen		32
#define	CSKBNameLen		32
#define	UserFilePathLen		112
#define	UserPrjPathLen		112

/********** project reserve bytes *************/
#define	ProjectReserve2		7

#define	CurPrjStrAddrLen	4
#define	NextPrjStrAddrLen	4
#define	PrjChkSumLen		1


#define	AutoRunCmdMaxLen		512 /* max 256 word / 512 bytes command */
#define	SRAMDataBufMaxLen		512 /* max 512 bytes sram data */
//#define	CFCardPrjMBRStrAddr	0x2000000 /* the 1st MBR start from 0x2000000 sector = 16G Byte, Max card = 128G Byte */
#define	CFCardPrjMBRStrAddr	(0x400000) /* the 1st MBR start from 0x400000 sector = 2G Byte for PC Buffer */
#define	CFCardPrjDataStrAddr	(CFCardPrjMBRStrAddr+0x1000) /* the 1st project data start sector */
#define	CFCardData16BitWidth	0x00 /* 0x00=8bit width, 0x01=16bit width */
#define	CFCardData8BitWidth		0x01


/*********** CF Card Volume Define *************************/
#define	CFCard1M			0
#define	CFCard2M			1
#define	CFCard4M			2
#define	CFCard8M			3
#define	CFCard16M			4
#define	CFCard32M			5
#define	CFCard64M			6
#define	CFCard128M			7
#define	CFCard256M			8
#define	CFCard512M			9
#define	CFCard1G			10
#define	CFCard2G			11
#define	CFCard4G			12
#define	CFCard8G			13
#define	CFCard16G			14
#define	CFCard32G			15
#define	CFCard64G			16
#define	CFCard128G			17
#define	CFCard256G			18


/*********** Project Attribute Option Define From PC ****************/
/* Project->Attribute[3], LoLo Byte */
#define	ReadFlag				bmBIT0
#define	ProgramFlag			bmBIT1
#define	BlockProgramFlag		bmBIT2
#define	IllegalChkFlag			bmBIT3
#define	FunctionTestFlag		bmBIT4
#define	VerifyFlag				bmBIT5
#define	EraseFlag				bmBIT6
#define	BlankChkFlag			bmBIT7
/* Project->Attribute[2], MiLo Byte */
#define	SecureFlag				bmBIT0
#define	ByteSumFlag			bmBIT1
#define	WordSumFlag			bmBIT2
#define	CRC16SumFlag			bmBIT3
#define	CRC32SumFlag			bmBIT4
#define	InsertionChkFlag		bmBIT5
#define	AddrReloadFlag			bmBIT6
#define	IDChkFlag				bmBIT7
/* Project->Attribute[1], MiHi Byte */
#define	EPROMAutoIDFlag		bmBIT0
#define	PCCChkFlag				bmBIT1
#define	PrtUnprtFlag				bmBIT2
#define	SumCompareFlag		bmBIT3
#define	UploadDataFlag			bmBIT4
#define	DataWidth0Flag			bmBIT5
#define	DataWidth1Flag			bmBIT6
#define	HiLoVerifyFlag			bmBIT7
/* Project->Attribute[0], HiHi Byte */
#define	LoopFunFlag				bmBIT0
#define	ReserveFlag1			bmBIT1
#define	ReserveFlag2			bmBIT2
#define	ReserveFlag3			bmBIT3
#define	ReserveFlag4			bmBIT4
#define	ReserveFlag5			bmBIT5
#define	ReserveFlag6			bmBIT6
#define	ReserveFlag7			bmBIT7


/*********************************************************************/
typedef struct {
	/* below for project name & all code information */
		uChar ProjectName[ProjectNameLen] ;
		uChar Attribute[AttributeLen] ; /* byte0~3=Hi,MH,ML,L0 byte */
		uChar DriverStartAddr[DriverStartAddrLen] ; /* LBS byte0~3 address=Head+BlockHi+BlockMi+BlockLo */
		uChar DriverCodeCnt[DriverCodeCntLen] ; /* sectors counter */
		uChar ATRCmdBufAddr[ATRCmdBufAddrLen] ; /* LBS address */
		uChar SRAMBufAddr[SRAMBufAddrLen] ; /* LBS address */
		uChar FPGAStartAddr[FPGAStartAddrLen] ; /* LBS address */
		uChar FPGACodeCnt[FPGACodeCntLen] ; /* sectors counter */
		uChar FileStartAddr[FileStartAddrLen] ; /* LBS address */
		uChar FileDataCnt[FileDataCntLen] ; /* sectors counter */
		uChar FileChkSum[FileChkSumLen] ;

	/* below for project setting & control setting */
		uChar fwAlgoICIndex/*[fwAlgoICIndexLen] */;
		uChar DeviceType/*[DeviceTypeLen]*/ ;
		uShort ManufactID/*[ManufactIDLen]*/ ;
		uShort DeviceIDHi/*[DeviceIDHiLen]*/ ;
		uShort DeviceIDLo/*[DeviceIDLoLen]*/ ;

		uChar MSKBIDCode[MSKBIDCodeLen] ; /* SKB Main board ID */
		uChar CSKBIDCode[CSKBIDCodeLen] ; /* SKB Child board ID */
		uChar CheckSumType ;
		uChar Reserve0Bytes[ProjectReserve0] ;

	/* below for Special Data Buffer, MAX 16M Byte=32K Sectors */
		uChar DataBuf0StartAddr[DataBuf0StartAddrLen] ; /* LBS byte0~3 address=Head+BlockHi+BlockMi+BlockLo */
		uChar DataBuf0Cnt[DataBuf0CntLen] ; /* byte0~3=Hi,MH,ML,L0 byte sectors counter */
		uChar DataBuf1StartAddr[DataBuf1StartAddrLen] ; /* LBS byte0~3 address=Head+BlockHi+BlockMi+BlockLo */
		uChar DataBuf1Cnt[DataBuf1CntLen] ; /* byte0~3=Hi,MH,ML,L0 byte sectors counter */

		uInt32 PassCounter ;
		uInt32 FailCounter ;
		uChar Reserve1Bytes[ProjectReserve1] ;

	/* below for device & project info */
		uChar Manufacture[ManufNameLen] ;
		uChar DeviceName[DeviceNameLen] ;
		uChar PackageName[PackageNameLen] ;
		uChar MSKBName[MSKBNameLen] ;
		uChar CSKBName[CSKBNameLen] ;
		uChar UserFilePath[UserFilePathLen] ;
		uChar UserPrjPath[UserPrjPathLen] ;

	/* below for reserve bytes */
		uChar Reserve2Bytes[ProjectReserve2] ;

		uChar CurPrjStrAddr[CurPrjStrAddrLen] ; /* LBS address */
		uChar NextPrjStrAddr[NextPrjStrAddrLen] ; /* LBS address */
		uChar PrjChkSum/*[PrjChkSumLen] */;
		}TProject ; /*** Project max length MUST be 0xFF !!! ***/



/******************************************************************/
/******* CF card checking & control *************/




/********************************************************************/
/* CF Card Control Register Addresses Define */
#define	DataRegAddr			(0x00<<1)

/**************************************/
#define	ErrorRegAddr			(0x01<<1)
#define	CFCError		0x00
#define	NoError			0x01
#define	FormatError		0x02
#define	SectBufError		0x03
#define	ECCError		0x04
#define	MPUError		0x05
#define	IDESlaveError	0x80


#define	FeaturesRegAddr		(0x01<<1)
#define	BlockCntRegAddr			(0x02<<1)
#define	BlockLowRegAddr		(0x03<<1)
#define	BlockMidRegAddr		(0x04<<1)
#define	BlockHighRegAddr		(0x05<<1)
#define	HeadRegAddr			(0x06<<1)

/*************************************/
#define	StatusRegAddr			(0x07<<1) /* page 118 */
#define	BUSYFlag		bmBIT7 /* 1-->Card is busy, must waiting */
#define	RDYFlag			bmBIT6 /* 1-->Card is ready, can receive command */
#define	DWFFlag		bmBIT5 /* 1-->Card Write Error */
#define	DSCFlag			bmBIT4 /* 1-->Card is ready */
#define	DRQFlag			bmBIT3 /* 1-->Card request data transfer */
#define	CORRFlag		bmBIT2 /* 1-->Data error but correct, can goon mul-sector read function */
#define	IDXFlag			bmBIT1 /* always be 0, MUST mask to 0 for some CF card be 1! */
#define	ERRFlag			bmBIT0 /* 1-->Something error, can check from error status register */

#define	CommandRegAddr		(0x07<<1)

#define	AStatusRegAddr			(0xE<<1)
#define	ACommandRegAddr		(0xE<<1)


/* CF Card Control Commands Define */
#define	DIAGNOSTIC				0x9090
#define	IDENTIFY				0xECEC
#define	WRITE_BLOCK			0x3030
#define	READ_BLOCK				0x2020

#define	CFCardModeChgx1us		2 /* 10 */
#define	CFCardPageLen			0x200
#define	CFCardModeReg			0xE0 /* select LBA mode & card 0 */
#define	CFCardVirginData		0xFFFF
#define	CFCardChkMaxCnt		0x3FFFFF /* normal be 40000 times! */
#define	CFCardRWMaxSect		128 /* enable CF card read/write max 256 sectors, here use 128 Sectrors = 64K bytes per loop */
#define	CFCardRWMinSect		1
#define	CFCardIDChkMaxCnt		10
#define	CFCardRdWrx80ns		0x02
#define	CFCardEnDupData		0x0400 /* only set A10=1 for duplicate data mode, page 113 */


/************************************************************\
	CF Card Firmware Information
\************************************************************/
#define	CFCardCylinderAddr		(1*2) /* word address */
#define	CFCardCylinderLen		(1*2) /* word counter */

#define	CFCardHeadAddr			(3*2)
#define	CFCardHeadLen			(1*2) /* only low byte is valid */

#define	CFCardTrackAddr		(6*2)
#define	CFCardTrackLen			(1*2)

#define	CFCardSerialNumAddr	(10*2)
#define	CFCardSerialNumLen		(10*2)

#define	CFCardFWRevAddr		(23*2)
#define	CFCardFWRevLen		(4*2)

#define	CFCardModelAddr		(27*2)
#define	CFCardModelLen			(20*2)

#define	CFCardCurCylinNumAddr	(54*2)
#define	CFCardCurCylinNumLen	(1*2)

#define	CFCardCurHeadNumAddr	(55*2)
#define	CFCardCurHeadNumLen	(1*2)

#define	CFCardCurSecNumAddr	(56*2)
#define	CFCardCurSecNumLen	(1*2)

#define	CFCardTotalSecNumAddr	(60*2)
#define	CFCardTotalSecNumLen	(2*2)

#define	CFCardSecureStatusAddr	(128*2)
#define	CFCardSecureStatusLen	(1*2)


/*********************************************************************/
typedef struct {
	/* below for project name & all code information */
		uChar Status ;
		uChar HeadReg ; /* only low 4 bits is available!! */
		uChar BlockHighReg ;
		uChar BlockMidReg ;
		uChar BlockLowReg ;
		uChar BlockCntReg ; /* one time max read/write 256 sectors * 512 Bytes = 128K Bytes!! */

		uChar CFDataWidth ; /* 0=16bit width, 0x1=8bit width */
		uChar SerialNumber[CFCardSerialNumAddr] ;
		uChar VolumeSector[CFCardTotalSecNumLen] ; /* 0,1,2,3=L-H */
		}TCFCard ;




/*********************************************************\
	CF Card FAT16 File Formating Define
\*********************************************************/
#define	CFCFat16FmtBoottIdt		0x01BE
#define	CFCFat16FmtStrHead		0x01BF /* Max be 255 */
#define	CFCFat16FmtStrSect		0x01C0 /* only low 6 bits for sector, so MAX be 63!! */
#define	CFCFat16FmtStrCyld		0x01C1 /* 8 bits + StrSect hi 2 bits, so MAX 1023!! */
#define	CFCFat16FmtSysID		0x01C2
#define	CFCFat16FmtEndHead	0x01C3
#define	CFCFat16FmtEndSect		0x01C4 /* only low 6 bits for sector */
#define	CFCFat16FmtEndCyld		0x01C5 /* 8 bits + EndSect hi 2 bits */
#define	CFCFat16FmtPartStrSect	0x01C6 /* 4 bytes length, this partition start sector number */
#define	CFCFat16FmtPartSectLen	0x01CA /* 4 bytes length, this partition length secotr counter */


/******************************************************/
/******** CF Card Word Define **************************/
#define	CFCardReadWordData()						(*Rx_buffer_HSMMC_ch0++)


/************ Some Macro Define *****************/
#define	ManufactID()		(Project->ManufactID)
#define	DeviceIDHi()			(Project->DeviceIDHi)
#define	DeviceIDLo()		(Project->DeviceIDLo)
#define	AlgoIC_x()			(Project->fwAlgoICIndex)
#define	ElementNum()		AlgoIC_x()


/*************************************************************/
/************ Varity Define Here ********************************/
extern TCFCard *CFCard ; /* max 512 bytes */
extern TProject *Project ; /* max 512 bytes */
extern uChar CFCardBuf[CFCardPageLen] ; /* for CF card data buffer */





/*************************************************************/
extern uChar CFCardCheck_RDY_DSC(void) ;
extern uChar CFCardCheck_RDY_DSC_DRQ(void) ;
extern uChar CFCardCheckWord_RDY_DSC(void) ;
extern uChar CFCardCheckWord_RDY_DSC_DRQ(void) ;
extern void CFCardReadWriteBlockCmd(uChar BlockCmd) ;
extern void CFCardReadSectorByteData(void) ;
extern void CFCardIdentifyCmd(void) ;
extern void CFCardDiagnostic(void) ;
extern void CFCardErrorAnalysis(void) ;
extern uChar ReadCFCDetPort(void) ;
extern void CFCardReadFillSector(void) ;
extern void CFCardCheckStatus(void) ;
extern void CFCardChkIdentify(void) ;
extern void InitialMenuProjectStatus(void) ;
extern void CFCardSettingLBSParaSendCmd(uChar CFCardCmd) ;
extern void SD0DMA_CFCardSettingLBSParaSendCmd(uChar CFCardCmd, U16 SD0BlockCnt, U32 DataBufAddr) ;
extern void CFCardReset(void) ;


#endif

