
/******************************************************************************
 ***                 Pin Name Constant Declaration			    ***
 ******************************************************************************/

#ifndef SimPinDef_H_
#define SimPinDef_H_


#define GPLDAT	0x560000F4
#define GPCDAT	0x56000024
#define GPGDAT	0x56000064
#define GPDDAT	0x56000034
#define GPFDAT	0x56000054
#define GPBDAT	0x56000014

#define GPLCON	0x560000F0
#define GPCCON	0x56000020
#define GPGCON	0x56000060
#define GPDCON	0x56000030
#define GPFCON	0x56000050
#define GPBCON	0x56000010

#define GPLUDP	0x560000F8
#define GPCUDP	0x56000028
#define GPGUDP	0x56000068
#define GPDUDP	0x56000038
#define GPFUDP	0x56000058
#define GPBUDP	0x56000018


/***** Direction *****/
#define InDir			0x0
#define OutDir		0x1
#define FunPin		0x2
#define ResPin		0x3

/***** Pull Up/Down *****/
#define DisPUD		0x0
#define PDn			0x1
#define PUp			0x2
#define NotPUD		0x3



/*****************************************************/
/********* Simple SDIP48 pin map re-map Table ***********/
/*****************************************************/
static const uInt32 SDIP48_PortDirReMapTbl[ ] =
{ /* MCU Port Address, Port Num Offset */
/* each pin use 2 bits, 00=In, 01=Out, 10=Fun, 11=res */
	GPLCON, 0, /* DIP1 */
	GPLCON, 1,
	GPLCON, 2,
	GPLCON, 3,
	GPLCON, 8,
	GPLCON, 9,
	GPLCON, 13,

	GPCCON, 0, /* DIP8*/
	GPCCON, 1,
	GPCCON, 2,
	GPCCON, 3,
	GPCCON, 4,
	GPCCON, 5,
	GPCCON, 6,
	GPCCON, 7,
	GPCCON, 8,
	GPCCON, 9,
	GPCCON, 10,
	GPCCON, 11,
	GPCCON, 12,
	GPCCON, 13,
	GPCCON, 14,
	GPCCON, 15,

	GPGCON, 3, /* DIP24 */
	GPGCON, 4,
	GPGCON, 5,
	GPGCON, 6,
	GPGCON, 7,

	GPDCON, 0, /* PD29 */
	GPDCON, 1,
	GPDCON, 2,
	GPDCON, 3,
	GPDCON, 4,
	GPDCON, 5,
	GPDCON, 6,
	GPDCON, 7,
	GPDCON, 8,
	GPDCON, 9,
	GPDCON, 10,
	GPDCON, 11,
	GPDCON, 12,
	GPDCON, 13,
	GPDCON, 14,
	GPDCON, 15,

	GPFCON, 0, /* PD45 */
	GPFCON, 3,
	GPFCON, 4,

	GPBCON, 0, /* PD48 */
} ;



/*****************************************************/
/********* Simple SDIP48 pin map re-map Table ***********/
/*****************************************************/
static const uInt32 SDIP48_PortDataReMapTbl[ ] =
{ /* MCU Port Address, Port Num Offset */
	GPLDAT, 0, /* DIP1 */
	GPLDAT, 1,
	GPLDAT, 2,
	GPLDAT, 3,
	GPLDAT, 8,
	GPLDAT, 9,
	GPLDAT, 13,

	GPCDAT, 0, /* DIP8*/
	GPCDAT, 1,
	GPCDAT, 2,
	GPCDAT, 3,
	GPCDAT, 4,
	GPCDAT, 5,
	GPCDAT, 6,
	GPCDAT, 7,
	GPCDAT, 8,
	GPCDAT, 9,
	GPCDAT, 10,
	GPCDAT, 11,
	GPCDAT, 12,
	GPCDAT, 13,
	GPCDAT, 14,
	GPCDAT, 15,

	GPGDAT, 3, /* DIP24 */
	GPGDAT, 4,
	GPGDAT, 5,
	GPGDAT, 6,
	GPGDAT, 7,

	GPDDAT, 0, /* PD29 */
	GPDDAT, 1,
	GPDDAT, 2,
	GPDDAT, 3,
	GPDDAT, 4,
	GPDDAT, 5,
	GPDDAT, 6,
	GPDDAT, 7,
	GPDDAT, 8,
	GPDDAT, 9,
	GPDDAT, 10,
	GPDDAT, 11,
	GPDDAT, 12,
	GPDDAT, 13,
	GPDDAT, 14,
	GPDDAT, 15,

	GPFDAT, 0, /* PD45 */
	GPFDAT, 3,
	GPFDAT, 4,

	GPBDAT, 0, /* PD48 */
} ;



/*****************************************************/
/********* Simple SDIP48 pin map re-map Table ***********/
/*****************************************************/
static const uInt32 SDIP48_PortUpDnReMapTbl[ ] =
{ /* MCU Port Address, Port Num Offset */
/* each pin use 2 bits, 00=DisPUD, 01=PullDown, 10=PullUP, 11=Not available */
	GPLUDP, 0, /* DIP1 */
	GPLUDP, 1,
	GPLUDP, 2,
	GPLUDP, 3,
	GPLUDP, 8,
	GPLUDP, 9,
	GPLUDP, 13,

	GPCUDP, 0, /* DIP8*/
	GPCUDP, 1,
	GPCUDP, 2,
	GPCUDP, 3,
	GPCUDP, 4,
	GPCUDP, 5,
	GPCUDP, 6,
	GPCUDP, 7,
	GPCUDP, 8,
	GPCUDP, 9,
	GPCUDP, 10,
	GPCUDP, 11,
	GPCUDP, 12,
	GPCUDP, 13,
	GPCUDP, 14,
	GPCUDP, 15,

	GPGUDP, 3, /* DIP24 */
	GPGUDP, 4,
	GPGUDP, 5,
	GPGUDP, 6,
	GPGUDP, 7,

	GPDUDP, 0, /* PD29 */
	GPDUDP, 1,
	GPDUDP, 2,
	GPDUDP, 3,
	GPDUDP, 4,
	GPDUDP, 5,
	GPDUDP, 6,
	GPDUDP, 7,
	GPDUDP, 8,
	GPDUDP, 9,
	GPDUDP, 10,
	GPDUDP, 11,
	GPDUDP, 12,
	GPDUDP, 13,
	GPDUDP, 14,
	GPDUDP, 15,

	GPFUDP, 0, /* PD45 */
	GPFUDP, 3,
	GPFUDP, 4,

	GPBUDP, 0, /* PD48 */
} ;



#endif

