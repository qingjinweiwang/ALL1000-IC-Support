/******************************************************************************\
	Programmer total 13 tasks driver setting:
POWER_ON_FUN							POWER_OFF_FUN
READ_FUN			PROGRAM_FUN		VERIFY_FUN
ERASE_FUN			BLANK_CHECK_FUN	ILLEGAL_CHECK_FUN
ID_CHECK_FUN		SECURE_FUN			CHECKSUM_FUN
PROTECT_FUN								UNPROTECT_FUN
\*******************************************************************************/

#include "ALx4_S32K324A.h"
#include "h\Disp.h"
#include "h\Key.h"
#include "h\FirmWare.h"
#include "h\BlockSectorDef.h"
#include "h\Type.h"
#include "h\SystemTask.h"
#include "h\Subroutine.h"
#include "h\ErrorCode.h"
#include "h\PassedCode.h"
#include "h\USBCom.h"
#include "h\CFControl.h"
#include "h\StatusCode.h"
#include "h\hs0_mmc.h"

/****************************************************************************\
			Type Definitions
\****************************************************************************/
/* Variable type Definitions: */
typedef	unsigned int	DWORD;
typedef	unsigned short WORD;
typedef	unsigned char BYTE;


/**************************************************************\
Notes: User define variable here,
		  the area is UserVarityAreaMin ~ UserVarityAreaMin
\**************************************************************/

/*********** For Blank Page *************/
uInt32 PageDataBuf[32]; /* Max Buffer Size be 32 D-word=128 Bytes */
uShort BlockEndFlag; /* if Seccess=End, Fail=Not End */
uShort AckData, OutDataLo, OutDataHi;
uInt32 SWD_Data;
uShort OptionByteCnt;
uInt32 BlankPageFlag;
uShort BlockEndFlag; /* if Seccess=End, Fail=Not End */
uShort ProgPageSize;
uShort Parity_Data;
/**************************************************************\
	FPGA Control Port Decode Address: 0xD2000~0xD2FF
	User should use the Macro Definition refer to FPGA file setting !!!
	User MUST MUST MUST CANNOT Use Other Address !!!
	Or System will be Destroy !!!!
\**************************************************************/
//#define	OneDataCmd				(FPGAOffset|0x0100<<1)
#define	SetSCKPinCmd			(FPGAOffset|0x8200<<1)
//#define	SetRSTPinCmd			(FPGAOffset|0x8000<<1) 
#define	SetTMSPinCmd			(FPGAOffset|0x8600<<1)
#define	SetTDIPinCmd			(FPGAOffset|0x8700<<1)
#define	SetJCOMPinCmd			(FPGAOffset|0x8800<<1)
/// any more here, need also modify FPGA to support new Pin  ///
/**************************************************************\
	Set Easy FPGA Pin Number:A=1--48;
	then auto offset in FPGA: B=A+48, C=A+96, D=A+144
\**************************************************************/
#define	SetPinInPut			0x0000
#define	SetPinOutPut		0xF000

#define	SetPinDisLo			0x0000 /* when disable sck, this pin default be 0=Lo */
#define	SetPinDisHi			0x0F00 /* when disable sck, this pin default be 1=Hi */

///// FPGA Pin Num Define Here /////////////////////////////////////////////////
#define	SetVCCPinNum		(FPGAOffset|0xB000<<1) /* 0xB000 for easy FPGA pin setting */
#define	SetGNDPinNum		(FPGAOffset|0xB100<<1)
#define	SetSWDPinNum		(FPGAOffset|0xB200<<1) /* in/output dir, can also be TDO for JTAG */
#define	SetSCKPinNum		(FPGAOffset|0xB300<<1) /* output, can also be TCK for JTAG */
#define	SetRSTPinNum		(FPGAOffset|0xB400<<1) /* output */
#define	SetTMSPinNum		(FPGAOffset|0xB500<<1) /* output, can be define for other pin */
#define	SetTDIPinNum		(FPGAOffset|0xB600<<1) /* output, can be define for other pin */
#define	SetJCOMPinNum		(FPGAOffset|0xB700<<1) /* output, can be define for other pin */
/// any more here, need also modify FPGA to support new Pin  ///

///// FPGA Pin Num Define Here /////////////////////////////////////////////////
uChar VCCPinNum;
uChar GNDPinNum;
uChar SWDPinNum;
uChar SCKPinNum;
uChar RSTPinNum;
uChar TMSPinNum;
uChar TDIPinNum;
uChar JCOMPinNum;
/// any more here, need also modify FPGA to support new Pin  ///

/**************************************************************\
	Single type
\**************************************************************/
#define TTL_HI					0xFF
#define TTL_LO					0x00

/**************************************************************\
Notes: Use 1 time power on/off control flag: 1=Use 1 time, 0=Many time
\**************************************************************/
#define OnlyUse1TimePowerCtrl		1

// /********* SWD parameter  ********************************/ //
/*
 TAR : Transter Address Register
 DRW : Data/Write Register
 CSW : The Control/Status Word

 In all phases data is transmitted LSB (least significant
 bit) first. The target will both sample and put data on the line on a rising clock edge.

 HOST CONSIST OF WITH BELOW
 START  AP/DP  W/R  ADDR[2..3]  PARITY   STOP   PARK
   1    0 / 1  0/1                        0       1

 Bit 5 is a parity bit. The parity bit is used by the target to verify
 the integrity of the request. This bit should be 1 if bits 1-4 contains an odd number of 1's. If the number
 of 1's are even, the parity bit should be zero. Bit 6 is the stop bit. This bit is always zero. Bit 7 is the
 park bit. This bit is always one.

 */
 // SWD header format
 // | Start | APnDP | RnW | ADDR[2:3] | Parity | STOP | Park |
/************************************************************************************************************/
#define DPACC_IDCODE_RD       0xA5
#define DPACC_ABORT_WR        0x81
#define DPACC_IDCODE_WR       0x81

#define DPACC_DP_CTRLSTAT_WR  0xA9
#define DPACC_DP_CTRLSTAT_RD  0x8D
#define DPACC_DP_SELECT_WR    0xB1
#define DPACC_READBUFF_RD     0xBD

#define APACC_AP_CTRLSTAT_WR  0xA3

#define APACC_ADDR_WR         0x8B
#define APACC_DATA_RD         0x9F
#define APACC_DATA_WR         0xBB //

#define MDMAP_STATE_WR        0xA3
#define MDMAP_STATE_RD        0x87
#define MDMAP_CTRL_WR         0x8B
#define MDMAP_CTRL_RD         0xAF
#define MDMAP_IDR_RD          0x9F

#define  DPACC_ABORT_WR        0x81 //  1       0        0       0 0        0       0       1
#define  DPACC_IDCODE_RD       0xA5 //  1       0        1       0 0        1       0       1
#define  DPACC_CTRLSTAT_WR     0xA9 //  1       0        0       1 0        1       0       1   -- Selected by CTRLSEL bit(0) in SELECT register
#define  DPACC_CTRLSTAT_RD     0x8D //  1       0        1       1 0        0       0       1   -- Selected by CTRLSEL bit(0) in SELECT register
#define  DPACC_WCR_WR          0xA9 //  1       0        0       1 0        1       0       1   -- Selected by CTRLSEL bit(1) in SELECT register
#define  DPACC_WCR_RD          0x8D //  1       0        1       1 0        0       0       1   -- Selected by CTRLSEL bit(1) in SELECT register
#define  DPACC_SELECT_WR       0xB1 //  1       0        0       0 1        1       0       1
#define  DPACC_READBUFF_RD     0xBD //  1       0        1       1 1        1       0       1

#define  APACC_CSW_WR          0xA3 //  1       1        0       0 0        1       0       1
#define  AP_CSW                0xA3 //  1       1        0       0 0        1       0       1
#define  APACC_CSW_RD          0x87 //  1       1        1       0 0        0       0       1
#define  APACC_TAR_WR          0x8B //  1       1        0       1 0        0       0       1
#define  AP_TAR                0x8B //  1       1        0       1 0        0       0       1
#define  APACC_TAR_RD          0xAF //  1       1        1       1 0        1       0       1
#define  APACC_DRW_WR          0xBB //  1       1        0       1 1        1       0       1
#define  AP_DRW                0xBB //  1       1        0       1 1        1       0       1
#define  APACC_DRW_RD          0x9F //  1       1        1       1 1        0       0       1

// FREESCALE MDM-AP request Value
#define  APACC_Status_WR       0xA3 //  1       1        0       0 0        1       0       1
#define  APACC_Status_RD       0x87 //  1       1        1       0 0        0       0       1
#define  APACC_Control_WR      0x8B //  1       1        0       1 0        0       0       1
#define  APACC_Control_RD      0xAF //  1       1        1       1 0        1       0       1
#define  APACC_IDR_RD          0x9F //  1       1        1       1 1        0       0       1

// ARM CoreSight SW-DP packet request masks
#define SW_REQ_PARK_START       0x81
#define SW_REQ_PARITY           0x20
#define SW_REQ_A32              0x18
#define SW_REQ_RnW              0x04
#define SW_REQ_APnDP            0x02

// ARM CoreSight SW-DP packet acknowledge values
#define SW_ACK_OK               0x1
#define SW_ACK_WAIT             0x2
#define SW_ACK_FAULT            0x4
#define SW_ACK_PARITY_ERR       0x8

// ARM CoreSight DAP command values
#define DAP_IDCODE_RD           0x02
#define DAP_ABORT_WR            0x00
#define DAP_CTRLSTAT_RD         0x06
#define DAP_CTRLSTAT_WR         0x04
#define DAP_SELECT_WR           0x08
#define DAP_RDBUFF_RD           0x0E

// ARM CoreSight DAP command masks
#define DAP_CMD_PACKED          0x80
#define DAP_CMD_A32             0x0C
#define DAP_CMD_RnW             0x02
#define DAP_CMD_APnDP           0x01
#define DAP_CMD_MASK            0x0F
///////////////////////////////////////////////////////
#define  DPACC_IDCODE_RD       0xA5
#define  DPACC_ABORT_WR        0x81
#define  DPACC_CTRLSTAT_WR     0xA9
#define  DPACC_SELECT_WR       0xB1
#define  DPACC_READBUFF_WR     0x99
#define  DPACC_READBUFF_RD     0xBD

#define  APACC_CTRLSTAT_WR     0xA3
#define  RAZ_WI_WR             0xA3
#define  RAZ_WI_RD             0x87
#define  APACC_ADDR_WR         0x8B
#define  APACC_DATA_RD         0x9F
#define  APACC_DATA_WR         0xBB

/****************************************************************\
Notes: User Macro Define Here

\****************************************************************/
#ifndef _S32K324A_H_
#define _S32K324A_H_

#define _S32K324A_H_

#define S32K324A

#define AHB_AP      0   
#define AP1_AP      1

#define DPIDR       (0x6BA02477L)
#define AHB_APIDR   (0x84770001L)
#define AP1_APIDR   (0x54770002L)

#define ARM_CPU_ID  (0x410FD214L)

//#define TOTAL_PAGE  8                   // Should be 256 pages. From 0x00200000 to 0x0021FFFF
#define PAGE_SIZE   512L                // Size of a flash page in bytes

#define WDT             0x4000A834L
#define FLASH_ENTRY1    0x00200000L
#define FLASH_ENTRY2    0x00202000L
#define FLASH_ENTRY3    0x00204000L
#define RAM_LOOP        0x20002000L


#define FALSH_APP_BASE  0x00400000L
#define FALSH_APP_BASE_312  0x00200000L
#define FALSH_APP_BASE_311  0x00100000L
#define DATA_APP_BASE   0x10000000L
#define FLASH_SEC_BASE  0x00220000L

#define RAM_APP_BASE    0x20004000L

  /*
   * Customer configuration page
   */
#define CONFIG_PAGE     0x0023C000L

   /// Address of DP read registers
#define DP_IDCODE       0
#define DP_CTRL         1
#define DP_RESEND       2
#define DP_RDBUFF       3

/// Addresses of DP write registers
#define DP_ABORT        0
#define DP_STAT         1
#define DP_SELECT       2

/// AHB-AP registers
#define AP_CSW          0
#define AP_TAR          1
#define AP_DRW          3
#define AP_IDR          3

#define AIRCR_RESET_CMD         (0x05FA0006UL)

/// Bit fields for the CSW register
#define AP_CSW_32BIT_TRANSFER   (0x02L)
#define AP_CSW_AUTO_INCREMENT   (0x10L)
#define AP_CSW_MASTERTYPE_DEBUG (1L << 29)
#define AP_CSW_HPROT            (1L << 25)
#define AP_CSW_DEFAULT          (AP_CSW_32BIT_TRANSFER | AP_CSW_MASTERTYPE_DEBUG | AP_CSW_HPROT)

/// Bit fields for the ABORT register
#define DP_ABORT_ORUNERRCLR     (1UL << 4)
#define DP_ABORT_WDERRCLR       (1UL << 3)
#define DP_ABORT_STKERRCLR      (1UL << 2)
#define DP_ABORT_STKCMPCLR      (1UL << 1)

/// Power up request and acknowledge bits in CTRL/STAT
#define DP_CTRL_CDBGPWRUPREQ    (1UL << 28)
#define DP_CTRL_CDBGPWRUPACK    (1UL << 29)
#define DP_CTRL_CSYSPWRUPREQ    (1UL << 30)
#define DP_CTRL_CSYSPWRUPACK    (1UL << 31)

#endif /* _S32K324A_H_ */

/*
 *  Module		: armadi.h
 *  Description : ARM Debug Interface Architecture Specification
 *				  ADIv5(Compatible with the ARM CoreSight Architecture)
 *				  DAP incluses two parts :
 *					DP : By DPACC access
 *					AP : By APACC access
 *
 *  Created on	: 2015/11/18
 *  Author		: Alpha Liu
 *
 *  Reference	: ARM Debug Interface Architecture Specification
 *  				ADIv5.0 to ADIv5.2(ARM IHI 0031C)
 *  Remark		: - Define SW-DP(Serial Wire Debug Port)
 *				  - Little endian definition
 *				  - DPv1 implement.
 *
 *  History 	:
 *  				V1.0 - Created
 *  				V1.1 - Modified&refined
 */
#ifndef _ARMADI_INC_
#define _ARMADI_INC_

 /*
  * SW-DP Register address : READ and WRITE OPERATION has different address
  *		DPv1 register Address 3:2. In spec., Address is 3...0
  *		That means we need shift right 2 bits in spec. address to get
  *			ADDR32.
  */
  /* DPIDR(Debug Port Identification Register Address : 0 */
#define DP_REG_ADDR32_IDCODE	0
#define DP_DPIDR_ADDR			0
/* DP.ABORT Register Address : 0, Access : WO */
#define DP_REG_ADDR32_ABORT		0
#define DP_ABORT_ADDR			0

