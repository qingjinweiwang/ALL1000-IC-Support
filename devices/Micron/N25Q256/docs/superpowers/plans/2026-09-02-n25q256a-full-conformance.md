# N25Q256A Full Driver Conformance Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Correct the N25Q256A driver to use verified EAR-based three-byte addressing, its one-byte Status Register, Micron OTP commands, and operation-specific Flag Status checks.

**Architecture:** Centralize three-byte address transmission, verified EAR selection, FSR handling, and OTP access in small internal helpers. Route every main-array access through the address helpers while preserving the existing ALL-1000 callbacks, tables, buffer accounting, and four-socket error handling.

**Tech Stack:** ARM RVCT C, ALL-1000 firmware ABI, FPGA gang-SPI registers, Micron N25Q256A Rev. X extended-SPI command set, Windows batch build.

## Global Constraints

- Main-array logical range remains `00000000h-01FFFFFFh`.
- Never send `B7h` or `E9h` and never rewrite NVCR.
- Require FSR bit 0 to indicate three-byte mode at power-on.
- Use `C5h/C8h` EAR bit 0 for lower/upper 16MB selection.
- OTP uses `4Bh/42h`, exactly three address bytes, and no EAR selection.
- Status Register uses only `05h/01h` with one data byte and protection mask `0x7C`.
- Preserve the 13-callback ABI, block table layout, pin map, voltage table, and partial-gang behavior.
- Do not activate sector or subsector erase paths.

---

### Task 1: Add N25Q256A Register and Address Helpers

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`

**Interfaces:**
- Produces: `Send3ByteAddress(uInt32 Address)`
- Produces: `CheckAllFlagStatusData(uChar DevData)` and `ClearFlagStatusData(void)`
- Produces: `CheckAllExtendedAddressData(uChar DevData)`, `SetExtendedAddress(uChar Bank)`, and `SelectAddressBank(uInt32 Address)`
- Produces: cached `EARBankState`, where `0xFF` means invalid

- [ ] **Step 1: Record incompatible symbols**

Run:

```powershell
rg -n "ReadStatus1Cmd|CheckAllStatus1Data|ProgramSecurityCmd|ReadSecurityCmd|EraseSecurityCmd|StatusLBMaskBit|FlagStatusErrorMask|WriteExtAddrRegCmd|ReadExtAddrRegCmd" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
```

Expected: the copied TH/GD symbols and the partial inline EAR implementation are present.

- [ ] **Step 2: Replace command and mask definitions**

Keep `05h`, `01h`, `70h`, `50h`, `C5h`, and `C8h`. Add `ReadOTPArrayCmd 0x4B`, `ProgramOTPArrayCmd 0x42`, `FlagStatusProgramErrorMask 0x12`, `FlagStatusEraseErrorMask 0x22`, `FlagStatusAddrModeMask 0x01`, `OTPDataSize 64`, and `OTPControlAddr 0x40`. Remove TH/GD security/status-2 symbols and the combined `0x3A` error mask.

- [ ] **Step 3: Implement common helpers**

Use the existing `PtrData`, `SPIRWDelay`, `GClrPageChkStatus`, and `GChkAllSckStatus` pattern. `Send3ByteAddress` sends exactly:

```c
PtrData(OneDataCmd) = LSB(MSW(Address)) ;
SPIRWDelay() ;
PtrData(OneDataCmd) = MSB(LSW(Address)) ;
SPIRWDelay() ;
PtrData(OneDataCmd) = LSB(LSW(Address)) ;
SPIRWDelay() ;
```

`SetExtendedAddress` sets `EARBankState = 0xFF`, performs WREN, writes masked bank bit with `C5h`, verifies bit 0 using `C8h`, and sets the cache only after all surviving sockets match. `SelectAddressBank` calls it only when `(Address >> 24) & 1` differs from the cached bank.

- [ ] **Step 4: Enforce the power-on premise**

After power rails stabilize, select all passing sockets, invalidate EAR, compare `70h` bit 0 with zero, then set and verify EAR bank 0. Return `E_DeviceInitialFail` on failure.

- [ ] **Step 5: Compile-check Task 1**

Run `cmd /c iBuild.bat` in the driver directory. Expected: helper definitions compile with no implicit declarations or unused static state.

### Task 2: Route Main-Array and Serial-Number Access Through EAR

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`

**Interfaces:**
- Consumes: `SelectAddressBank()` and `Send3ByteAddress()`
- Preserves: `ReadFun`, `ProgramFun`, `VerifyFun`, `BlankCheckFun`, and `DevSNProgAndVerify`

- [ ] **Step 1: Replace inline ReadFun EAR code**

Delete the duplicated C5h/C8h branches. Call `SelectAddressBank(DeviceAddress)` before `03h`, use `Send3ByteAddress`, increment `DeviceAddress` with each packet, and restart the read transaction after crossing `01000000h` when data remains.

- [ ] **Step 2: Correct page programming**

For every nonblank main-array page, call `SelectAddressBank(DeviceAddress)`, clear FSR, then issue the page-specific WREN and `02h + Send3ByteAddress`. After WIP clears, compare FSR bits `0x12` against zero before continuing.

- [ ] **Step 3: Correct verify and blank check**

