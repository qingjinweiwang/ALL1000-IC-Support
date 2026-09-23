# M25PE40 T9HX BP Protection and Bulk Erase Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the M25PE40 T9HX-only driver use Status Register `BP2:BP0` protection and reliably execute `C7h` Bulk Erase on all active gang sockets.

**Architecture:** Preserve the ALL-1000 firmware ABI and existing FPGA SPI primitives. Keep the current three-entry block model, route gang protection decisions through masked FPGA comparisons, centralize WRSR sequencing in one BP helper, and make Bulk Erase distinguish a rejected command from a completed command by checking the WIP transition.

**Tech Stack:** ARM C89, ALL-1000 firmware ABI, FPGA gang SPI registers, ARMCC RVCT 2.2, PowerShell source assertions

## Global Constraints

- Supported silicon is **M25PE40 T9HX only**; do not claim T7X or generic-process compatibility.
- User-visible protection uses only Status Register `BP2:BP0`, mask `0x1C`; do not expose SRWD or Lock Registers.
- Erase uses only `BULK ERASE (C7h)` and automatically clears BP protection first.
- Keep main-array programming on `PAGE PROGRAM (02h)` and serial-number updates on `PAGE WRITE (0Ah)`.
- Preserve JEDEC ID `20h-80h-13h`, voltage settings, SPI delay, VFQFPN8 pin-map values, and the 13-entry firmware ABI table.
- Keep all generated verification artifacts below `WAx4_M25PE40-VMPxxx/build/t9hx-bp-verification`; do not deploy to `Algo1`.
- Preserve unrelated dirty-worktree files and stage only files named by the current task.

---

### Task 1: Align T9HX metadata and SPC protection options

**Files:**
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h:1-68`
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h:138-160`
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h:310-354`
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.spc:1-37`

**Interfaces:**
- Consumes: ALL-1000 `Block_M25PE40VMPxxx[]` and SPC special-data format
- Produces: one main-array block, one Status Register special block at SRAM offset 0, one driver-control block at SRAM offset 3, and BP-only UI options

- [ ] **Step 1: Run failing metadata assertions**

```powershell
$headerText = Get-Content -Raw 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.h'
$spcText = Get-Content -Raw 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.spc'
if ($headerText -notmatch 'T9HX only') { throw 'header does not limit the target to T9HX' }
if ($headerText -notmatch 'ChipEraseTimeTable[\s\S]*?11,') { throw 'erase timeout is not 11 seconds' }
if ($spcText -notmatch 'T9HX ONLY') { throw 'SPC does not identify T9HX-only scope' }
if ($spcText -notmatch 'E04,04 BP0 = 1') { throw 'SPC BP0 option missing' }
if ($spcText -notmatch 'E08,08 BP1 = 1') { throw 'SPC BP1 option missing' }
if ($spcText -notmatch 'E10,10 BP2 = 1') { throw 'SPC BP2 option missing' }
if ($spcText -match 'LOCK REGISTER|S000[1-8]L0100') { throw 'SPC still exposes Lock Registers' }
```

Expected: FAIL because the header lacks the T9HX-only label, the erase table is 10 seconds, and the SPC exposes Lock Registers.

- [ ] **Step 2: Update the header identity and erase timeout**

Change the file header to state `M25PE40 T9HX only`, explain that BP/SRWD/W#/Bulk Erase are T9HX features, and keep the implemented-command list limited to the active flow. Set:

```c
#define DevDRV_Version         0x3132  /* Version: 1.2, Use Ascii code */
```

Change the erase-time table to:

```c
const uShort ChipEraseTimeTable[ ] =
{
    11,  /* tBE max 10s; one-second driver polling margin */
};
```

Retain:

```c
const uShort ProtectRegisterTable[ ] =
{
    0x001C,
};

const uChar ProtectRegisterDefaultTable[ ] =
{
    0x00,
};
```

