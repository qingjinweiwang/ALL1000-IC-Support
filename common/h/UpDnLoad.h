


#ifndef UpDnLoad_H_
#define UpDnLoad_H_

#include "Type.h"


/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/*                            FirmWare Command Macro Define Table                                                   */
/*				All below command get from USB Vender Request Command !!                    */
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

/* Driver Master Version Number ----------------------------------------------*/
#define SysCmdNull				0x0000 /* Null command in buffer */

/*------------------ <<<  Driver Site Command Table  >>> --------------------*/
#define	DeviceSelect			0x4E00	/*{N #xxxx	- Device Select Command}*/
#define	DeviceSelectAndSN		0x4E01	/*{N #xxxx	- Device Select Command}*/
#define	BuzzerOff				0x5A00
#define	BuzzerOn				0x5A01

/*------------------- <<<  File Upload Sub-Command Table  >>> ------------------*/
#define	UploadProgramData		0x5500	/*{U &0	- Upload device program data from SDRAM}*/
#define	UploadSpecialBit			0x5501	/*{U &1	- Upload device special bit data from int.DM of DSP}*/
#define	UploadSystemInfo		0x5502	/*{U &2	- Upload system information}*/
#define	UploadDevBlock			0x5503	/*{U &3	- Upload device block structure}*/
#define	UploadDevSpec			0x5504	/*{U &4	- Upload device specifications}*/
#define	UploadSKBInfo			0x5505	/*{U &5	- Upload socket board information}*/
#define	UploadSKTPinMap		0x5506	/*{U &6	- Upload socket pin map.}*/
#define	UploadProjectData		0x5507	/* Upload all project data from CF Card to PC */
#define	UploadFpgaCode			0x5508	/* Upload Fpga Driver Code from CF Card to PC */
#define	UploadFWCode			0x5509	/* Upload Fw CPU Driver Code from CF Card to PC */

/*------------------- <<<  File Download Sub-Command Table  >>> ----------------*/
#define	DownloadProgramData	0x4400	/*{D &0	- Download device program data into SDRAM}*/
#define	DownloadSpecialBit		0x4401	/*{D &1	- Download device special bit data into int.DM of DSP}*/
#define	DownloadDevSpec		0x4402	/*{D &2	- Download device specifications.}*/
#define	DownloadFPGACFG		0x4403	/*{D &3	- Download FPGA equation file for CPD board.}*/
#define	DownloadSKBInfo		0x4404	/*{D &4	- Download SkbEEPROM information.}*/
#define	DownloadSKTPinMap		0x4405	/*{D &5	- Download Socket Pin Map.}*/
#define	DownloadProjectData	0x4406	/* Download all project data from PC to CF Card */
#define	DownloadFpgaCode		0x4407	/* Download Fpga Driver Code from CF Card to PC */
#define	DownloadFWCode		0x4408	/* Download Fw CPU Driver Code from CF Card to PC */

/*------------------- <<<  Device Power Sub-Command Table  >>> ------------------*/
#define	DevicePwrOning			0x4F00	/*{O &0	- Device Power On}*/
#define	DevicePwrOffing			0x4F01	/*{O &1	- Device Power Off}*/

/*------------------- <<<  Device Read Sub-Command Table  >>> ------------------*/
#define	DeviceReading			0x5200	/*{R &0	- Device Read}*/

/*------------------- <<<  Device Check Sub-Command Table  >>> -----------------*/
#define	DeviceBlankChecking		0x4200	/*{B &0	- Device Blank Check}*/

/*------------------- <<<  Device Check Sub-Command Table  >>> -----------------*/
#define	DeviceIllegalChecking	0x4C00	/*{L &0	- Device Illegal Bit Check}*/

/*------------------- <<<  Device Erase Sub-Command Table  >>> -----------------*/
#define	DeviceErasing			0x4500	/*{E &0	- Device Erase}*/
#define	DevEraseAfterBNKFail	0x4501	/*{E &1	- Device Erase if Blank Check fail!}*/
#define	DevEraseAfterILGFail	0x4502	/*{E &2	- Device Erase if Illegal Bit Check fail!}*/

/*------------------- <<<  Device Program Sub-Command Table  >>> ---------------*/
#define	DeviceProgramming		0x5000	/*{P &0	- Device Program}*/
#define	DeviceProtecting			0x5001	/*{P &1	- Device Protect}*/
#define	DeviceUnprotecting		0x5002	/*{P &2	- Device Unprotect}*/

/*------------------- <<<  Device Verify Sub-Command Table  >>> ----------------*/
#define	DeviceVerifying			0x5600	/*{V &0	- Device Verify}*/
#define	DevHiVccVerifying		0x5601	/*{V &1	- Device Hi-VCC Verify}*/
#define	DevLoVccVerifying		0x5602	/*{V &2	- Device Lo-VCC Verify}*/

/*------------------- <<<  Device Secure Sub-Command Table  >>> ----------------*/
#define	DeviceSecuring			0x4300	/*{C &0	- Device Secure}*/

/*------------------- <<<  Device Checksum Sub-Command Table  >>> --------------*/
#define	ChecksumCalculating		0x5300	/*{S &0	- Checksum calculating against Byte}*/
#define	SetComparedSum		0x5301	/*{S &1	- Set Compared Checksum}*/
#define	ChecksumByteCal		(ChecksumCalculating | ByteSum)
#define	ChecksumWordCal		(ChecksumCalculating | WordSum)
#define	ChecksumCRC16Cal		(ChecksumCalculating | CRC16Sum)
#define	ChecksumCRC32Cal		(ChecksumCalculating | CRC32Sum)

/*------------------- <<<  Device Insertion Sub-Command Table  >>> -------------*/
#define	InsertionCheck			0x4900	/*{I &0	- Device insertion check}*/
#define	PinConnectCheck			0x4901	/*{I &1	- Device pin Connect check}*/

/*---------------- <<<  Device Auto Identify Sub-Command Table  >>> ------------*/
#define	DeviceIDChecking		0x4601	/*{F &1 #xxxx	- Device ID Code Check}*/

/*----------------<<< System In Run Mode Command Table >>>-------------*/
#define	SysHandleRunMode		0x4100	/*{ A & 0 }*/
#define	SysProductRunMode		0x4101	/*{ A & 1 }*/

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
#define	SysInforBufMaxLen		32 /* upload system status to PC by EP0, max 64 bytes */
#define	SysInforBufCmd			0
#define	SysInforBufCnt			1
#define	SysInforBufRamAddrHi	2
#define	SysInforBufRamAddrLo	3
#define	SysInforBufRamData		4
#define	SysInforBufDevAddrHi	5
#define	SysInforBufDevAddrLo	6
#define	SysInforBufDevData		7


 /* Buffer format: Cmd+Lenn+Data0,1...(n-1) */
extern uShort SysInforBuf[SysInforBufMaxLen] ;
extern INT32 UpDnSectCnt ;





#endif

