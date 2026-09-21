

#ifndef FPGADrv_H_
#define  FPGADrv_H_

#include "Type.h"

/******************************************************************\
		FPGA configuration port definition with CPU

\******************************************************************/







extern INT32 FPGACFGCnt ; /* buffer counter for FPGA CFG */
#define	FPGACFGLEN		(0x71544) /* total 464196 bytes = 907 sectors */
#define	FPGACFGSecCnt	(907)


extern void FpgaCfgPortSetting(void) ;
extern void GetFPGACFGDataFromUSB(uChar *pFPGACode) ;


#endif


