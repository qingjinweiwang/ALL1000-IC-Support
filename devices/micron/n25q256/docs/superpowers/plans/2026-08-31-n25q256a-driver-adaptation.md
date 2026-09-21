# N25Q256A Driver Adaptation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the TH25Q80HB template with a buildable N25Q256Ax3ESFxxx extended-SPI driver covering the full 32MB main array.

**Architecture:** Initialize common four-byte address mode once at power-on, then route every main-array address through one `Send4ByteAddress()` helper. Keep the fixed 13-callback ABI while simplifying status handling to one byte and using Flag Status Register checks for modifying operations.

**Tech Stack:** ARM RVCT C, ALL-1000 firmware ABI tables, SPI command protocol, SPC configuration, `iBuild.bat`.

## Global Constraints

- Support Micron N25Q256Ax3ESFxxx at 2.7V to 3.6V in extended SPI.
- Main-array range is exactly `00000000h` through `01FFFFFFh`.
- JEDEC ID is exactly `20h-BAh-19h`.
- Do not enable dual, quad, DTR, XIP, OTP, or sector Lock Register features.
- Preserve the fixed callback table and unrelated formatting.

---

### Task 1: Convert device metadata and programmer tables

**Files:**
- Modify: `WJx4_N25Q256Ax3ESFxxx.h`

**Interfaces:**
- Produces: `Block_N25Q256Ax3ESFxxx`, `AlgoIdx_N25Q256A`, `BlkIdx_N25Q256Ax3ESFxxx`, protection mask `0x007C`, default status `0x00`.

- [ ] Replace TH25Q names, ID constants, 1MB range, three Security blocks, and two-byte status block with one 32MB main block, one one-byte status block, and the control pseudo-block.
- [ ] Set page size to `256` and bulk erase timeout to `500` seconds.
- [ ] Preserve the existing spec and pin-map tables.
- [ ] Scan the header for `TH25`, `Security Register`, `CMP`, `LB1`, `ReadStatus1`, and `0FFFFF`; expect no matches.

### Task 2: Add N25Q256A commands and four-byte addressing

**Files:**
- Modify: `WJx4_N25Q256Ax3ESFxxx.c`

**Interfaces:**
- Produces: `void Send4ByteAddress(uInt32 Address)`, `uChar Enter4ByteAddressMode(void)`, `uChar ReadFlagStatusData(void)`, `void ClearFlagStatusData(void)`.

- [ ] Fix the include to `WJx4_N25Q256Ax3ESFxxx.h`.
- [ ] Define `B7h`, `70h`, and `50h`; delete `35h`, `48h`, `42h`, and `44h` TH25Q definitions.
- [ ] Implement address transmission as:

```c
PtrData(OneDataCmd) = MSB(MSW(Address)) ;
SPIRWDelay() ;
PtrData(OneDataCmd) = LSB(MSW(Address)) ;
SPIRWDelay() ;
PtrData(OneDataCmd) = MSB(LSW(Address)) ;
SPIRWDelay() ;
PtrData(OneDataCmd) = LSB(LSW(Address)) ;
SPIRWDelay() ;
```

- [ ] Enter four-byte mode during `PowerOnFun()` with WREN/B7/WRDI and return `E_PowerOnFail` on WEL failure if that status code exists; otherwise return `FAIL` without inventing a new ABI symbol.
- [ ] Replace every main-array and serial-number three-address-byte sequence with `Send4ByteAddress(DeviceAddress)`.

### Task 3: Remove Security/SR2 and adapt protection

**Files:**
- Modify: `WJx4_N25Q256Ax3ESFxxx.c`

**Interfaces:**
- Produces: `uChar ReadStatusData(void)` and one-byte protect/unprotect flows.

- [ ] Delete every `bBlkAttri_Res15` branch and Security Register control macro.
- [ ] Delete `CheckAllStatus1Data`, `SR2Data`, `PrtReg1DefaultBit`, CMP, and LB logic.
- [ ] Change status-block dispatch from `Noun_Block4` to `Noun_Block1`.
- [ ] Make `ProtectFun()` and `UnProtectFun()` issue `WREN`, `01h`, and exactly one data byte masked by `0x7C`.
- [ ] Keep `SecureFun()` as:

```c
uChar SecureFun(void)
{
    return SUCCESS ;
}
```

### Task 4: Add reliable program/erase status checks

**Files:**
- Modify: `WJx4_N25Q256Ax3ESFxxx.c`

**Interfaces:**
- Consumes: `ReadFlagStatusData()` and `ClearFlagStatusData()` from Task 2.
- Produces: program and erase paths that reject N25Q Flag Status error bits.

- [ ] Clear stale Flag Status before page program and bulk erase.
- [ ] After WIP clears, read Flag Status and map program/protection errors to existing program failure codes and erase/protection errors to `E_EraseFail`.
- [ ] Reject bulk erase whenever `ReadStatusData() & 0x7C` is nonzero.
- [ ] Use the 500-second timeout from the header table.

### Task 5: Synchronize SPC and verify

**Files:**
- Modify: `WJx4_N25Q256Ax3ESFxxx.spc`

**Interfaces:**
- Produces: status byte at SRAM offset 0 and operation control byte at SRAM offset 3.

- [ ] Replace the TH25Q groups with BP0 `04h`, BP1 `08h`, BP2 `10h`, TB `20h`, BP3 `40h`, and status verify/blank-check `80h` at offset 3.
- [ ] Scan `.c`, `.h`, and `.spc` for TH25Q names, Security commands, SR2/CMP/LB, three-byte address-send comments, and 1MB ranges; expect no matches.
- [ ] Run `iBuild.bat WJx4_N25Q256Ax3ESFxxx`; expect `[SUCCESS] Build successful.` and updated BIN/SPC files.