/* DP.CTRL/STAT Address : 0x4, Access : RW */
#define DP_REG_ADDR32_CTRLSTAT	1
#define DP_CTRLSTAT_ADDR		(DP_REG_ADDR32_CTRLSTAT<<2)
/* DP.SELECT Address : 0x8, Access: WO */
#define DP_REG_ADDR32_SELECT	2
#define	DP_SELECT_ADDR			(DP_REG_ADDR32_SELECT<<2)
 /* DP.RDBUFF Address : 0xc, Access : RO */
#define DP_REG_ADDR32_RDBUFF	3
#define DP_RDBUFF_ADDR			(DP_REG_ADDR32_RDBUFF<<2)

//==================================================================================================
/*
 * DP Abort Register definitions
 */
#define DAPABORT_MSK	0x00000001L	// b0: DAP abort
 /* Write 1 to this bit to clear the CTRL/STAT.STICKYCMP sticky compare bit to 0 */
#define	STKCMPCLR_MSK	0x00000002L	// b1: sticky compare
/* Write 1 to this bit to clear the CTRL/STAT.STICKYERR sticky error bit to 0. */
#define STKERRCLR_MSK	0x00000004L	// b2: sticky error bit
/* Write 1 to this bit to clear the CTRL/STAT.WDATAERR write data error bit to 0. */
#define WDERRCLR_MSK	0x00000008L	// b3: write data error bit
/* Write 1 to this bit to clear the CTRL/STAT.STICKYORUN overrun error bit to 0. */
#define ORUNERRCLR_MSK	0x00000010L	// b4: overrun error bit. 

/*
 * DP CTRL/STAT register
 *	Provides control of the DP and status information about DP.
 *	Access : WR
 */
 /* This bit is set to 1 to enable overrun detection. */
#define ORUNDETECT_SFT		(0)						// b0. Over Run detect
#define ORUNDETECT_MSK		(1L<<ORUNDETECT_SFT)

/* If overrun detection is enabled, this bit is set to 1 when an overrun occurs */
#define	STICKORUN_SFT		(1)						// b1. Sticky Overrun
#define	STICKORUN_MSK		(2L<<STICKORUN_SFT)	// b1. Sticky Overrun
/* This field sets the transfer mode for AP operations. */
#define	TRNMODE_SFT			(2)						// b3-2
#define	TRNMODE_NORMAL		0
/* This bit is set to 1 when a match occurs on a pushed-compare or a pushed-verify operation. */
#define	STICKYCMP_SFT		4 			// b4
/* This bit is set to 1 if an error is returned by an AP transaction */
#define	STICKYERR_SFT		5			// b5
/* indicates the response to the last AP read access */
#define	READOK_SFT			6			// b6
/* This bit is set to 1 if a Write Data Error occurs. */
#define	WDATAERR_SFT		7			// b7
/* Indicates the bytes to be masked in pushed-compare and pushed-verify operations.*/
#define	MASKLANE_SFT		8			// b11-8
/* Transaction counter. */
#define	TRNCNT_SFT			12			// b23-12
/* Debug reset request */
#define CDBGRSTREQ_SFT		26			// b26
/* Debug reset acknowledge. RO.*/
#define	CDBGRSTACK_SFT		27			// b27
/* Debug powerup request */
#define	CDBGPWRUPREQ_SFT	28			// b28
/* Debug powerup acknowledge R.O */
#define	CDBGPWRUPACK_SFT	29			// b29
/* System powerup request */
#define	CSYSPWRUPREQ_SFT	30			// b30
/* System powerup acknowledge. RO. */
#define	CSYSPWRUPACK_SFT	31			// b31

/*
 * AP Select register, SELECT
 *		Selects an Access Port (AP) and the active register banks within that AP.
 *		Selects the DP address bank.
 *	Access : WO
 */
 /* Debug Port address bank select */
#define SELECT_DPBANKSEL_MSK	0x0000000FL		// b0-b3 DPBANKSEL, SW-DP only
#define SELECT_DPBANKSEL_SFT	0				// b0-b3 DPBANKSEL, SW-DP only
/* Selects the active four-word register bank on the current AP */
#define SELECT_APBANKSEL_MSK	0x000000F0L		// b7-b4
#define SELECT_APBANKSEL_SFT	4				// b7-b4
/* Reserved */
//SELECT_RES0_MSK		:16;	// b23-b8
/* Selects an AP */
#define SELECT_APSEL_MSK		0xff000000L		// b31-b24 Select the current access port
#define SELECT_APSEL_SFT		24				// shift to APSEL field

//==================================================================================================
// ARM CoreSight SWD-DP packet request values. LSB first
/* Read DP's IDCODE Register */
#define  DP_IDCODE_RD      		0xA5

/* defined for DPIDR Version */
#define	DPv0	(0)
#define	DPv1	(1)
#define DPv2	(2)

/*
* SW-DP IDCODE
*	DPIDR of CoreSight-compatible Debug Access Port(DAP)
*/
/* IDCODE defined for ARM Minimal Debug Port (MINDP) : architecture version 1 and uses the Minimal Debug Port*/
#define	DP_IDCODE_CORETEX_M0_MINDP	(0x0BB11477L)

/* IDCODE defined for ARM CoreTex M0 DP Architecture 1*/
#define DP_IDCODE_CORETEX_M0_ARCH1		(0x0BC11477L)

/* IDCODE defined for ARM CoreTex M0 DP Architecture 2*/
#define DP_IDCODE_CORETEX_M0_ARCH2		(0x0BC12477L)

/* IDCODE defined for ARM CoreTex-M3 */
#define DP_IDCODE_CORETEX_M3		(0x1BA01477L)

/* IDCODE defined for ARM CoreTex-M0+ */
#define DP_IDCODE_CORETEX_M0P		(0x2BA01477L)

/*
 * Serial Wire Debug Request Phase
 * 	8-bits
 */
typedef union _SWD_REQUEST
{
	unsigned char rawByte;
	struct
	{
		unsigned int Start : 1;	// Start bit. 1 to indicate Start
		unsigned int APnDP : 1;	// Debug Port(0) or Access Port Access Register(1)
		unsigned int RnW : 1;	// Read(0) or Write request
		unsigned int A : 2;	// DP or AP register address
		unsigned int Parity : 1;	// Even Parity is used
		unsigned int Stop : 1;	// always 0
		unsigned int Park : 1;	// always 1
	}PACKET;
}SWD_REQUEST, * PSWD_REQUEST;

/*
 * Serial Wire Debug ACK
 *	3-bits from b0 to b2
 */
#define SWD_ACK_MSK			(0x7)
#define SWD_ACK_OK			(1)
#define SWD_ACK_WAIT		(2)
#define SWD_FAULT			(4)

 /* Define ARM AMBA AHB*/
 /*
  * ARM AMBA Access Port
  */
#define	AP_AHB_DEFAULT	(0)

  /* APSEL Decode. SELECT[31:24]. a.k.a. Access Port number */
  /* ARM AHB-AP IDR value */
#define AHB_AP_IDR_CORETEX_M0		(0x04770021L)
#define AHB_AP_IDR_CORETEX_M0P		(0x04770031L)
#define AHB_AP_IDR_CORETEX_M3_M4	(0x24770011L)
#define AHB_AP_IDR_CORETEX_M3		(0x14770011L)
#define AHB_AP_IDR_CORESIGHT		(0x34770001L)

/*
 * Access Port CSW Address : Bank 0x0, Offset 0x0
 * 		   b7  b6  b5  b4       b3  b2		b1  b0
 * 		 +------------------+--------------+-------+
 * 		 |-> Bank Address <-+->  Offset <--| 0    0|
 * 		 +------------------+--------------+-------+
 * Binary  0   0    0   0         0    0	 0    0
 */
#define AP_AHB_CSW			0x00

 /*
  * Access Port TAR Address : Bank 0x0, Offset 0x4
  * 		   b7  b6  b5  b4       b3  b2		b1  b0
  * 		 +------------------+--------------+-------+
  * 		 |-> Bank Address <-+->  Offset <--| 0    0|
  * 		 +------------------+--------------+-------+
  * Binary  0   0    0   0         0    1	 0    0
  */
#define AP_AHB_TAR			0x04

  /*
   * Access Port DRW Address : Bank 0x0, Offset 0xC
   * 		   b7  b6  b5  b4       b3  b2		b1  b0
   * 		 +------------------+--------------+-------+
   * 		 |-> Bank Address <-+->  Offset <--| 0    0|
   * 		 +------------------+--------------+-------+
   * Binary  0   0    0   0         1    1	 0    0
   */
#define AP_AHB_DRW			0x0C

   /*
	* Access Port BD0 Address : Bank 0x1, Offset 0x0
	* 		   b7  b6  b5  b4       b3  b2		b1  b0
	* 		 +------------------+--------------+-------+
	* 		 |-> Bank Address <-+->  Offset <--| 0    0|
	* 		 +------------------+--------------+-------+
	* Binary  0   0    0   1         0    0	 0    0
	*/
#define AP_AHB_BD0			0x10

	/*
	 * Access Port BD1 Address : Bank 0x1, Offset 0x4
	 * 		   b7  b6  b5  b4       b3  b2		b1  b0
	 * 		 +------------------+--------------+-------+
	 * 		 |-> Bank Address <-+->  Offset <--| 0    0|
	 * 		 +------------------+--------------+-------+
	 * Binary  0   0    0   1         0    1	 0    0
	 */
#define AP_AHB_BD1			0x14

	 /*
	  * Access Port BD2 Address : Bank 0x1, Offset 0x8
	  * 		   b7  b6  b5  b4       b3  b2		b1  b0
	  * 		 +------------------+--------------+-------+
	  * 		 |-> Bank Address <-+->  Offset <--| 0    0|
	  * 		 +------------------+--------------+-------+
	  * Binary  0   0    0   1         1    0	 0    0
	  */
#define AP_AHB_BD2			0x18

	  /*
	   * Access Port BD3 Address : Bank 0x1, Offset 0xC
	   * 		   b7  b6  b5  b4       b3  b2		b1  b0
	   * 		 +------------------+--------------+-------+
	   * 		 |-> Bank Address <-+->  Offset <--| 0    0|
	   * 		 +------------------+--------------+-------+
	   * Binary  0   0    0   1         1    1	 0    0
	   */
#define AP_AHB_BD3			0x1C

	   /*
		* Access Port DEBUG ROME TABLE Address : Bank 0xF, Offset 0x8
		* 		   b7  b6  b5  b4       b3  b2		b1  b0
		* 		 +------------------+--------------+-------+
		* 		 |-> Bank Address <-+->  Offset <--| 0    0|
		* 		 +------------------+--------------+-------+
		* Binary  1   1    1   1         1    0	 0    0
		*/
#define AP_AHB_DBGROMTABLE	0xF8

		/*
		 * Access Port Register ID Address : Bank 0xF, Address 0xC
		 * 		   b7  b6  b5  b4       b3  b2		b1  b0
		 * 		 +------------------+--------------+-------+
		 * 		 |-> Bank Address <-+-> Address <--| 0    0|
		 * 		 +------------------+--------------+-------+
		 * Binary  1   1    1   1       1    1		 0    0
		 */
		 /* AHB IDR in Bank 0xF, and Register number 0xC */
#define AP_AHB_IDR					0xFC

/*
 * MEM-AP BANK Address : Bank 0xF, Address 0x8
 * 	Debug Base Address Register
 */
#define AP_AHB_BASE					0xF8
#define BASE_MASK					0xFFFFF000L

 /*
  * AP CSW(Control Status Word Register)
  */
#define APREG_CSW_SIZE_SHIFT		0
#define	CSW_SIZE_8 					0		// 8-bits
#define	CSW_SIZE_16 				1		// 16-bits
#define	CSW_SIZE_32					2		// 32-bits
#define	CSW_SIZE_64					3		// 64-bits
#define APREG_CSW_ADDRINC_SHIFT		4
#define APREG_CSW_DEVICEEN_SHIFT	6
#define APREG_CSW_TRINPTOG_SHIFT	7
#define APREG_CSW_MODE_SHIFT		8
#define APREG_CSW_TYPE_SHIFT		12
#define APREG_CSW_SPIDEN_SHIFT		23
#define APREG_CSW_PORT_SHIFT		24
#define APREG_CSW_DBGSWENABLE_SHIFT	31

  /*
   * ARM THUMB SW BKPT(Software BreakPoint) machine code
   */
#define ARM_BKPT			0xBEAABEAAUL	// 0xBEBEBEBEL

   /*
	* ARM 32-bits BKPT
	*/
#define ARM_UND				0xFEDEFFE7L

#define ROMTABLE_BASE		0xE00FF000L		// ROM table base address
#define SCS_BASE			0xE000E000L		// System Control Space base address
#define DWT_BASE			0xE0001000L		// Data Watch Point
#define FPB_BASE			0xE0002000L		// Flash Patch and Breakpoint Unit
#define TPIU_BASE			0xE0004000L		// Embedded Trace Macrocell

#define PID4_OFFSET			0xFD0L
#define PID_OFFSET			0xFE0L


	/*
	 * DWT: Data Watchpoint and Trace Unit
	 */
#define DWT_CTRL		(DWT_BASE+0x000L)
#define DWT_PCSR		(DWT_BASE+0x01CL)
#define DWT_COMPx		(DWT_BASE+0x020L)
#define DWT_MASKx		(DWT_BASE+0x024L)
#define DWT_FUNCTIONx	(DWT_BASE+0x028L)

#define BP_CTRL			(FPB_BASE+0x000L)
#define BP_COMPx		(FPB_BASE+0x008L)		/* Breakpoint Comparator registers */
#define BP_COMP0		(FPB_BASE+0x008L)		/* Breakpoint Comparator register 0 */
#define BP_COMP1		(FPB_BASE+0x00CL)		/* Breakpoint Comparator register 1 */
#define BP_COMP2		(FPB_BASE+0x010L)		/* Breakpoint Comparator register 2 */
#define BP_COMP3		(FPB_BASE+0x014L)		/* Breakpoint Comparator register 3 */

#define FP_CTRL			(FPB_BASE+0x000L)
#define FP_COMPx		(FPB_BASE+0x008L)		/* Breakpoint Comparator registers */
#define FP_COMP0		(FPB_BASE+0x008L)		/* Breakpoint Comparator register 0 */
#define FP_COMP1		(FPB_BASE+0x00CL)		/* Breakpoint Comparator register 1 */
#define FP_COMP2		(FPB_BASE+0x010L)		/* Breakpoint Comparator register 2 */
#define FP_COMP3		(FPB_BASE+0x014L)		/* Breakpoint Comparator register 3 */
#define FP_COMP4		(FPB_BASE+0x018L)		/* Breakpoint Comparator register 4 */
#define FP_COMP5		(FPB_BASE+0x01CL)		/* Breakpoint Comparator register 5 */
#define FP_COMP6		(FPB_BASE+0x020L)		/* Breakpoint Comparator register 6 */
#define FP_COMP7		(FPB_BASE+0x024L)		/* Breakpoint Comparator register 7 */


	 /*
	  * SCB(System control block)
	  * The System control block (SCB) provides system implementation information,
	  * and system control. This includes configuration, control, and reporting of
	  * the system exceptions.
	  * https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-control-block
	  */
