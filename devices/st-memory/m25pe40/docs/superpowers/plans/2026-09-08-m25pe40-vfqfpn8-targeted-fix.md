# M25PE40 VFQFPN8 Targeted Fix Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Correct the M25PE40 MP/VFQFPN8 driver block model, serial-number rewrite behavior, and package labeling without adding unsupported device features.

**Architecture:** Preserve the existing ALL-1000 driver ABI and SPI transfer functions. Make the header expose separate main-array, status-register, and virtual-control blocks; keep normal array operations unchanged; and use the M25PE40 PAGE WRITE command for arbitrary serial-number page updates.

**Tech Stack:** ARM C89, ALL-1000 firmware ABI, ARMCC RVCT 2.2, PowerShell static checks

## Global Constraints

- Target package is Micron MP: VFQFPN8, 6 x 5 mm.
- Do not implement Lock Register, PAGE ERASE, SUBSECTOR ERASE, or SECTOR ERASE workflows.
- Do not change main-array read, bulk erase, program, verify, blank-check, checksum, JEDEC ID, voltage, or programmer ABI behavior.
- Do not deploy generated BIN or SPC files outside this workspace.

---

### Task 1: Correct the device block table and package identity

**Files:**
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h:25-358`

**Interfaces:**
- Consumes: `Block_M25PE40VMPxxx[]`, `DrvElement_Table`, `DevPmpIdx_Table`
- Produces: one-byte status block `Noun_Block1`, virtual control block `Noun_BlkPtStatus`, local `Pmp_H3V3VFQFPN8UG4A`

- [x] **Step 1: Run failing source assertions**

```powershell
$h = Get-Content -Raw WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.h
if ($h -match '4K-bit secured OTP') { throw 'unsupported OTP block is active' }
if ($h -notmatch '0x0000,0x0000,0x0000,0x0000,[^\r\n]*\r?\n\s*Noun_Block1') { throw 'one-byte status block missing' }
if ($h -match 'H3V3SO8UG4A') { throw 'SO8 package label remains' }
```

Expected: FAIL because the active header contains the OTP block and SO8 identifier.

- [x] **Step 2: Replace the three block entries**

Keep the main block unchanged. Replace the OTP/status area with:

```c
    /************* Status Register *************/
    0x0223,0x00FF,0x00FF,
    0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,
    Noun_Block1,

    /************* Driver operation control *************/
    0x02FF,0x00FF,0x0000,
    0x0000,0x0003,
    0x0000,0x0003,0x0000,0x0003,
    Noun_BlkPtStatus,
```

- [x] **Step 3: Rename local package-map symbols**

Rename `PmpIdx_H3V3SO8UG4A` and `Pmp_H3V3SO8UG4A` to their
`H3V3VFQFPN8UG4A` forms and change the map comment to `M25PE40 MP package,
VFQFPN8 6 x 5 mm`. Keep every terminal value unchanged.

- [x] **Step 4: Remove unsupported declarations and correct capability comments**

Remove the unused `SecureFun` declaration. State that the active driver uses
PAGE PROGRAM for the erased main array, PAGE WRITE for serial-number updates,
BULK ERASE, and BP2:BP0 status protection; do not claim implemented PE/SSE/SE
or Lock Register workflows.

- [x] **Step 5: Re-run the Task 1 assertions**

Expected: PASS with no output.

---

### Task 2: Make serial-number page updates byte-alterable

**Files:**
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c:40-1050`

**Interfaces:**
- Consumes: `PageWriteCmd`, `WriteStatusWELBit()`, `CheckAllStatusData()`
- Produces: `DevSNProgAndVerify()` with WEL validation and bounded WIP polling

- [x] **Step 1: Run failing source assertions**

```powershell
$c = Get-Content -Raw WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.c
$sn = $c.Substring($c.IndexOf('void DevSNProgAndVerify'))
if ($sn -match 'PtrData\(OneDataCmd\) = PageProgCmd') { throw 'SN path still uses Page Program' }
if ($sn -notmatch 'WriteStatusWELBit') { throw 'SN path does not verify WEL' }
if ($sn -notmatch 'CheckAllStatusData\(0x00\)') { throw 'SN path does not poll WIP' }
```

Expected: FAIL because the current serial-number path sends `PageProgCmd` and uses a fixed delay.

- [x] **Step 2: Remove unused template-only commands and function**

Keep commands used by active code: WREN, RDSR, WRSR, READ, FAST_READ, PAGE
WRITE, PAGE PROGRAM, BULK ERASE, and RDID. Remove unused WRDI, PE, SSE, SE,
RDLR, WRLR, DP, and RDP macros. Remove the unreferenced `SecureFun()` and change
the checksum special-block comment to describe virtual status/control data.

- [x] **Step 3: Implement PAGE WRITE with WEL and WIP checks**

Before the serial-number write transaction, call `WriteStatusWELBit()`. On
failure set `SNDataBuf(1)`, `SNDataBuf(2)`, and `E_DeviceSerialCodeSetFail`,
restore the gang socket mask, and return. Send `PageWriteCmd` instead of
`PageProgCmd`. After CS rises, poll `StatusWIPBit` every 1 ms with a 30-count
timeout, which exceeds the datasheet 23 ms maximum. Use
`GangCheckIfAllSocketError()` to retire failed sockets and report all-socket
failure through the same serial-number error path.

- [x] **Step 4: Re-run the Task 2 assertions**

Expected: PASS with no output.

---

### Task 3: Validate source and ARMCC build

**Files:**
- Verify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h`
- Verify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c`

**Interfaces:**
- Consumes: completed Tasks 1 and 2
- Produces: compiler object and link result contained under `WAx4_M25PE40-VMPxxx/build/verification`

- [x] **Step 1: Run source inventory checks**

```powershell
rg -n 'OTP|boot lock|H3V3SO8|WriteDisableCmd|PageEraseCmd|SubsectorEraseCmd|SectorEraseCmd|WriteLockRegCmd|ReadLockRegCmd|DeepPowerDownCmd|ReleasePowerDownCmd|SecureFun' WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.h WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.c
```

Expected: no matches.

- [x] **Step 2: Run whitespace and diff review**

```powershell
git diff --check -- WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c
git diff -- WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c
```

Expected: no whitespace errors; diff contains only the approved targeted fixes.

- [x] **Step 3: Compile and link locally**

Invoke ARMCC, ARMLINK, and FROMELF with the same arguments and object list as
`iBuild.bat`, but place `.o`, `.axf`, `.bin`, and map output under
`WAx4_M25PE40-VMPxxx/build/verification` and do not run its deployment copy or
cleanup section.

Expected: ARMCC and ARMLINK exit 0 and a local BIN is produced.

- [x] **Step 4: Record final status**

Report the files changed, static-check result, compiler warnings/errors, and
remaining hardware-validation items. Do not commit source changes unless the
user asks for a commit.
