/*****************************************
 NAME: MMU.H
 DESC: MMU Header
 HISTORY:
 2008.02.22 : ver 0.0
******************************************/

#ifndef __MMUCACHE_H__
#define __MMUCACHE_H__

#ifdef __cplusplus
extern "C" {
#endif



//------------------------------
// in MMUCache_asm.s
//------------------------------
// CPSR I,F bit
int SET_IF(void);
void WR_IF(int cpsrValue);
void CLR_IF(void);
// MMU Cache/TLB/etc on/off functions
void MMU_EnableICache(void);
void MMU_DisableICache(void);
void MMU_EnableDCache(void);
void MMU_DisableDCache(void);
void MMU_EnableMMU(void);
void MMU_DisableMMU(void);
void MMU_WaitForInterrupt(void);
// Process ID
void MMU_SetProcessId(U32 pid);

void Init_MMU_Reg(void) ;


#ifdef __cplusplus
}
#endif

#endif /*__MMUCACHE_H__*/