#define ICTR 			0xE000E004L		// ICTR. RO. Interrupt Control Type Register
#define SCB_ACTLR		0xE000E008L		// Auxiliary Control Register
#define SCB_BASE		0xE000ED00L		// System Control Space Base address
#define CPUID			0xE000ED00L		// CPUID Base Register. Reset value is 0x412FC230
#define ICSR			0xE000ED04L		//
#define VTOR 			0xE000ED08L		//
#define AIRCR			0xE000ED0CL		//
#define SCR  			0xE000ED10L		//
#define CCR  			0xE000ED14L		//
#define SHPR1			0xE000ED18L		//
#define SHPR2			0xE000ED1CL		//
#define SHPR3			0xE000ED20L		//
#define SHCRS			0xE000ED24L		//
#define CFCR 			0xE000ED28L		//
#define MMSR 			0xE000ED28L		//
#define BFSR 			0xE000ED29L		//
#define UFSR 			0xE000ED2AL		//
#define HFSR 			0xE000ED2CL		//
#define DFSR			0xE000ED30L		// Debug Fault Status Register.
#define MMAR 			0xE000ED34L		//
#define BFAR 			0xE000ED38L		//
#define AFSR 			0xE000ED3CL		//
#define CLIDR			0xE000ED78L	 /* Cache Level ID Register */

#define CPACR 			0xE000ED88L		// Coprocessor Access Control Register
	  /*
	   * SCB : Debug Registers
	   */
#define	DHCSR			0xE000EDF0L		// Debug Halting Control and Status Register, DHCSR
#define	DCRSR			0xE000EDF4L		// (DCRSR)Debug Core Register Selector Register address. Debug register support in the SCS
#define	DCRDR			0xE000EDF8L		// (DCRDR)Debug Core Register Data Register address. Debug register support in the SCS
#define DEMCR			0xE000EDFCL		// Debug Exception and Monitor Control Register address. Debug register support in the SCS

#define CSSELR			0xE000ED84L

	   /* Application Interrupt and Reset Control Register Address */
#define AIRCR_VECTKEY				0x05FA			// WO
#define AIRCR_VECTKEY_SHIFT			16
#define AIRCR_ENDIANESS_SHIFT		15				// RO. 0:Little-endian, 1:Big-endian
#define AIRCR_SYSRESETREQ_SHIFT		2				// WO.
#define AIRCR_VECTCLRACTIVE_SHIFT	0				// WO

/*
 * DHCSR(Debug Halting Control and Status Register) address. Debug register support in the SCS
 */
#define DHCSR_C_DEBUGEN_SHIFT		(0)
#define DHCSR_C_HALT_SHIFT			(1)
#define DHCSR_C_STEP_SHIFT			(2)
#define DHCSR_C_NASKINTS_SHIFT		(3)
#define DHCSR_RESERVED_SHIFT		(4)
#define DHCSR_C_SNAPSTALL_SHIFT		(5)
#define DHCSR_DBGKEY_SHIFT			(16)			// WO
#define DHCSR_DBGKEY				(0xA05FUL)		// [31:16] is a ¨debug key〃 value which must be set to 0xA05F to allow [15:0] be updated.
#define DHCSR_S_REGRDY_SHIFT		(16)			// RO
#define DHCSR_S_HALT_SHIFT			(17)			// RO
#define DHCSR_S_SLEEP_SHIFT			(18)			// RO
#define DHCSR_S_LOCKUP_SHIFT		(19)			// RO
#define DHCSR_S_RETIRE_ST_SHIFT		(24)			// RO
#define DHCSR_S_RESET_ST_SHIFT		(25)			// RO

 /*
  * content for DCRSR_REGSEL
  */
#define DCRSR_REGSEL_SHIFT		(0)			// Occupy 7-bits
#define DCRSR_REGSEL_MASK		(0x7FL)		// Occupy 7-bits
  /*
   * Specifies the access direction of the transfer
   * 0 = read
   * 1 = write
   */
#define DCRSR_REGWnR_SHIFT		(16)		// 1 bit.