Select the initial bank and use `Send3ByteAddress`. Increment `DeviceAddress` after every 256-byte group. If more data remains and the low 24 bits become zero, close CS, select the next bank, and reissue `03h` without double-advancing any buffer counter.

- [ ] **Step 4: Correct serial-number page access**

Select the page bank before both reads. Before page program, select the bank first, then issue WREN. Use `Send3ByteAddress` in all three transactions and preserve the existing one-page overflow rejection.

- [ ] **Step 5: Inspect routing**

Run:

```powershell
rg -n -B6 -A10 "SelectAddressBank|Send3ByteAddress|DeviceAddress \+=" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
```

Expected: every active main-array address transaction is routed through both helpers and long operations restart at the 16MB boundary.

### Task 3: Replace TH/GD Protection With the N25Q256A Status Register

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h`

**Interfaces:**
- Preserves: `ReadStatusData`, `ProtectFun`, and `UnProtectFun`
- Removes: all status-register-2 and LB/CMP helpers

- [ ] **Step 1: Make status access one byte**

Change `ReadStatusData` to return the `05h` byte only. Remove `ReadStatus1Cmd`, `CheckAllStatus1Data`, and every high-byte status read or comparison.

- [ ] **Step 2: Make protection writes one byte**

For protect and unprotect, read current SR, merge only mask `0x7C`, issue WREN, send `01h` plus one byte, poll WIP, then verify the masked `05h` result. Preserve SRWD and do not transmit a second byte.

- [ ] **Step 3: Correct erase protection and FSR checks**

Reject bulk erase when `(ReadStatusData() & 0x7C) != 0`. Otherwise clear FSR, issue WREN and `C7h`, poll for at most 500 seconds, then reject FSR bits `0x22`.

- [ ] **Step 4: Correct the header table**

Keep `ProtectRegisterTable` at `0x007C` and its default at `0x00`. Update comments to describe the one-byte SR and remove all CMP/SR2 claims.

- [ ] **Step 5: Scan obsolete logic**

Run:

```powershell
rg -n "ReadStatus1Cmd|CheckAllStatus1Data|CMP|QE|SRP1|LB1|LB2|LB3|0x35" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h
```

Expected: no active code matches.

### Task 4: Implement Micron OTP Secure Flow

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`
- Inspect: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h`

**Interfaces:**
- Produces: `ProgramOTPArray(uChar Address, uChar *Data, uChar Count)`
- Produces: `VerifyOTPArray(uChar Address, uChar *Data, uChar Count, uChar Mask)`
- Replaces: unreachable TH/GD body of `SecureFun`

- [ ] **Step 1: Implement three-byte OTP command framing**

Both OTP helpers send `00h`, `00h`, and the one-byte OTP offset directly. They never call `SelectAddressBank`. READ `4Bh` adds one dummy byte; PROGRAM `42h` sends at least one byte after WREN.

- [ ] **Step 2: Add OTP program completion checks**

Clear FSR before each `42h`, poll `05h.WIP`, then reject FSR bits `0x12`. Restore CS and the current passing-socket selection on all exits.

- [ ] **Step 3: Implement two-phase SecureFun**

Load 64 data bytes and one control request byte. Program and verify the 64 data bytes first. If control bit 0 is zero, program `0xFE` at offset `0x40`, then verify bit 0 is zero. Set `E_SecureFail` if every socket fails.

- [ ] **Step 4: Remove template security-register branches**

Delete `48h`, `44h`, erase-security behavior, status-LB checks, and the unreachable early return. Non-Secure callbacks skip the OTP block according to its block attributes.

- [ ] **Step 5: Inspect OTP framing**

Run:

```powershell
rg -n -B8 -A18 "ReadOTPArrayCmd|ProgramOTPArrayCmd|ProgramOTPArray|VerifyOTPArray|SecureFun" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
```

Expected: only `4Bh/42h`, three address bytes, one read dummy byte, and data-before-lock sequencing.

### Task 5: Final Conformance and Build Verification

**Files:**
- Verify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`
- Verify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h`
- Verify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.spc`

**Interfaces:**
- Produces: buildable N25Q256A driver source and generated binary

- [ ] **Step 1: Run static conformance scans**

Run:

```powershell
rg -n "0xB7|0xE9|Send4ByteAddress|ReadStatus1Cmd|ProgramSecurityCmd|ReadSecurityCmd|EraseSecurityCmd|StatusLBMaskBit|FlagStatusErrorMask" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
rg -n "0x01FF,0xFFFF|0x0200,0x0040|0x007C|Noun_Block1|Noun_Block2" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h
git diff --check -- WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h
```

Expected: obsolete-symbol scan is empty, header geometry/mask matches, and diff check is silent.

- [ ] **Step 2: Build**

Run `cmd /c iBuild.bat` from `WAx4_N25Q256Ax3ESFxxx`. Expected: ARM compile, link, and `fromelf` finish with zero errors. Report an external artifact-copy failure separately if the destination is unavailable.

- [ ] **Step 3: Inspect only in-scope changes**

Run `git status --short` and inspect the two driver files plus this plan. Do not stage or modify unrelated user files.

- [ ] **Step 4: Commit source when authorized**

Stage only the driver source/header and use commit message `fix: conform N25Q256A driver to Micron command set`.
