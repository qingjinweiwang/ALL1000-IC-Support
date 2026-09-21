
/*****************************************************************************/
/*****************************************************************************/
#ifndef TYPE_H_
#define TYPE_H_


/****************************************************************************\
			Type Definitions
\****************************************************************************/
/* Variable type Definitions: */
typedef	unsigned int	uInt32;
typedef	unsigned short uShort;
typedef	unsigned char uChar;
typedef	unsigned int	INT32;
typedef	float FLT32;



//////////////////////////////////////////
#define 	U32 unsigned int
#define 	U16 unsigned short
#define 	S32 int
#define 	S16 short int
#define 	U8  unsigned char
#define	S8  char
#define	bool unsigned char

/////////////////////////////////////////
#define   TRUE	1
#define	FALSE	0


///////////////////////////////////////
#define bmBIT0		(1<<0)
#define bmBIT1		(1<<1)
#define bmBIT2		(1<<2)
#define bmBIT3		(1<<3)
#define bmBIT4		(1<<4)
#define bmBIT5		(1<<5)
#define bmBIT6		(1<<6)
#define bmBIT7		(1<<7)
#define bmBIT8		(1<<8)
#define bmBIT9		(1<<9)
#define bmBIT10		(1<<10)
#define bmBIT11		(1<<11)
#define bmBIT12		(1<<12)
#define bmBIT13		(1<<13)
#define bmBIT14		(1<<14)
#define bmBIT15		(1<<15)
#define bmBIT16		(1<<16)
#define bmBIT17		(1<<17)
#define bmBIT18		(1<<18)
#define bmBIT19		(1<<19)
#define bmBIT20		(1<<20)
#define bmBIT21		(1<<21)
#define bmBIT22		(1<<22)
#define bmBIT23		(1<<23)
#define bmBIT24		(1<<24)
#define bmBIT25		(1<<25)
#define bmBIT26		(1<<26)
#define bmBIT27		(1<<27)
#define bmBIT28		(1<<28)
#define bmBIT29		(1<<29)
#define bmBIT30		(1<<30)
#define bmBIT31		(1<<31)



////////////////////////////////////////////////////////
typedef struct {
	uChar t10ms;/* 10ms counter */
	uChar seconds;
	uChar minutes;
	}stTimer ;


////////////////////////////////////////////////////////
typedef struct{
	uChar tim_1mscnt	;/* 5ms timer flag */
	uChar cnt_5ms		;/* 5ms cnt */
	uChar cnt_10ms		;/* 10ms cnt */
	uChar cnt_50ms		;/* 50ms cnt */
	uChar cnt_100ms		;/* 100ms cnt */
	uChar cnt_500ms		;/* 500ms cnt */
	}stBase;


/////////////////////////////////////////////////////
typedef union{
			uShort all; /* little endie byte mode!!! */
			struct{
	#ifdef __BIG_ENDIAN
				uChar PinType;/* hi byte data */
				uChar PinNum;/* lo byte data */
	#else
				uChar PinNum;/* lo byte data */
				uChar PinType;/* hi byte data */
	#endif
				}Byte;
			}PinPty;

///////////////////////////////////////////////////
typedef union{
			uShort all; /* little endie byte mode!!! */
			struct{
	#ifdef __BIG_ENDIAN
				uChar HiByte;/* hi byte data */
				uChar LoByte;/* lo byte data */
	#else
				uChar LoByte;/* lo byte data */
				uChar HiByte;/* hi byte data */
	#endif
				}Byte;
			}stuShort;



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#ifndef IN_NULL
#define IN_NULL			0
#endif

#ifndef FAIL
#define FAIL				0
#endif

#ifndef SUCCESS
#define SUCCESS			!0
#endif





#endif

