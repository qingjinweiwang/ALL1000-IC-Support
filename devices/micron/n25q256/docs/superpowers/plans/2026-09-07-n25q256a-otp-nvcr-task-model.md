# N25Q256A OTP and NVCR Task Model Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement Attribute-correct OTP and NVCR Read/Program/Verify/BlankCheck/Secure/Checksum behavior in the N25Q256A Gang4 driver without changing Main Flash behavior.

**Architecture:** Keep task dispatch in the existing three-file driver. Reuse and harden the N25Q256A OTP helpers, add focused NVCR helpers, split OTP data/control reads into separate transactions, and preserve Gang4 selection/error semantics at every boundary.

**Tech Stack:** ARM C for RVDS 2.2, ALL-1000 Programmer Framework, Gang4 FPGA SPI registers, `.spc` special-bit metadata, PowerShell static checks.

## Global Constraints

- Datasheet authority: Micron `n25q_256mb_65nm.pdf`, Rev. X 06/18.
- PY25Q01GLC supplies architecture only; do not copy PY-specific commands.
- OTP attribute remains exactly `0x8028`; NVCR remains exactly `0x0227`; Status remains exactly `0x022B`.
- SRAM mapping remains Status `[0]`, NVCR low `[1]`, NVCR high `[2]`, Driver Control `[3]`.
- OTP Read uses separate 64-byte data and 1-byte control `4Bh` transactions.
- NVCR bit5 is forced to 1 on write and excluded from Verify.
- Reject reserved XIP encodings 5/6 and driver-strength encodings 0/4 before `B1h`.
- Main Flash command paths must remain unchanged.

---

### Task 1: Lock the Block and SPC contracts

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h`
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.spc`

**Interfaces:**
- Consumes: Programmer Framework block attribute bit definitions.
- Produces: exact OTP/NVCR block attributes and immutable SRAM layout consumed by all C task functions.

- [ ] **Step 1: Run the pre-change contract scan**

```powershell
rg -n "0x8028|0x022B|0x0227|0x02FF|S000[0-3]L0100" WAx4_N25Q256Ax3ESFxxx.h WAx4_N25Q256Ax3ESFxxx.spc
```

Expected: attributes and addresses already exist, but the OTP header comment still says Secure-only and must fail semantic review.

- [ ] **Step 2: Correct comments without changing numeric contracts**

Use these exact block descriptions:

```c
/************* OTP Array *************/
0x8028,0x00FF,0x00FF, /* DRAM OTP: Read/Program/Verify/BlankCheck/Checksum; no Erase/Secure */
```

Keep the SPC records exactly mapped as:

```text
S0000L0100 STATUS REGISTER
S0001L0100 NVCR LOW BYTE
S0002L0100 NVCR HIGH BYTE
S0003L0100 DRIVER OPERATION CONTROL
```

- [ ] **Step 3: Verify the contract**

```powershell
$h = Get-Content -Raw WAx4_N25Q256Ax3ESFxxx.h
$spc = Get-Content -Raw WAx4_N25Q256Ax3ESFxxx.spc
if ($h -notmatch '0x8028' -or $h -notmatch '0x0227') { throw 'block attributes changed' }
0..3 | ForEach-Object { if ($spc -notmatch ('S000' + $_ + 'L0100')) { throw "missing SRAM[$_]" } }
```

Expected: no output and exit code 0.

### Task 2: Add and harden low-level OTP/NVCR helpers

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`

**Interfaces:**
- Consumes: `SPIRWDelay`, `Send4ByteAddress`, `WriteStatusWELBit`, FSR helpers, `GangErrSckMask`.
- Produces: `ReadOTPArrayData`, `ProgramOTPArray`, `VerifyOTPArray`, `ReadNVCRData`, `WriteNVCRData`, `CheckAllNVCRData`, `NVCRDataIsValid`.

- [ ] **Step 1: Run the pre-change helper check**

```powershell
rg -n "ReadNVCRCmd|WriteNVCRCmd|ReadNVCRData|WriteNVCRData|CheckAllNVCRData|NVCRDataIsValid" WAx4_N25Q256Ax3ESFxxx.c
```

Expected: no matches.

- [ ] **Step 2: Add constants and declarations**

```c
#define ReadNVCRCmd                 0xB5
#define WriteNVCRCmd                0xB1
#define OTPDataSize                 64
#define OTPControlAddr              0x40
#define OTPBlockSize                65
#define OTPControlMask              0x01
#define NVCRLowVerifyMask           0xDF
#define NVCRHighVerifyMask          0xFF
#define NVCRReservedBit5            0x0020
#define NVCRWriteTimeoutMs          3500

