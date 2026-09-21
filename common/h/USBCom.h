


#ifndef USBCom_H_
#define USBCom_H_

#include "Type.h"
#include "2450addr.h"


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define VR_UPLOAD			0xc0 // firmware upload/download page 80
#define VR_DOWNLOAD		0x40


/********** cmd 0xa0 ~ 0xaf for system remain ******************/
#define VR_ANCHOR_DLD   	0xa0 // handled by core
#define VR_EEPROM			0xa2 // loads (uploads) EEPROM
#define VR_CFCardInfo		0xa3 // loads (uploads) external ram
#define VR_SETI2CADDR		0xa4
#define VR_GETI2C_TYPE		0xa5 // 8 or 16 byte address
#define VR_GET_CHIP_REV	0xa6 // Rev A, B = 0, Rev C = 2 // NOTE: New TNG Rev
#define VR_TEST_MEM		0xa7 // runs mem test and returns result
#define VR_RENUM			0xa8 // renum
#define VR_DB_FX			0xa9 // Force use of double byte address EEPROM (for FX)
#define VR_I2C_100			0xaa // put the i2c bus in 100Khz mode
#define VR_I2C_400			0xab // put the i2c bus in 400Khz mode
#define VR_NOSDPAUTO		0xac // test code. does uploads using SUDPTR with manual length override

#define VR_LoadCPUCode		0xad // EP0 format: AddrLL,ML,MH.HH + LthLL,ML,MH,HH total 64 bytes
#define VR_GO_RUN_CPU		0xae // start run system
#define VR_GO_RUN_BIOS		0xaf // start run bios for re-load cpu code */


/****************************************************\
\*****	PC<-->USB Communication Command Define ******\
\****************************************************/
/* Setup Data Format: VRType(Up or Dn) + VRCmd(0xB0~0xBF) + Ep0BufAddrLoHi + Ep0BufLenLoHi */
#define VR_FwCtrlCmd		0xB0 /* download curr-device processing command buffer list */
	/* sub-control command refer to UpDnload.h */

#define VR_DeviceInform		0xB1 /* download curr-device info: algo, type, mft/dev id, skb code... */
	/* EP0 format: 1B-Algo+1B-Type+1W-MftID+1W-DevIDL+1W-DevIDH+1W-CSkbCodeL+1W-CSkbCode+1W-MSkbCode+2W-DevLth */

#define VR_SpecialBit			0xB2 /* dn/up curr-device special bit information setting */
#define VR_SkbInform		0xB3 /* dn/up curr-skb information */
#define VR_DriverVersion		0xB4 /* upload driver version */
	/* EP0 format: V.v Memory/MPU/PLD, use ASCII code format */

#define VR_BlockInform		0xB5 /* upload curr-device total block information */
#define VR_BlockInformLen	0xB9 /* upload curr-device total block information length, feedback 2 bytes data */
#define VR_PinMapInform		0xB6 /* upload curr-device total pin map information */
	/* EP0 format: total pin map for MPU/PLD, but not include IO port for Memory */
#define VR_PinMapInformLen	0xBA /* upload curr-device total pin map information length, feedback 2 bytes data */

#define VR_Specification		0xB7 /* dn/upload curr-device total ele-parameter information */
	/* EP0 format: 6*2 bytes per parameter for all */

#define VR_SystemInform		0xB8 /* upload curr-system status information */
	/* EP0 format: Cmd + Len + D0...Dn-1 */

#define VR_ProjectMBR		0xBB /* dn/upload project MBR information */
	/* Setup Data0=VRDir, Data1=VRCmd, Data2,3=MBRAddr, Data4,5="LC", Data6,7=MBRLen(always be 512) */
	/* EP0 format: total 512 Bytes data for per project's MBR */

#define VR_FWBiosCode		0xBC /* upload curr-system status information */
	/* EP0 format: Cmd + Len + D0...Dn-1 */

#define VR_LoadFPGACode	0xBD

#define VR_PartitionTable		0xBE /* upload or download Nand flash partition table info */

#define VR_ProgSNData		0xBF /* upload/download sck x SN data/status, alway be 512 */
	/* Download: EP0 format: WrSck+SNStep+SNType+SNLen+4DevAddrHH-HM-LM-LL + SNData0,1,2...max=(512-8) */
	/* Upload: EP0 format: WrSck+SckOkNg+SckEnDis+SNLen+4DevAddr + SNData0,1,2...max=(512-8) */