#endif _ARMADI_INC_ 
/**************************************************************\
Notes: User Macro define variable here
\**************************************************************/
const DWORD init_ap4_data[] = {
		0x402DC134L, 0x0000F7DFL, 0x402DC100L, 0x00000001L, 0x402DC104L, 0x00000001L, 0x402DC000L, 0x00005AF0L,
		0x402DC000L, 0x0000A50FL, 0x402DC330L, 0xB1E0FFF8L,     0x402DC334L, 0x812AA407L, 0x402DC338L, 0xBBF3FE7EL,
		0x402DC33CL, 0x00000141L, 0x402DC300L, 0x00000001L, 0x402DC304L, 0x00000001L, 0x402DC000L, 0x00005AF0L,
		0x402DC000L, 0x0000A50FL, 0x402DC530L, 0x29FFFFF0L, 0x402DC534L, 0xC48987F9L, 0x402DC500L, 0x00000001L,
		0x402DC504L, 0x00000001L, 0x402DC000L, 0x00005AF0L,     0x402DC000L, 0x0000A50FL
};
const DWORD param_program_init[] = {
		0x20004758L, 0x00000200L, 0x00200000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,
		0x20004B80L, 0x20004B90L, 0x7FFFFFFEL, 0x00000008L
};
const DWORD param_program_1[] = {
		0x20004958L, 0x00000200L, 0x00200000L, 0x00000000L, 0x00000200L, 0x00000000L, 0x00000000L, 0x00000000L,
		0x20004BB8L, 0x20004B58L, 0x7FFFFFFEL, 0x00000006L
};
const DWORD param_program_2[] = {
		0x20004758L, 0x00000200L, 0x00200000L, 0x00000000L, 0x00000200L, 0x00000000L, 0xEDB88320L, 0xFFFFFFFFL,
		0x20004B80L, 0x20004B90L, 0x7FFFFFFEL, 0x00000003L
};
const DWORD param_program_done[] = {
		0x20004958L, 0x00000200L, 0x00200000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,
		0x20004BB8L, 0x20004B58L, 0x7FFFFFFEL, 0x00000009L
};
const DWORD erase_program_app[] = {
		0xA801BE00L, 0xF0009900L, 0xE7F9F99DL, 0x11223344L, 0x00000001L, 0x304046ECL, 0x230F4685L, 0x20041C1AL,
		0xD3000852L, 0x1E40404AL, 0xB404D1FAL, 0xD5F51E5BL, 0x477046E5L, 0x9D04B5E0L, 0x1E7F2700L, 0x8816D41AL,
		0x072E4075L, 0x598E0EB6L, 0x4075092DL, 0x0EB6072EL, 0x092D598EL, 0x072E4075L, 0x598E0EB6L, 0x4075092DL,
		0x0EB6072EL, 0x092D598EL, 0x1C924075L, 0xD1E41E9BL, 0xBDE01C28L, 0xF897F000L, 0xB5F1E7E1L, 0x2A102700L,
		0x1E7FD40DL, 0xC971D407L, 0x191B181BL, 0x199B195BL,     0xD1F43A10L, 0x9800E00AL, 0xF885F000L, 0x1E7FE7F3L,
		0x680ED406L, 0x199B1D09L, 0xD1F81F12L, 0xBDF21C18L, 0xF0009800L, 0xE7F4F878L, 0x680AB5F2L, 0x688C684BL,
		0x27002600L, 0xD40D1E7FL, 0x599D5991L, 0xD10442A9L, 0x1EA41D36L, 0x2000D1F6L, 0x199BBDF2L, 0x600B9900L,
		0xBDF22001L, 0xF85FF000L, 0xB5F1E7EEL, 0x2A102700L, 0x1E7FD41CL, 0xC971D40BL, 0xD10C4283L, 0xD10D42A3L,
		0xD10E42ABL, 0xD10F42B3L, 0xD1F03A10L, 0x9800E016L, 0xF849F000L, 0x3910E7EFL, 0xE0111C06L, 0x1C26390CL,
		0x3908E00EL, 0xE00B1C2EL, 0xE0091F09L, 0xD40D1E7FL, 0x42B3680EL, 0x1D09D104L, 0xD1F71F12L, 0xBDF22000L,
		0x60019806L, 0x60069807L, 0xE7F82001L, 0xF0009800L, 0xE7EDF82AL, 0x467CB5F2L, 0x02A40AA4L, 0x68D16825L,
		0x69121909L, 0x23002700L, 0xD4161E7FL, 0x682650CBL, 0xD10242AEL, 0x42931D1BL, 0x6025D1F6L, 0x1E7F2600L,
		0x598DD40EL, 0xD10242B5L, 0x429E1D36L, 0xBC02D1F7L, 0x2600608EL, 0xBDF0600EL, 0xF000E732L, 0xE7E5F804L,
		0xF801F000L, 0xB50FE7EDL, 0xF8C7F000L, 0xBD0F27FFL,     0xB096B5F7L, 0xA8056991L, 0xFF2CF7FFL, 0x68809818L,
		0x68C99918L, 0x98181845L, 0x68006907L, 0x90029003L, 0x69469818L, 0xD53D2F00L, 0x087F007FL, 0xD0412E00L,
		0xA8042400L, 0x90009001L, 0x69C39818L, 0x0029003AL, 0xF7FF9816L, 0x2800FF7AL, 0x9802D102L, 0xE0102101L,
		0x68009803L, 0xD10E0781L, 0x68499903L, 0x68929A18L, 0x68DB9B18L, 0x581018D2L, 0x42889004L, 0x9802D003L,
		0x60012102L, 0x9400E00CL, 0x002A003BL, 0x9816A905L, 0xFF00F7FFL, 0x98020004L, 0x60012100L, 0x60449802L,
		0x30089803L, 0x98029003L, 0x90023008L, 0x1E7619EDL, 0xE007D1C6L, 0x003B9600L, 0xA905002AL, 0xF7FF9816L,
		0x0004FEE9L, 0x60849817L, 0x21009817L, 0xB0196001L, 0xB538BDF0L, 0x0011000CL, 0x690A694BL, 0x68C9688DL,
		0xF7FF1869L, 0x60A0FEFAL, 0x60202000L, 0xB530BD31L, 0x000CB083L, 0x00220011L, 0x9201320CL, 0x32080022L,
		0x694B9200L, 0x688D690AL, 0x186968C9L, 0xFF1DF7FFL, 0xBD376020L, 0xB086B570L, 0x000C0005L, 0x68300016L,
		0x68B09000L, 0x184068F1L, 0x69309001L, 0x0A400200L, 0x69709002L, 0x69B09003L, 0x69F09004L, 0x46689005L,
		0xF874F000L, 0xD1062800L, 0x01C96931L, 0x4669D403L, 0xF7FF0028L, 0x9900FEE1L, 0x602060A1L, 0xBD70B006L,
		0x000CB510L, 0x69C30010L, 0x69416982L, 0xF0006900L, 0x6020F953L, 0xB510BD10L, 0xF000000CL, 0xE003FA42L,
		0x000CB510L, 0xFA3FF000L, 0x280060E0L, 0x2000D502L, 0xE00043C0L, 0x60202000L, 0xB580BD10L, 0xFA30F000L,
		0x0000BD01L, 0x000E0004L, 0x0032E007L, 0xF7FF0020L, 0x002EFFE7L, 0xF7FF0020L, 0x6AF0FFF0L, 0xD0F92800L,
		0x6A756A31L, 0x600A4A18L, 0x62F22200L, 0x28091E40L, 0xA201D8EFL, 0x44975612L, 0xD81C1208L, 0x443A30D8L,
		0x003226D0L, 0xF7FF0020L, 0xE7E2FEE5L, 0x00200032L, 0xFF7DF7FFL, 0x0032E7DDL, 0xF7FF0020L, 0xE7D8FF09L,
		0x00200032L, 0xFF65F7FFL, 0x0032E7D3L, 0xF7FF0020L, 0xE7CEFF81L, 0x00200032L, 0xFFA2F7FFL, 0x0032E7C9L,
		0xF7FF0020L, 0xE7C4FFA8L, 0x7FFFFFFFL, 0xB08FB4F0L, 0x68026841L, 0x68C59205L, 0x92006902L, 0x035B23FAL,
		0xD102429AL, 0x03922280L, 0x23F4E00CL, 0x429A031BL, 0x2280D102L, 0xE0050352L, 0x02DB23E8L, 0xD102429AL,
		0x03122280L, 0x68809200L, 0x09C00040L, 0x20009004L, 0x900843C0L, 0x20C04A61L, 0x60500280L, 0x48602300L,
		0x9E001808L, 0x1C5BE001L, 0x1C5CB2DBL, 0x42A04374L, 0x2401D2F9L, 0x6800485BL, 0x26010C00L, 0x46684006L,
		0x20FF7706L, 0x4E580340L, 0x40066976L, 0x466FD107L, 0x2F007F3FL, 0x4E54D103L, 0x403069B6L, 0x48530006L,
		0x06006800L, 0x90010980L, 0x90031B80L, 0x00181B4DL, 0x26013011L, 0x960D4086L, 0x08409801L, 0x98039006L,
		0x1B809E06L, 0x484A900CL, 0x18369E03L, 0x4E49960BL, 0x19BE9F00L, 0x9E01960AL, 0x90091830L, 0x18084840L,
		0x42B09E03L, 0x9E01D302L, 0xD31D42B0L, 0x7F36466EL, 0xD0092E00L, 0x05679E00L, 0xD10F42BEL, 0x42B09E0BL,
		0x9E09D302L, 0xD30F42B0L, 0x4285980AL, 0x9800D30EL, 0x04A61A28L, 0x0B401980L, 0xE0083020L, 0x42B09E0CL,
		0x9E06D3F2L, 0xD2EF42B0L, 0xE0542008L, 0xB2C00C28L, 0x9E089002L, 0xD01942B0L, 0x00984607L, 0xD30A2F20L,
		0x19804E9BL, 0x46B46806L, 0x26013F20L, 0x466740BEL, 0x600743B7L, 0x4E97E008L, 0x68301986L, 0x20014684L,
		0x466740B8L, 0x60374387L, 0x90089802L, 0x60014892L, 0x26204892L, 0x683F9F05L, 0x9F056007L, 0x97051D3FL,
		0x1E761D00L, 0x6810D1F6L, 0x43060D96L, 0x48166016L, 0x9E0D6800L, 0xD1FA4006L, 0x43206810L, 0x68506010L,
		0xD5FC0400L, 0x04466850L, 0x2000D402L, 0xE01243C0L, 0x02B626C0L, 0xD0014006L, 0xE00C2006L, 0x43A06810L,
		0x4E7F6010L, 0x60164006L, 0x98043180L, 0x90041E40L, 0x28003580L, 0xB00FD182L, 0x4770BCF0L, 0x402EC000L,
		0xFFC00000L, 0x402AC000L, 0x4039C028L, 0x40290008L, 0xFFE00000L, 0xFFFC0000L, 0xB40DB4F8L, 0x22FAB08BL,
		0x42930352L, 0x2080D102L, 0xE00C0380L, 0x031222F4L, 0xD1024293L, 0x03402080L, 0x22E8E005L, 0x429302D2L,
		0x2080D102L, 0x900D0300L, 0x900520FFL, 0x20C04A65L, 0x60500280L, 0x48642300L, 0x9D0D1808L, 0x1C5BE001L,
		0x1C5CB2DBL, 0x42A0436CL, 0x2401D2F9L, 0x6800485FL, 0x25010C00L, 0x46684005L, 0x20FF7405L, 0x4D5C0340L,
		0x4005696DL, 0x466ED107L, 0x2E007C36L, 0x4D58D103L, 0x402869ADL, 0x48570005L, 0x06006800L, 0x90000980L,
		0x90021B40L, 0x1A0D980CL, 0x30110018L, 0x40862601L, 0x9800960AL, 0x90030840L, 0x9E039802L, 0x90091B80L,
		0x9E02484DL, 0x96081836L, 0x9F0D4E4CL, 0x960719BEL, 0x18309E00L, 0x48449006L, 0x9E021808L, 0xD30242B0L,
		0x42B09E00L, 0x466ED31DL, 0x2E007C36L, 0x9E0DD009L, 0x42BE0567L, 0x9E08D10FL, 0xD30242B0L, 0x42B09E06L,
		0x9807D30FL, 0xD30E4285L, 0x1A28980DL, 0x198004A6L, 0x30200B40L, 0x9E09E008L, 0xD3F242B0L, 0x42B09E03L,
		0x2008D2EFL, 0x0C28E050L, 0x9001B2C0L, 0x42B09E05L, 0x4607D018L, 0x2F20009EL, 0x4825D309L, 0x46845830L,
		0x20013F20L, 0x466740B8L, 0x48214387L, 0x4821E007L, 0x46845830L, 0x40B82001L, 0x43874667L, 0x5037481DL,
		0x90059801L, 0x6001481CL, 0x2600481CL, 0x68106006L, 0x43B02620L, 0x43062610L, 0x481D6016L, 0x9E0A6800L,
		0xD1FA4006L, 0x43206810L, 0x68506010L, 0xD5FC0400L, 0x04466850L, 0x2000D402L, 0xE01543C0L, 0x02B626C0L,
		0xD0014006L, 0xE00F2006L, 0x43A06810L, 0x26306010L, 0x601043B0L, 0x18090360L, 0x1E40980BL, 0x0360900BL,
		0x980B182DL, 0xD1862800L, 0xBCF0B00FL, 0x00004770L, 0x40268340L, 0x4026835CL, 0x40268300L, 0x402EC100L,
		0xFFFFFEFFL, 0x402EC000L, 0xFFC00000L, 0x402AC000L, 0x4039C028L, 0x40290008L, 0xFFE00000L, 0xFFFC0000L,
		0x20004770L, 0x20004770L, 0x00004770L
};
const DWORD erase_dflash[] = {
		0xA801BE00L, 0xF0009900L, 0xE7F9F99DL, 0x11223344L, 0x00000001L, 0x304046ECL, 0x230F4685L, 0x20041C1AL,
		0xD3000852L, 0x1E40404AL, 0xB404D1FAL, 0xD5F51E5BL, 0x477046E5L, 0x9D04B5E0L, 0x1E7F2700L, 0x8816D41AL,
		0x072E4075L, 0x598E0EB6L, 0x4075092DL, 0x0EB6072EL, 0x092D598EL, 0x072E4075L, 0x598E0EB6L, 0x4075092DL,
		0x0EB6072EL, 0x092D598EL, 0x1C924075L, 0xD1E41E9BL, 0xBDE01C28L, 0xF897F000L, 0xB5F1E7E1L, 0x2A102700L,
		0x1E7FD40DL, 0xC971D407L, 0x191B181BL, 0x199B195BL, 0xD1F43A10L, 0x9800E00AL, 0xF885F000L, 0x1E7FE7F3L,
		0x680ED406L, 0x199B1D09L, 0xD1F81F12L, 0xBDF21C18L, 0xF0009800L, 0xE7F4F878L, 0x680AB5F2L, 0x688C684BL,
		0x27002600L, 0xD40D1E7FL, 0x599D5991L, 0xD10442A9L, 0x1EA41D36L, 0x2000D1F6L, 0x199BBDF2L, 0x600B9900L,
		0xBDF22001L, 0xF85FF000L, 0xB5F1E7EEL, 0x2A102700L, 0x1E7FD41CL, 0xC971D40BL, 0xD10C4283L, 0xD10D42A3L,
		0xD10E42ABL, 0xD10F42B3L, 0xD1F03A10L, 0x9800E016L, 0xF849F000L, 0x3910E7EFL, 0xE0111C06L, 0x1C26390CL,
		0x3908E00EL, 0xE00B1C2EL, 0xE0091F09L, 0xD40D1E7FL, 0x42B3680EL, 0x1D09D104L, 0xD1F71F12L, 0xBDF22000L,
		0x60019806L, 0x60069807L, 0xE7F82001L, 0xF0009800L, 0xE7EDF82AL, 0x467CB5F2L, 0x02A40AA4L, 0x68D16825L,
		0x69121909L, 0x23002700L, 0xD4161E7FL, 0x682650CBL, 0xD10242AEL, 0x42931D1BL, 0x6025D1F6L, 0x1E7F2600L,
		0x598DD40EL, 0xD10242B5L, 0x429E1D36L, 0xBC02D1F7L, 0x2600608EL, 0xBDF0600EL, 0xF000E732L, 0xE7E5F804L,
		0xF801F000L, 0xB50FE7EDL, 0xF8C7F000L, 0xBD0F27FFL, 0xB096B5F7L, 0xA8056991L, 0xFF2CF7FFL, 0x68809818L,
		// 0x2000_01C0
		0x68C99918L, 0x98181845L, 0x68006907L, 0x90029003L, 0x69469818L, 0xD53D2F00L, 0x087F007FL, 0xD0412E00L,
		0xA8042400L, 0x90009001L, 0x69C39818L, 0x0029003AL, 0xF7FF9816L, 0x2800FF7AL, 0x9802D102L, 0xE0102101L,
		0x68009803L, 0xD10E0781L, 0x68499903L, 0x68929A18L, 0x68DB9B18L, 0x581018D2L, 0x42889004L, 0x9802D003L,
		0x60012102L, 0x9400E00CL, 0x002A003BL, 0x9816A905L, 0xFF00F7FFL, 0x98020004L, 0x60012100L, 0x60449802L,
		0x30089803L, 0x98029003L, 0x90023008L, 0x1E7619EDL, 0xE007D1C6L, 0x003B9600L, 0xA905002AL, 0xF7FF9816L,
		0x0004FEE9L, 0x60849817L, 0x21009817L, 0xB0196001L, 0xB538BDF0L, 0x0011000CL, 0x690A694BL, 0x68C9688DL,
		0xF7FF1869L, 0x60A0FEFAL, 0x60202000L, 0xB530BD31L, 0x000CB083L, 0x00220011L, 0x9201320CL, 0x32080022L,
		0x694B9200L, 0x688D690AL, 0x186968C9L, 0xFF1DF7FFL, 0xBD376020L, 0xB086B570L, 0x000C0005L, 0x68300016L,
		0x68B09000L, 0x184068F1L, 0x69309001L, 0x0A400200L, 0x69709002L, 0x69B09003L, 0x69F09004L, 0x46689005L,
		0xF874F000L, 0xD1062800L, 0x01C96931L, 0x4669D403L, 0xF7FF0028L, 0x9900FEE1L, 0x602060A1L, 0xBD70B006L,
		0x000CB510L, 0x69C30010L, 0x69416982L, 0xF0006900L, 0x6020F8D8L, 0xB510BD10L, 0xF000000CL, 0xE003F934L,
		0x000CB510L, 0xF931F000L, 0x280060E0L, 0x2000D502L, 0xE00043C0L, 0x60202000L, 0xB580BD10L, 0xF922F000L,
		0x0000BD01L, 0x000E0004L, 0x0032E007L, 0xF7FF0020L, 0x002EFFE7L, 0xF7FF0020L, 0x6AF0FFF0L, 0xD0F92800L,
		0x6A756A31L, 0x600A4A18L, 0x62F22200L, 0x28091E40L, 0xA201D8EFL, 0x44975612L, 0xD81C1208L, 0x443A30D8L,
		0x003226D0L, 0xF7FF0020L, 0xE7E2FEE5L, 0x00200032L, 0xFF7DF7FFL, 0x0032E7DDL, 0xF7FF0020L, 0xE7D8FF09L,
		0x00200032L, 0xFF65F7FFL, 0x0032E7D3L, 0xF7FF0020L, 0xE7CEFF81L, 0x00200032L, 0xFFA2F7FFL, 0x0032E7C9L,
		0xF7FF0020L, 0xE7C4FFA8L, 0x7FFFFFFFL, 0xB083B4F0L, 0x68580003L, 0x689A6819L, 0x09D20052L, 0x930168DBL,
		0x25C04C63L, 0x606502ADL, 0x682D4D62L, 0x0DAE01ADL, 0x006D25BBL, 0xD00842AEL, 0x42AE1CADL, 0x3512D005L,
		0xD00242AEL, 0x42AE1CADL, 0x4D5BD102L, 0xE001354CL, 0x35504D59L, 0x25019500L, 0x68379E00L, 0x9F0146BCL,
		0x40BE2601L, 0x43B74667L, 0x60379E00L, 0x429E9E01L, 0x9E00D011L, 0x46B46836L, 0x27019E01L, 0x466640B7L,
		0x9E004337L, 0x9E006037L, 0x26016837L, 0x43B7409EL, 0x60379E00L, 0x4B489301L, 0x4B486018L, 0x680F2620L,
		0x1D09601FL, 0x1E761D1BL, 0x6823D1F9L, 0x431E0DA6L, 0x68236026L, 0x6023432BL, 0x041B6863L, 0x6863D5FCL,
		0x02B626D0L, 0x03AB401EL, 0xD002429EL, 0x43C02000L, 0x6826E014L, 0x602643AEL, 0x40334B39L, 0x30806023L,
		0x061B23F0L, 0x0B5B18C3L, 0xD1BF1E52L, 0x68009800L, 0x408D9901L, 0x98004305L, 0x20006005L, 0xBCF0B003L,
		0xB4F04770L, 0x4B2AB082L, 0x02A424C0L, 0x4C29605CL, 0x01A46824L, 0x24BB0DA5L, 0x42A50064L, 0x1CA4D008L,
		0xD00542A5L, 0x42A53412L, 0x1CA4D002L, 0xD10242A5L, 0x344C4C21L, 0x4C20E001L, 0x94003450L, 0x24012501L,
		0x9E004094L, 0x43A66836L, 0x603E9F00L, 0x60314E1AL, 0x27004E1AL, 0x681E6037L, 0x43BE2720L, 0x43372710L,
		0x681E601FL, 0x601D4335L, 0x022D2580L, 0x402E685EL, 0x685DD0FCL, 0x02B626D0L, 0x2580402EL, 0x42AE01EDL,
		0x2000D002L, 0xE01143C0L, 0x2601681DL, 0x601D43B5L, 0x43B52630L, 0x9D00601DL, 0x432C682DL, 0x602C9D00L,
		0x24801C52L, 0x190901A4L, 0xD1C71E40L, 0x4770BCF6L, 0x402EC000L, 0x40290004L, 0x40268300L, 0x402EC100L,
		0xFFFFFEFFL, 0x20004770L, 0x20004770L, 0x00004770L,
};

const DWORD program_8D4_init[12] = { 0x20000944L, 0x00003B00L, 0x00400000L, 0x00000000L, 0x00000000L, 0x00000000L, 
		0x00000000L, 0x00000000L,  0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000008L };
const DWORD program_90C[12] = { 0x20004444L, 0x00003B00L, 0x00400000L, 0x00000000L, 0x00002000L, 0x00400000L, 0x00100000L,
		0x00000000L, 0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000006L };
const DWORD program_8D4[12] = { 0x20000944L, 0x00003B00L, 0x00400000L, 0x00002000L, 0x00002000L, 0x00400000L, 0x00100000L,
		0x00000000L, 0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000006L };

const DWORD erase_6B4[] = {
		0x20000724L, 0x00000800L, 0x10000000L, 0x00000000L,     0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,     0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000008L
};
const DWORD erase_6B4_1[] = {
		0x20000724L, 0x00000800L, 0x10000000L, 0x00000000L,     0x00000010L, 0x10000000L, 0x00000000L, 0x00000000L,     0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000007L
};
const DWORD erase_6EC[] = {
		0x20000F24L, 0x00000800L, 0x10000000L, 0x00000000L,     0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,     0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000002L
};
const DWORD erase_6EC_1[] = {
		0x20000F24L, 0x00000800L, 0x10000000L, 0x00000000L,     0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,     0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000009L
};

