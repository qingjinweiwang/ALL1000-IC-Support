
////////////////////////////////////////////////////////////////////////
// STMicroelectronics STM32G030F6P6 (TSSOP20)
// Cortex-M0+, 32KB Flash, 8KB SRAM
////////////////////////////////////////////////////////////////////////
[0]
.special.
GMessage:
GOption[ 0x1FFF7800+Offset, 31~0 bit ]:

G--------------------------------------------------------------------------------------------------------
G00, OPTR
t0000L04OPTR       
G--------------------------------------------------------------------------------------------------------
G18, WRP1AR
t0008L04WRP1AR  
G--------------------------------------------------------------------------------------------------------
G20, WRP1BR
t000CL04WRP1BR  
G--------------------------------------------------------------------------------------------------------
GNote : FLASH_OPTR Byte0 [ RDP ]
G  0xAA: Level 0, no read protection
G  Others: Level 1, memories read protection active
G  0xCC: Level 2, chip read protection active (irreversible)
G--------------------------------------------------------------------------------------------------------
GNote : WRP1AR / WRP1BR [ 31:0 ]
G  STM32G030F6P6 effective WRP fields: WRP1x_STRT = bits [4:0], WRP1x_END = bits [20:16]
G  Only START=0x1F, END=0 is accepted as no write protection
G  STM32G030F6P6 full 32KB protection in one area: START=0, END=15, value=0x000F0000
G  STM32G030F6P6 no protection: START=0x1F, END=0, value=0x0000001F
G  Driver rejects RDP 0xCC (irreversible Level 2).
G  RDP 0xAA is Level 0; all other accepted RDP values are Level 1.
G  WRP ranges must use pages 0~15, or disabled form START=0x1F END=0.
G  Full-chip WRP, including combined Area A + Area B coverage, is rejected.