#define VR_DebugInform		0xDF /* dn/up debug information, max length 4k per time */


/*****************************************************/
#define EP0BUFF_SIZE		64
#define EP1BUFF_SIZE		512
#define EP2BUFF_SIZE		512
#define EP3BUFF_SIZE		512
#define EP4BUFF_SIZE		512
#define EP5BUFF_SIZE		512 // Max 1024
#define EP6BUFF_SIZE		512 // Max 1024
#define EP7BUFF_SIZE		512 // Max 1024
#define EP8BUFF_SIZE		512 // Max 1024


#define	EP5OUT			0x02
#define	EP6IN			0x86




////////////////////////////////////////////////////////////////////
extern U16 ep0csr ;
extern U16 ep5csr ;
extern U16 ep6csr ;
extern U16 ep7csr ;
extern U16 ep8csr ;


struct USB_DEVICE_DESCRIPTOR;
struct USB_GET_STATUS;
struct USB_INTERFACE_GET;
struct USB_DESCRIPTORS;
struct DEVICE_REQUEST;


//////////////////////////////////////////////////////////////
#define Outp32(addr, data) (*(volatile U32 *)(addr) = (data))
#define Outp16(addr, data) (*(volatile U16 *)(addr) = (data))
#define Outp8(addr, data)  (*(volatile U8 *)(addr) = (data))
#define Inp32(addr, data) (data = (*(volatile U32 *)(addr)))
#define Inp16(addr, data) (data = (*(volatile U16 *)(addr)))
#define Inp8(addr, data)  (data = (*(volatile U8 *)(addr)))
#define Input32(addr)     (*(volatile U32 *)(addr))
#define Input16(addr)     (*(volatile U16 *)(addr))
#define Input8(addr)      (*(volatile U8 *)(addr))

#define	GetRamWordData()				(Input16(EP5_FIFO))
#define	SaveRamWordData(_WordData_)	(Outp16(EP6_FIFO, _WordData_))



///////////////////////////////////////////////////////////////////
#define USBDEV_BASE 		0x49800000

#define		USBD20_REGS		         U32
#define		INDEX_REG                (USBDEV_BASE+0x00) // Index register
#define		EP_INT_REG               (USBDEV_BASE+0x04) // EP Interrupt pending and clear
#define		EP_INT_EN_REG            (USBDEV_BASE+0x08) // EP Interrupt enable
#define		FUNC_ADDR_REG            (USBDEV_BASE+0x0c) // Function address
#define		FRAME_NUg_REG            (USBDEV_BASE+0x10) // Frame number
#define		EP_DIR_REG               (USBDEV_BASE+0x14) // Endpoint direction
#define		TEST_REG                 (USBDEV_BASE+0x18) // Test register
#define		SYS_STATUS_REG           (USBDEV_BASE+0x1c) // System status
#define		SYS_CON_REG              (USBDEV_BASE+0x20) // System control
#define		EP0_STATUS_REG           (USBDEV_BASE+0x24) // Endpoint 0 status
#define		EP0_CON_REG              (USBDEV_BASE+0x28) // Endpoint 0 control
#define		EP_STATUS_REG            (USBDEV_BASE+0x2c) // Endpoints status
#define		EP_CON_REG               (USBDEV_BASE+0x30) // Endpoints control
#define		BYTE_READ_CNT_REG        (USBDEV_BASE+0x34) // read count, word counter format!!
#define		BYTE_WRITE_CNT_REG       (USBDEV_BASE+0x38) // write count, byte counter format!!
#define		MAX_PKT_REG              (USBDEV_BASE+0x3c) // Max packet size
#define		DMA_CON_REG              (USBDEV_BASE+0x40) // DMA control
#define		DMA_CNT_REG              (USBDEV_BASE+0x44) // DMA count
#define		DMA_FIFO_CNT_REG         (USBDEV_BASE+0x48) // DMA FIFO count
#define		DMA_TOTAL_CNT1_REG       (USBDEV_BASE+0x4c) // DMA Total count1
#define		DMA_TOTAL_CNT2_REG       (USBDEV_BASE+0x50) // DMA Total count2
#define		DMA_IF_CON_REG           (USBDEV_BASE+0x84) // DMA interface Control
#define		DMA_MEM_BASE_ADDR        (USBDEV_BASE+0x88) // Mem Base Addr
#define		DMA_MEg_CURRENT_ADDR     (USBDEV_BASE+0x8c) // Mem current Addr
#define		EP0_FIFO                 (USBDEV_BASE+0x60) // Endpoint 0 FIFO
#define		EP1_FIFO                 (USBDEV_BASE+0x64) // Endpoint 1 FIFO
#define		EP2_FIFO                 (USBDEV_BASE+0x68) // Endpoint 2 FIFO
#define		EP3_FIFO                 (USBDEV_BASE+0x6c) // Endpoint 3 FIFO
#define		EP4_FIFO                 (USBDEV_BASE+0x70) // Endpoint 4 FIFO
#define		EP5_FIFO                 (USBDEV_BASE+0x74) // Endpoint 5 FIFO
#define		EP6_FIFO                 (USBDEV_BASE+0x78) // Endpoint 6 FIFO
#define		EP7_FIFO                 (USBDEV_BASE+0x7c) // Endpoint 7 FIFO
#define		EP8_FIFO                 (USBDEV_BASE+0x80) // Endpoint 8 FIFO
#define		FCON                     (USBDEV_BASE+0x100) // Burst Fifo Control