- [ ] **Step 3: Preserve and document the exact three-entry block model**

Keep these entries unchanged except for T9HX-specific comments:

```c
const uShort Block_M25PE40VMPxxx[] =
{
    MainByte8Width,
    0x0000,0x0000,
    0x0000,0x0000, 0x0007,0xFFFF,

    0x0000,0x00FF,0x00FF,
    0x0000,0x0000,
    0x0000,0x0000, 0x0007,0xFFFF,
    Noun_Block0,

    0x0223,0x00FF,0x00FF,
    0x0000,0x0000,
    0x0000,0x0000, 0x0000,0x0000,
    Noun_Block1,

    0x02FF,0x00FF,0x0000,
    0x0000,0x0003,
    0x0000,0x0003, 0x0000,0x0003,
    Noun_BlkPtStatus,
};
```

- [ ] **Step 4: Replace the SPC Lock Register UI with BP options**

Replace the active `.special.` section with:

```text
////////////////////////////////////////////////////////////////////////
// MICRON M25PE40 T9HX ONLY
////////////////////////////////////////////////////////////////////////

[0]
.special.
GMessage :
G<<STATUS REGISTER PROTECTION OPTIONS - T9HX ONLY>>

S0000L0100 STATUS REGISTER :
    E04,04 BP0 = 1
    E08,08 BP1 = 1
    E10,10 BP2 = 1

G<<STATUS REGISTER OPERATION OPTIONS>>

S0003L0100 DRIVER OPERATION CONTROL :
    E80,80 Enable Status Register Verify/Blank Check
```

- [ ] **Step 5: Re-run the Task 1 assertions**

Expected: PASS with no output.

- [ ] **Step 6: Commit the metadata unit**

```powershell
git add -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h' 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.spc'
git commit -m 'fix: align M25PE40 T9HX BP metadata'
```

Expected: one commit containing only the header and SPC changes.

---

### Task 2: Centralize BP-only WRSR and gang verification

**Files:**
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c:63-71`
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c:593-769`
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c:784-862`

**Interfaces:**
- Consumes: `WriteStatusWELBit(void)`, `CheckAllStatusData(uChar)`, `PrtRegMaskBit`, `PrtReg0DefaultBit`
- Produces: `WriteStatusBPBits(uChar BPData)` returning `SUCCESS` or `FAIL`; BP-only `ProtectFun()` and `UnProtectFun()`

- [ ] **Step 1: Run failing protection-flow assertions**

```powershell
$sourceText = Get-Content -Raw 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.c'
$protectStart = $sourceText.IndexOf('uChar ProtectFun(void)')
$helperStart = $sourceText.IndexOf('void GetDeviceParameterInfo(void)')
$protectText = $sourceText.Substring($protectStart, $helperStart - $protectStart)
if ($sourceText -notmatch 'uChar WriteStatusBPBits\(uChar BPData\)') { throw 'BP helper missing' }
if ($protectText -match 'CurrSRData|ReadStatusData\(') { throw 'gang protection still copies one socket status' }
if ($protectText -notmatch 'WriteStatusBPBits\(SR1Data\)') { throw 'ProtectFun does not use BP helper' }
if ($protectText -notmatch 'WriteStatusBPBits\(PrtReg0DefaultBit\)') { throw 'UnProtectFun does not use BP helper' }
```

Expected: FAIL because the common BP helper is absent and protection uses `ReadStatusData()`.

- [ ] **Step 2: Declare and implement the BP write helper**

Add the declaration:

```c
uChar WriteStatusBPBits(uChar BPData) ;
```

Implement the helper next to the existing status helpers:

```c
uChar WriteStatusBPBits(uChar BPData)
{
uShort PollingTimeCnt ;
uChar StatusData ;

    BPData &= LSB(PrtRegMaskBit) ; /* SRWD=0; only BP2-BP0 are writable here */

    if( !WriteStatusWELBit() )
        return FAIL ;

    PtrData(SetCSPinCmd) = 0 ;
    Delay_10nsX(10) ;
    PtrData(OneDataCmd) = WriteStatusCmd ;
    SPIRWDelay() ;
    PtrData(OneDataCmd) = BPData ;
    SPIRWDelay() ;
    Delay_10nsX(10) ;
    PtrData(SetCSPinCmd) = 1 ;
    Delay_10nsX(10) ;

    PtrData(GSetAllMaskDataLo) = StatusWIPBit ;
    PollingTimeCnt = 20 ; /* tW max 15ms */
    while(1)
    {
        StatusData = CheckAllStatusData(0x00) ;
        if( !StatusData )
            break ;
        if( !(--PollingTimeCnt) )
        {
            if( GangCheckIfAllSocketError(StatusData) )
                return FAIL ;
            break ;
        }
        Delay_1ms(1) ;
    }

    PtrData(GSetAllMaskDataLo) = LSB(PrtRegMaskBit) ;
    StatusData = CheckAllStatusData(BPData) ;
    if( StatusData )
    {
        if( GangCheckIfAllSocketError(StatusData) )
            return FAIL ;
    }

    return SUCCESS ;
}
```

This sequence completes WRSR polling before BP verification and uses the FPGA gang comparison for every active socket.

- [ ] **Step 3: Replace ProtectFun and UnProtectFun with BP-only wrappers**

Use:

```c
uChar ProtectFun(void)
{
uChar SR1Data ;

    PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
    Delay_10nsX(3) ;

    SRAMPointer = 0x00 ;
    SR1Data = GetSRAMByteData() & LSB(PrtRegMaskBit) ;
    if( !WriteStatusBPBits(SR1Data) )
    {
        SysStatusCode = E_ProtectFail ;
        return FAIL ;
    }
    return SUCCESS ;
}

