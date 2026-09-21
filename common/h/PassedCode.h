


/******************************************************************************
 ***                  Firmware System Pass Code Constant Declaration			    ***

 ******************************************************************************/

#ifndef PassedCode_H_
#define PassedCode_H_


#define	P_MinPassedCode				0x0600

/* 0x0400~0x040F reserve for system */


/************ System & Firmware Fail/Error Message Macro Define *************/
#define	P_DriverInitialPass				0x0610 
#define	P_FWSecureCheckPass			0x0611
#define	P_FWSKBSetPass					0x0612
#define	P_SelfTestPass					0x0613
#define	P_DACCalibratePass				0x0614
#define	P_DevicePinConnectCheckPass	0x0615
#define	P_DeviceSerialCodeSetPass		0x0616
#define	P_FPGAConfigurePass			0x0617
#define	P_FunctionalTestPass			0x0618
#define	P_CFCardTestPass				0x0619
#define	P_CFCardDetectPass				0x061A
#define	P_DeviceControlModePass		0x061B
#define	P_RePwrOnToLoadNewPrjPass	0x061C


/************** For File Up/Download...****************************/
#define	P_SystemInfoUploadPass			0x0640 
#define	P_SkbInfoUploadPass			0x0641 
#define	P_DataUploadPass				0x0642 
#define	P_SpecialBitUploadPass			0x0643 
#define	P_DevBlockUploadPass			0x0644 
#define	P_DevSpecUploadPass			0x0645 
#define	P_DataBroadcastPass			0x0647 
#define	P_DataDownloadPass				0x0648 
#define	P_DevSpecDownloadPass			0x0649 
#define	P_SpecialBitDownloadPass		0x064A 


/********************* For Device Operating ***********************/
#define	P_DeviceInitialPass				0x0680 
#define	P_ReadPass						0x0681 
#define	P_BlankPass						0x0682 
#define	P_IllegalPass					0x0683 
#define	P_ErasePass						0x0684 
#define	P_ProgramPass					0x0685 
#define	P_VerifyPass					0x0686 
#define	P_HiVccVerifyPass				0x0687 
#define	P_LoVccVerifyPass				0x0688 
#define	P_SecurePass					0x0689 
#define	P_ProtectPass					0x068A 
#define	P_UnprotectPass					0x068B 
#define	P_AutoIDPass					0x068C 
#define	P_HiddenROMIllegalPass			0x068D 
#define	P_HiddenROMErasePass			0x068E 
#define	P_HiddenROMProgramPass		0x068F 
#define	P_HiddenROMVerifyPass			0x0690
#define	P_HiddenROMBlankPass			0x0691 
#define	P_BadBlockOverLimitPass		0x0692 
#define	P_ChecksumCalculatePass		0x0693
#define	P_ChecksumComparePass		0x0694


/****************************************************/
#define	P_KeyFunctionRequest			0x0880 /* if in online mode PC check system per 100ms */


#define	P_MaxPassedCode				0x08FF


#endif