// Descriptor Types
#define DESC_TYPE	unsigned char

#define		DEVICE_TYPE			0x1
#define		CONFIGURATION_TYPE	2
#define		STRING_TYPE			3
#define		INTERFACE_TYPE		4
#define		ENDPOINT_TYPE		5


// configuration descriptor: bmAttributes
#define DESC_CONF		unsigned char

#define	CONF_ATTR_DEFAULT        0x80 // Spec 1.0 it was BUSPOWERED bit.
#define	CONF_ATTR_REMOTE_WAKEUP  0x20
#define	CONF_ATTR_SELFPOWERED    0x40


// endpoint descriptor
#define DESC_ENDPT		unsigned char

#define		EP_ADDR_IN               0x80
#define		EP_ADDR_OUT            0x00

#define		EP_ATTR_CONTROL         0x0
#define		EP_ATTR_ISOCHRONOUS  0x1
#define		EP_ATTR_BULK            0x2
#define		EP_ATTR_INTERRUPT      0x3


#define EP_INDEX		unsigned char

#define	EP0		0 /* for CPU system use */
#define	EP1		1 /* for driver system download */
#define	EP2		2 /* for driver system upload */
#define	EP3		3 /* for file data download use */
#define	EP4		4 /* for file data upload use */
#define	EP5		5
#define	EP6		6
#define	EP7		7
#define	EP8		8


typedef		struct __USB_DEVICE_DESCRIPTOR
{
	U8 bLength;
	U8 bDescriptorType;
	U8 bcdUSBL;
	U8 bcdUSBH;
	U8 bDeviceClass;
	U8 bDeviceSubClass;
	U8 bDeviceProtocol;
	U8 bMaxPacketSize0;
	U8 idVendorL;
	U8 idVendorH;
	U8 idProductL;
	U8 idProductH;
	U8 bcdDeviceL;
	U8 bcdDeviceH;
	U8 iManufacturer;
	U8 iProduct;
	U8 iSerialNumber;
	U8 bNumConfigurations;
}USB_DEVICE_DESCRIPTOR;

typedef		struct __USB_CONFIGURATION_DESCRIPTOR
{
	U8 bLength;
	U8 bDescriptorType;
	U8 wTotalLengthL;
	U8 wTotalLengthH;
	U8 bNumInterfaces;
	U8 bConfigurationValue;
	U8 iConfiguration;
	U8 bmAttributes;
	U8 maxPower;
}USB_CONFIGURATION_DESCRIPTOR;

typedef		struct __USB_INTERFACE_DESCRIPTOR
{
	U8 bLength;
	U8 bDescriptorType;
	U8 bInterfaceNumber;
	U8 bAlternateSetting;
	U8 bNumEndpoints;
	U8 bInterfaceClass;
	U8 bInterfaceSubClass;
	U8 bInterfaceProtocol;
	U8 iInterface;
}USB_INTERFACE_DESCRIPTOR;

typedef		struct __USB_ENDPOINT_DESCRIPTOR
{
	U8 bLength;
	U8 bDescriptorType;
	U8 bEndpointAddress;
	U8 bmAttributes;
	U8 wMaxPacketSizeL;
	U8 wMaxPacketSizeH;
	U8 bInterval;
}USB_ENDPOINT_DESCRIPTOR;