const DWORD erase_8D4_40[9][12] = {
		{ 0x20000944L, 0x00003B00L, 0x00400000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000008L },
		{ 0x20000944L, 0x00003B00L, 0x00400000L, 0x00000000L, 0x00000010L, 0x00400000L, 0x00400000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00400000L, 0x00020000L, 0x00000010L, 0x00420000L, 0x00400000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00400000L, 0x00040000L, 0x00000010L, 0x00440000L, 0x00400000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00400000L, 0x00060000L, 0x00000010L, 0x00460000L, 0x00400000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00400000L, 0x00080000L, 0x00000010L, 0x00480000L, 0x00400000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00400000L, 0x000A0000L, 0x00000010L, 0x004A0000L, 0x00400000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00400000L, 0x000C0000L, 0x00000010L, 0x004C0000L, 0x00400000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00400000L, 0x000E0000L, 0x00000010L, 0x004E0000L, 0x00400000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
};
const DWORD erase_90C_40[9][12] = {
		{ 0x20004444L, 0x00003B00L, 0x00400000L, 0x00000000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00400000L, 0x00020000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00400000L, 0x00040000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00400000L, 0x00060000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00400000L, 0x00080000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00400000L, 0x000A0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00400000L, 0x000C0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00400000L, 0x000E0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00400000L, 0x00000000L, 0x00000000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000009L },
};
const DWORD erase_8D4_50[9][12] = {
		{ 0x20000944L, 0x00003B00L, 0x00500000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000008L },
		{ 0x20000944L, 0x00003B00L, 0x00500000L, 0x00000000L, 0x00000010L, 0x00500000L, 0x00500000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00500000L, 0x00020000L, 0x00000010L, 0x00520000L, 0x00500000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00500000L, 0x00040000L, 0x00000010L, 0x00540000L, 0x00500000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00500000L, 0x00060000L, 0x00000010L, 0x00560000L, 0x00500000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00500000L, 0x00080000L, 0x00000010L, 0x00580000L, 0x00500000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00500000L, 0x000A0000L, 0x00000010L, 0x005A0000L, 0x00500000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00500000L, 0x000C0000L, 0x00000010L, 0x005C0000L, 0x00500000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00500000L, 0x000E0000L, 0x00000010L, 0x005E0000L, 0x00500000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
};
const DWORD erase_90C_50[9][12] = {
		{ 0x20004444L, 0x00003B00L, 0x00500000L, 0x00000000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00500000L, 0x00020000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00500000L, 0x00040000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00500000L, 0x00060000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00500000L, 0x00080000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00500000L, 0x000A0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00500000L, 0x000C0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00500000L, 0x000E0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00500000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000009L },
};
const DWORD erase_8D4_60[9][12] = {
		{ 0x20000944L, 0x00003B00L, 0x00600000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000008L },
		{ 0x20000944L, 0x00003B00L, 0x00600000L, 0x00000000L, 0x00000010L, 0x00600000L, 0x00600000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00600000L, 0x00020000L, 0x00000010L, 0x00620000L, 0x00600000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00600000L, 0x00040000L, 0x00000010L, 0x00640000L, 0x00600000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00600000L, 0x00060000L, 0x00000010L, 0x00660000L, 0x00600000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00600000L, 0x00080000L, 0x00000010L, 0x00680000L, 0x00600000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00600000L, 0x000A0000L, 0x00000010L, 0x006A0000L, 0x00600000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00600000L, 0x000C0000L, 0x00000010L, 0x006C0000L, 0x00600000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00600000L, 0x000E0000L, 0x00000010L, 0x006E0000L, 0x00600000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
};
const DWORD erase_90C_60[9][12] = {
		{ 0x20004444L, 0x00003B00L, 0x00600000L, 0x00000000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00600000L, 0x00020000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00600000L, 0x00040000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00600000L, 0x00060000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00600000L, 0x00080000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00600000L, 0x000A0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00600000L, 0x000C0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00600000L, 0x000E0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00600000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000009L },
};
const DWORD erase_8D4_70[9][12] = {
		{ 0x20000944L, 0x00003B00L, 0x00700000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000008L },
		{ 0x20000944L, 0x00003B00L, 0x00700000L, 0x00000000L, 0x00000010L, 0x00700000L, 0x00700000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00700000L, 0x00020000L, 0x00000010L, 0x00720000L, 0x00700000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00700000L, 0x00040000L, 0x00000010L, 0x00740000L, 0x00700000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00700000L, 0x00060000L, 0x00000010L, 0x00760000L, 0x00700000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00700000L, 0x00080000L, 0x00000010L, 0x00780000L, 0x00700000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00700000L, 0x000A0000L, 0x00000010L, 0x007A0000L, 0x00700000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00700000L, 0x000C0000L, 0x00000010L, 0x007C0000L, 0x00700000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
		{ 0x20000944L, 0x00003B00L, 0x00700000L, 0x000E0000L, 0x00000010L, 0x007E0000L, 0x00700000L, 0x00100000L,       0x200008FCL, 0x2000090CL, 0x7FFFFFFEL, 0x00000007L },
};
const DWORD erase_90C_70[9][12] = {
		{ 0x20004444L, 0x00003B00L, 0x00700000L, 0x00000000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00700000L, 0x00020000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00700000L, 0x00040000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00700000L, 0x00060000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00700000L, 0x00080000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00700000L, 0x000A0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00700000L, 0x000C0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00700000L, 0x000E0000L, 0x00020000L, 0xFFFFFFFFL, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000002L },
		{ 0x20004444L, 0x00003B00L, 0x00700000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L,       0x20000934L, 0x200008D4L, 0x7FFFFFFEL, 0x00000009L },
};


const DWORD program_d_param[65][12] = {
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00000000L, 0x00000800L, 0x00000000L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00000800L, 0x00000800L, 0x00000000L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00001000L, 0x00000800L, 0x00000000L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00001800L, 0x00000800L, 0x00000000L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00002000L, 0x00000800L, 0x00000001L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00002800L, 0x00000800L, 0x00000001L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00003000L, 0x00000800L, 0x00000001L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00003800L, 0x00000800L, 0x00000001L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00004000L, 0x00000800L, 0x00000002L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00004800L, 0x00000800L, 0x00000002L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00005000L, 0x00000800L, 0x00000002L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00005800L, 0x00000800L, 0x00000002L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00006000L, 0x00000800L, 0x00000003L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00006800L, 0x00000800L, 0x00000003L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00007000L, 0x00000800L, 0x00000003L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00007800L, 0x00000800L, 0x00000003L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00008000L, 0x00000800L, 0x00000004L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00008800L, 0x00000800L, 0x00000004L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00009000L, 0x00000800L, 0x00000004L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00009800L, 0x00000800L, 0x00000004L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x0000A000L, 0x00000800L, 0x00000005L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0000A800L, 0x00000800L, 0x00000005L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0000B000L, 0x00000800L, 0x00000005L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0000B800L, 0x00000800L, 0x00000005L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x0000C000L, 0x00000800L, 0x00000006L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0000C800L, 0x00000800L, 0x00000006L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0000D000L, 0x00000800L, 0x00000006L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0000D800L, 0x00000800L, 0x00000006L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x0000E000L, 0x00000800L, 0x00000007L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0000E800L, 0x00000800L, 0x00000007L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0000F000L, 0x00000800L, 0x00000007L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0000F800L, 0x00000800L, 0x00000007L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00010000L, 0x00000800L, 0x00000008L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00010800L, 0x00000800L, 0x00000008L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00011000L, 0x00000800L, 0x00000008L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00011800L, 0x00000800L, 0x00000008L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00012000L, 0x00000800L, 0x00000009L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00012800L, 0x00000800L, 0x00000009L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00013000L, 0x00000800L, 0x00000009L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00013800L, 0x00000800L, 0x00000009L, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00014000L, 0x00000800L, 0x0000000AL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00014800L, 0x00000800L, 0x0000000AL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00015000L, 0x00000800L, 0x0000000AL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00015800L, 0x00000800L, 0x0000000AL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00016000L, 0x00000800L, 0x0000000BL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00016800L, 0x00000800L, 0x0000000BL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00017000L, 0x00000800L, 0x0000000BL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00017800L, 0x00000800L, 0x0000000BL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00018000L, 0x00000800L, 0x0000000CL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00018800L, 0x00000800L, 0x0000000CL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00019000L, 0x00000800L, 0x0000000CL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x00019800L, 0x00000800L, 0x0000000CL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001A000L, 0x00000800L, 0x0000000DL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001A800L, 0x00000800L, 0x0000000DL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001B000L, 0x00000800L, 0x0000000DL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001B800L, 0x00000800L, 0x0000000DL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001C000L, 0x00000800L, 0x0000000EL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001C800L, 0x00000800L, 0x0000000EL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001D000L, 0x00000800L, 0x0000000EL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001D800L, 0x00000800L, 0x0000000EL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001E000L, 0x00000800L, 0x0000000FL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001E800L, 0x00000800L, 0x0000000FL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001F000L, 0x00000800L, 0x0000000FL, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000724L, 0x00000800L, 0x10000000L, 0x0001F800L, 0x00000800L, 0x0000000FL, 0x00000000L, 0x00000000L, 0x200006DCL, 0x200006ECL, 0x7FFFFFFEL, 0x00000006L },
		{ 0x20000F24L, 0x00000800L, 0x10000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x00000000L, 0x20000714L, 0x200006B4L, 0x7FFFFFFEL, 0x00000009L }
};

/****************************************************************\
Notes: User define yoursel subroutine here

\****************************************************************/
void SetEasyFPGAPinDefine(void);
void GetDeviceParameterInfo(void);
void Get1PageDataChkBlank(void);
void Swd_Bus_Reset(void);
uShort Receive_ACK(void);
void Check_Parity_Bit(uShort Parity_Lo, uShort Parity_Hi);
void Read_32Bit_Data(void);
void Send_8Bit_DataCmd(uChar TempData);
void Send_32Bit_Data(uShort DatLo, uShort DatHi);
void Send_One_Clk(void);
void Write_AHB(uInt32 addr, uInt32 data);
void Write_AHB_S(uInt32 addr, uInt32 data);
void Read_AHB(uInt32 addr);
void Read_AHB_S(uInt32 addr);
void SWD_Write(uShort cmd, uInt32 data);
void SWD_Read(uShort cmd);
void JTAG_to_SWD(void);
uChar ChkAllSckRead_32Bit_Data(uShort DatLo, uShort DatHi);
uChar SWD_Verify(uChar cmd, uInt32 data, uInt32 mask);
void InitIO(void);
void SWDIDcode();
uChar SWDIDcode_Verify(uInt32 data);
uChar SWDInit(void);
//
void connectDAP(void);
void connecting();
void swd_writeAPResource(DWORD targetAddr, DWORD wData);
BYTE swd_makeRequestHeader(const unsigned int bAPnDP, const unsigned int bRnW, const BYTE a32);
DWORD swd_readAPResource(DWORD targetAddr);
void prepareFun();
void runFun();
void swd_corDbgWrReg(DWORD regNum, DWORD val);
void writeAPResource(DWORD address, DWORD d);
void program_init(DWORD start_addr, DWORD* pParam90C, DWORD* pParam8D4);
void download(DWORD addr, const DWORD* pData, int byte_size);
void program_dinit();
void program_16K(DWORD start_addr, DWORD* pData, DWORD* pParam90C, DWORD* pParam8D4);
void program_end_block(DWORD address, DWORD* pParam);
void program_dFlashk2K(const DWORD* pPar, DWORD* pData);
void erase_block(DWORD base_addr);
void erase_page(const DWORD* erase_par_8D4, const DWORD* erase_par_90C, int rows);
void erase_d();
void program_dFlash_end();


/********************* Power On ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar PowerOnFun(void)
{
#if OnlyUse1TimePowerCtrl
	if (Use1TimePowerOnMode)
		return SUCCESS; /* if already in power on status then return back */
	else
		Use1TimePowerOnMode = 1;
#endif

	Set_VccDAC(); /* setting DAC array to rise VCC/VPP/VPE/VPS/VPIO volts level */
	Set_VihDAC();
	Set_VppDAC();
	Set_VpioDAC();
	SetEasyFPGAPinDefine(); /* only for use easy fpga file!! other need mask!! */

	Delay_1ms(50); /* delay for stable volte */

	PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask); /* re-enable all pass socket */
	Delay_10nsX(3);
	InitIO();
	GangRiseAllPassSocketVccIccPin(); /* open VCC/VPP/VPE/VPS/VPIO pin driving */
	GangRiseAllPassSocketVppIppPin();/* open VCC/VPP/VPE/VPS/VPIO pin driving */
	Delay_1ms(100);
	GetDeviceParameterInfo();
	connectDAP();
	return SUCCESS;
}

/********************* Power On ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar PowerOffFun(void)
{
#if OnlyUse1TimePowerCtrl
	if (Use1TimePowerOnMode && SysCmdBuf(0))
		return SUCCESS; /* if command buffer have other process then donot power off */
	else
		Use1TimePowerOnMode = 0;
#endif

	PtrData(OneDataCmd) = TTL_LO;
	PtrData(SetSCKPinCmd) = TTL_LO;
	PtrData(SetRSTPinCmd) = TTL_LO;
	Delay_1ms(10);

	Fall_VpioIpioAll();
	Fall_VppIppAll();
	Fall_VihAll();
	Fall_VccAll();
	Delay_1ms(50); /* delay for stable volte */
	return SUCCESS;
}

/****************** Read Subroutine **************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar ReadFun(void)
{
	uInt32 dev_addr, i;

	dev_addr = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();

	if (IfSpecialBitBlk())
		return SUCCESS;
	else
	{
		while (1)
		{
			if (dev_addr < 0x003F4000L)
			{
				swd_readAPResource(dev_addr + FALSH_APP_BASE);
			}
			else if (dev_addr < 0x00400000L)
			{
				OutDataLo = 0xFFFF;
				OutDataHi = 0xFFFF;
			}
			else
			{
				swd_readAPResource(dev_addr - 0x00400000L + 0x10000000L);
			}
			ReadDataBuf[ReadBufCnt / 2 + 0] = OutDataLo;
			ReadDataBuf[ReadBufCnt / 2 + 1] = OutDataHi;
			ReadBufCnt += 4;
			DevCurrBlkLen.all -= 4;
			if (SaveChkBlkLenEnd()) /* checking block length end */
			{
				return SUCCESS;
			}
			dev_addr += 4;
		}
	}
	return SUCCESS;

}

/****************** Program Subroutine **********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar ProgramFun(void)
{
	uInt32 dev_addr,USBData, tmp_addr, d_address;
	uChar buf_flag = 0;
	uChar sckStatus;
	uChar blank_flag = 0;

	WORD size, i, j,k;
	DWORD p8D4[12];
	DWORD p90C[12];
	int par_index;
	int null_flag = 0;
	DWORD pMem[4096];  // 16K buffer
	size = 16384;

	if (IfSpecialBitBlk())
		return SUCCESS;
	else
	{
		Use1TimePowerOnMode = 0;
		PowerOffFun();
		Delay_1Second(1);
		PowerOnFun();
		connectDAP();
		/* above 16M bit=2S handshake 1 time to PC, handshake period time MUST <=4S */
		HandleEvent_EP0();
		Delay_1ms(10); /* NEED delay 10ms here */

		SWDIDcode_Verify(0x6BA02477L);

		dev_addr = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();
		while (1)
		{
			if ((dev_addr == 0x0L) || (dev_addr == 0x100000L) || (dev_addr == 0x200000L) || (dev_addr == 0x300000L))
				program_init(dev_addr + FALSH_APP_BASE, p90C, p8D4);
			else if (dev_addr == 0x400000L)
			{
				program_dinit();
				par_index = 0;
			}

			for (i = 0; i < size;)
			{
				null_flag = 0;
				for (j = 0; j < (16384 / 4); j++)
				{
					USBData = CFCardReadWordData();                // Receive data form CF
					USBData |= (DWORD)CFCardReadWordData() << 16;  // Receive data form CF
					pMem[j] = USBData;
					DramBufCnt -= 4;
					DevCurrBlkLen.all -= 4;
					BlockEndFlag = GetChkBlkLenEnd(); /* checking block length end */
					if (USBData != 0xFFFFFFFFL)
						null_flag |= 1;
				}

				if (dev_addr < 0x400000L)
				{
					if (null_flag == 1)
					{
						Delay_1ms(100);
						p90C[3] = (dev_addr + FALSH_APP_BASE) - p90C[2];
						p8D4[3] = p90C[3] + 0x00002000L;
						program_16K(dev_addr + FALSH_APP_BASE, pMem, p90C, p8D4);

						if ((p8D4[3] == 0x000FE000L) || (((p8D4[2] == 0x00700000L) && (p8D4[3] == 0x000F2000L))))
						{
							p90C[3] = p90C[4] = p90C[5] = p90C[6] = p90C[7] = 0;
							p90C[11] = 9;
							program_end_block(0x2000090CL, p90C);
						}
					}
				}

				else
				{
					for (tmp_addr = dev_addr; tmp_addr < (dev_addr + 16384L); tmp_addr += 0x00000800L)
					{
						d_address = program_d_param[par_index][3] + FALSH_APP_BASE;
						if ((d_address == tmp_addr))
						{
							program_dFlashk2K(program_d_param[par_index], &pMem[(int)(tmp_addr - dev_addr) / 4]);
							par_index++;
						}
					}
				}
				i += 16384;
			}
			dev_addr += size;

			if (BlockEndFlag) /* checking block length end */
			{
				program_dFlash_end();
				return SUCCESS;
			}

		}

	}
}

