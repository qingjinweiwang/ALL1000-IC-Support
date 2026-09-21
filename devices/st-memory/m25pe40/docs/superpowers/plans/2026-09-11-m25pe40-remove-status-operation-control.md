# M25PE40 Status Operation Control Removal Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove the user-visible S0003 driver-control byte and make BP2-BP0 verification unconditional for the Status Register block.

**Architecture:** Remove the user option from `.spc` and both C decision gates, while retaining the existing virtual metadata block for driver compatibility. Preserve the real one-byte Status Register block and all user-owned uncommitted changes already present in the driver files.

**Tech Stack:** ARM C, ALL-1000 driver metadata, SPC configuration syntax, ARMCC/RVCT 2.2, PowerShell static assertions.

## Global Constraints

- Modify only `WAx4_M25PE40-VMPxxx.c`, `WAx4_M25PE40-VMPxxx.h`, and `WAx4_M25PE40-VMPxxx.spc`.
- Remove only `S0003`, `StatusCheckOn`, and the two SRAM[3] skip gates; retain `Noun_BlkPtStatus`.
- Keep `S0000L0100 STATUS REGISTER` and its BP0/BP1/BP2 settings.
- Status Register Verify and Blank Check must always call `CheckAllStatusData()`.
- Preserve all existing uncommitted FPGA-polarity, EP0, socket-selection, voltage, and SPI-timing edits exactly as found.
- Do not deploy the verification image.

---

### Task 1: Remove the virtual operation-control feature as one coordinated unit

**Files:**
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.spc:15-18`
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h:156-160`
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c:57,304-307,480-483`
- Create build outputs only: `WAx4_M25PE40-VMPxxx/build/no-status-control-verification/`

**Interfaces:**
- Consumes: SRAM[0] BP2-BP0 setting, `PrtRegMaskBit`, `DevBlkVirgin`, `CheckAllStatusData(uChar)`
- Produces: retained three-block metadata and unconditional Status Register Verify/Blank Check behavior

- [ ] **Step 1: Record the pre-existing working-tree scope**

```powershell
git diff -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c' 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h' 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.spc'
```

Expected: existing user modifications are visible in `.c` and `.h`; `.spc` is clean.

- [ ] **Step 2: Run the failing removal assertion**

```powershell
$d = 'WAx4_M25PE40-VMPxxx'
$c = Get-Content -Raw -Encoding UTF8 -LiteralPath "$d\WAx4_M25PE40-VMPxxx.c"
$h = Get-Content -Raw -Encoding UTF8 -LiteralPath "$d\WAx4_M25PE40-VMPxxx.h"
$spc = Get-Content -Raw -Encoding UTF8 -LiteralPath "$d\WAx4_M25PE40-VMPxxx.spc"
if ($spc -match 'S0003|DRIVER OPERATION CONTROL|STATUS REGISTER OPERATION OPTIONS') { throw 'SPC operation control remains' }
if ($h -notmatch 'Noun_BlkPtStatus') { throw 'virtual metadata block is missing' }
if ($c -match 'StatusCheckOn|SRAMPointer\s*=\s*0x03') { throw 'driver control gate remains' }
```

Expected: FAIL with `SPC operation control remains`.

- [ ] **Step 3: Remove the SPC option**

Delete exactly:

```text
G<<STATUS REGISTER OPERATION OPTIONS>>

S0003L0100 DRIVER OPERATION CONTROL :
    E80,80 Enable Status Register Verify/Blank Check
```

- [ ] **Step 4: Retain or restore the virtual metadata block**

Ensure this existing block entry remains in `Block_M25PE40VMPxxx`:

```c
    /************* Driver operation control *************/
    0x02FF,0x00FF,0x0000,
    0x0000,0x0003,
    0x0000,0x0003, 0x0000,0x0003,
    Noun_BlkPtStatus,
```

- [ ] **Step 5: Remove the C macro and both skip gates**

Delete:

```c
#define StatusCheckOn 0x80 /* for status reg verify/blank enable */
```

From both `VerifyFun()` and `BlankCheckFun()`, delete:

```c
SRAMPointer = 0x03 ; /* to get sram EFA control status */
RamWordData = GetSRAMByteData() ;
if( !(RamWordData & StatusCheckOn) )
    return SUCCESS ;
