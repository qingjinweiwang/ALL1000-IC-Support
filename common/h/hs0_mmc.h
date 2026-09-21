#ifndef __HS0_MMC_H__
#define __HS0_MMC_H__

#ifdef __cplusplus
extern "C" {
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////
extern void __irq HS_WRITE_INT_CH0(void);
extern void __irq HS_READ_INT_CH0(void);
extern void __irq HS_DMA_INT_CH0(void);

extern void Test_HS_MMC_CH0(int TestFun);
extern void WaitSD0_DMAReadEnd(void) ;
extern void WaitSD0_DMAWriteEnd(void) ;
extern void HS_MMC_ReadTest_CH0(U32 BlockStartAddr, U16 uNumOfBlocks, U32 ReadBuffAddr); /* Max 65535 blocks per loop */
extern void HS_MMC_WriteTest_CH0(U32 BlockStartAddr, U16 uNumOfBlocks, U32 WriteBuffAddr); /* Max 65535 blocks per loop */
extern void HS_MMC_EraseBlock_CH0(U32 BlockStartAddr, U32 uNumOfBlocks);
extern int ReadExtCSD_CH0(void);

extern void HS_MMC_Reset_CH0(void);
extern int HS_MMC_init_CH0(void);

void GetResponseData_CH0(U32 uCmd);
void ClockOnOff_CH0(int OnOff);
void ClockConfig_CH0(U32 Clksrc, U32 Divisior);
void HostCtrlSpeedMode_CH0(U8 SpeedMode);

int WaitForCommandComplete_CH0(void);
int WaitForTransferComplete_CH0(void);
int WaitForBufferWriteReady_CH0(void);

void ClearBufferWriteReadyStatus_CH0(void);
void ClearBufferReadReadyStatus_CH0(void);
void ClearCommandCompleteStatus_CH0(void);
void ClearTransferCompleteStatus_CH0(void);
void ClearErrInterruptStatus_CH0(void);

void SetTransferModeReg_CH0(U32 MultiBlk,U32 DataDirection, U32 AutoCmd12En,U32 BlockCntEn,U32 DmaEn);
void SetArgumentReg_CH0(U32 uArg);
void SetBlockCountReg_CH0(U16 uBlkCnt);
void SetSystemAddressReg_CH0(U32 SysAddr);
void SetBlockSizeReg_CH0(U16 uDmaBufBoundary, U16 uBlkSize);
void SetMMCSpeedMode_CH0(U32 eSDSpeedMode);
void SetSDSpeedMode_CH0(U32 eSDSpeedMode);
void SetCommandReg_CH0(U16 uCmd,U32 uIsAcmd);
void SetClock_CH0(U32 ClkSrc, U16 Divisor);
void SetSdhcCardIntEnable_CH0(U8 ucTemp);
int SetDataTransferWidth_CH0(U8 BD_BUS_WIDTH);
int SetSDOCR_CH0(void);
int SetMMCOCR_CH0(void);
int IsCardInProgrammingState_CH0(void);
int IssueCommand_CH0( U16 uCmd, U32 uArg, U32 uIsAcmd);

void InterruptEnable_CH0(U16 NormalIntEn, U16 ErrorIntEn);
int WaitForBufferReadReady_CH0(void);
void DisplayCardInfo_CID_CH0(void);
void DisplayCardInfo_CSD_CH0(void);

/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
#define SDI_Tx_buffer_HSMMC_CH0 (RAM_DMA_BUF_STR)
#define SDI_Rx_buffer_HSMMC_CH0 (RAM_DMA_BUF_STR) /* 8M Byte */
#define Card_OneBlockSize_ver1 512

// Global variables
extern U16 *Tx_buffer_HSMMC_ch0;	
extern U16 *Rx_buffer_HSMMC_ch0;













/////////////////////////////////////////////////////////////////////////////////////////////////
#define	SD_HCLK_CH0	1
#define	SD_EPLL_CH0		2
#define	SD_EXTCLK_CH0	3

#define	NORMAL_CH0	0
#define	HIGH_CH0	1

//Normal Interrupt Signal Enable
#define	READWAIT_SIG_INT_EN_CH0				(1<<10)
#define	CARD_SIG_INT_EN_CH0					(1<<8)
#define	CARD_REMOVAL_SIG_INT_EN_CH0			(1<<7)
#define	CARD_INSERT_SIG_INT_EN_CH0			(1<<6)
#define	BUFFER_READREADY_SIG_INT_EN_CH0	(1<<5)
#define	BUFFER_WRITEREADY_SIG_INT_EN_CH0	(1<<4)
#define	DMA_SIG_INT_EN_CH0					(1<<3)
#define	BLOCKGAP_EVENT_SIG_INT_EN_CH0		(1<<2)
#define	TRANSFERCOMPLETE_SIG_INT_EN_CH0	(1<<1)
#define	COMMANDCOMPLETE_SIG_INT_EN_CH0		(1<<0)

//Normal Interrupt Status Enable
#define	READWAIT_STS_INT_EN_CH0				(1<<10)
#define	CARD_STS_INT_EN_CH0					(1<<8)
#define	CARD_REMOVAL_STS_INT_EN_CH0			(1<<7)
#define	CARD_INSERT_STS_INT_EN_CH0			(1<<6)
#define	BUFFER_READREADY_STS_INT_EN_CH0	(1<<5)
#define	BUFFER_WRITEREADY_STS_INT_EN_CH0	(1<<4)
#define	DMA_STS_INT_EN_CH0					(1<<3)
#define	BLOCKGAP_EVENT_STS_INT_EN_CH0		(1<<2)
#define	TRANSFERCOMPLETE_STS_INT_EN_CH0	(1<<1)
#define	COMMANDCOMPLETE_STS_INT_EN_CH0		(1<<0)

#ifdef __cplusplus
}
#endif
#endif /*__HS1_MMC_H__*/