/****************** Verify Subroutine ************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar VerifyFun(void)
{
	uInt32 dev_addr, i, USBData;
	uChar sckStatus;



	if (IfSpecialBitBlk())
		return SUCCESS;
	else
	{
		dev_addr = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();
		while (1)
		{
			USBData = CFCardReadWordData();                // Receive data form CF
			USBData |= (DWORD)CFCardReadWordData() << 16;  // Receive data form CF
			if (dev_addr < FALSH_APP_BASE)
			{
				if (dev_addr >= (FALSH_APP_BASE - 0xC000))
				{ // Code Flash 3 just have 192K byte
				}
				else
				{
					Read_AHB(dev_addr + FALSH_APP_BASE);
					sckStatus = SWD_Verify(DPACC_READBUFF_RD, USBData, 0xFFFFFFFFL);
					if (GangCheckIfAllSocketError(sckStatus))
					{ /* if all socket error then cancle function */
						SysStatusCode = E_AutoIDFail; /* sending error type code */
						return FAIL;
					}
				}

			}
			else
			{
				Read_AHB(dev_addr + (DATA_APP_BASE - FALSH_APP_BASE));
				sckStatus = SWD_Verify(DPACC_READBUFF_RD, USBData, 0xFFFFFFFFL);
				if (GangCheckIfAllSocketError(sckStatus))
				{ /* if all socket error then cancle function */
					SysStatusCode = E_AutoIDFail; /* sending error type code */
					return FAIL;
				}
			}

			DramBufCnt -= 4;
			DevCurrBlkLen.all -= 4;
			if (GetChkBlkLenEnd()) /* checking block length end */
			{
				return SUCCESS;
			}
			dev_addr += 4;
			if (!(DevCurrBlkLen.all & 0xFFFF))
			{ /* above 16M bit=2S handshake 1 time to PC, handshake period time MUST <=4S */
				HandleEvent_EP0();
				Delay_1ms(10); /* NEED delay 10ms here */
			}
		}
	}
}

/******************* Erase Subroutine ***********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar EraseFun(void)
{
	uInt32 cs,i;
	uChar sckStatus;

	if (IfFirstTimeEntry())
	{

		erase_block(0x00400000L);
		erase_block(0x00500000L);
		erase_block(0x00600000L);
		erase_block(0x00700000L);
		erase_d();

		return SUCCESS;
	}

}

/****************** Blank Check ****************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar BlankCheckFun(void)
{
	uInt32 dev_addr, i, USBData;
	uChar sckStatus;

	if (IfSpecialBitBlk())
		return SUCCESS;
	else
	{
		dev_addr = (GetBlkChipStartAddrHi() << 16) + GetBlkChipStartAddrLo();
		while (1)
		{

			if (dev_addr < FALSH_APP_BASE)
			{
				if (dev_addr >= (FALSH_APP_BASE - 0xC000))
				{
				}
				else
				{
					Read_AHB(dev_addr + FALSH_APP_BASE);
					sckStatus = SWD_Verify(DPACC_READBUFF_RD, 0xFFFFFFFFL, 0xFFFFFFFFL);
					if (GangCheckIfAllSocketError(sckStatus))
					{ /* if all socket error then cancle function */
						SysStatusCode = E_BlankFail; /* sending error type code */
						return FAIL; /* if all sck check error then fail */
					}
				}
			}
			else
			{
				Read_AHB(dev_addr + (DATA_APP_BASE - FALSH_APP_BASE));
				sckStatus = SWD_Verify(DPACC_READBUFF_RD, 0xFFFFFFFFL, 0xFFFFFFFFL);
				if (GangCheckIfAllSocketError(sckStatus))
				{ /* if all socket error then cancle function */
					SysStatusCode = E_BlankFail; /* sending error type code */
					return FAIL; /* if all sck check error then fail */
				}
			}
			DramBufCnt -= 4;
			DevCurrBlkLen.all -= 4;
			if (!DevCurrBlkLen.all) /* checking block length end */
			{
				return SUCCESS;
			}
			dev_addr += 4;
			if (!(DevCurrBlkLen.all & 0xFFFF))
			{ /* above 16M bit=2S handshake 1 time to PC, handshake period time MUST <=4S */
				HandleEvent_EP0();
				Delay_1ms(10); /* NEED delay 10ms here */
			}

		}
	}
}

/******************* ID Check ******************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar IDCheckFun(void)
{

	uChar sckStatus = 0x00;
	DWORD g_DeviceID;

	sckStatus |= GangErrSckMask;
	g_DeviceID = ((uInt32)DeviceIDLo()) | (((uInt32)DeviceIDHi()) << 16);

	//JTAG_to_SWD();
	sckStatus |= SWDIDcode_Verify(g_DeviceID);
	Delay_1ms(5);
	if (GangCheckIfAllSocketError(sckStatus&0xF))
	{ /* if all socket error then cancle function */
		SysStatusCode = E_AutoIDFail; /* sending error type code */
		return FAIL;
	}

	return SUCCESS;
}

/******************* Secure Function *************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar SecureFun(void)
{
	uInt32 k;
	uChar sckStatus;

	if (GangCheckIfAllSocketError(sckStatus))
	{ /* if all socket error then cancle function */
		SysStatusCode = E_SecureFail; /* sending error type code */
		return FAIL;
	}
	return SUCCESS;
}

/******************* CheckSumFun***************************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar CheckSumFun(void)
{
	uShort TempData;
	uInt32 ChecksumAll;
	ChecksumAll = (StdChecksumHi << 16) + StdChecksumLo; /* get hi-lo 32 bit checksum data */
	while (1)
	{
		TempData = CFCardReadWordData();
		ChecksumAll += LSB(TempData);
		DramBufCnt--;
		DevCurrBlkLen.all--;
		if (GetChkBlkLenEnd()) /* checking block length end */
		{
			StdChecksumLo = LSW(ChecksumAll);
			StdChecksumHi = MSW(ChecksumAll);
			return SUCCESS;
		}

		ChecksumAll += MSB(TempData);
		DramBufCnt--;
		DevCurrBlkLen.all--;
		if (GetChkBlkLenEnd()) /* checking block length end */
		{
			StdChecksumLo = LSW(ChecksumAll);
			StdChecksumHi = MSW(ChecksumAll);
			return SUCCESS;
		}
	}
	return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*******************************************************************\
Each Pin Define: Bit15-12 for Pin Default Direction;
						Bit11-8 for Pin Default Status When Disable Sck (Not NG Sck)
			   Bit7-0 for A-Sck Pin Number:1--48, FPGA will offset to B,C,D
\*******************************************************************/
void SetEasyFPGAPinDefine(void)
{
	uChar TempPmpIndex;

	TempPmpIndex = DrvElement_Table[Project->fwAlgoICIndex][PmpIdxOffset];

	VCCPinNum = (U8)DmmIOPortDefine[TempPmpIndex][0];  /* get each pin number */
	GNDPinNum = (U8)DmmIOPortDefine[TempPmpIndex][1];
	SWDPinNum = (U8)DmmIOPortDefine[TempPmpIndex][2];
	SCKPinNum = (U8)DmmIOPortDefine[TempPmpIndex][3];
	RSTPinNum = (U8)DmmIOPortDefine[TempPmpIndex][4];
	TMSPinNum = (U8)DmmIOPortDefine[TempPmpIndex][5];
	TDIPinNum = (U8)DmmIOPortDefine[TempPmpIndex][6];
	JCOMPinNum = (U8)DmmIOPortDefine[TempPmpIndex][7];

	/*** set Easy FPGA each pin number ***/
	PtrData(SetVCCPinNum) = SetPinInPut | SetPinDisLo | VCCPinNum; /* set pin default DIR + Disable satus +Pin number */
	Delay_10nsX(3);
	PtrData(SetGNDPinNum) = SetPinInPut | SetPinDisLo | GNDPinNum; /* set pin default DIR + Disable satus +Pin number */
	Delay_10nsX(3);

	PtrData(SetSWDPinNum) = SetPinOutPut | SetPinDisHi | SWDPinNum; /* set pin default DIR + Disable satus +Pin number */
	Delay_10nsX(3);
	PtrData(SetSCKPinNum) = SetPinOutPut | SetPinDisHi | SCKPinNum;
	Delay_10nsX(3);
	PtrData(SetRSTPinNum) = SetPinOutPut | SetPinDisHi | RSTPinNum; /* some device CANNOT reset when in disable status, or will exit program mode! */
	Delay_10nsX(3);

	PtrData(SetTMSPinNum) = SetPinInPut | SetPinDisLo | TMSPinNum; /* No use pin NEED set to input dir!or NOT define here */
	Delay_10nsX(3);
	PtrData(SetTDIPinNum) = SetPinInPut | SetPinDisLo | TDIPinNum; /* No use pin NEED set to input dir!or NOT define here */
	Delay_10nsX(3);
	PtrData(SetJCOMPinNum) = SetPinInPut | SetPinDisLo | JCOMPinNum; /* No use pin NEED set to input dir!or NOT define here */
	Delay_10nsX(3);
}
/************************************************************\
		Firmware or System Using Subroutine
		CAN NOT Modify!!!
\************************************************************/
uChar GetDrvElementLen(void)
{
	DevDrvVersion = DevDRV_Version; /* get driver version */
	GangType = SystemGang;
	return (uChar)(sizeof(DrvElement_Table) / 5);
}

/**********************************************************\
		Subrountine Function Define Here
\**********************************************************/
void GetDeviceParameterInfo(void)
{
	ProgPageSize = ChipPageSizeTable[fwAlgoIndex];
}

/**********************************************************\
		If Blank Page Data then return Success
\**********************************************************/
void Get1PageDataChkBlank(void)
{
	uShort BufferCnt;

	BlankPageFlag = 0xFFFFFFFF;
	for (BufferCnt = 0; BufferCnt < ProgPageSize; BufferCnt++)
	{ /* divide by 2 for byte data width */
		PageDataBuf[BufferCnt] = CFCardReadWordData(); /* Get low word */
		PageDataBuf[BufferCnt] |= (CFCardReadWordData() << 16); /* Get hi word */
		BlankPageFlag &= PageDataBuf[BufferCnt];

	}
	if (BlankPageFlag != 0xFFFFFFFF)
		BlankPageFlag = 0x0000;
}

////////////////////////////////////////////////////////////////////////////
uShort Receive_ACK(void)
{
	uChar TempData, BitCnt;

	PtrData(GDevChkDataLoInc0) = 0xFF; /* change to read status before read data */
	Delay_10nsX(3);
	Send_One_Clk();

	PtrData(GClrPageChkStatus) = 0x0000;
	Delay_10nsX(3);
	PtrData(GSetAllMaskDataLo) = 0xE0;  /* send byte checking mask data to FPGA */
	Delay_10nsX(3);

	TempData = 0x00;
	for (BitCnt = 3; BitCnt > 0; BitCnt--) /* 3 ACK bits */
	{ //! LSW bit shift 'in' first
		PtrData(GDevChkDataLoInc0) = 0xFF;
		Send_One_Clk();
	}
	TempData = PtrData(GChkAllSckStatus); //! LSW bit shift 'out' first
	return (TempData);
}

////////////////////////////////////////////////////////////////////////////
void Swd_Bus_Reset(void)
{
	uShort TempCnt;

	PtrData(OneDataCmd) = 0xFF;
	PtrData(SetSCKPinCmd) = 0x00;
	Delay_10nsX(10);
	for (TempCnt = 0; TempCnt < 60; TempCnt++)
	{
		PtrData(SetSCKPinCmd) = 0xFF;
		Delay_10nsX(10);
		PtrData(SetSCKPinCmd) = 0x00;
		Delay_10nsX(10);
	}
}

////////////////////////////////////////////////////////////////////////////
void Read_32Bit_Data(void)
{
	uShort BitCnt;

	BitCnt = PtrData(OneDataCmd); /* change to read status before read data */
	Delay_10nsX(10);
	OutDataLo = 0x0000;
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{
		OutDataLo >>= 1; // !LSW bit shift 'OUT' first
		if (PtrData(OneDataCmd) & 0x01)
			OutDataLo |= 0x8000;
		Send_One_Clk();
	}

	OutDataHi = 0x0000;
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{
		OutDataHi >>= 1; // !LSW bit shift 'OUT' first
		if (PtrData(OneDataCmd) & 0x01)
			OutDataHi |= 0x8000;
		Send_One_Clk();
	}
	Send_One_Clk();	/* Get parity bit */
	Send_One_Clk();	/* Get Trm bit */
}

////////////////////////////////////////////////////////////////////////////
uChar ChkAllSckRead_32Bit_Data(uShort DatLo, uShort DatHi)
{ /* send MSB firstly */
	uShort BitCnt;
	uChar sckStatusLo, sckStatusHi;
	uChar sckStatus;

	PtrData(GDevChkDataLoInc0) = 0xFF; /* change to read status before read data */
	Delay_10nsX(10);

	PtrData(GSetAllMaskDataLo) = 0xFFFF;  /* send byte checking mask data to FPGA */
	Delay_10nsX(10);
	PtrData(GClrPageChkStatus) = 0x0000;
	Delay_10nsX(10);
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{ /* read LSB firstly to FPGA then checking to fee-back data */
		PtrData(GDevChkDataLoInc0) = DatLo;
		Send_One_Clk();
	}
	sckStatusLo = PtrData(GChkAllSckStatus); /* get fpga checking all socket result */


	PtrData(GClrPageChkStatus) = 0x0000;
	Delay_10nsX(10);
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{ /* read LSB firstly to FPGA then checking to fee-back data */
		PtrData(GDevChkDataLoInc0) = DatHi;
		Send_One_Clk();
	}
	Send_One_Clk();
	Send_One_Clk();
	sckStatusHi = PtrData(GChkAllSckStatus); /* get fpga checking all socket result */
	sckStatus = sckStatusLo | sckStatusHi;
	return sckStatus;
}

