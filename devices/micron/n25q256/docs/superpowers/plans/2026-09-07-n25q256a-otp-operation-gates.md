# N25Q256A OTP Operation Gates Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add PY25Q01GLC-style independent Program, Verify, BlankCheck, and Checksum controls to the N25Q256A 65-byte OTP block while keeping OTP Read unconditional and preserving all framework counters in disabled paths.

**Architecture:** Keep OTP as the existing `0x8028` Res15 DRAM block. SRAM[3] supplies four task-specific control bits; every task checks only its own bit. A small local helper consumes one 65-byte OTP CF block for disabled input tasks so no path uses the 512-byte `CFCardReadFillSector()` shortcut.

**Tech Stack:** ALL-1000 C driver ABI, Gang4 FPGA SPI registers, RVDS ARMCC 2.2, SPC option format, PowerShell static contract checks.

## Global Constraints

- OTP Attribute remains exactly `0x8028`.
- OTP Read remains unconditional and continues using separate 64-byte and 1-byte `4Bh` transactions.
- SRAM[3] bit values are Program `0x01`, Verify `0x02`, BlankCheck `0x08`, and Checksum `0x10`.
- OTP Erase and Secure remain disabled.
- CF transport consumes 33 words, but counters and checksum account for exactly 65 logical bytes.
- No OTP disabled path may call `CFCardReadFillSector()`.
- Main Flash, Status, NVCR, Protect/UnProtect, ID, and SN behavior must not change.
- NVCR remains hidden in the customer SPC and `SecureFun()` remains a no-op.

---

### Task 1: Add the four customer OTP controls

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:74`
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.spc:18-22`

**Interfaces:**
- Consumes: SRAM byte selected by `SRAMPointer = 0x03`.
- Produces: `OTPProgramOn`, `OTPVerifyOn`, `OTPBlankCheckOn`, and `OTPChecksumOn` masks.

- [x] **Step 1: Run a failing static contract check**

```powershell
$c = Get-Content -Raw WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
$spc = Get-Content -Raw WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.spc
if ($c -notmatch '#define\s+OTPVerifyOn\s+0x02') { throw 'OTP Verify control missing' }
if ($c -notmatch '#define\s+OTPBlankCheckOn\s+0x08') { throw 'OTP BlankCheck control missing' }
if ($c -notmatch '#define\s+OTPChecksumOn\s+0x10') { throw 'OTP Checksum control missing' }
if ($spc -notmatch 'E10,10 Enable OTP Checksum Calculate') { throw 'SPC OTP controls incomplete' }
```

Expected: FAIL because only `OTPProgramOn` currently exists.

- [x] **Step 2: Add the exact C masks**

```c
#define OTPProgramOn            0x01
#define OTPVerifyOn             0x02
#define OTPBlankCheckOn         0x08
#define OTPChecksumOn           0x10
```

- [x] **Step 3: Replace the SPC control block**

```text
S0003L0100 OTP OPERATION OPTIONS :
E01,01 Enable OTP Program
E02,02 Enable OTP Verify
E08,08 Enable OTP Blank Check
E10,10 Enable OTP Checksum Calculate
```

- [x] **Step 4: Re-run the static contract check**

Expected: PASS with no output.

---

### Task 2: Centralize disabled-path OTP CF consumption

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:92-108`
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:1149-1173`

**Interfaces:**
- Consumes: `CFCardReadWordData()`, `OTPDataSize`, and `OTPBlockSize`.
- Produces: `void SkipOTPBlockData(void)`.

- [x] **Step 1: Prove the helper is absent**

Run:

```powershell
rg -n "SkipOTPBlockData" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
```

Expected: no matches.

- [x] **Step 2: Add the prototype and implementation**

```c
void SkipOTPBlockData(void) ;

void SkipOTPBlockData(void)
{
uShort TempCnt ;

    for(TempCnt=0; TempCnt<OTPDataSize; TempCnt+=2)
        (void)CFCardReadWordData() ;
    (void)CFCardReadWordData() ; /* low byte is control; high byte is padding */

    DramBufCnt -= OTPBlockSize ;
    DevCurrBlkLen.all -= OTPBlockSize ;
    DeviceAddress += OTPBlockSize ;
    GetChkBlkLenEnd() ;
}
```

- [x] **Step 3: Confirm the implementation consumes 33 words and accounts for 65 bytes**

Inspect the helper and confirm the loop executes 32 times, the final read once,
and every logical counter uses `OTPBlockSize` rather than 66.

---

### Task 3: Apply Program and Verify controls

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:290-351`
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:486-516`

**Interfaces:**
- Consumes: SRAM[3], `OTPProgramOn`, `OTPVerifyOn`, and `SkipOTPBlockData()`.
- Produces: Program OFF without `42h`; Verify OFF without `4Bh`; aligned CF and block state.

- [x] **Step 1: Move Program OFF check before CF loading and call the skip helper**

```c
SRAMPointer = 0x03 ;
RamWordData = GetSRAMByteData() ;
if( !(RamWordData & OTPProgramOn) )
{
    SkipOTPBlockData() ;
    return SUCCESS ;
}
```

The enabled branch then loads the 64 data bytes plus control byte and retains
the existing N25Q256A Program and lock logic.

- [x] **Step 2: Add Verify OFF handling before expected-data loading**

```c
SRAMPointer = 0x03 ;
RamWordData = GetSRAMByteData() ;
if( !(RamWordData & OTPVerifyOn) )
{
    SkipOTPBlockData() ;
    return SUCCESS ;
}
```

