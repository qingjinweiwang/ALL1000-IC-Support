
#ifndef SHA_H_
#define SHA_H_

#include "Type.h"
#include "FirmWare.h"


/*********************************************************************/
typedef struct {
		uChar Page0[32] ; /* 0x00 ~ 0x1F */
		uChar Page1[32] ; /* 0x20 ~ 0x3F */
		uChar Page2[32] ; /* 0x40 ~ 0x5F */
		uChar Page3[32] ; /* 0x60 ~ 0x7F */

		uChar SecretCode[8] ; /* 0x80 ~ 0x87 */

		uChar WritePrtSecret ; /* 0x88 */
		uChar WritePrtAllPage ; /* 0x89 */
		uChar WritePrtUserByte ; /* 0x8A */
		uChar FactoryByteA ; /* 0x8B, Read only */
		uChar EPROMModePage1 ; /* 0x8C */
		uChar WritePrtPage0 ; /* 0x8D */
		uShort ManufactID ; /* 0x8E,0x8F=L-H */

		uChar ROMID[8] ; /* 0x90 ~ 0x97 */

		uChar WritePrtPage3 ; /* 0x98 */
		uChar ReadPrtPage3 ; /* 0x99 */

		uChar FactoryByteB[6] ; /* 0x9A ~0x9F, Read Only */
		}TSecretInfo ;



///////////////////////////////////////////////////////
//Interface Function Prototypes
U8 Read_Rom(U8* buffer);
U8 Write_xByte(U8 address, U8* romid, U8* secret, U8* data, U8 bytes);
U8 Load_First_Secret(U8* secret);
U8 Read_Auth_Page(U8 page, U8* romid, U8* secret, U8* resultdata, U8* challenge);
U8 Update_Next_Secret(U8 page, U8* secret, U8* spdata);
void Compute_Page3_For_Secret(U8* secret, U8* spdata, U8* romid);
U8 Protect_Secret(U8* romid, U8* secret);
U8 Protect_Page_0(U8* romid, U8* secret);
U8 Protect_All_Pages(U8* romid, U8* secret);
U8 Page_1_EPROM_Mode(U8* romid, U8* secret);
U8 Activate_Overdrive(void);
U8 Deactivate_Overdrive(void);
///////////////////////////////////////////////////////


///////////////////////////////////////////////////////
//Memory Function Prototypes
U8 Read_ScratchPad(U8* data);
U8 Write_ScratchPad(U8 address, U8* data, U8 bytes);
U8 Read_Block(U8 address, U8* data, U8 bytes);


///////////////////////////////////////////////////////
void ResetSecureDevice(void) ;
void Send1ByteData(uChar TempData) ;
uChar Receive1ByteData(void) ;
uChar GetChallengeFromTimer(U8* Challenge) ;
uChar Compare2BufferData(U8* Buf0, U8* Buf1, U16 CmpCnt) ;
uChar UpdateMBSecretInfo(void) ;
uChar UpdateCSKBSecretInfo(uChar ChkSck) ;
uChar UpdateMSKBSecretInfo(void) ;
uChar MaxCounterChecking(U8* CurCnt, U32 MaxCntHi, U32 MaxCntLo) ;


///////////////////////////////////////////////////////
//Miscellaneous Function Prototypes
U8 Get_Auth(U8* bytes);
U8 Verify_CRC8(U8 crc, U8 test);
U8 Verify_CRC16(U16 crc, U16 test);
U8 VerifyMacInfo(U8* MacInfo, U8* MacCal, U8 MacCnt) ;
U8 Calculate_CRC8(U8* data, U8 bytes);
U16 Calculate_CRC16(U8* data, U8 bytes);
void Generate_MAC(U8* data, U8* result);
U32 NLF(U32 b, U32 c, U32 d, U8 num);
U32 KTN(U8 num);
void Generate_MB_MAC(U8* MacData, U8 MacCnt, U8* RomID, U8* Result, U8 ResCnt) ;
U32 MBNLF(U32 b, U32 c, U32 d, U8 num);
U32 MBKTN(U8 num);




///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
#define Bit_Test(_TempData, _DataBit)		(_TempData & (0x0001 << _DataBit))
#define Make16(_HiByte, _LoByte)			(U16)((_HiByte<<8)+_LoByte)
#define Make32(_HiHiByte, _MiHiByte, _MiLoByte, _LoLoByte)		(U32)((_HiHiByte<<24)+(_MiHiByte<<16)+(_MiLoByte<<8)+_LoLoByte)
//#define Make32(_LoLoByte, _MiLoByte, _MiHiByte, _HiHiByte)		(U32)((_HiHiByte<<24)+(_MiHiByte<<16)+(_MiLoByte<<8)+_LoLoByte)


///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
//DS2432 ROM Command Codes
#define READ_ROM_CMD			0x33
#define MATCH_ROM_CMD			0x55
#define SEARCH_ROM_CMD		0xF0
#define SKIP_ROM_CMD			0xCC
#define OD_SKIP_ROM_CMD		0x3C
#define OD_MATCH_ROM_CMD		0x69
#define RESUME_CMD				0xA5


///////////////////////////////////////////////////////
//DS2432 Memory and SHA Command Codes
#define WRITE_SP_CMD			0x0F
#define READ_SP_CMD			0xAA
#define LOAD_SECRET_CMD		0x5A
#define COMPUTE_SECRET_CMD	0x33
#define COPY_SP_CMD			0x55
#define REFRESH_SP_CMD			0xA3
#define READ_AUTH_PAGE_CMD	0xA5
#define READ_MEMORY_CMD		0xF0


///////////////////////////////////////////////////////
/********* use high speed for testing ******************************/
#define	SecurityRstDelay			55 /* 480us<lo-speed<640us, 48us<hi-speed<80us */
#define	SecurityStartDelay		1 /* 1us<lo-speed<15us, 1us<hi-speed<2us */
#define	SecurityDataDelay		10 /* 60us<lo-speed<240us, 8us<hi-speed<24us */
#define	SecurityEndDelay		5 /* 1us<lo-speed, 1us<hi-speed */


///////////////////////////////////////////////////////
extern TSecretInfo MBInfo ;
extern TSecretInfo MSKBInfo ;
extern TSecretInfo CSKBInfo[MaxSocketNum] ;
extern TSecretInfo DIPSKBInfo ;

#endif

