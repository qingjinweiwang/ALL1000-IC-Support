


#ifndef ErrorCode_H_
#define ErrorCode_H_


/******************************************************************************
 ***                  Firmware System Error Code Constant Declaration			    ***

 ******************************************************************************/
#define	System_Standby					0xFFFF


#define	E_MinFailedCode					0x0010
/************ System & Firmware Fail/Error Message Macro Define *************/
#define	E_CommandError				0x0010 
#define	E_DriverInitialFail				0x0011
#define	E_FWSecureCheckFail			0x0012
#define	E_FWSKBSetFail					0x0013
#define	E_SelfTestFail					0x0014
#define	E_DACCalibrateFail				0x0015
#define	E_NoDevInSocket				0x0016 
#define	E_BackwardDevInserted			0x0017 
#define	E_DevicePinConnectCheckFail		0x0018
#define	E_DeviceSerialCodeSetFail		0x0019
#define	E_FPGAConfigureFail				0x001A
#define	E_FunctionalTestFail				0x001B
#define	E_CFCardTestFail				0x001C
#define	E_CFCardDetectFail				0x001D
#define	E_SKBCheckFail					0x001E
#define	E_SKBDetectFail					0x001F
#define	E_CFCardNoProjectFail			0x0020


/************** For File Up/Download...****************************/
#define	E_SystemInfoUploadFail			0x0040 
#define	E_SkbInfoUploadFail				0x0041 
#define	E_DataUploadFail				0x0042 
#define	E_SpecialBitUploadFail			0x0043 
#define	E_DevBlockUploadFail			0x0044 
#define	E_DevSpecUploadFail			0x0045 
#define	E_DataBroadcastFail				0x0046 
#define	E_DataDownloadFail				0x0047 
#define	E_DevSpecDownloadFail			0x0048 
#define	E_SpecialBitDownloadFail			0x0049 



/********************* For Device Operating *******************************/
/***** all gang error message donot include address & data inform *******/
#define	E_DeviceInitialFail				0x0080 
#define	E_ReadFail						0x0081 
#define	E_BlankFail						0x0082 
#define	E_IllegalFail						0x0083 
#define	E_EraseFail						0x0084 
#define	E_ProgramFail					0x0085 
#define	E_VerifyFail						0x0086 
#define	E_HiVccVerifyFail				0x0087 
#define	E_LoVccVerifyFail				0x0088 
#define	E_SecureFail					0x0089 
#define	E_ProtectFail					0x008A 
#define	E_UnprotectFail					0x008B 
#define	E_AutoIDFail					0x008C 
#define	E_IncorrectManufacture			0x008D 
#define	E_IncorrectDevice				0x008E
#define	E_HiddenROMIllegalFail			0x008F /* also display fail address & data info */
#define	E_ExtendBlockIllegalFail			0x008F /* also display fail address & data info */
#define	E_HiddenROMEraseFail			0x0090 /* also display fail address & data info */
#define	E_ExtendBlockEraseFail			0x0090 /* also display fail address & data info */
#define	E_HiddenROMProgramFail		0x009F /* also display fail address & data info */
#define	E_ExtendBlockProgramFail		0x009F /* also display fail address & data info */
#define	E_HiddenROMVerifyFail			0x0091 /* also display fail address & data info */
#define	E_ExtendBlockVerifyFail			0x0091 /* also display fail address & data info */
#define	E_HiddenROMBlankFail			0x0092  /* also display fail address & data info */
#define	E_ExtendBlockBlankFail			0x0092  /* also display fail address & data info */
#define	E_ChecksumCalculateFail			0x0093
#define	E_ChecksumCompareFail			0x0094
#define	E_PowerOnFail					0x0095
#define	E_PowerOffFail					0x0096
#define	E_ChkBlkPtyFail					0x0097
#define	E_ProtectStatusFail				0x0098 /* only fail msg, no address & data info */
#define	E_UserPrtRegIllegalFail			0x0099 /* only fail msg, no address & data info */
#define	E_UserPrtRegProgramFail		0x009A /* only fail msg, no address & data info */
#define	E_UserPrtRegVerifyFail			0x009B /* only fail msg, no address & data info */
#define	E_UserPrtRegBlankFail			0x009C /* only fail msg, no address & data info */
#define	E_ProgramNoDataFail			0x009D /* fail but donot display error data */
#define	E_ProgramNoAddrFail			0x009E /* fail but donot display error address */
#define	E_ReadConnectDAPFail		0x00A0 /* Read: connectDAP SWD link fail */
#define	E_ReadIDVerifyFail			0x00A1 /* Read: DP IDCODE verify fail */
#define	E_ReadAHBFail				0x00A2 /* Read: AHB read data fail */


/********** For NAND Flash Message *******************/
#define	E_UnknowFormat				0x00C0 /* for NAND Flash */
#define	E_NoData						0x00C1 /* for NAND Flash */
#define	E_MapTableInvalid				0x00C2 /* for NAND Flash */
#define	E_SizeNotEnough				0x00C3 /* for NAND Flash */
#define	E_OverMaxBlockLimit			0x00C4 /* for NAND Flash */
#define	E_NoMapTable					0x00C5 /* for NAND Flash */
#define	E_HaveNewBadBlock				0x00C6 /* for NAND Flash */
#define	E_BadBlockWasOverLimit			0x00C7 /* for NAND Flash */
#define	E_ProgramFailInECC				0x00C8 /* for NAND Flash */
#define	E_BadBlockOverLimitFail			0x00C9 /* for NAND Flash */
#define	E_BadBootBlockFail				0x00CA /* for NAND Flash */
#define	E_PartitionTableFail				0x00CB /* for NAND Flash */



#define	E_MaxFailedCode					0x02FF



/******************************************************************\
		Error Code Type Define For Debug
\******************************************************************/
/* Error Code 01~29 resume for system, max code=255 */
#define	DriverDownLoad_Error			30
#define	FPGACFG_Error					31
#define	BlockProperty_Error				32
#define	FWSecureCheck_Error			33
#define	USBCommunication_Error			34
#define	CFCardCommunication_Error		35
#define	DioPinMapDefine_Error			36
#define	VccPinMapDefine_Error			37
#define	GndPinMapDefine_Error			38
#define	PinMapConflict_Error				39
#define	AlgoIndex_Error					40
#define	VppesPinMapDefine_Error		41
#define	VpioPinMapDefine_Error			42
#define	NuNcPinMapDefine_Error			43


#endif