typedef		struct __USB_CONFIGURATION_SET
{
	U8 ConfigurationValue;
}USB_CONFIGURATION_SET;

typedef		struct __USB_GET_STATUS
{
	U8 Device;
	U8 Interface;
	U8 Endpoint0; /* system end point */
	U8 Endpoint1; /* data download end point */
	U8 Endpoint2; /* data upload end point */
	U8 Endpoint3; /* system info download end point */
	U8 Endpoint4; /* system info upload end point */
}USB_GET_STATUS;


typedef		struct __USB_INTERFACE_GET
{
	U8 AlternateSetting;
}USB_INTERFACE_GET;


typedef		struct __USB_DESCRIPTORS
{
	USB_CONFIGURATION_DESCRIPTOR oDescConfig;
	USB_INTERFACE_DESCRIPTOR oDescInterface;
	USB_ENDPOINT_DESCRIPTOR oDescEndpt1;
	USB_ENDPOINT_DESCRIPTOR oDescEndpt2;
	USB_ENDPOINT_DESCRIPTOR oDescEndpt3;
	USB_ENDPOINT_DESCRIPTOR oDescEndpt4;
}USB_DESCRIPTORS;

typedef		struct __DEVICE_REQUEST
{
	U8 bReqType;  // Device Request offset 0
	U8 bRequest;       // Device Request offset 1
	U8 wValue_L;       // Device Request offset 2
	U8 wValue_H;       // Device Request offset 3
	U8 wIndex_L;       // Device Request offset 4
	U8 wIndex_H;       // Device Request offset 5
	U8 wLength_L;      // Device Request offset 6
	U8 wLength_H;      // Device Request offset 7
}DEVICE_REQUEST;

// EP0 CSR register Bits
#define EP0_LAST_WORD_ODD      	(0x1<<6)
#define EP0_SENT_STALL              (0x01<<4)
#define EP0_DATA_END                (0x01<<3)
#define EP0_SETUP_END               (0x03<<2)
#define EP0_TX_SUCCESS              (0x01<<1)
#define EP0_RX_SUCCESS              (0x01<<0)

//  Defines for Endpoint CSR Register Bits
#define DMA_TOTAL_COUNT_ZERO        (0x1<<9)
#define SHORT_PKT_RECEIVED          (0x1<<8)
#define EP_FIFO_FLUSH               (0x1<<6)
#define EP_SENT_STALL               (0x1<<5)
#define EP_LAST_WORD_ODD      		(0x1<<4)
#define EP_PKT_STATUS_FIFO     		(0x3<<2) /* 0=no,1=single pak,2=double pak */
#define EP_TX_SUCCESS               (0x1<<1)
#define EP_RX_SUCCESS               (0x1<<0)

// Define for EndPoint ECR register bits
#define EPx_Stall_Set			(0x01<<1)
#define OUTPKTHLD_Set		(0x01<<11)
#define INPKTHLD_Set		(0x01<<12)



////////////////////////////////////////////////////////////////////////
#define	EPx_LAST_WORD_ODD(_EPx_)		(_EPx_ & EP_LAST_WORD_ODD)

#define EP0_STATE_INIT              (0)
#define EP0_STATE_GD_DEV_0_0        (11)
//#define EP0_STATE_GD_DEV_0_1        (14)
#define EP0_STATE_GD_DEV_1          (12)
#define EP0_STATE_GD_DEV_2          (13)
#define EP0_STATE_GD_CFG_0          (21)
#define EP0_STATE_GD_CFG_1          (22)
#define EP0_STATE_GD_CFG_2          (23)
#define EP0_STATE_GD_CFG_3          (24)
#define EP0_STATE_GD_CFG_4          (25)
#define EP0_STATE_GD_CFG_ONLY_0     (41)
#define EP0_STATE_GD_CFG_ONLY_1     (42)
#define EP0_STATE_GD_IF_ONLY_0      (44)
#define EP0_STATE_GD_IF_ONLY_1      (45)
#define EP0_STATE_GD_EP0_ONLY_0     (46)
#define EP0_STATE_GD_EP1_ONLY_0     (47)
#define EP0_STATE_GD_EP2_ONLY_0     (48)
#define EP0_STATE_GD_EP3_ONLY_0     (49)
#define EP0_STATE_GD_STR_I0         (30)
#define EP0_STATE_GD_STR_I1         (31)
#define EP0_STATE_GD_STR_I2         (32)
#define EP0_STATE_GD_DEV_QUALIFIER  (33)
#define EP0_STATE_GD_DEV_QUALIFIER_1 (55)
#define EP0_INTERFACE_GET           (34)