uChar ReadOTPArrayData(uChar Address, uChar *Data, uChar Count);
uShort ReadNVCRData(void);
uChar WriteNVCRData(uShort Data);
uChar CheckAllNVCRData(uShort Data);
uChar NVCRDataIsValid(uShort Data);
```

- [ ] **Step 3: Implement `ReadOTPArrayData` and harden OTP helpers**

Each OTP transaction must have this shape:

```c
PtrData(SetCSPinCmd) = 0;
PtrData(OneDataCmd) = ReadOTPArrayCmd;
Send4ByteAddress((uInt32)Address);
PtrData(OneDataCmd) = 0xFF; /* 8 dummy clocks */
/* read or Gang-check Count bytes */
PtrData(SetCSPinCmd) = 1;
```

`ProgramOTPArray()` retains `42h`, WREN/WEL, WIP polling, and FSR mask `0x12`.
Both Program and Verify must call `GangCheckIfAllSocketError()` on mismatch,
return FAIL only when all sockets fail, and refresh:

```c
PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask);
```

after partial failure.

- [ ] **Step 4: Implement NVCR read, validation, write, and Gang check**

Validation is exact:

```c
uChar NVCRDataIsValid(uShort Data)
{
    uChar XIPData = (uChar)((Data >> 9) & 0x07);
    uChar DriverData = (uChar)((Data >> 6) & 0x07);
    if ((XIPData == 5) || (XIPData == 6)) return FAIL;
    if ((DriverData == 0) || (DriverData == 4)) return FAIL;
    return SUCCESS;
}
```

`ReadNVCRData()` sends `B5h` and reads low then high. `WriteNVCRData()` owns
Clear FSR, WREN/WEL, `B1h` low/high, CS-high initiation, WIP polling with
`NVCRWriteTimeoutMs`, FSR `0x12`, and partial Gang recovery. It does not do
final readback. `CheckAllNVCRData()` sends one `B5h`, compares low with mask
`0xDF`, high with mask `0xFF`, and returns the OR of both socket-error masks.

- [ ] **Step 5: Verify command and lifecycle structure**

```powershell
$c = Get-Content -Raw WAx4_N25Q256Ax3ESFxxx.c
@('0xB5','0xB1','ReadNVCRData','WriteNVCRData','CheckAllNVCRData','NVCRDataIsValid') |
  ForEach-Object { if ($c -notmatch [regex]::Escape($_)) { throw "missing $_" } }
```

Expected: no output and exit code 0.

### Task 3: Route OTP through ordinary DRAM tasks

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`

**Interfaces:**
- Consumes: OTP helpers from Task 2 and CF/DRAM word transport.
- Produces: Attribute-correct OTP behavior in Read/Program/Verify/BlankCheck/Checksum.

- [ ] **Step 1: Run the pre-change legacy-branch check**

```powershell
rg -n "OTP is programmed only by SecureFun|OTP verification is part of SecureFun|OTP blank state is handled by SecureFun|OTP checksum is disabled" WAx4_N25Q256Ax3ESFxxx.c
```

Expected: four legacy branches are found.

- [ ] **Step 2: Implement OTP Read**

Select socket A, call `ReadOTPArrayData(0, Data, 64)`, then
`ReadOTPArrayData(0x40, &Control, 1)`. Pack 64 bytes into 32 words and the
control into the low byte of the final word. Advance upload counters and
logical addresses by 65 only, then restore all valid sockets.

- [ ] **Step 3: Implement OTP Program**

Read 32 full words plus the low byte of the 33rd transport word. Program and
verify 64 data bytes. If control bit0 is 0, program `FEh`; if it is 1, issue no
control Program command. In both cases verify control bit0. On success update:

```c
DramBufCnt -= OTPBlockSize;
DevCurrBlkLen.all -= OTPBlockSize;
DeviceAddress += OTPBlockSize;
GetChkBlkLenEnd();
```

- [ ] **Step 4: Implement OTP Verify and BlankCheck**

Verify loads the same 65 logical CF bytes and performs two Gang reads:

```c
VerifyOTPArray(0x00, OTPData, OTPDataSize, 0xFF);
VerifyOTPArray(OTPControlAddr, &OTPControlData, 1, OTPControlMask);
```

BlankCheck performs the same two device transactions using 64 `FFh` expected
bytes and control bit0 equal to 1. It does not issue WREN or Erase.

- [ ] **Step 5: Implement odd-length OTP checksum**

Use the established framework pattern: read one word, add low, decrement one
logical byte and check end; add high only if data remains. This guarantees byte
66 is not included while the word transport physically consumes its padding.

- [ ] **Step 6: Verify removal of the old OTP architecture**

```powershell
if (rg -n "OTP is programmed only by SecureFun|OTP verification is part of SecureFun|OTP blank state is handled by SecureFun|OTP checksum is disabled" WAx4_N25Q256Ax3ESFxxx.c) { throw 'legacy OTP path remains' }
```

