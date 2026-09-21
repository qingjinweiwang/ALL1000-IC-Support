
/******************************************************************************
 ***                 Status Code Constant Declaration			    ***
 ******************************************************************************/

#ifndef StatusCode_H_
#define StatusCode_H_


#define	S_MinStatusCode					0x0300

/* 0x0200~0x020F reserve for system */


/************ System & Firmware Fail/Error Message Macro Define *************/
#define	S_DriverInitializing				0x0310
#define	S_FWSecureChecking			0x0311
#define	S_FWSKBSetting					0x0312
#define	S_SelfTesting					0x0314
#define	S_DACCalibrating				0x0315
#define	S_DevicePinConnectChecking		0x0316
#define	S_DeviceSerialCodeSetting		0x0317
#define	S_DeviceInsertionChecking		0x0318
#define	S_FPGAConfiguring				0x0319
#define	S_FunctionalTesting				0x031A
#define	S_CFCardTesting					0x031B
#define	S_CFCardDetecting				0x031C
#define	S_AutoModeDeviceInputWaiting	0x031D


/************** For File Up/Download...****************************/
#define	S_SystemInfoUploading			0x0340 
#define	S_SkbInfoUploading				0x0341 
#define	S_DataUploading					0x0342 
#define	S_SpecialBitUploading			0x0343 
#define	S_DevBlockUploading			0x0344 
#define	S_DevSpecUploading				0x0345 
#define	S_DataBroadcasting				0x0346 
#define	S_DataDownloading				0x0347 
#define	S_DevSpecDownloading			0x0348 
#define	S_SpecialBitDownloading			0x0349 


/********************* For Device Operating *******************************/
#define	S_DeviceReading				0x0380
#define	S_DeviceBlankChecking			0x0381
#define	S_DeviceIllegalChecking			0x0382
#define	S_DeviceErasing					0x0383
#define	S_DeviceProgramming			0x0384
#define	S_DeviceVerifying				0x0385
#define	S_DeviceHiVerifying				0x0386
#define	S_DeviceLoVerifying				0x0387
#define	S_DeviceProtecting				0x0388
#define	S_DeviceUnprotecting			0x0389
#define	S_DeviceSecuring				0x038A
#define	S_DeviceIDChecking				0x038B
#define	S_HiddenROMIllegaling			0x038C
#define	S_HiddenROMErasing				0x038D
#define	S_HiddenROMProgramming		0x038E
#define	S_HiddenROMVerifying			0x038F
#define	S_HiddenROMBlanking			0x0390
#define	S_ChecksumCalculating			0x0391
#define	S_ChecksumComparing			0x0392




#define	S_MaxStatusCode				0x05FF


#endif