The enabled branch retains both existing `VerifyOTPArray()` calls and the
65-byte counter update.

- [x] **Step 3: Check command reachability**

Inspect both branches and confirm `ProgramOTPArray()` and `VerifyOTPArray()`
appear only after their corresponding OFF return.

---

### Task 4: Apply BlankCheck and Checksum controls

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:662-686`
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:798-833`

**Interfaces:**
- Consumes: SRAM[3], `OTPBlankCheckOn`, `OTPChecksumOn`, and `SkipOTPBlockData()`.
- Produces: BlankCheck OFF without SPI and Checksum OFF without checksum mutation.

- [x] **Step 1: Add BlankCheck OFF block completion**

```c
SRAMPointer = 0x03 ;
TempCnt = GetSRAMByteData() ;
if( !(TempCnt & OTPBlankCheckOn) )
{
    DeviceAddress += OTPBlockSize ;
    DevCurrBlkLen.all -= OTPBlockSize ;
    return SUCCESS ;
}
```

Do not consume CF data and do not call `VerifyOTPArray()` in this OFF branch.

- [x] **Step 2: Add Checksum OFF CF consumption**

```c
SRAMPointer = 0x03 ;
RamWordData = GetSRAMByteData() ;
if( !(RamWordData & OTPChecksumOn) )
{
    SkipOTPBlockData() ;
    return SUCCESS ;
}
```

This check must occur before `ChecksumAll` is initialized or changed.

- [x] **Step 3: Preserve enabled behavior**

Confirm enabled BlankCheck still checks 64 bytes equal to `FFh` and control
bit0 equal to 1, and enabled Checksum still exits after the 65th low byte
without adding the transport padding byte.

---

### Task 5: Regression and build validation

**Files:**
- Verify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`
- Verify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h`
- Verify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.spc`

**Interfaces:**
- Consumes: completed driver source and SPC.
- Produces: static-contract, compiler, linker, and binary-conversion evidence.

- [x] **Step 1: Run final static checks**

```powershell
rg -n "OTPProgramOn|OTPVerifyOn|OTPBlankCheckOn|OTPChecksumOn|SkipOTPBlockData" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
rg -n "E01,01|E02,02|E08,08|E10,10" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.spc
if (rg -n "StatusCheckOn|CFCardReadFillSector" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c) { throw 'forbidden legacy path found' }
git diff --check -- WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.spc
```

Expected: four controls and the skip helper are present; no forbidden legacy
path exists; diff check is clean.

- [x] **Step 2: Compile with RVDS ARMCC to a temporary object**

```powershell
$armcc = 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium\armcc.exe'
$rvds = 'C:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows'
$out = Join-Path $env:TEMP 'WAx4_N25Q256Ax3ESFxxx-otp-controls.o'
& $armcc -c --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend `
  -I'D:\HILO\ALL-1000\WAx4_N25Q256Ax3ESFxxx' `
  -I'D:\HILO\ALL-1000\All1000Code\h' `
  -I'D:\HILO\ALL-1000\All1000Code' `
  -J$rvds -J(Join-Path $rvds 'rw') `
  'D:\HILO\ALL-1000\WAx4_N25Q256Ax3ESFxxx\WAx4_N25Q256Ax3ESFxxx.c' -o $out
```

Expected: zero compiler errors and a generated object file.

- [x] **Step 3: Link and convert using the existing ALL1000Code object set**

Run:

```powershell
$toolRoot = 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium'
$objRoot = 'D:\HILO\ALL-1000\All1000Code'
$driverObj = Join-Path $env:TEMP 'WAx4_N25Q256Ax3ESFxxx-otp-controls.o'
$axf = Join-Path $env:TEMP 'WAx4_N25Q256Ax3ESFxxx-otp-controls.axf'
$bin = Join-Path $env:TEMP 'WAx4_N25Q256Ax3ESFxxx-otp-controls.bin'
$map = Join-Path $env:TEMP 'WAx4_N25Q256Ax3ESFxxx-otp-controls.map.txt'
$names = @(
  '_stackheap.o','CFControl.o','Disp.o','FPGADrv.o','FWDMM.o','hs0_mmc.o',
  'Hspi.o','Initial.o','ISR.o','Key.o','Main.o','MemInit.o','MMUCache_asm.o',
  'SHA.o','startup.o','Subroutine.o','SysClock.o','SystemTask.o','uMROS.o',
  'UpDnLoad.o','USBCom.o'
)
$objs = @($names | ForEach-Object { Join-Path $objRoot $_ }) + $driverObj
& (Join-Path $toolRoot 'armlink.exe') $objs -o $axf --map `
  --ro_base=0x30000000 --rw_base=0x30200000 `
  '--first=startup.o(Init)' --entry=0x30000000 `
  --info=sizes --info=totals --list $map
if ($LASTEXITCODE -ne 0) { throw 'link failed' }
& (Join-Path $toolRoot 'fromelf.exe') --bin $axf --output $bin
if ($LASTEXITCODE -ne 0) { throw 'BIN conversion failed' }
```

All AXF, map, and BIN outputs remain under `$env:TEMP`; repository and
deployment directories are not modified.

Expected: `armlink` and `fromelf` exit zero and generate AXF, map, and BIN.

- [x] **Step 4: Review final scope**

Use `git diff` to confirm Main Flash, Status, NVCR, Protect/UnProtect, ID, and
SN logic have no functional changes. Record the remaining hardware validation
requirement for CF odd-byte padding and irreversible OTP lock testing.
