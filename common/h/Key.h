


#ifndef Key_H_
#define Key_H_

#include "Type.h"
#include "Initial.h"


/********** Global variables *******************/
extern uChar key ;
extern uChar TempKey ;


/***********************************************************************/
#define SKBChkOKFlag		*((uChar *)(CtrlFlagBufAddr)+0) /* 0 = skb chk error or no skb, 1 = chk pass */
#define GoRunInCPUFlag		*((uChar *)(CtrlFlagBufAddr)+1) /* 1 = download cpu code ok, then jump to new code */
#define SKBInfoUpFlag		*((uChar *)(CtrlFlagBufAddr)+2)
#define SKBInfoDnFlag		*((uChar *)(CtrlFlagBufAddr)+3)

#define OnLineFlag			*((uChar *)(CtrlFlagBufAddr)+4) /* 0 = offline from PC, 1 = on line with PC */
#define GangIDStatus			*((uChar *)(CtrlFlagBufAddr)+5) /* define transfer/receive ID number */
//////////////////////////////////
#define	GangMaster1		0x00 /* master ID for Auto Machine */
/////////////////////////////////
#define	GangSlave1			0x00
#define	GangSlave2			0x01
#define	GangSlave3			0x02
#define	GangSlave4			0x03
//////////////////////////////////
#define	GangMaster4		0x03 /* master ID for GP16 */
/////////////////////////////////
#define	GangSlave5			0x04
#define	GangSlave6			0x05
#define	GangSlave7			0x06
#define	GangSlave8			0x07
#define	GangSlave9			0x08
#define	GangSlave10		0x09
#define	GangSlave11		0x0A
#define	GangSlave12		0x0B
#define	GangSlave13		0x0C
#define	GangSlave14		0x0D
#define	GangSlave15		0x0E
#define	GangSlave16		0x0F



#define BootDriverFlag			*((uChar *)(CtrlFlagBufAddr)+6) /* 0 = boot pass, 1 = boot fail */
#define	BootPass		0x00
#define	BootFail			0x01


/************************************************************************************/
#define	GangChkSckStatus		*((uChar *)(CtrlFlagBufAddr)+7) /*  1 = socket check fail, 0 = socket check pass */
#define	GangErrSckMask			*((uChar *)(CtrlFlagBufAddr)+8) /* 1 = check error or disable socket, 0 = check pass or enable socket */

#define DeviceInformDnFlag		*((uChar *)(CtrlFlagBufAddr)+9) /* 0 = not get inform, 1 = get inform ok */
#define SpecialBitDnFlag			*((uChar *)(CtrlFlagBufAddr)+10) /* 0 = not get special bit, 1 = get ok */
#define DriverVersionUpFlag		*((uChar *)(CtrlFlagBufAddr)+11)
#define BlockInformUpFlag		*((uChar *)(CtrlFlagBufAddr)+12)
#define SystemInformUpFlag		*((uChar *)(CtrlFlagBufAddr)+13)
#define SKBInputFlag				*((uChar *)(CtrlFlagBufAddr)+14) /* 0 = no skb, 1 = socket board input ok */
#define CFCInputFlag				*((uChar *)(CtrlFlagBufAddr)+15) /* 0 = no cf card, 1 = cf card input ok */

#define ExitProductModeKey		*((uChar *)(CtrlFlagBufAddr)+16) /* 0 = no key cmd, 1 = press exit key in product mode */
#define GangModeFlag			*((uChar *)(CtrlFlagBufAddr)+17) /* 0 = Single Mode, 1 = Gang Mode */
#define ProductModeFlag			*((uChar *)(CtrlFlagBufAddr)+18) /* 0 = Handle Control Mode, 1 = Product Auto Control Mode */

#define CntEnFlag				*((uChar *)(CtrlFlagBufAddr)+19) /* 0 = Disable/1 = Enable Counter in OffLine Mode */
#define PCCEnFlag				*((uChar *)(CtrlFlagBufAddr)+20) /* 0 = Disable/1 = Enable PCC Function */
#define IDChkEnFlag				*((uChar *)(CtrlFlagBufAddr)+21) /* 0 = Disable/1 = Enable ID Check Function */
#define BeepEnFlag				*((uChar *)(CtrlFlagBufAddr)+22) /* 0 = Disable/1 = Enable Beep Sound */