uChar UnProtectFun(void)
{
    PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
    Delay_10nsX(3) ;

    if( !WriteStatusBPBits(PrtReg0DefaultBit) )
    {
        SysStatusCode = E_UnprotectFail ;
        return FAIL ;
    }
    return SUCCESS ;
}
```

- [ ] **Step 4: Keep single-byte status reads out of gang decisions**

Retain `ReadStatusData()` only for the `ReadFun()` path after it selects `EnSckACtrlFlag`. Remove its use from `ProtectFun()`, `UnProtectFun()`, and `EraseFun()`. Keep `CheckAllStatusData()` as the only BP/WEL/WIP approval path for gang operations.

- [ ] **Step 5: Re-run protection-flow assertions and inspect references**

```powershell
rg -n 'ReadStatusData\(|CheckAllStatusData\(|WriteStatusBPBits\(' 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.c'
```

Expected: `ReadStatusData()` is called only by `ReadFun()`; gang protection paths call `WriteStatusBPBits()` and `CheckAllStatusData()`.

- [ ] **Step 6: Commit the protection unit**

```powershell
git add -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c'
git commit -m 'fix: make M25PE40 BP protection gang safe'
```

Expected: one source-only commit for WRSR and BP behavior.

---

### Task 3: Make power transitions and Bulk Erase rejection-safe

**Files:**
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c:73-105`
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c:374-429`

**Interfaces:**
- Consumes: `UnProtectFun(void)`, `WriteStatusWELBit(void)`, `CheckAllStatusData(uChar)`, `ChipEraseTime`
- Produces: explicit S# power sequencing and `EraseFun()` that detects C7h acceptance and completion per active socket

- [ ] **Step 1: Run failing power/erase assertions**

```powershell
$sourceText = Get-Content -Raw 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.c'
$powerOn = $sourceText.Substring($sourceText.IndexOf('uChar PowerOnFun(void)'), $sourceText.IndexOf('uChar PowerOffFun(void)') - $sourceText.IndexOf('uChar PowerOnFun(void)'))
$powerOff = $sourceText.Substring($sourceText.IndexOf('uChar PowerOffFun(void)'), $sourceText.IndexOf('uChar ReadFun(void)') - $sourceText.IndexOf('uChar PowerOffFun(void)'))
$erase = $sourceText.Substring($sourceText.IndexOf('uChar EraseFun(void)'), $sourceText.IndexOf('uChar BlankCheckFun(void)') - $sourceText.IndexOf('uChar EraseFun(void)'))
if ($powerOn.IndexOf('PtrData(SetCSPinCmd) = 1') -lt 0 -or $powerOn.IndexOf('PtrData(SetCSPinCmd) = 1') -gt $powerOn.IndexOf('GangRiseAllPassSocketVccIccPin')) { throw 'PowerOn does not raise S# before VCC' }
if ($powerOff.IndexOf('PtrData(SetCSPinCmd) = 1') -lt 0 -or $powerOff.IndexOf('PtrData(SetCSPinCmd) = 1') -gt $powerOff.IndexOf('Fall_VccAll')) { throw 'PowerOff does not raise S# before VCC falls' }
if ($erase -notmatch 'UnProtectFun\(\)') { throw 'Erase does not automatically clear BP' }
if ($erase -notmatch 'CheckAllStatusData\(StatusWIPBit\)') { throw 'Erase does not verify WIP assertion' }
if ($erase -match 'ReadStatusData\(') { throw 'Erase approves BP from a single-byte read' }
```

Expected: FAIL because S# is not explicit, erase does not call `UnProtectFun()`, and no WIP-start verification exists.

- [ ] **Step 2: Drive S# HIGH on the safe side of both power transitions**

At the start of `PowerOnFun()` add:

```c
    PtrData(SetCSPinCmd) = 1 ;
    Delay_10nsX(10) ;
