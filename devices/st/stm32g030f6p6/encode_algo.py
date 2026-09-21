#!/usr/bin/env python3
"""
STM32G030F6P6 Flash programming algorithm -- Thumb-1 machine code encoder
Cortex-M0+ only supports Thumb-1 (16-bit) instructions

Parameter block layout (at 0x20000000):
  [0x00]: flash_addr   (input, 4B)
  [0x04]: data_addr    (input, 4B)
  [0x08]: word_count   (input, 4B)
  [0x0C]: status       (output: 0=success, non-zero=error)
  [0x10]: FLASH_SR const (0x40022010)
  [0x14]: FLASH_CR const (0x40022014)

Algorithm entry: 0x20000020

Register usage:
  R0 = scratch/return
  R1 = flash_addr
  R2 = data_addr
  R3 = word_count
  R4 = FLASH_SR (0x40022010)
  R5 = FLASH_CR (0x40022014)
  R6 = PARAM_BASE (0x20000000)
  R7 = scratch
"""

# Thumb-1 instruction encodings (ARMv6-M)

def LDR_imm5(Rd, Rn, imm5):
    """LDR Rd, [Rn, #imm5*4] -- 01101 imm5 Rn Rd"""
    assert 0 <= Rd <= 7 and 0 <= Rn <= 7 and 0 <= imm5 <= 31
    return 0x6800 | (imm5 << 6) | (Rn << 3) | Rd

def STR_imm5(Rd, Rn, imm5):
    """STR Rd, [Rn, #imm5*4] -- 01100 imm5 Rn Rd"""
    assert 0 <= Rd <= 7 and 0 <= Rn <= 7 and 0 <= imm5 <= 31
    return 0x6000 | (imm5 << 6) | (Rn << 3) | Rd

def LDR_lit(Rd, off8):
    """LDR Rd, [PC, #off8*4] -- 01001 Rd imm8"""
    assert 0 <= Rd <= 7 and 0 <= off8 <= 255
    return 0x4800 | (Rd << 8) | off8

def MOVS(Rd, imm8):
    """MOVS Rd, #imm8 -- 00100 Rd imm8"""
    assert 0 <= Rd <= 7 and 0 <= imm8 <= 255
    return 0x2000 | (Rd << 8) | imm8

def ADDS_imm3(Rd, Rn, imm3):
    """ADDS Rd, Rn, #imm3 -- 0001110 imm3 Rn Rd"""
    assert 0 <= Rd <= 7 and 0 <= Rn <= 7 and 0 <= imm3 <= 7
    return 0x1C00 | (imm3 << 6) | (Rn << 3) | Rd

def SUBS_imm3(Rd, Rn, imm3):
    """SUBS Rd, Rn, #imm3 -- 0001111 imm3 Rn Rd"""
    assert 0 <= Rd <= 7 and 0 <= Rn <= 7 and 0 <= imm3 <= 7
    return 0x1E00 | (imm3 << 6) | (Rn << 3) | Rd

def CMP_imm8(Rn, imm8):
    """CMP Rn, #imm8 -- 00101 Rn imm8"""
    assert 0 <= Rn <= 7 and 0 <= imm8 <= 255
    return 0x2800 | (Rn << 8) | imm8

def B_short(off8):
    """B label -- 11100 imm11 (signed offset/2 in halfwords from PC)"""
    return 0xE000 | (off8 & 0x7FF)

def BEQ(off8):
    """BEQ label -- 1101 0000 imm8"""
    return 0xD000 | (off8 & 0xFF)

def BNE(off8):
    """BNE label -- 1101 0001 imm8"""
    return 0xD100 | (off8 & 0xFF)

def BMI(off8):
    """BMI label -- 1101 0100 imm8"""
    return 0xD400 | (off8 & 0xFF)

def ORRS(Rd, Rm):
    """ORRS Rd, Rd, Rm -- 0100001100 Rm Rd"""
    return 0x4300 | (Rm << 3) | Rd

def BICS(Rd, Rm):
    """BICS Rd, Rm -- 0100001110 Rm Rd (Rd &= ~Rm)"""
    return 0x4380 | (Rm << 3) | Rd

def ANDS(Rd, Rm):
    """ANDS Rd, Rm -- 0100000000 Rm Rd (flags set)"""
    return 0x4000 | (Rm << 3) | Rd

def LSLS_imm5(Rd, Rm, imm5):
    """LSLS Rd, Rm, #imm5 -- 00000 imm5 Rm Rd"""
    return (imm5 << 6) | (Rm << 3) | Rd

def BKPT(imm8):
    """BKPT #imm8 -- 10111110 imm8"""
    return 0xBE00 | imm8

def NOP():
    return 0xBF00

