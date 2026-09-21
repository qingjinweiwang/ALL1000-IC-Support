
/*****************************************************************/
/*****************************************************************/
#ifndef INITIAL_H_
#define INITIAL_H_

/*****************************************************************/
//0x4000_0000	SRAM
//0x3800_0000	DDRAM1
//0x3000_0000	DDRAM0
//0x2800_0000	Bank5
//0x2000_0000	Bank4
//0x1800_0000	Bank3
//0x1000_0000	Bank2
//0x0800_0000	Bank1
//0x0000_0000	Bank0
/*****************************************************************/


//---------------------------------------------------------------------
//---------------------------------------------------------------------
#define	LOW_INT_VECTOR_ADDR		0x00000000
#define 	DRV_BL_STARTADDRESS		0x00000000
#define	DRV_BL_ENT_OFFSET			0x000000E8
#define	DRV_BL_ENDADDRESS		0x08000000

#define	DRV_IO_MAPPED_START		0x08000000
#define	DRV_IO_MAPPED_END		0x30000000

#define 	RAM_STARTADDRESS			0x30000000 /* if use for ram use, then set to cach on, write back */
#define	RAM_USBDRV_CODE_STR		0x30000000 /* 2M Byte for driver code */
#define	RAM_USBDRV_ENT_OFFSET	0x000000E8
#define	RAM_USBDRV_CODE_END		0x30200000
#define	RAM_USBDRV_DATA_STR		0x30200000 /* 10M Byte for driver data */

#define	RAM_USBDRV_BUF0_STR		0x30C00000 /* 2M Byte, can use for Nand Flash Partition Tbl/ Serial Num. Paramter */
#define	RAM_USBDRV_BUF0_END		0x30E00000
#define	RAM_USBDRV_BUF1_STR		0x30E00000 /* 2M Byte, can use for eMMC Deep Analysis Parameter, per word for 2M Bytes */
#define	RAM_USBDRV_BUF1_END		0x31000000
#define	RAM_USBDRV_DATA_END		0x31000000


#define	RAM_DRVFIFO_BUF0_STR		0x31000000 /* 16M Byte */
#define	RAM_DRVFIFO_BUF0_END		0x32000000
#define	RAM_DRVFIFO_BUF1_STR		0x32000000 /* 16M Byte */
#define	RAM_DRVFIFO_BUF1_END		0x33000000

#define	RAM_DMA_BUF_STR			0x33000000 /* 15M Byte for DMA Buffer, this buf NO CACHE */
#define	RAM_DMA_BUF_END			0x33F00000

#define	RAM_BL_DATA_STR			0x33F00000 /* 64K Byte for BootLoader data & stack */
#define	RAM_BL_DATA_END			0x33F0F000
#define	RAM_BL_STK_STR			0x33F0F000
#define	RAM_BL_STK_END			0x33F10000 /* 4K Byte for BootLoader stack */


/***************************************************************************/
/***************************************************************************/
#define	RAM_BL_USBDRV_BUF_STR	0x33F80000 /* 64K Byte for BootLoader & Usb System data */
#define	CtrlFlagBufAddr				0x33F80000 /* max 512 bytes */
#define	CtrlFlagLen					0x200

#define	AutoCmdBufAddr				0x33F80200 /* max 512 bytes */
#define	SysCmdBufAddr				0x33F80400 /* max 512 bytes */
#define	SysCmdBufLen				(0x200/2) /* total 512 bytes for EP0 buffer length, MAX 250 cmds are available!! */
#define	AutoCmdBuf(_Addr_)			(*((uShort *)(AutoCmdBufAddr)+_Addr_))
#define	SysCmdBuf(_Addr_)			(*((uShort *)(SysCmdBufAddr)+_Addr_))

#define	CFCardBufAddr 				0x33F80600 /* max 512 bytes */
#define	CFCardLen					0x200

#define	ProjectBufAddr 				0x33F80800 /* max 512 bytes */
#define	EveryPrjInfoLen				0x200 /* information length for every project */

#define	SpecialBitBufAddr			0x33F80A00 /* max 512 bytes */
#define	SpecialBitBufLen				0x200
#define	SpecialBitBuf(_Addr_)		(*((uChar *)(SpecialBitBufAddr)+_Addr_))