Expected: no matches and exit code 0.

### Task 4: Route NVCR through Special tasks and Secure

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`

**Interfaces:**
- Consumes: SRAM[1:3], NVCR helpers from Task 2.
- Produces: NVCR Read/Verify/BlankCheck and Secure write.

- [ ] **Step 1: Add NVCR Read dispatch**

For `Noun_Block3`, explicitly select socket A, read `B5h`, set
`SRAMPointer=0x01`, save low, set `SRAMPointer=0x02`, save high, and restore
`~GangErrSckMask`. For `Noun_BlkPtStatus`, return without SPI activity.

- [ ] **Step 2: Add NVCR Verify and BlankCheck**

Both tasks first gate on SRAM[3] bit7. Verify assembles expected low/high from
SRAM[1:2] and calls `CheckAllNVCRData()`. BlankCheck passes `0xFFFF`.
All-socket errors map to `E_UserPrtRegVerifyFail` and
`E_UserPrtRegBlankFail`; partial failures refresh socket selection.

- [ ] **Step 3: Implement NVCR Secure write**

```c
if (GetCurrBlkNum() != Noun_Block3) return SUCCESS;
/* SRAM[1] low, SRAM[2] high */
TargetNVCR |= NVCRReservedBit5;
if (!NVCRDataIsValid(TargetNVCR)) {
    SysStatusCode = E_UserPrtRegIllegalFail;
    return FAIL;
}
if (!WriteNVCRData(TargetNVCR)) {
    SysStatusCode = E_SecureFail;
    return FAIL;
}
StatusData = CheckAllNVCRData(TargetNVCR);
```

Call `GangCheckIfAllSocketError()` for readback mismatches and return
`E_SecureFail` only if all sockets fail.

- [ ] **Step 4: Verify Special routing**

```powershell
rg -n "Noun_Block2|Noun_Block3|Noun_BlkPtStatus|StatusCheckOn|E_UserPrtRegIllegalFail" WAx4_N25Q256Ax3ESFxxx.c
```

Expected: independent Status, NVCR, and virtual-control branches are present.

### Task 5: Compile and perform final risk audit

**Files:**
- Verify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`
- Verify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h`
- Verify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.spc`

**Interfaces:**
- Consumes: completed driver.
- Produces: compiler evidence and final handoff audit.

- [ ] **Step 1: Compile to a temporary output**

```powershell
$armcc = 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium\armcc.exe'
$rvds = 'C:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows'
$out = Join-Path $env:TEMP 'WAx4_N25Q256Ax3ESFxxx.o'
& $armcc -c --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend `
  -I'D:\HILO\ALL-1000\WAx4_N25Q256Ax3ESFxxx' `
  -I'D:\HILO\ALL-1000\All1000Code\h' `
  -I'D:\HILO\ALL-1000\All1000Code' `
  -J$rvds -J(Join-Path $rvds 'rw') `
  'D:\HILO\ALL-1000\WAx4_N25Q256Ax3ESFxxx\WAx4_N25Q256Ax3ESFxxx.c' -o $out
```

Expected: ARM Compiler reports zero errors and creates the temporary object.

- [ ] **Step 2: Run static contract checks**

```powershell
rg -n "#define ReadOTPArrayCmd.*0x4B|#define ProgramOTPArrayCmd.*0x42|#define ReadNVCRCmd.*0xB5|#define WriteNVCRCmd.*0xB1" WAx4_N25Q256Ax3ESFxxx.c
rg -n "0x8028|0x022B|0x0227|0x02FF" WAx4_N25Q256Ax3ESFxxx.h
git diff --check -- WAx4_N25Q256Ax3ESFxxx.c WAx4_N25Q256Ax3ESFxxx.h WAx4_N25Q256Ax3ESFxxx.spc
```

Expected: all required constants/attributes are present and diff check is clean.

- [ ] **Step 3: Audit risk-sensitive paths**

Confirm manually from the final diff:

- control bit0=1 cannot issue OTP control Program;
- `4Bh` data and control reads are separate;
- no OTP Erase path exists;
- NVCR reserved encodings fail before `B1h`;
- NVCR bit5 writes 1 and Verify ignores it;
- every CS-low path reaches CS-high;
- socket-A Read paths restore Gang selection;
- logical counters use 65, not 66;
- Main Flash Read/Program/Verify/Erase/BlankCheck/SN code is unchanged.

- [ ] **Step 4: Commit only the target implementation files**

```powershell
git add WAx4_N25Q256Ax3ESFxxx.c WAx4_N25Q256Ax3ESFxxx.h WAx4_N25Q256Ax3ESFxxx.spc
git commit -m "feat: add N25Q256A OTP and NVCR task support"
```
