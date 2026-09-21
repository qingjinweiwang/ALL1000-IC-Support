


_ISR_STARTADDRESS	EQU		0x33FFFF00

RAM_BL_STK_STR		EQU		0x33F0F000
RAM_BL_STK_END		EQU		0x33F10000 ;/* 0x33F0F000~0x33F10000 4K Byte for BootLoader stack */
top_of_stacks		EQU		RAM_BL_STK_END

MMU_TBL_BASE		EQU		0x33FF0000 ;/* 0x33FF0000~0x33FF4000=16K area for MMU Table */

; --- Amount of memory (in bytes) allocated for stacks
Len_FIQ_Stack    EQU     256
Len_IRQ_Stack    EQU     256
Len_ABT_Stack    EQU     128
Len_UND_Stack    EQU     128
Len_SVC_Stack    EQU     1024
Len_SYS_Stack    EQU     2048


; Offsets will be loaded as immediate values.
; Offsets must be 8 byte aligned.
Offset_FIQ_Stack         EQU     0
Offset_IRQ_Stack         EQU     (Offset_FIQ_Stack + Len_FIQ_Stack)
Offset_ABT_Stack         EQU     (Offset_IRQ_Stack + Len_IRQ_Stack)
Offset_UND_Stack         EQU     (Offset_ABT_Stack + Len_ABT_Stack)
Offset_SVC_Stack         EQU     (Offset_UND_Stack + Len_UND_Stack)
Offset_SYS_Stack         EQU     (Offset_SVC_Stack + Len_SVC_Stack)


; --- Standard definitions of mode bits and interrupt (I & F) flags in PSRs
Mode_USR        EQU     0x10
Mode_FIQ        EQU     0x11
Mode_IRQ        EQU     0x12
Mode_SVC        EQU     0x13
Mode_ABT        EQU     0x17
Mode_UND        EQU     0x1B
Mode_SYS        EQU     0x1F ; available on ARM Arch 4 and later

I_Bit           EQU     0x80 ; when I bit is set, IRQ is disabled
F_Bit           EQU     0x40 ; when F bit is set, FIQ is disabled
NOINT           EQU     0xC0 ; 1100 0000  





;======================================================================================================
;	Fin = 12MHz,
;
;	MPLLout = (m x Fin)/(p x 2^s), m=MDIV, p=PDIV, s=SDIV, Fin=10~30MHz
;	(350,3,2)=350Mhz, (400,3,2)=400Mhz, (225,3,1)=450Mhz, (250,3,1)=500Mhz, (267,3,1)=534Mhz
;	(400,3,1)=800Mhz
;
;	EPLLout = ((m + K/2^16)xFin)/(p x 2^s), m=MDIV, p=PDIV, s=SDIV, K=KDIV, Fin=10~40MHz
;	(48,1,4)=36Mhz, (32,1,3)=48Mhz, (40,1,3)=60Mhz, (48,1,3)=72Mhz, (28,1,2)=84Mhz, (32,1,2)=96Mhz
;======================================================================================================
;	When you intend to change CLK setup, you should consider a few factors to operate S3C2443 correctly
;	1. Setup Startup_MPLL, 
;		Startup_Mdiv, 
;		Startup_Pdiv, 
;		Startup_Sdiv, 
;		Startup_ARMCLKdiv, 
;		Startup_PREdiv,
;		Startup_HCLKdiv,
;		Startup_PCLKdiv
;	2. Define mSDR or mDDR
;	3. Setup tRAS, tARFC, CAS, tRCD, tRP, REFCYC parameters according to HCLK, refer to table in this code
;=====================================================================================		
;In  	M   	P   	S    	MPLL       
;12	267	3	1	534.000 	45%~55%
;12	400	3	1	800.000 	40%~60%


FIN			EQU	12000000


Startup_Mdiv		EQU 	400	;400/* set MPLL to 400MHz */
Startup_Pdiv		EQU 	3
Startup_Sdiv		EQU 	1

Startup_EMdiv		EQU 	32	;/* set EPLL to 96MHz */
Startup_EPdiv		EQU 	1
Startup_ESdiv		EQU 	2


Startup_ARMCLKdiv	EQU	1	;	0	 :	ARMCLK	= MPLL/1
					;	1	 :	ARMCLK	= MPLL/2
					;	2	 :	ARMCLK	= MPLL/3
					;	3	 :	ARMCLK	= MPLL/4
					;	5	 :	ARMCLK	= MPLL/6
					;	7	 :	ARMCLK	= MPLL/8
					;	13	 :	ARMCLK	= MPLL/12
					;	15	 :	ARMCLK	= MPLL/16