# ============================================================
# Build instruction sequence
# ============================================================
BASE = 0x20000020  # code start address
hw = []             # half-word list

def addr_of(index):
    """Return address of the index-th halfword"""
    return BASE + index * 2

# --- Prologue: load constants ---
# LDR R6, [PC, #lit_off] -- load PARAM_BASE from literal pool
idx_load_r6 = len(hw)
hw.append(LDR_lit(6, 0))  # placeholder

# LDR R1, [R6, #0]   -- flash_addr
hw.append(LDR_imm5(1, 6, 0))
# LDR R2, [R6, #4]   -- data_addr
hw.append(LDR_imm5(2, 6, 1))
# LDR R3, [R6, #8]   -- word_count
hw.append(LDR_imm5(3, 6, 2))
# LDR R4, [R6, #16]  -- FLASH_SR const (param+0x10)
hw.append(LDR_imm5(4, 6, 4))
# LDR R5, [R6, #20]  -- FLASH_CR const (param+0x14)
hw.append(LDR_imm5(5, 6, 5))

# === LOOP ===
idx_loop = len(hw)

# CMP R3, #0
hw.append(CMP_imm8(3, 0))
# BEQ done (placeholder)
idx_beq_done = len(hw)
hw.append(0)

# === wait_bsy ===
idx_wait1 = len(hw)
# LDR R7, [R4, #0]
hw.append(LDR_imm5(7, 4, 0))
# LSLS R7, R7, #15  -- check bit16 (BSY)
hw.append(LSLS_imm5(7, 7, 15))
# BMI wait_bsy
delta = idx_wait1 - (len(hw) + 1)
hw.append(BMI(delta & 0xFF))

# === Set PG bit ===
# LDR R7, [R5, #0]
hw.append(LDR_imm5(7, 5, 0))
# MOVS R0, #1
hw.append(MOVS(0, 1))
# ORRS R7, R7, R0
hw.append(ORRS(7, 0))
# STR R7, [R5, #0]
hw.append(STR_imm5(7, 5, 0))

# === Write first word ===
# LDR R7, [R2, #0]
hw.append(LDR_imm5(7, 2, 0))
# STR R7, [R1, #0]
hw.append(STR_imm5(7, 1, 0))
# ADDS R1, #4
hw.append(ADDS_imm3(1, 1, 4))
# ADDS R2, #4
hw.append(ADDS_imm3(2, 2, 4))

# === Write second word (STM32G0 double-word programming) ===
# LDR R7, [R2, #0]
hw.append(LDR_imm5(7, 2, 0))
# STR R7, [R1, #0]
hw.append(STR_imm5(7, 1, 0))
# ADDS R1, #4
hw.append(ADDS_imm3(1, 1, 4))
# ADDS R2, #4
hw.append(ADDS_imm3(2, 2, 4))

# === wait_bsy2 ===
idx_wait2 = len(hw)
# LDR R7, [R4, #0]
hw.append(LDR_imm5(7, 4, 0))
# LSLS R7, R7, #15
hw.append(LSLS_imm5(7, 7, 15))
# BMI wait_bsy2
delta = idx_wait2 - (len(hw) + 1)
hw.append(BMI(delta & 0xFF))

# === Check errors: SR & 0xF8 ===
# LDR R7, [R4, #0]
hw.append(LDR_imm5(7, 4, 0))
# MOVS R0, #0xF8
hw.append(MOVS(0, 0xF8))
# ANDS R7, R7, R0  -- R7 = SR & 0xF8, flags set
hw.append(ANDS(7, 0))
# BNE error (placeholder)
idx_bne_error = len(hw)
hw.append(0)

# === Clear PG bit ===
# LDR R7, [R5, #0]
hw.append(LDR_imm5(7, 5, 0))
# MOVS R0, #1
hw.append(MOVS(0, 1))
# BICS R7, R0  -- R7 &= ~1
hw.append(BICS(7, 0))
# STR R7, [R5, #0]
hw.append(STR_imm5(7, 5, 0))

# SUBS R3, R3, #2
hw.append(SUBS_imm3(3, 3, 2))

# B loop
delta = idx_loop - (len(hw) + 1)
hw.append(B_short(delta & 0x7FF))

# === DONE ===
idx_done = len(hw)
# Backfill BEQ done
beq_delta = idx_done - (idx_beq_done + 1)
hw[idx_beq_done] = BEQ(beq_delta & 0xFF)

# MOVS R0, #0
hw.append(MOVS(0, 0))
# STR R0, [R6, #12]  -- status = 0
hw.append(STR_imm5(0, 6, 3))
# B common_exit (placeholder, backfilled after common_exit is placed)
idx_b_done_to_exit = len(hw)
hw.append(0)