#define	PartitionTblBufAddr			RAM_USBDRV_BUF0_STR /* max 256 words! */
#define	PartitionTblBufLen			0x100 /* word address mode for CF word data mode */
#define	MaxSysPartitionCnt			32 /* max 32 partition Table */
#define	PartitionTblBuf(_Addr_)		(*((uShort *)(PartitionTblBufAddr)+_Addr_))

#define	SNDataBufAddr				(PartitionTblBufAddr+0x1000) /* max 512 bytes! */
#define	SNDataBufLen				0x200 /* max 512 for prog device SN */
#define	SNDataBuf(_Addr_)			(*((uChar *)(SNDataBufAddr)+_Addr_))

#define	RAM_BL_USBDRV_BUF_END	0x33F90000
/****************************************************************************/
/****************************************************************************/


#define	MMU_TBL_BASE				0x33FF0000 /* 0x33FF0000~0x33FF4000=16K area for MMU Table, MAX=60K */


#define	RAM_USBDRV_STK_STR		0x33FFF000 /* usb system driver stack */
#define	RAM_USBDRV_STK_END		0x33FFFF00


#define 	_ISR_STARTADDRESS			0x33FFFF00 /* 0x33FFFF00~0x33FFFFFF=0x100 area for ISR function Table */
#define 	RAM_ENDADDRESS			0x40000000

#define 	CPU_SRAM_STR				0x40000000 /* can for special using because these area be fastest!! */
#define 	CPU_SRAM_END				0x40100000

#define	MMU_FULL_ACCESS		(3<<10) /* AP=b11, enable all access */
#define	MMU_DOMAIN			(0<<5) /* define for which Domain */
#define	MMU_SPECIAL			(1<<4) /* MUST be 1 */
#define	MMU_CACHEABLE			(1<<3) /* cachealbe C=1 for cach on */
#define	MMU_WRITEBACK		(1<<2) /* bufferable B=1 for write back mode, 0 for write through */
#define	MMU_SECTION			(2<<0) /* b10 means be section */


#define	MMU_SECDESC			(MMU_FULL_ACCESS|MMU_DOMAIN|MMU_SPECIAL|MMU_SECTION) //110000010010
#define	MMU_SECDESC_CB		(MMU_FULL_ACCESS|MMU_DOMAIN|MMU_SPECIAL|MMU_CACHEABLE|MMU_WRITEBACK|MMU_SECTION) //110000011110
#define	MMU_SECDESC_CT		(MMU_FULL_ACCESS|MMU_DOMAIN|MMU_SPECIAL|MMU_CACHEABLE|MMU_SECTION) //110000011010
#define	MMU_SECTION_SIZE		0x00100000 /* each section be 1M bytes */


#define	CPU_IO_MAPPED_START		0x40100000
#define	CPU_IO_MAPPED_END		0xFFF00000

#define	HIGH_INT_VECTOR_ADDR		0xFFF00000



//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------
// nGCS0 = Bank0 Controller Parameter setting;  // for boot rom flash control
//---------------------------------------
#define		IDCY0					0x0 // Idle or turnaround cycles IDCY*HCLK, max=0xF
#define		WSTRD0					0x4 // Read wait state = tacc, max=0x1F, Time=60ns+n*15ns, MUST>=4
#define		WSTWR0				0x4 // wrie wait state, max=0x1F, Time=60ns+n*15ns, MUST>=4
#define		WSTOEN0				0 // output enable assertion delay from CS, max=0x0F
#define		WSTWEN0				0 // write enable assertion delay, max=0x0F