#define DevInputStatus			*((uChar *)(CtrlFlagBufAddr)+23)
#define NoInputDev			0
#define JustInputDev			1
#define JustOutputDev		2
#define OKInputDev			3

#define EP0IntFlag				*((uChar *)(CtrlFlagBufAddr)+24)
#define EP1IntFlag				*((uChar *)(CtrlFlagBufAddr)+25)
#define EP2IntFlag				*((uChar *)(CtrlFlagBufAddr)+26)
#define EP3IntFlag				*((uChar *)(CtrlFlagBufAddr)+27)
#define EP4IntFlag				*((uChar *)(CtrlFlagBufAddr)+28)
#define EP5IntFlag				*((uChar *)(CtrlFlagBufAddr)+29)
#define EP6IntFlag				*((uChar *)(CtrlFlagBufAddr)+30)
#define EP7IntFlag				*((uChar *)(CtrlFlagBufAddr)+31)
#define EP8IntFlag				*((uChar *)(CtrlFlagBufAddr)+32)

#define GoRunInBIOSFlag			*((uChar *)(CtrlFlagBufAddr)+33) /* 1 = go back to Bios */
#define FstChkMBBIOSFlag		*((uChar *)(CtrlFlagBufAddr)+34) /* 1 = write first time to mb bios info */
#define CSKBCntOverFlag			*((uChar *)(CtrlFlagBufAddr)+35) /* 1 = current sck counter overlimet! */

#define ProgNeedCntFlag			*((uChar *)(CtrlFlagBufAddr)+36) /* 1 = has program function, need update counter */

/***** below for Gang 64 System Using: LL16, ML16, MH16, HH16  *********************/
#define	Gang8ErrSckMask		*((uChar *)(CtrlFlagBufAddr)+37) /* 1 = check error or disable socket, 0 = check pass or enable socket */
#define	Gang16ErrSckMask		*((uChar *)(CtrlFlagBufAddr)+38) /* 1 = check error or disable socket, 0 = check pass or enable socket */
#define	Gang24ErrSckMask		*((uChar *)(CtrlFlagBufAddr)+39) /* 1 = check error or disable socket, 0 = check pass or enable socket */
#define	Gang32ErrSckMask		*((uChar *)(CtrlFlagBufAddr)+40) /* 1 = check error or disable socket, 0 = check pass or enable socket */
#define	Gang40ErrSckMask		*((uChar *)(CtrlFlagBufAddr)+41) /* 1 = check error or disable socket, 0 = check pass or enable socket */
#define	Gang48ErrSckMask		*((uChar *)(CtrlFlagBufAddr)+42) /* 1 = check error or disable socket, 0 = check pass or enable socket */
#define	Gang56ErrSckMask		*((uChar *)(CtrlFlagBufAddr)+43) /* 1 = check error or disable socket, 0 = check pass or enable socket */
/*****************************************************************************/

#define	DIPSKBChkOKFlag		*((uChar *)(CtrlFlagBufAddr)+44) /* 0 = dipskb chk error or no dipskb, 1 = chk pass */

#define	Use1TimePowerOnMode	*((uChar *)(CtrlFlagBufAddr)+45) /* 1 = use 1 time power on mode, 0 = not use */
#define	ISPGangProgType		*((uChar *)(CtrlFlagBufAddr)+46) /* for ISPx1 or x4 type */

#define	SNProgFlag				*((uChar *)(CtrlFlagBufAddr)+47) /* 1 = enable program SN */



/******* Extern Function define ************/
extern void CheckSKBAlwaysForProtect(uChar UpdateFlag) ;
extern void AddSKBCntCmdForProtect(void) ;
extern void Read_KeyStatus(void);
extern void KeyCmd_Processing(void);


#endif