///////////////////////////////////////////////////////////
void Send_32Bit_Data(uShort DatLo, uShort DatHi)
{
	uShort BitCnt;

	Check_Parity_Bit(DatLo, DatHi);

	PtrData(GDevChkDataLoInc0) = 0xFF; /* change to read status before read data */
	Delay_10nsX(10);
	Send_One_Clk(); /* Get Trm */

	for (BitCnt = 16; BitCnt > 0; BitCnt--) /* send lo 16 bit */
	{ //! LSW bit shift 'in' first
		if (DatLo & 0x0001)
			PtrData(OneDataCmd) = 0xFF; //! a bit shift 'in'
		else
			PtrData(OneDataCmd) = 0x00; //! a bit shift 'in'
		DatLo >>= 1;
		Send_One_Clk();
	}

	for (BitCnt = 16; BitCnt > 0; BitCnt--) /* send hi 16 bit */
	{ //! LSW bit shift 'in' first
		if (DatHi & 0x0001)
			PtrData(OneDataCmd) = 0xFF; //! a bit shift 'in'
		else
			PtrData(OneDataCmd) = 0x00; //! a bit shift 'in'
		DatHi >>= 1;
		Send_One_Clk();
	}

	PtrData(OneDataCmd) = Parity_Data; /* send parity bit */
	Send_One_Clk();
}

///////////////////////////////////////////////////////////
void Send_8Bit_DataCmd(uChar TempData)
{
	uChar BitCnt;

	for (BitCnt = 8; BitCnt > 0; BitCnt--)
	{ //! LSW bit shift 'in' first
		if (TempData & 0x01)
			PtrData(OneDataCmd) = 0xFF; //! a bit shift 'in'
		else
			PtrData(OneDataCmd) = 0x00; //! a bit shift 'in'
		TempData >>= 1;
		Send_One_Clk();
	}
}

////////////////////////////////////////////////////////////////////////////////////
void Send_One_Clk(void)
{
//	Delay_10nsX(3);
	PtrData(SetSCKPinCmd) = 1;			//! TCK -> low
//	Delay_10nsX(3);
	PtrData(SetSCKPinCmd) = 0;			//! TCK -> high
//	Delay_10nsX(3);
}

////////////////////////////////////////////////////////////////////////////////////
void Check_Parity_Bit(uShort Parity_Lo, uShort Parity_Hi)
{ /* OOB parity check */
	uShort BitCnt;

	Parity_Data = 0;
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{
		Parity_Data += (Parity_Lo & 0x0001);
		Parity_Lo >>= 1;
	}
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{
		Parity_Data += (Parity_Hi & 0x0001);
		Parity_Hi >>= 1;
	}
	if (Parity_Data & 0x0001)
		Parity_Data = 0xFF;
	else
		Parity_Data = 0x00;
}

////////////////////////////////////////////////////////////////////////////
void Write_AHB(uInt32 addr, uInt32 data)
{
	SWD_Write(APACC_ADDR_WR, addr);
	SWD_Write(APACC_DATA_WR, data);
}

////////////////////////////////////////////////////////////////////////////
void Write_AHB_S(uInt32 addr, uInt32 data)
{
	SWD_Write(DPACC_DP_SELECT_WR, 0x00000000);

	Send_8Bit_DataCmd(0x8B); /* write TAR */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Send_32Bit_Data(LSW(addr), MSW(addr));
	Send_8Bit_DataCmd(0x00);

	SWD_Write(DPACC_DP_SELECT_WR, 0x00000000);

	Send_8Bit_DataCmd(0xBB); /* write DRW */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Send_32Bit_Data(LSW(data), MSW(data));
	Send_8Bit_DataCmd(0x00);
	Delay_1us(5);
}

////////////////////////////////////////////////////////////////////////////
void SWD_Write(uShort cmd, uInt32 data)
{
	Send_8Bit_DataCmd(cmd); /* write CTRL/STAT reg */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Send_32Bit_Data(LSW(data), MSW(data));
	Send_8Bit_DataCmd(0x00);
	//Delay_1us(5);

}

////////////////////////////////////////////////////////////////////////////
void SWD_Read(uShort cmd)
{
	Send_8Bit_DataCmd(cmd);
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Read_32Bit_Data();
	Send_8Bit_DataCmd(0x00);
	SWD_Data = (((uInt32)OutDataHi) << 16) | ((uInt32)OutDataLo);
	//Delay_1us(5);

}

////////////////////////////////////////////////////////////////////////////
void Read_AHB(uInt32 addr)
{
	SWD_Write(APACC_ADDR_WR, addr);
	SWD_Read(APACC_DATA_RD);
}

////////////////////////////////////////////////////////////////////////////
uChar ChkAllSckRead_AHB(uInt32 addr, uShort DatLo, uShort DatHi)
{
	uShort SckStatus;

	SWD_Write(DPACC_DP_SELECT_WR, 0x00000000);

	Send_8Bit_DataCmd(0x8B); /* write TAR */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Send_32Bit_Data(LSW(addr), MSW(addr));
	Send_8Bit_DataCmd(0x00);

	SWD_Write(DPACC_DP_SELECT_WR, 0x00000000);

	Send_8Bit_DataCmd(0x9F); /* read DRW dummy */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	ChkAllSckRead_32Bit_Data(DatLo, DatHi); /* dummy read data */
	Send_8Bit_DataCmd(0x00);

	Send_8Bit_DataCmd(0xBD); /* read rdbuf */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	SckStatus = ChkAllSckRead_32Bit_Data(DatLo, DatHi); /* realy read true data */
	Send_8Bit_DataCmd(0x00);

	return SckStatus;
}

////////////////////////////////////////////////////////////////////////////
void Read_AHB_S(uInt32 addr)
{
	Send_8Bit_DataCmd(0x8B); /* write TAR */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Send_32Bit_Data(LSW(addr), MSW(addr));
	Send_8Bit_DataCmd(0x00);

	Send_8Bit_DataCmd(0x9F); /* read DRW dummy */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Read_32Bit_Data(); /* dummy read data */
	Send_8Bit_DataCmd(0x00);

	Send_8Bit_DataCmd(0xBD); /* read rdbuf */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Read_32Bit_Data(); /* realy read true data */
	Send_8Bit_DataCmd(0x00);
}

////////////////////////////////////////////////////////////////////////////
uChar ChkAllSckRead_AHB_S(uInt32 addr, uShort DatLo, uShort DatHi)
{
	uShort SckStatus;

	Send_8Bit_DataCmd(0x8B); /* write TAR */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	Send_32Bit_Data(LSW(addr), MSW(addr));
	Send_8Bit_DataCmd(0x00);

	Send_8Bit_DataCmd(0x9F); /* read DRW dummy */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	ChkAllSckRead_32Bit_Data(DatLo, DatHi); /* dummy read data */
	Send_8Bit_DataCmd(0x00);

	Send_8Bit_DataCmd(0xBD); /* read rdbuf */
	Receive_ACK(); /* Get Trm & 3 bits ACK */
	SckStatus = ChkAllSckRead_32Bit_Data(DatLo, DatHi); /* realy read true data */
	Send_8Bit_DataCmd(0x00);

	return SckStatus;
}

////////////////////////////////////////////////////////////////////////////
void JTAG_to_SWD(void)
{
	/****** change JTAG to SWD mode *****/
	Swd_Bus_Reset();
	Send_8Bit_DataCmd(0x9E);
	Send_8Bit_DataCmd(0xE7);
	Send_8Bit_DataCmd(0xAA);
	Send_8Bit_DataCmd(0x00);
	Swd_Bus_Reset();
	Send_8Bit_DataCmd(0x00);
}

////////////////////////////////////////////////////////////////////////////
/*
* function：check data with enable bits
* data:32bits data
* mask:32bits mask
* return:FAIL/SUCESS&&SOCKET status
*/
uChar ChkAllSckRead_32Bit_Data_Mask(uInt32 data, uInt32  mask)
{
	uShort mask_hi;
	uShort mask_lo;
	uShort data_lo;
	uShort data_hi;
	uShort BitCnt;
	uChar SckStatusLo, SckStatusHi;
	uChar sckStatus;
	data_lo = data;
	data_hi = data >> 16;
	mask_lo = mask;
	mask_hi = mask >> 16;


	PtrData(GDevChkDataLoInc0) = 0xFF; /* change to read status before read data */
	Delay_10nsX(10);
	//lsb 
	PtrData(GSetAllMaskDataLo) = mask_lo;  /* send byte checking mask data to FPGA */
	Delay_10nsX(10);
	PtrData(GClrPageChkStatus) = 0x0000;
	Delay_10nsX(10);
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{ /* read LSB firstly to FPGA then checking to fee-back data */
		PtrData(GDevChkDataLoInc0) = data_lo;
		Send_One_Clk();
	}
	SckStatusLo = PtrData(GChkAllSckStatus); /* get fpga checking all socket result */

	//hsb
	PtrData(GSetAllMaskDataLo) = mask_hi;  /* send byte checking mask data to FPGA */
	Delay_10nsX(10);
	PtrData(GClrPageChkStatus) = 0x0000;
	Delay_10nsX(10);
	for (BitCnt = 16; BitCnt > 0; BitCnt--)
	{ /* read LSB firstly to FPGA then checking to fee-back data */
		PtrData(GDevChkDataLoInc0) = data_hi;
		Send_One_Clk();
	}
	Send_One_Clk();
	Send_One_Clk();
	SckStatusHi = PtrData(GChkAllSckStatus); /* get fpga checking all socket result */

	sckStatus = (SckStatusHi | SckStatusLo);

	return sckStatus;
}

////////////////////////////////////////////////////////////////////////////
uChar SWD_Verify(uChar cmd, uInt32 data, uInt32 mask)
{
	uChar sckStatus = 0;

	Send_8Bit_DataCmd(cmd);
	Receive_ACK();
	sckStatus = ChkAllSckRead_32Bit_Data_Mask(data, mask);
	Send_8Bit_DataCmd(0x00);
	//Delay_1us(5);
	return sckStatus;
}

////////////////////////////////////////////////////////////////////////////
void SWDIDcode()
{
	Send_8Bit_DataCmd(0xA5);
	Receive_ACK();
	Read_32Bit_Data();
	Send_8Bit_DataCmd(0x00);
}

////////////////////////////////////////////////////////////////////////////
void InitIO(void)
{
	PtrData(OneDataCmd) = TTL_HI;
	PtrData(SetSCKPinCmd) = TTL_HI;
	PtrData(SetRSTPinCmd) = TTL_HI;
}

////////////////////////////////////////////////////////////////////////////
uChar SWDIDcode_Verify(uInt32 data)
{
	uChar sckStatus;

	Send_8Bit_DataCmd(0xA5);
	Receive_ACK();
	sckStatus = ChkAllSckRead_32Bit_Data_Mask(data, 0xFFFFFFFF);
	Send_8Bit_DataCmd(0x00);

	return sckStatus;
}

////////////////////////////////////////////////////////////////////////////
uChar SWDInit(void)
{

}

////////////////////////////////////////////////////////////////////////////
void connectDAP(void)
{
	uInt32 i;
	uInt32 addr;

	//Delay_1Second(1);
	connecting();

	SWD_Write(DPACC_SELECT_WR, 0x070000F0L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x300000F0L);

	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x23000002L);
	for (i = 0; i < 38; i += 2)
		swd_writeAPResource(init_ap4_data[i], init_ap4_data[i + 1]);

	connecting();
	SWD_Write(DPACC_ABORT_WR, 0x1EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x23000002L);

	swd_writeAPResource(DHCSR, 0xA05F0001L);
	swd_writeAPResource(BP_CTRL, 0x10000083L);
	swd_writeAPResource(DEMCR, 0x01000000L);
	swd_writeAPResource(DWT_CTRL, 0x40000001L);

	swd_readAPResource(CPACR);
	swd_writeAPResource(CPACR, 0x00F00000L);
	swd_readAPResource(CPACR);
	swd_writeAPResource(CPACR, 0);

	swd_writeAPResource(CSSELR, 0x00000001L);
	swd_readAPResource(0xE000ED80L);
	swd_writeAPResource(CSSELR, 0x00000000L);
	swd_readAPResource(0xE000ED80L);

	for (addr = 0x20000000L; addr <= 0x2000000CL; addr += 4L)
	{
		swd_writeAPResource(addr, 0xDEADBEEFL);
	}

	swd_writeAPResource(DHCSR, 0xA05F0003L);
	prepareFun();
	swd_corDbgWrReg(0, 0x20000010L);
	swd_corDbgWrReg(1, 0x00000FFEL);
	swd_corDbgWrReg(2, 0xDEADBEEFL);
	swd_corDbgWrReg(15, 0x20000000L);
	runFun();
	swd_writeAPResource(FP_COMP0, 0);

	prepareFun();
	swd_corDbgWrReg(0, 0x20400000L);
	swd_corDbgWrReg(1, 0x00000800L);
	swd_corDbgWrReg(2, 0xDEADBEEFL);
	swd_corDbgWrReg(15, 0x20000000L);
	runFun();
	swd_writeAPResource(FP_COMP0, 0);

	connecting();
	SWD_Write(DPACC_SELECT_WR, 0x07000080L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x300000F0L);

	SWD_Write(DPACC_SELECT_WR, 0x07000090L);
	SWD_Write(APACC_CTRLSTAT_WR, 0);
	Delay_1us(50);

	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x23000002L);
	swd_writeAPResource(DHCSR, 0xA05F0003L);
	swd_writeAPResource(DEMCR, 0);
	swd_writeAPResource(FP_CTRL, 0x00000003L);
	swd_writeAPResource(DEMCR, 0x01000000L);

	swd_writeAPResource(DWT_CTRL, 0x40000001L);

	SWD_Verify(DPACC_IDCODE_RD, DPIDR,0xFFFFFFFF);


}

void connecting()
{
	JTAG_to_SWD();
	JTAG_to_SWD();
	JTAG_to_SWD();

	SWD_Verify(DPACC_IDCODE_RD, DPIDR, 0xFFFFFFFF);


	SWD_Write(DPACC_ABORT_WR, 0x1EL);
	SWD_Write(DPACC_SELECT_WR, 0);
	SWD_Write(DPACC_CTRLSTAT_WR, 0x50000000L);
	Delay_1ms(1);
}

void swd_writeAPResource(DWORD targetAddr, DWORD wData)
{
	//BYTE request;
	//request = swd_makeRequestHeader(1, 0, (AP_AHB_TAR >> 2) & 0x03);
	//SWD_Write(request, targetAddr);
	//request = swd_makeRequestHeader(1, 0, (AP_AHB_DRW >> 2) & 0x03);
	//SWD_Write(request, wData);

	SWD_Write(0x8B, targetAddr);
	SWD_Write(0xBB, wData);
}