# === ERROR ===
idx_error = len(hw)
# Backfill BNE error
bne_delta = idx_error - (idx_bne_error + 1)
hw[idx_bne_error] = BNE(bne_delta & 0xFF)

# STR R7, [R6, #12]  -- status = error_code
hw.append(STR_imm5(7, 6, 3))
# STR R7, [R4, #0]   -- FLASH_SR = R7 (clear error flags)
hw.append(STR_imm5(7, 4, 0))
# Load LOCK bit and set it
# LDR R0, [PC, #lit_off] -- 0x80000000
idx_lit_lock = len(hw)
hw.append(LDR_lit(0, 0))  # placeholder
# LDR R7, [R5, #0]
hw.append(LDR_imm5(7, 5, 0))
# ORRS R7, R7, R0
hw.append(ORRS(7, 0))
# STR R7, [R5, #0]
hw.append(STR_imm5(7, 5, 0))
# Fall through to common_exit

# === common_exit: single BKPT (both DONE and ERROR converge here) ===
idx_common_exit = len(hw)
hw.append(BKPT(0))

# Backfill DONE's B common_exit
b_delta = idx_common_exit - (idx_b_done_to_exit + 1)
hw[idx_b_done_to_exit] = B_short(b_delta & 0x7FF)

# === Literal pool (4-byte aligned) ===
while len(hw) % 2 != 0:
    hw.append(NOP())

lit_start = len(hw)

# [lit0] 0x20000000 (PARAM_BASE) -- for R6
idx_lit_param = len(hw)
hw.append(0x0000)
hw.append(0x2000)

# [lit1] 0x80000000 (LOCK bit)
idx_lit_lock_val = len(hw)
hw.append(0x0000)
hw.append(0x8000)

# --- Backfill LDR_lit offsets ---
def calc_lit_offset(pc, lit_hw_idx):
    """Calculate LDR_lit offset from PC to literal pool entry"""
    pc_aligned = (pc + 4) & ~0x3  # (PC+4) word-aligned
    lit_addr = addr_of(lit_hw_idx)
    assert lit_addr % 4 == 0, f"Literal at {lit_addr:08X} not word-aligned"
    off = (lit_addr - pc_aligned) // 4
    assert 0 <= off <= 255, f"Offset {off} out of range for LDR_lit"
    return off

# Backfill idx_load_r6 (LDR R6, =0x20000000)
off = calc_lit_offset(addr_of(idx_load_r6), idx_lit_param)
hw[idx_load_r6] = LDR_lit(6, off)

# Backfill idx_lit_lock (LDR R0, =0x80000000)
off = calc_lit_offset(addr_of(idx_lit_lock), idx_lit_lock_val)
hw[idx_lit_lock] = LDR_lit(0, off)

# ============================================================
# Output C array
# ============================================================
print('// ================================================================')
print('// STM32G030F6P6 Flash double-word programming algorithm (Thumb-1)')
print('// Entry: PC = 0x20000020')
print('// Param block 0x20000000:')
print('//   [0x00]: flash_addr (in)')
print('//   [0x04]: data_addr  (in)')
print('//   [0x08]: word_count (in)')
print('//   [0x0C]: status     (out: 0=OK, non-zero=error_code)')
print('//   [0x10]: FLASH_SR   (const: 0x40022010)')
print('//   [0x14]: FLASH_CR   (const: 0x40022014)')
print('// Common BKPT at 0x200000?? -- set FP_COMP0 to this address')
print('// ================================================================')
print('')
print(f'// Code: {lit_start} half-words = {lit_start*2} bytes')
print(f'// Literal pool: {len(hw)-lit_start} half-words = {(len(hw)-lit_start)*2} bytes')
print(f'// Total size: {len(hw)*2} bytes')
print(f'// Common BKPT address: 0x{addr_of(idx_common_exit):08X}')
print('')
print('const DWORD stm32g0_prog_algo[] =')
print('{')

for i in range(0, len(hw), 2):
    lo = hw[i]
    hi = hw[i+1] if i+1 < len(hw) else 0
    dword = lo | (hi << 16)
    print(f'    0x{dword:08X}L,  // [{i*2:3d}] 0x{lo:04X} 0x{hi:04X}')

print('};')

# Output parameter block init constants
print('')
print('// Parameter block init (write to 0x20000000)')
print('const DWORD stm32g0_param_init[] =')
print('{')
print('    0x00000000L,  // [0x00] flash_addr (filled by host)')
print('    0x00000000L,  // [0x04] data_addr  (filled by host)')
print('    0x00000000L,  // [0x08] word_count (filled by host)')
print('    0x00000000L,  // [0x0C] status (output by algorithm)')
print('    0x40022010L,  // [0x10] FLASH_SR')
print('    0x40022014L,  // [0x14] FLASH_CR')
print('};')