```

Keep this before `GangRiseAllPassSocketVccIccPin()`. At the start of `PowerOffFun()` add the same two statements before `Fall_VpioIpioAll()`.

- [ ] **Step 3: Replace EraseFun with the BP-clear, WIP-start, WIP-finish flow**

Use this implementation:

```c
uChar EraseFun(void)
{
uShort StatusData, PollingTimeCnt ;

    PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
    Delay_10nsX(3) ;

    if( IfFirstTimeEntry() )
    {
        if( !UnProtectFun() )
        {
            SysStatusCode = E_EraseFail ;
            return FAIL ;
        }

        PtrData(GSetAllMaskDataLo) = LSB(PrtRegMaskBit) ;
        StatusData = CheckAllStatusData(0x00) ;
        if( StatusData )
        {
            if( GangCheckIfAllSocketError(StatusData) )
            {
                SysStatusCode = E_EraseFail ;
                return FAIL ;
            }
        }

        if( !WriteStatusWELBit() )
        {
            SysStatusCode = E_EraseFail ;
            return FAIL ;
        }

        PtrData(SetCSPinCmd) = 0 ;
        Delay_10nsX(10) ;
        PtrData(OneDataCmd) = BulkEraseCmd ;
        SPIRWDelay() ;
        Delay_10nsX(10) ;
        PtrData(SetCSPinCmd) = 1 ;
        Delay_10nsX(10) ;

        PtrData(GSetAllMaskDataLo) = StatusWIPBit ;
        PollingTimeCnt = 100 ;
        while(1)
        {
            StatusData = CheckAllStatusData(StatusWIPBit) ;
            if( !StatusData )
                break ;
            if( !(--PollingTimeCnt) )
            {
                if( GangCheckIfAllSocketError(StatusData) )
                {
                    SysStatusCode = E_EraseFail ;
                    return FAIL ;
                }
                break ;
            }
            Delay_1us(20) ;
        }

        PollingTimeCnt = (ChipEraseTime * 100) ;
        while(1)
        {
            StatusData = CheckAllStatusData(0x00) ;
            if( !StatusData )
                break ;
            if( !(--PollingTimeCnt) )
            {
                if( GangCheckIfAllSocketError(StatusData) )
                {
                    SysStatusCode = E_EraseFail ;
                    return FAIL ;
                }
                break ;
            }
            Delay_1ms(10) ;
        }
    }
    return SUCCESS ;
}
```

The initial WIP loop expects `WIP=1`; the completion loop expects `WIP=0`. A protected or rejected socket is retired through existing gang-error handling rather than being treated as already erased.

- [ ] **Step 4: Re-run the Task 3 assertions**

Expected: PASS with no output.

- [ ] **Step 5: Commit the erase/power unit**

```powershell
git add -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c'
git commit -m 'fix: harden M25PE40 bulk erase flow'
```

Expected: one source-only commit for S# sequencing and Bulk Erase.

---

### Task 4: Verify Page Write polling, compile, and review

**Files:**
- Verify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c`
- Verify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h`
- Verify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.spc`
- Create build artifacts only: `WAx4_M25PE40-VMPxxx/build/t9hx-bp-verification/`