#define EP0_GET_STATUS0             (35)
#define EP0_GET_STATUS1             (36)
#define EP0_GET_STATUS2             (37)
#define EP0_GET_STATUS3             (38)
#define EP0_GET_STATUS4             (39)
#define EP0_STATE_GD_DEV_OTHER_SPEED (40)
#define EP0_STATE_GD_DEV_OTHER_SPEED_1 (56)

// SPEC1.1
// Standard bReqType (Type)
#define STANDARD_TYPE               0x00
#define CLASS_TYPE                  0x20
#define VENDOR_TYPE                 0x40
#define RESERVED_TYPE               0x60

// Standard bReqType (Recipient)
#define DEVICE_RECIPIENT            0
#define INTERFACE_RECIPIENT         1
#define ENDPOINT_RECIPIENT          2
#define OTHER_RECIPIENT             3

// Standard bRequest codes
#define STANDARD_GET_STATUS         0
#define STANDARD_CLEAR_FEATURE      1
#define STANDARD_RESERVED_1         2
#define STANDARD_SET_FEATURE        3
#define STANDARD_RESERVED_2         4
#define STANDARD_SET_ADDRESS        5
#define STANDARD_GET_DESCRIPTOR     6
#define STANDARD_SET_DESCRIPTOR     7
#define STANDARD_GET_CONFIGURATION  8
#define STANDARD_SET_CONFIGURATION  9
#define STANDARD_GET_INTERFACE      10
#define STANDARD_SET_INTERFACE      11
#define STANDARD_SYNCH_FRAME        12

// Descriptor types
#define DEVICE_DESCRIPTOR           1
#define CONFIGURATION_DESCRIPTOR    2
#define STRING_DESCRIPTOR           3
#define INTERFACE_DESCRIPTOR        4
#define ENDPOINT_DESCRIPTOR         5
#define DEVICE_QUALIFIER            6
#define OTHER_SPEED_CONFIGURATION   7

// string descriptor
#define LANGID_US_L                 (0x09)
#define LANGID_US_H                 (0x04)

// USB Endpoints states
#define EP0_STATE_IDLE              (0)
#define EP0_STATE_TRANSFER          (1)
#define EP0_STATE_RECEIVER          (2)

#define BULK_OUT_STATUS_NOSTALL     (0x0000)
#define BULK_OUT_STATUS_STALL       (0x0001)

#define DEVICE_STATUS_DEFAULT       (0x0000)
#define DEVICE_STATUS_SELFPOWERED   (0x0001)
#define DEVICE_STATUS_REMOTEWAKEUP  (0x0002)


#define DEVICE_DESC_SIZE            18
#define STRING_DESC0_SIZE           4
#define STRING_DESC1_SIZE           22
#define STRING_DESC2_SIZE           44
#define CONFIG_DESC_TOTAL_SIZE      32
#define CONFIG_DESC_SIZE            9
#define INTERFACE_DESC_SIZE         9
#define ENDPOINT_DESC_SIZE          7
#define DEVICE_QUALIFIER_SIZE       10
#define OTHER_SPEED_CONFIGURATION_SIZE 9

// INT_REG status value
#define INT_ERR                     (0xff80)
#define INT_REG_ERROR               (0xff1<<6)
#define INT_REG_VBUS                (0x1<<8)
#define INT_REG_VBUS_CLEAR          (0x1<<6)
#define INT_REG_HSP                 (0x1<<4)
#define INT_REG_SDE                 (0x1<<3)
#define INT_REG_RESET               (0x1)
#define INT_REG_RESUME              (0x1<<2)
#define INT_REG_SUSPEND             (0x1<<1)
#define INT_REG_EP8                 (0x1<<8)
#define INT_REG_EP7                 (0x1<<7)
#define INT_REG_EP6                 (0x1<<6)
#define INT_REG_EP5                 (0x1<<5)
#define INT_REG_EP4                 (0x1<<4)
#define INT_REG_EP3                 (0x1<<3)
#define INT_REG_EP2                 (0x1<<2)
#define INT_REG_EP1                 (0x1<<1)
#define INT_REG_EP0                 (0x1)
#define INT_DTB_MISMATCH            (0x1FF<<7)