```

Keep each following `if( GetCurrBlkNum() == Noun_Block1 )` block unchanged.

- [ ] **Step 6: Run complete source assertions**

```powershell
$d = 'WAx4_M25PE40-VMPxxx'
$c = Get-Content -Raw -Encoding UTF8 -LiteralPath "$d\WAx4_M25PE40-VMPxxx.c"
$h = Get-Content -Raw -Encoding UTF8 -LiteralPath "$d\WAx4_M25PE40-VMPxxx.h"
$spc = Get-Content -Raw -Encoding UTF8 -LiteralPath "$d\WAx4_M25PE40-VMPxxx.spc"
if ($spc -match 'S0003|DRIVER OPERATION CONTROL|STATUS REGISTER OPERATION OPTIONS') { throw 'SPC operation control remains' }
if ($spc -notmatch 'S0000L0100 STATUS REGISTER') { throw 'real Status Register setting was removed' }
if ($h -notmatch 'Noun_BlkPtStatus') { throw 'virtual metadata block is missing' }
if ($h -notmatch 'Noun_Block1') { throw 'Status Register block was removed' }
if ($c -match 'StatusCheckOn|SRAMPointer\s*=\s*0x03') { throw 'driver control gate remains' }
if ([regex]::Matches($c, 'GetCurrBlkNum\(\)\s*==\s*Noun_Block1').Count -ne 3) { throw 'Status Register paths changed unexpectedly' }
if ([regex]::Matches($c, 'CheckAllStatusData\(RamWordData\)').Count -lt 2) { throw 'Verify/Blank Check no longer compare BP bits' }
```

Expected: PASS with no exception.

- [ ] **Step 7: Check whitespace and inspect only the intended additions to the existing diff**

```powershell
git diff --check -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c' 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h' 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.spc'
git diff -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c' 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h' 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.spc'
```

Expected: no whitespace errors; the new delta is limited to the four coordinated removals while previous user changes remain.

- [ ] **Step 8: Compile, link, and convert without deployment**

```powershell
$verifyDir = 'D:\HILO\ALL-1000\WAx4_M25PE40-VMPxxx\build\no-status-control-verification'
New-Item -ItemType Directory -Force -Path $verifyDir | Out-Null
$toolDir = 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium'
$rvdsInclude = 'C:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows'
$driverDir = 'D:\HILO\ALL-1000\WAx4_M25PE40-VMPxxx'
$objectDir = 'D:\HILO\ALL-1000\All1000Code'
& "$toolDir\armcc.exe" -c --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend -I"$driverDir" -I"$objectDir\h" -I"$objectDir" -J"$rvdsInclude" -J"$rvdsInclude\rw" "$driverDir\WAx4_M25PE40-VMPxxx.c" -o "$verifyDir\WAx4_M25PE40-VMPxxx.o"
if ($LASTEXITCODE -ne 0) { throw 'ARMCC compilation failed' }
$objects = @(
    "$objectDir\_stackheap.o", "$objectDir\CFControl.o", "$objectDir\Disp.o", "$objectDir\FPGADrv.o",
    "$objectDir\FWDMM.o", "$objectDir\hs0_mmc.o", "$objectDir\Hspi.o", "$objectDir\Initial.o",
    "$objectDir\ISR.o", "$objectDir\Key.o", "$objectDir\Main.o", "$objectDir\MemInit.o",
    "$objectDir\MMUCache_asm.o", "$objectDir\SHA.o", "$objectDir\startup.o", "$objectDir\Subroutine.o",
    "$objectDir\SysClock.o", "$objectDir\SystemTask.o", "$objectDir\uMROS.o", "$objectDir\UpDnLoad.o",
    "$objectDir\USBCom.o", "$verifyDir\WAx4_M25PE40-VMPxxx.o"
)
& "$toolDir\armlink.exe" $objects -o "$verifyDir\WAx4_M25PE40-VMPxxx.axf" --map --ro_base=0x30000000 --rw_base=0x30200000 '--first=startup.o(Init)' --entry=0x30000000 --info=sizes --info=totals --list "$verifyDir\map.txt"
if ($LASTEXITCODE -ne 0) { throw 'ARMLINK failed' }
& "$toolDir\fromelf.exe" --bin "$verifyDir\WAx4_M25PE40-VMPxxx.axf" --output "$verifyDir\WAx4_M25PE40-VMPxxx.bin"
if ($LASTEXITCODE -ne 0) { throw 'FROMELF conversion failed' }
```

Expected: ARMCC, ARMLINK, and FROMELF exit 0; no deployment occurs.

- [ ] **Step 9: Preserve the dirty worktree for user review**

Do not stage or commit the three driver files because `.c` and `.h` already contain overlapping user-owned uncommitted edits. Report the complete resulting diff and verification outputs to the user.