**Interfaces:**
- Consumes: completed Tasks 1-3 and the existing ARMCC object set under `All1000Code`
- Produces: static-check results plus local `.o`, `.axf`, `.bin`, and map outputs

- [ ] **Step 1: Assert the serial-number Page Write sequence**

```powershell
$sourceText = Get-Content -Raw 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.c'
$snText = $sourceText.Substring($sourceText.IndexOf('void DevSNProgAndVerify(void)'))
if ($snText -notmatch 'PtrData\(OneDataCmd\) = PageWriteCmd') { throw 'SN path does not use Page Write 0Ah' }
if ($snText -notmatch 'WriteStatusWELBit\(\)') { throw 'SN path does not verify WEL' }
if ($snText -notmatch 'PollingTimeCnt = 30') { throw 'SN Page Write timeout is not 30ms' }
if ($snText -notmatch 'StatusData = CheckAllStatusData\(0x00\)') { throw 'SN path does not poll WIP to zero' }
if ($snText -match 'Delay_1ms\(10\)\s*;\s*\/\/.*write') { throw 'SN path relies on a fixed post-write delay' }
```

Expected: PASS. If it fails, make the minimum local correction in `DevSNProgAndVerify()` so the existing order is WREN/WEL check, `0Ah`, WIP polling with 30 ms bound, then full-page read-back verification.

- [ ] **Step 2: Run complete source assertions**

```powershell
$headerText = Get-Content -Raw 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.h'
$sourceText = Get-Content -Raw 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.c'
$spcText = Get-Content -Raw 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.spc'
if ($headerText -notmatch 'T9HX only') { throw 'T9HX scope missing' }
if ($headerText -notmatch '0x001C') { throw 'BP mask missing' }
if ($spcText -match 'LOCK REGISTER') { throw 'Lock Register UI remains' }
if ($sourceText -match '#define\s+(PageEraseCmd|SubsectorEraseCmd|SectorEraseCmd|ReadLockRegCmd|WriteLockRegCmd)') { throw 'unsupported command macro remains' }
if ($sourceText -notmatch 'PtrData\(OneDataCmd\) = BulkEraseCmd') { throw 'C7h path missing' }
if ($sourceText -notmatch 'CheckAllStatusData\(StatusWIPBit\)') { throw 'WIP-start check missing' }
```

Expected: PASS with no output.

- [ ] **Step 3: Run whitespace and scoped diff checks**

```powershell
git diff --check -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c' 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h' 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.spc'
git diff -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c' 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h' 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.spc'
```

Expected: no whitespace errors; only approved T9HX BP, power, Page Write polling, and Bulk Erase changes are present.

- [ ] **Step 4: Compile the driver into the local verification directory**

Run from PowerShell:

