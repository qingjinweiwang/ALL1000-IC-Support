#ifndef __HSPI_H__
#define __HSPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "Type.h"
#include "2450addr.h"


#define HSPI_BYTE			0
#define HSPI_HWORD			1
#define HSPI_WORD			2

#define Master 			0
#define Slave 			1

#define DMAMODE		0
#define INTMODE			1
#define POLLMODE		2

#define CPOLHIGH		0
#define CPOLLOW			1
#define FORMAT_A		0
#define FORMAT_B		1

#define IntMaxWaitTime		0x02000000 /* MAX 1500ms */
#define HSPI_CLK_33M		0 /* set HPCLK /2/(scaler+1) */
#define HSPI_CLK_16M5		1
#define HSPI_CLK_11M		2
#define HSPI_CLK_8M25		3
#define HSPI_CLK_6M6		4
#define HSPI_CLK_5M5		5
#define HSPI_CLK_4M7		6
#define HSPI_CLK_3M			10
#define HSPI_CLK_2M1		15
#define HSPI_CLK_1M			32


#define	CH_SW_RST               (1<<5)
#define	CH_MASTER               (0<<4)
#define	CH_SLAVE                (1<<4)
#define	CH_RISING               (0<<3)
#define	CH_FALLING              (1<<3)
#define	CH_FORMAT_A             (0<<2)
#define	CH_FORMAT_B             (1<<2)
#define	CH_RXCH_OFF             (0<<1)
#define	CH_RXCH_ON              (1<<1)
#define	CH_TXCH_OFF             (0<<0)
#define	CH_TXCH_ON              (1<<0)
                             
#define	CLK_CLKSEL_PCLK         (0<<9)
#define	CLK_CLKSEL_HCLK         (1<<9)
#define	CLK_CLKSEL_ECLK         (2<<9)
#define	CLK_CLKSEL_MPLL         (3<<9)
#define	CLK_ENCLK_DISABLE       (0<<8)
#define	CLK_ENCLK_ENABLE        (1<<8)

#define	MODE_BUS_SZ_BYTE        (0<<18)
#define	MODE_BUS_SZ_WORD        (1<<18)
#define	FEED_BACK_DELAY        (1<<17)
#define	MODE_BUS_4BURST        (1<<0)
#define	MODE_BUS_SINGLE        (0<<0)
#define	BURST       	 			1
#define	SINGLE       	 			0
//#define	MODE_SWAP_DISABLE       (0<<3)
//#define	MODE_SWAP_ENABLE        (1<<3)
#define	MODE_RXDMA_ON           (1<<2)
#define	MODE_TXDMA_ON           (1<<1)
#define	MODE_DMA_SINGLE         0
#define	MODE_DMA_4BURST         1

#define	INT_TRAILING            (1<<6)
#define	INT_RX_OVERRUN          (1<<5)
#define	INT_RX_UNDERRUN         (1<<4)
#define	INT_TX_OVERRUN          (1<<3)
#define	INT_TX_UNDERRUN         (1<<2)
#define	INT_RX_FIFORDY          (1<<1)
#define	INT_TX_FIFORDY          (1<<0)

#define	STUS_TX_DONE            (1<<21)
#define	STUS_TRAILCNT_ZERO      (1<<20)
#define	STUS_RX_OVERRUN         (1<<5)
#define	STUS_RX_UNDERRUN        (1<<4)
#define	STUS_TX_OVERRUN         (1<<3)
#define	STUS_TX_UNDERRUN        (1<<2)
#define	STUS_RX_FIFORDY         (1<<1)
#define	STUS_TX_FIFORDY         (1<<0)

enum DMA_HS_MODE
{
	DEMAND, HANDSHAKE
};

enum DMA_REQUEST_MODE
{
	SOFTWARE, HARDWARE
};

#define LOCAHB	0
#define LOCAPB	1
#define ADDRINC	0
#define ADDRFIX	1



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
extern int SpiTranDataCnt ;
extern int TransferdDataCnt ;
extern int ReceivedDataCnt ;
extern int SpiIntWaitTime ;
extern int SpiTxIntEnd ;
extern int SpiRxIntEnd ;

extern unsigned char  *TxBufAddr, *RxBufAddr;


#define ArmSpi_TxReg()		(rSPI_TX_DATA) /* Spi Tx reg */
#define ArmSpi_RxReg()		(rSPI_RX_DATA) /* Spi Rx reg */



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
extern void __irq HSPI_Tx_Int(void);
extern void __irq HSPI_Rx_Int(void);

extern void ResetArmSpiPortReg(void) ;
extern void SetArmSpi_TxChlReg(void) ;
extern void SetArmSpi_RxChlReg(void) ;
extern void SetArmSpi_RxDataInt(uShort RxDataCnt, U8* RxDataBufAddr) ; /* RxDataCnt MUST <= 64K per loop */
extern void SetArmSpi_TxDataInt(uShort TxDataCnt, U8* TxDataBufAddr) ; /* TxDataCnt MUST <= 64K per loop */
extern void InitHSPI(U32 MasterSlave, U32 Cpol, U32 Cpha, U32 ClkSel);
extern uInt32 ChkIfTxFifoEmpty(void) ;
extern uInt32 ChkIfTxDone(void) ;
extern void nSSLow(void);
extern void nSSHigh(void);
extern void SpiGPIOPortSet(uChar SpiClk);






#ifdef __cplusplus
}
#endif

#endif //__SPI_H__
