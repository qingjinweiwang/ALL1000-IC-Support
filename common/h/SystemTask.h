


#ifndef SystemTask_H_
#define SystemTask_H_

#include "Type.h"


/*********************************************************************\
	For system subrounte function setting here
\*********************************************************************/

/*-------operation stages:------*/
#define	OPERATION_0			0
#define	OPERATION_1			1
#define	OPERATION_2			2
#define	OPERATION_3			3
#define	OPERATION_4			4
#define	OPERATION_5			5
#define	OPERATION_6			6
#define	OPERATION_7			7
#define	OPERATION_8			8
#define	OPERATION_9			9
#define	OPERATION_10			10
#define	OPERATION_11			11
#define	OPERATION_12			12
#define	OPERATION_13			13
#define	OPERATION_14			14
#define	OPERATION_15			15
#define	OPERATION_16			16
#define	OPERATION_17			17
#define	OPERATION_18			18
#define	OPERATION_19			19
#define	OPERATION_20			20
#define	OPERATION_21			21
#define	OPERATION_22			22
#define	OPERATION_23			23
#define	OPERATION_24			24
#define	OPERATION_25			25

#define	OPERATION_TEMP		30
#define	OPERATION_X			31
#define	OPERATION_Y			32
#define	OPERATION_Z			33
#define	OPERATION_END			34



#define	NULL_TASK					0x00
#define	ALL_TASKS_START			0x01
/************** system task **********************/
#define	SECURE_SETTING_TASK		(NULL_TASK+0x01)
#define	CHECK_SECURECODE			1
#define	UPDATE_SECURECODE		2
/************************************************/
#define	SKB_SETTING_TASK			(NULL_TASK+0x02)
#define	CHECK_SKBCODE				1
#define	UPDATE_SKBCODE			2
#define	GET_SKBINFOR				3
/************************************************/
#define	DIP_PIN_CNT_CHK_TASK		(NULL_TASK+0x03) /* for DIP48 PCC */
#define	PIN_CONNECT_CHECK_TASK	(NULL_TASK+0x04)
#define	AUTO_RUN_MODE			1

#define	SERIALCODE_SETTING_TASK	(NULL_TASK+0x05)
#define	FUNCTION_TEST_TASK		(NULL_TASK+0x06)
#define	UPLOAD_TASK				(NULL_TASK+0x07)
#define	DOWNLOAD_TASK			(NULL_TASK+0x08)
#define	CFCard_Data				0
#define	SDRAM_Data					1

#define	FPGA_CFG_TASK				(NULL_TASK+0x09)
#define	WRITE_FPGA_CFG			0
#define	READ_FPGA_CFG				1

/******** algorithm task *******************/
#define	POWER_ON_TASK			(NULL_TASK+0x20)
#define	POWER_OFF_TASK			(NULL_TASK+0x21)
#define	READ_TASK					(NULL_TASK+0x22)
#define	PROGRAM_TASK				(NULL_TASK+0x23)
#define	VERIFY_TASK				(NULL_TASK+0x24)
#define	ERASE_TASK					(NULL_TASK+0x25)
#define	BLANK_CHECK_TASK			(NULL_TASK+0x26)
#define	ILLEGAL_CHECK_TASK		(NULL_TASK+0x27)
#define	ID_CHECK_TASK				(NULL_TASK+0x28)
#define	SECURE_TASK				(NULL_TASK+0x29)
#define	CHECKSUM_TASK				(NULL_TASK+0x2A)
#define	PROTECT_TASK				(NULL_TASK+0x2B)
#define	UNPROTECT_TASK			(NULL_TASK+0x2C)

/*-----------------------------------*/
#define	BEEP_TASK					(NULL_TASK+0x40)
#define	NORMAL_BEEP				0x0000
#define	ERROR_BEEP					0x0001


/***************************************************/
#define	 ALL_TASKS_END				0xFF




/*********************************************************************\
			System Task List

\*********************************************************************/
extern uChar PinConnectCheck_Task(void) ;
extern uChar DIPPinConnectChk_Task(void) ;
extern uChar UpLoad_Task(void) ;
extern uChar DownLoad_Task(void) ;
extern uChar FPGA_CFG_Task(void) ;

/******** algorithm function list **********/
extern uChar AlgoPowerOn_Task(void) ;
extern uChar AlgoPowerOff_Task(void) ;
extern uChar AlgoRead_Task(void) ;
extern uChar AlgoProgram_Task(void) ;
extern uChar AlgoVerify_Task(void) ;
extern uChar AlgoErase_Task(void) ;
extern uChar AlgoBlankCheck_Task(void) ;
extern uChar AlgoIllegalCheck_Task(void) ;
extern uChar AlgoIDCheck_Task(void) ;
extern uChar AlgoSecure_Task(void) ;
extern uChar AlgoChecksum_Task(void) ;
extern uChar AlgoProtect_Task(void) ;
extern uChar AlgoUnProtect_Task(void) ;


/************* below for security checking *****************/
extern uChar CheckingMBSecurity(uChar UpdateFlag);
extern uChar CheckingCSKBSecurity(uChar ChkSck, uChar UpdateFlag) ;
extern uChar CheckingMSKBSecurity(uChar UpdateFlag) ;
extern uChar CheckingAllSKBSecurity(uChar UpdateFlag) ;
extern uChar CheckingDIPSKBSecurity(uChar UpdateFlag) ;
extern uChar CheckFWGangID(void) ;
extern void CheckNewDeviceInput(void) ;


#endif