```powershell
$verifyDir = 'D:\HILO\ALL-1000\WAx4_M25PE40-VMPxxx\build\t9hx-bp-verification'
New-Item -ItemType Directory -Force -Path $verifyDir | Out-Null
$toolDir = 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium'
$rvdsInclude = 'C:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows'
$driverDir = 'D:\HILO\ALL-1000\WAx4_M25PE40-VMPxxx'
$objectDir = 'D:\HILO\ALL-1000\All1000Code'
& "$toolDir\armcc.exe" -c --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend -I"$driverDir" -I"$objectDir\h" -I"$objectDir" -J"$rvdsInclude" -J"$rvdsInclude\rw" "$driverDir\WAx4_M25PE40-VMPxxx.c" -o "$verifyDir\WAx4_M25PE40-VMPxxx.o"
if ($LASTEXITCODE -ne 0) { throw 'ARMCC compilation failed' }
```

Expected: ARMCC exits 0 and creates `WAx4_M25PE40-VMPxxx.o`.

- [ ] **Step 5: Link and convert without deployment**

```powershell
$objects = @(
    "$objectDir\_stackheap.o", "$objectDir\CFControl.o", "$objectDir\Disp.o",
    "$objectDir\FPGADrv.o", "$objectDir\FWDMM.o", "$objectDir\hs0_mmc.o",
    "$objectDir\Hspi.o", "$objectDir\Initial.o", "$objectDir\ISR.o",
    "$objectDir\Key.o", "$objectDir\Main.o", "$objectDir\MemInit.o",
    "$objectDir\MMUCache_asm.o", "$objectDir\SHA.o", "$objectDir\startup.o",
    "$objectDir\Subroutine.o", "$objectDir\SysClock.o", "$objectDir\SystemTask.o",
    "$objectDir\uMROS.o", "$objectDir\UpDnLoad.o", "$objectDir\USBCom.o",
    "$verifyDir\WAx4_M25PE40-VMPxxx.o"
)
& "$toolDir\armlink.exe" $objects -o "$verifyDir\WAx4_M25PE40-VMPxxx.axf" --map --ro_base=0x30000000 --rw_base=0x30200000 '--first=startup.o(Init)' --entry=0x30000000 --info=sizes --info=totals --list "$verifyDir\map.txt"
if ($LASTEXITCODE -ne 0) { throw 'ARMLINK failed' }
& "$toolDir\fromelf.exe" --bin "$verifyDir\WAx4_M25PE40-VMPxxx.axf" --output "$verifyDir\WAx4_M25PE40-VMPxxx.bin"
if ($LASTEXITCODE -ne 0) { throw 'FROMELF conversion failed' }
Get-ChildItem -LiteralPath $verifyDir -File | Select-Object Name,Length,LastWriteTime
```

Expected: link and conversion exit 0; `.o`, `.axf`, `.bin`, and `map.txt` exist only in the verification directory.

- [ ] **Step 6: Review final behavior and record hardware gates**

Confirm from the scoped diff that:

- WRSR polls WIP to 0 before BP verification.
- BP, WEL, and WIP gang decisions use `CheckAllStatusData()`.
- `ReadStatusData()` is limited to a selected single-socket read.
- Both power functions explicitly hold S# HIGH before their VCC transition.
- `C7h` is sent with no address/data bytes and WIP must assert before completion polling.
- Page Write uses a 30 ms WIP timeout and verifies the full page.

Record as hardware-only gates: T9HX marking confirmation, W# HIGH, RESET# HIGH, all eight BP input combinations, mixed four-socket failures, C7h rejection detection, successful Bulk Erase plus Blank Check, JEDEC ID, Page Program, and serial-number Page Write.

- [ ] **Step 7: Commit any Task 4 source correction**

If Step 1 required a serial-number correction, run:

```powershell
git add -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c'
git commit -m 'fix: poll M25PE40 page write completion'
```

If no source correction was required, do not create an empty commit.