#define		BlWriteEn				1 // bit21-SMBAA signal control:0-1at all times, 1 active for sync
#define		AddrValidWriteEn		1 // bit20-SMADDRVALD during write:0-always high,1-active for write 	
#define		BurstLenWrite			0 // bit1819-burst transfer length:0-4,1-8,3-continu(sync only) 	
#define		SyncWriteDev			0 // bit17-0:async, 1:sync 	
#define		BMWrite					0 // bit16-burt mode write : 0-non-burst, 1-burst 	
#define		WrapRead				0 // bit14-0-disable, 1 enable
#define		BlReadEn				1 // bit13-SMBAA signal :0-1 at all time, 1-active for sync read
#define		AddrValidReadEn			1 // bit12-SMADDRVALID signal: 0-always HIGH, 1-active for async & sync read
#define		BurstLenRead			0 // bit1011-burst transfer length:0-4,1-8,2-16,3-cont(sync only)
#define		SyncReadDev			0 // bit9-sync access :0-async, 1-sync
#define		BMRead					0 // bit8-burst mode red and async page mode
#define		SMBLSPOL				0 // bit6-polarit of signal nSMBLS
#define		MW						1 // bit45-memory width : 00-8bit,01-16bit,10-32bit, only for data port, addr always be 16 bit mode!
#define		WP						0 // bit3-write protect
#define		WaitEn					0 // bit2-external wait signal enable
#define		WaitPol					0 // bit1-polarity of the external wait input for actiation
#define		RBLE					0 // bit0-read byte lane enable
#define		SMBCR0_0				((BMRead<<8)+(SMBLSPOL<<6)+(MW<<4)+(WP<<3)+(WaitEn<<2)+(WaitPol<<1)+RBLE)
#define		SMBCR0_1				((WrapRead<<14)+(BlReadEn<<13)+(AddrValidReadEn<<12)+(BurstLenRead<<10)+(SyncReadDev<<9))
#define		SMBCR0_2				((BlWriteEn<<21)+(AddrValidWriteEn<<20)+(BurstLenWrite<<18)+(SyncWriteDev<<17)+(BMWrite<<16))

#define		WaitTourErr0			0 //  external wait timeout error flag

#define		WSTBRD0				0x1f //  burst read wait state, max=0x1F

#define		MemClkRatio				1 //  SMMEMCLK :0-HCLK,1-HCLK/2,2-HCLK/3, MUST be 1 for Bios-MMU
#define		SMClockEn				1 //  SMCLK enable 0-only active during mem access,1-always running	


//---------------------------------------
// nGCS1 = Bank1 Controller Parameter setting;  // for system control
//---------------------------------------
#define		WSTRD1					0x2 // Read wait state = tacc, Time=60ns+n*15ns
#define		WSTWR1				0x2 // wrie wait state, Time=60ns+n*15ns

//---------------------------------------
// nGCS2 = Bank2 Controller Parameter setting;  // for CF card control
//---------------------------------------
#define		WSTRD2					0x2 // Read wait state = tacc, Time=60ns+n*15ns
#define		WSTWR2				0x2 // wrie wait state, Time=60ns+n*15ns

//---------------------------------------
// nGCS3 = Bank3 Controller Parameter setting;  // for FPGA control
//---------------------------------------
#define		WSTRD3					0x2 // Read wait state = tacc, Time=60ns+n*15ns
#define		WSTWR3				0x2 // wrie wait state, Time=60ns+n*15ns

//---------------------------------------
// nGCS4 = Bank4 Controller Parameter setting;  // for EBI0 control
//---------------------------------------
#define		WSTRD4					0x2 // Read wait state = tacc, Time=60ns+n*15ns
#define		WSTWR4				0x2 // wrie wait state, Time=60ns+n*15ns

//---------------------------------------
// nGCS5 = Bank5 Controller Parameter setting;  // for EBI1 control
//---------------------------------------
#define		WSTRD5					0x2 // Read wait state = tacc, Time=60ns+n*15ns
#define		WSTWR5				0x2 // wrie wait state, Time=60ns+n*15ns





/**************************************************************************\
		System Import Varity Setting
\**************************************************************************/
#define	XTALIn_12M			0
#define	EPLL_96M			1
#define	FCLK_400M			2
#define	HCLK_133M			3
#define	PCLK_66M			4 /* for hi speed nor, spi & nand flash */
#define	DCLK0_Div66M		5 /* for low speed nor flash */
#define	OSCIn_12M			6
#define	Rev_0M				7















/********************************************************/
extern void InitialAllBankSetting(void) ;
extern void InitialBank2Setting(void) ;

extern void Init_Port(void) ;
extern void Init_SFR(void) ;
extern void Load_Defaults(void) ;
extern void ADCPortInitial(void) ;
extern void __irq Timer0Sampling(void) ;
extern void __irq Timer4Sampling(void) ;


#endif