BYTE swd_makeRequestHeader(const unsigned int bAPnDP, const unsigned int bRnW, const BYTE a32)
{
	SWD_REQUEST rHeader;

	rHeader.PACKET.APnDP = bAPnDP;
	rHeader.PACKET.RnW = bRnW;
	rHeader.PACKET.A = a32;

	return rHeader.rawByte;
}

DWORD swd_readAPResource(DWORD targetAddr)
{
	//BYTE request = swd_makeRequestHeader(1, 0, (AP_AHB_TAR >> 2) & 0x03);
	//SWD_Write(request, targetAddr);
	SWD_Write(0x8B, targetAddr);


	//request = swd_makeRequestHeader(1, 1, (AP_AHB_DRW >> 2) & 0x03);
	//SWD_Read(request);
	SWD_Read(0x9F);


	//request = swd_makeRequestHeader(0, 1, DP_REG_ADDR32_RDBUFF);
	//SWD_Read(request);
	SWD_Read(0xBD);

	return SWD_Data;
}

void prepareFun()
{
	swd_writeAPResource(0x20000000L, 0x2202E8E0L);
	swd_writeAPResource(0x20000004L, 0xD1FB1E49L);
	swd_writeAPResource(0x20000008L, 0xE7FEE7FEL);

	swd_writeAPResource(FP_CTRL, 0x00000003L);
	swd_writeAPResource(FP_COMP0, 0x20000008L | 1L);
	swd_writeAPResource(FP_COMP1, 0);
	swd_writeAPResource(FP_COMP2, 0);
	swd_writeAPResource(FP_COMP3, 0);
	swd_writeAPResource(FP_COMP4, 0);
	swd_writeAPResource(FP_COMP5, 0);
	swd_writeAPResource(FP_COMP6, 0);
	swd_writeAPResource(FP_COMP7, 0);
}

void runFun()
{
	swd_writeAPResource(DFSR, 0x0000001FL);
	swd_writeAPResource(DHCSR, 0xA05F0001L);

	Delay_1ms(1);
	Delay_1ms(1);

}

void swd_corDbgWrReg(DWORD regNum, DWORD val)
{
	swd_writeAPResource(DCRDR, val);
	swd_writeAPResource(DCRSR, (DWORD)((regNum & DCRSR_REGSEL_MASK) | (1L << DCRSR_REGWnR_SHIFT)));
}

void program_init(DWORD start_addr, DWORD* pParam90C, DWORD* pParam8D4)
{
	int i;
	DWORD addr;
	DWORD pParam8D4Init[12];

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);
	download(0x20000000L, erase_program_app, sizeof(erase_program_app));

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);

	for (i = 0; i < 8; i++)
		swd_writeAPResource(0x200008B0L + (DWORD)(i * 4), 0L);

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x03000002L);
	writeAPResource(DHCSR, 0xA05F0003L);
	Delay_1us(200);

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);
	for (i = 0; i < 73; i++)
		swd_writeAPResource(0x200007B0L + (DWORD)(i * 4), 0L);
	swd_writeAPResource(0x200007B0L + 64L * 4L, 0x200008D4L);

	addr = 0x200008FCL;
	swd_writeAPResource(addr, 0x7FFFFFFEL);
	swd_writeAPResource(addr + 4L, 0L);
	swd_writeAPResource(addr + 8L, 0L);
	swd_writeAPResource(addr + 12L, 0L);

	addr = 0x20000934L;
	swd_writeAPResource(addr, 0x7FFFFFFEL);
	swd_writeAPResource(addr + 4L, 0L);
	swd_writeAPResource(addr + 8L, 0L);
	swd_writeAPResource(addr + 12L, 0L);

	for (i = 0; i < 12; i++)
	{
		pParam8D4Init[i] = program_8D4_init[i];
		pParam90C[i] = program_90C[i];
		pParam8D4[i] = program_8D4[i];
	}
	pParam8D4Init[2] = start_addr & 0x00F00000L;
	pParam90C[2] = pParam90C[5] = start_addr & 0x00F00000L;
	pParam8D4[2] = pParam8D4[5] = start_addr & 0x00F00000L;
	if (start_addr == 0x00700000L)
		pParam90C[6] = pParam8D4[6] = 0x000F4000L;
	download(0x200008D4L, pParam8D4Init, 4 * 12);

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x03000012L);

	swd_corDbgWrReg(15, 0x20000002L);
	swd_corDbgWrReg(16, 0x01000000L);
	swd_corDbgWrReg(17, 0x200008B0L);

	writeAPResource(DFSR, 0x0000001FL);
	writeAPResource(DHCSR, 0xA05F000BL);
	Delay_1us(200);
	writeAPResource(DHCSR, 0xA05F0009L);
	Delay_1us(500);
}

void download(DWORD addr,const DWORD* pData, int byte_size)
{
	int i;

	for (i = 0; i < (byte_size / 4); i++)
	{
		swd_writeAPResource(addr, pData[i]);
		addr += 4;
	}

	/* above 16M bit=2S handshake 1 time to PC, handshake period time MUST <=4S */
	HandleEvent_EP0();
	Delay_1ms(10); /* NEED delay 10ms here */

}

void writeAPResource(DWORD address, DWORD d)
{
	SWD_Write(DPACC_ABORT_WR, 0x1EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x23000002L);

	swd_writeAPResource(address, d);
}

void program_dinit()
{
	int i;
	DWORD addr;

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);
	download(0x20000000L, erase_dflash, sizeof(erase_dflash));

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);

	for (i = 0; i < 8; i++)
		swd_writeAPResource(0x20000690L + (DWORD)(i * 4), 0L);

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x03000002L);
	writeAPResource(DHCSR, 0xA05F0003L);

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);
	for (i = 0; i < 73; i++)
		swd_writeAPResource(0x20000590L + (DWORD)(i * 4), 0L);
	swd_writeAPResource(0x20000590L + (DWORD)(64 * 4), 0x200006B4L);

	addr = 0x200006DCL;
	swd_writeAPResource(addr, 0x7FFFFFFEL);
	swd_writeAPResource(addr + 4L, 0L);
	swd_writeAPResource(addr + 8L, 0L);
	swd_writeAPResource(addr + 12L, 0L);

	addr = 0x20000714L;
	swd_writeAPResource(addr, 0x7FFFFFFEL);
	swd_writeAPResource(addr + 4L, 0L);
	swd_writeAPResource(addr + 8L, 0L);
	swd_writeAPResource(addr + 12L, 0L);

	download(0x200006B4L, erase_6B4, sizeof(erase_6B4));

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x03000012L);

	swd_corDbgWrReg(15, 0x20000002L);
	swd_corDbgWrReg(16, 0x01000000L);
	swd_corDbgWrReg(17, 0x20000690L);

	writeAPResource(DFSR, 0x0000001FL);
	writeAPResource(DHCSR, 0xA05F000BL);
	Delay_1us(200);
	writeAPResource(DHCSR, 0xA05F0009L);
	Delay_1ms(100);
}


void program_16K(DWORD start_addr, DWORD* pData, DWORD* pParam90C, DWORD* pParam8D4)
{
	int i;
	DWORD addr;

	addr = pParam90C[0];
	for (i = 0; i < (8 * 1024); i += 4)
		swd_writeAPResource(addr + (DWORD)i, *pData++);
	download(pParam8D4[9], pParam90C, 4 * 12);

	Delay_1ms(100);

	addr = pParam8D4[0];
	for (i = 0; i < (8 * 1024); i += 4)
		swd_writeAPResource(addr + (DWORD)i, *pData++);

	download(pParam90C[9], pParam8D4, 4 * 12);

	//Delay_1ms(100);

}

void program_end_block(DWORD address, DWORD* pParam)
{
	download(address, pParam, 4 * 12);  // download parameters to 0x2000090C. pParam8D4[9] == 0x2000090C
	Delay_1ms(200);

	writeAPResource(DHCSR, 0xA05F000BL);
	Delay_1us(200);
	writeAPResource(DHCSR, 0xA05F0003L);

	Delay_1ms(400);
}

void program_dFlashk2K(const DWORD* pPar, DWORD* pData)
{
	DWORD addr;

	if (pData != 0)
		download(pPar[0], pData, 2 * 1024);

	if (pPar[9] == 0x200006B4L)
	{
		addr = 0x200006ECL;
	}
	else
	{
		addr = 0x200006B4L;
	}
	download(addr, pPar, 12 * 4);

	Delay_1ms(100);
}

void erase_block(DWORD base_addr)
{
	int i, rows;
	DWORD addr;
	const DWORD* p8D4;
	const DWORD* p90C;

	if (base_addr == 0x00400000L)
	{
		p8D4 = erase_8D4_40[0];
		p90C = erase_90C_40[0];
		rows = 9;
	}
	else if (base_addr == 0x00500000L)
	{
		p8D4 = erase_8D4_50[0];
		p90C = erase_90C_50[0];
		rows = 9;
	}
	else if (base_addr == 0x00600000L)
	{
		p8D4 = erase_8D4_60[0];
		p90C = erase_90C_60[0];
		rows = 9;
	}
	else if (base_addr == 0x00700000L)
	{
		p8D4 = erase_8D4_70[0];
		p90C = erase_90C_70[0];
		rows = 9;
	}

	SWD_Write(DPACC_ABORT_WR, 0x1EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);
	download(0x20000000L, erase_program_app, sizeof(erase_program_app));

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);
	for (i = 0; i < 8; i++)
		swd_writeAPResource(0x200008B0L + (DWORD)(i * 4), 0L);

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x03000002L);

	writeAPResource(DHCSR, 0xA05F0003L);

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);
	for (i = 0; i < 73; i++)
		swd_writeAPResource(0x200007B0L + (DWORD)(i * 4), 0L);
	swd_writeAPResource(0x200007B0L + (DWORD)(64 * 4), 0x200008D4L);

	addr = 0x200008FCL;
	swd_writeAPResource(addr, 0x7FFFFFFEL);
	swd_writeAPResource(addr + 4L, 0L);
	swd_writeAPResource(addr + 8L, 0L);
	swd_writeAPResource(addr + 12L, 0L);

	addr = 0x20000934L;
	swd_writeAPResource(addr, 0x7FFFFFFEL);
	swd_writeAPResource(addr + 4L, 0L);
	swd_writeAPResource(addr + 8L, 0L);
	swd_writeAPResource(addr + 12L, 0L);

	download(0x200008D4L, p8D4, 4 * 12);
	p8D4 += 12;

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x03000012L);

	swd_corDbgWrReg(15, 0x20000002L);
	swd_corDbgWrReg(16, 0x01000000L);
	swd_corDbgWrReg(17, 0x200008B0L);

	writeAPResource(DFSR, 0x0000001FL);
	writeAPResource(DHCSR, 0xA05F000BL);
	writeAPResource(DHCSR, 0xA05F0009L);

	Delay_1ms(1);
	download(0x2000090CL, p90C, 12 * 4);
	p90C += 12;

	Delay_1ms(1);
	erase_page(p8D4, p90C, rows - 1);   // init has been passed before

	writeAPResource(DHCSR, 0xA05F000BL);
	Delay_1us(100);
	writeAPResource(DHCSR, 0xA05F0003L);
	Delay_1ms(100);
}

void erase_page(const DWORD* erase_par_8D4, const DWORD* erase_par_90C, int rows)
{
	int i;

	for (i = 0; i < rows; i++)
	{
		SWD_Write(DPACC_ABORT_WR, 0x1EL);
		SWD_Write(DPACC_SELECT_WR, 0x04000000L);
		SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);

		download(0x200008D4L, erase_par_8D4, 12 * 4);
		erase_par_8D4 += 12;

		// Delay 1s. Actually is 569ms
		Delay_1Second(1);
		Delay_1ms(500);

		SWD_Write(DPACC_ABORT_WR, 0x1EL);
		SWD_Write(DPACC_SELECT_WR, 0x04000000L);
		SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);

		Delay_1ms(50);
		download(0x2000090CL, erase_par_90C, 12 * 4);
		Delay_1ms(1);
		erase_par_90C += 12;
		Delay_1ms(1);

	}
	Delay_1ms(500);
}

void erase_d()
{
	int i;
	DWORD addr;

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);

	download(0x20000000L, erase_dflash, sizeof(erase_dflash));

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);
	for (i = 0; i < 8; i++)
		swd_writeAPResource(0x20000690L + (DWORD)(i * 4), 0L);
	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);
	for (i = 0; i < 73; i++)
		swd_writeAPResource(0x20000590L + (DWORD)(i * 4), 0L);
	swd_writeAPResource(0x20000590L + (DWORD)(64 * 4), 0x200006B4L);

	addr = 0x200006DCL;
	swd_writeAPResource(addr, 0x7FFFFFFEL);
	swd_writeAPResource(addr + 4L, 0L);
	swd_writeAPResource(addr + 8L, 0L);
	swd_writeAPResource(addr + 12L, 0L);

	addr = 0x20000714L;
	swd_writeAPResource(addr, 0x7FFFFFFEL);
	swd_writeAPResource(addr + 4L, 0L);
	swd_writeAPResource(addr + 8L, 0L);
	swd_writeAPResource(addr + 12L, 0L);

	download(0x200006B4L, erase_6B4, sizeof(erase_6B4));

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x03000012L);

	swd_corDbgWrReg(15, 0x20000002L);
	swd_corDbgWrReg(16, 0x01000000L);
	swd_corDbgWrReg(17, 0x20000690L);

	writeAPResource(DFSR, 0x0000001FL);
	writeAPResource(DHCSR, 0xA05F000BL);
	writeAPResource(DHCSR, 0xA05F0009L);

	Delay_1us(200);
	download(0x200006ECL, erase_6EC, sizeof(erase_6EC));
	Delay_1us(200);

	//erase_6B4[4] = 0x00000010L;
	//erase_6B4[5] = 0x10000000L;
	//erase_6B4[11] = 7L;
	download(0x200006B4L, erase_6B4_1, sizeof(erase_6B4_1));

	// Delay 1s. Actually is 25 ms
	Delay_1ms(50);

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x1F000012L);


	Delay_1us(200);
	//erase_6EC[4] = 0x00000000L;
	//erase_6EC[5] = 0x00000000L;
	//erase_6EC[11] = 9L;
	download(0x200006ECL, erase_6EC_1, sizeof(erase_6EC_1));
	Delay_1us(200);

	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x03000002L);

	writeAPResource(DHCSR, 0xA05F000BL);
	writeAPResource(DHCSR, 0xA05F0003L);

	Delay_1ms(1);

}

void program_dFlash_end()
{
	SWD_Write(DPACC_ABORT_WR, 0x0000001EL);
	SWD_Write(DPACC_SELECT_WR, 0x04000000L);
	SWD_Write(APACC_CTRLSTAT_WR, 0x03000002L);

	writeAPResource(DHCSR, 0xA05F000BL);
	Delay_1us(200);
	writeAPResource(DHCSR, 0xA05F0003L);

	Delay_1Second(1);
}
void DevSNProgAndVerify(void)
{
}