// USB Dma Operation
#define DMA_AUTO_RX_DISABLE         (0x1<<5)
#define DMA_FLY_ENABLE              (0x1<<4)
#define DMA_FLY_DISABLE             (0x0<<4)
#define DMA_DEMEND_ENABLE           (0x1<<3)
#define DMA_DEMEND_DISABLE          (0x0<<3)
#define DMA_TX_START                (0x1<<2)
#define DMA_TX_STOP                 (0x0<<2)
#define DMA_RX_START                (0x1<<1)
#define DMA_RX_STOP                 (0x0<<1)
#define USB_DMA_MODE                (0x1<<0)
#define USB_INT_MODE                (0x0<<0)

#define MAX_BURST_INCR16            (0x3<<0)
#define MAX_BURST_INCR8             (0x2<<0)
#define MAX_BURST_INCR4             (0x1<<0)

#define DMA_ENABLE                  (0x1<<8)
#define DMA_DISABLE                 (0x0<<8)


// Feature Selectors
	#define EP_STALL          		0
	#define DEVICE_REMOTE_WAKEUP    1
	#define TEST_MODE				2

	/* Test Mode Selector*/
	#define TEST_J					1
	#define TEST_K					2
	#define TEST_SE0_NAK			3
	#define TEST_PACKET				4
	#define TEST_FORCE_ENABLE		5

	#define		TR_RSVD					0x4F20
	#define		TR_VBUS					0x8000
	#define		TR_EUERR				0x2000
	#define		TR_PERR					0x1000
	#define		TR_SPDSEL				0x00C0
	#define		TR_TMD					0x0010
	#define		TR_TPS					0x0008
	#define		TR_TKS					0x0004
	#define		TR_TJS					0x0002
	#define		TR_TSNS					0x0001



//////////////////////////////////////////////////////////////////////
extern USB_DEVICE_DESCRIPTOR g_poDescDevice;
extern USB_GET_STATUS g_poStatusGet;
extern USB_INTERFACE_GET g_poInterfaceGet;
extern USB_DESCRIPTORS g_poDesc;
extern DEVICE_REQUEST g_poDevReq;


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#define EZUSB_STALL_EPx()		(rECR |= EPx_Stall_Set)
#define EZUSB_UNSTALL_EPx()	(rECR &= (~EPx_Stall_Set))

#define ReArmUsbDelay			30



///////////////////////////////////////////////////////////////
extern void Isr_Init(void);
extern void Isr_ReInit_OffLineMode(void) ;
extern void Vbus_detect_init(void);
extern void Usb_power_en(void);
extern void Usb_power_dis(void);
extern void Usb_Reset(void);
extern void Usb_Init(void);
extern void USB_OnOff(void);
extern void IsrIRQ(void); /* call in start.s */


extern void HandleEvent(void);

extern void SetDescriptorTable(void);
extern void SetEndpoint(void);
extern void HandleEvent_EP0(void);
extern void DR_VendorCmd(void) ;
extern void RdPktEp5(U8 *buf,int num);
extern void WrPktEp6(U8 *buf, int num) ;
extern void ResetEP1To8FIFO(U16 EP1_8) ;
extern void SetMaxPktSizes(void);	

extern uShort CheckEP0_RXStatus(void) ;
extern uShort CheckEP0_TXStatus(void) ;
extern uShort CheckEP5_RXStatus(void) ;
extern uShort CheckEP5_DMARxEndStatus(void) ;
extern uShort CheckEP6_TXStatus(void) ;
extern uShort CheckEP6_DMATxEndStatus(void) ;
extern uShort CheckEP7_RXStatus(void) ;
extern uShort CheckEP8_TXStatus(void) ;



/*************************************************************/
extern void USB_Initial(void) ;

extern void SaveChkEP6Buffer(void) ;
extern void GetChkEP5Buffer(void) ;
extern void SaveSRAMByteData(uChar SRAMByteData) ;
extern void SaveSRAMWordData(uShort SRAMWordData) ;
extern uChar GetSRAMByteData(void) ;
extern uShort GetSRAMWordData(void) ;
extern uChar SaveChkBlkLenEnd(void) ;
extern uChar GetChkBlkLenEnd(void) ;
extern void GetDevDataFromUSB(void) ;

extern void ReArmCFCSkipEBAData(void) ;

extern void DevSNProgAndVerify(void) ; /* program device SN */


/***************************************************/
extern void (*run)(void);




#endif