; PREDiv clock should be less than 266MHz, so divider MUST >=1
Startup_PREdiv		EQU	2	;	0  :	PREDIV_CLK	= MPLL
					;	1 :	PREDIV_CLK	= MPLL/2
					;	2 :	PREDIV_CLK	= MPLL/3
					;	3 :	PREDIV_CLK	= MPLL/4
										
					;  	HCLKdiv for USB Device speed,DMA,Memory Bankx!!
Startup_HCLKdiv		EQU	1	;	0  :	HCLK	= PREDIV_CLK
					;	0x1 :	HCLK	= PREDIV_CLK/2
					;	0x3 :	HCLK	= PREDIV_CLK/4

					;  	PCLKdiv for GPIO speed
Startup_PCLKdiv		EQU	1	;	0	 :	PCLK	= HCLK
					;	1	 :	PCLK	= HCLK/2


;---------------------------------------
; BANKCFG register  : DDR2 SDRAM configure
;---------------------------------------
RASBW0			EQU	2	;	RAS addr 00=11bit,01-12bit,10=13bit, 11=14bit
RASBW1			EQU	2	;	RAS addr 00=11bit,01-12bit,10=13bit, 11=14bit
CASBW0			EQU	2	;	CAS addr 00=8bit,01-9bit,10=10bit, 11=11bit
CASBW1			EQU	2	;	CAS addr 00=8bit,01-9bit,10=10bit, 11=11bit
ADDRCFG0		EQU	1	; 	addre configure
					;   	00={BA,RAS,CAS}, 01={RAS,BA,CAS}

ADDRCFG1		EQU	1	; 	addre configure
					;   	00={BA,RAS,CAS}, 01={RAS,BA,CAS}

MEMCFG			EQU	1	; 	Ext.Mem 000=SDR,010=MSDR,100=DDRz,110=MDDR,001=DDR2
BW			EQU	1	; 	Bus width 00=32bit,01=16bit


;---------------------------------------
; BANKCON1 register : DDR2 SDRAM timing control
;---------------------------------------
PADLOAD			EQU	1
BStop			EQU	0	;	read burst stop control
WBUF			EQU	1	;	write buffer control
AP			EQU	0	;	auto precharge control
PWRDN			EQU	0	;	power down mode
DQSDelay		EQU	4	;	DQS delay

;-----------------------[DDR2 SDRAM]--------------------------
;    HCLK	tRAS	tARFC	CAS	tRCD	tRP	REFCYC
;---------------------------------------------------------------
;18MHz		0	1	3	0	0	140
;36MHz		1	2	3	0	0	280
;100Mhz		4	7	3	2	2	780
;110Mhz		4	8	3	2	2	858
;120Mhz		5	9	3	2	2	936
;133Mhz		5	10	3	2	2	1037		
;---------------------------------------------------------------

;---------------------------------------
; BANKCON2 register : DDR2 SDRAM timing control
;---------------------------------------
tRAS			EQU	5	;	Row active time
tARFC			EQU	13	;	Row cycle time
CL			EQU	3	;	CAS latency control
tRCD			EQU	1	;	RAS to CAS delay
tRP			EQU	1	; 	Row pre-charge time


;---------------------------------------
; BANKCON3 register : DDR2 SDRAM MRS/EMRS register
;---------------------------------------
BA_EMRS1		EQU	1	;	BA : EMRS
DLL_DIS			EQU	1
DLL_EN			EQU	0
nDQS_DIS		EQU	1
RDQS_DIS		EQU	0
OCD_MODE_EXIT		EQU	0
OCD_MODE_DEFAULT	EQU	7
BA_EMRS2		EQU	2	;	BA : EMRS
BA_EMRS3		EQU	3	;	BA : EMRS
DS			EQU	0	;	Driver strength
PASR			EQU	0	;	PASR
BA_MRS			EQU	0	;	BA : MRS
TM			EQU	0	; 	Test Mode - mode register set
CL_MRS			EQU	3	; 	CAS Latency
DLL_RESET_HIGH		EQU	1
DLL_RESET_LOW		EQU	0

;---------------------------------------
; REFRESH register : DDR2 SDRAM refresh register
;---------------------------------------
REFCYC			EQU	1037	; 	refresh cycle
;---------------------------------------------------------------------------
					




	END
