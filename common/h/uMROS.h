
/*****************************************************************************/
/*****************************************************************************/
#ifndef uMROS_H_
#define uMROS_H_

#include "String.h"
#include "Type.h"

/*---------------------------------------------------------------------
   Customizable Parameters
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Make sure all parameters here are customized to your needs.
 ---------------------------------------------------------------------*/
#define MAX_TASK_NUMBER 		8
#define USE_BYTE_1TASK			9

#define FreeMutex				0x00/* task is in idle state */
#define FWFPGAMutex			0xFF/* perform FW & FPGA configuration setting */
#define FWTestMutex				0xFF/* perform FW selftesting functions */
#define FWCheckMutex			0xFF/* perform checking before algo functions */
#define AlgoMutex				0xFF/* perform 13 algo functions */
#define FWPCMutex				0xFF/* perform FW<-->PC functions */
#define FWGangMutex			0xFF/* perform FW<-->Gang functions */
#define RESERVE_MUTEX6			0xFF
#define RESERVE_MUTEX7			0xFF

#define BeepMutex				0xFF
#define AllMutex					0xFF

/*---------------------------------------------------------------------
   Stationary Parameters
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   All parameters here can't modified ad arbitrium.
 ---------------------------------------------------------------------*/
#define TERMINATE				0xFF
#define TERMINATE1				0xFE

#define ENDED					1
#define WAITING					!ENDED

/*****************************************************************************\
			Constant Definitions
\*****************************************************************************/
#define OS_NULL					0
#define OS_IDLE					0
#define OS_RUNNING				1
#define OS_READY				2
#define OS_WAITING				3
#define OS_SUSPEND				4


/* --------------------------------------------------------------------
   Task Structure
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   This control structure defines a task.  When a task is suspended,
   all of the registers including instruction address, flags, etc...
   are stored in the task's stack and the suspended status bit is set.
   The suspended status bit indicates that the stack contains registers
   to pop before starting the task.
---------------------------------------------------------------------- */
typedef union
{
	unsigned char bytes[USE_BYTE_1TASK];
	struct
	{
/*BYTE0:*/		unsigned char stat:6;	/* task's running state */
				unsigned char wait:1;	/* waiting for some signal */
				unsigned char term:1;	/* in terminating status */
/*BYTE1:*/		unsigned char mutex;	/* mutual exclusion */
/*BYTE2:*/		unsigned char id;		/* id:1~255 */
/*BYTE3:*/		unsigned char ops;	/* task operations:0~31*/
/*BYTE4,5:*/		unsigned short in;		/* input parameter */
/*BYTE6,7:*/		unsigned short tmr;	/* timer */
/*BYTE8:*/		unsigned char cnt;		/* counter */
	}mem;
}Ttask;
/*-------------------------------------------------------------------- */

typedef struct
{
	unsigned char sptr;/* suspended pointer */
	unsigned char wptr;/* waiting pointer */
	unsigned char rptr;/* ready pointer */
	unsigned char id;/* current task's id */
	unsigned char err;/* error clew: bit0---task stack overflow */
	Ttask	task[MAX_TASK_NUMBER];
}TuMROS;

extern TuMROS uMROS;

#define  operation		(uMROS.task[0].mem.ops)
#define  GoTo(_x) 		(operation=_x)


/*01*/extern void OSScheduler(void);
/*02*/extern void OSTasksClear(void);
/*03*/extern void OSTaskCreate(unsigned char id,unsigned char mutex,unsigned short in);
/*04*/extern unsigned char OSTaskSearch(unsigned char tsk1,unsigned char tsk2 );
/*05*/extern void OSTaskTerminate(unsigned char tsk1,unsigned char tsk2 );
/*06*/extern void OSTaskExecute(unsigned char status);/*execute task*/
/*07*/extern void OSDelayStart(unsigned short ticks);
/*08*/extern void OSWaitingStart(unsigned short ticks);
/*09*/extern void OSTimerUpdating(void);/* Note:Put it into timer ISR(recommend:1ms) */
/*10*/extern void OSSetParameter(unsigned short in);
/*11*/extern unsigned short OSGetParameter(void);
/*12*/extern void OSSetCounter(unsigned char cnt);
/*13*/extern unsigned char OSGetCounter(void);
/*14*/extern unsigned char OSWaitTimeEnd(void);
/*15*/extern unsigned char OSWaitSignalEnd(uChar signal);
/*16*/extern unsigned char OSWaitTimeOrSignalEnd(uChar signal);
/*17*/extern unsigned char OSWaitTimeAndSignalEnd(uChar signal);

//////////////////////////////////////////////////////////

#endif

