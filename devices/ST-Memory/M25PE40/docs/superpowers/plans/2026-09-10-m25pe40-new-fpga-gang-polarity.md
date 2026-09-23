# M25PE40 New-FPGA Gang Polarity Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Convert every M25PE40 gang comparison to the new FPGA PASS-mask convention, service EP0 during long Bulk Erase polling, and restore all valid sockets after parameter loading.

**Architecture:** Keep the FPGA register helpers returning their raw PASS mask and normalize it at every decision boundary with `ChangeBits()`. Preserve all expected device data, SPI commands, error codes, retry bounds, and existing T9HX protection behavior.

**Tech Stack:** ARM C, ALL-1000 FPGA register interface, ARMCC/RVCT 2.2, PowerShell static assertions.

## Global Constraints

- Target the new FPGA convention exclusively: active `GChkAllSckStatus` bits are `1` for PASS and `0` for FAIL.
- Modify only `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c`.
- Do not change WEL expected data: `WriteStatusWELBit()` must still compare `StatusWELBit`.
- Do not change SPI commands, timing-table values, BP semantics, power sequencing, serial-number command selection, block metadata, or `.spc` data.
- Do not deploy the verification image.

---

### Task 1: Normalize all gang results to error masks

**Files:**
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c:30-805,941`

**Interfaces:**
- Consumes: `GangErrSckMask`, raw PASS masks returned by `GChkAllSckStatus` and `CheckAllStatusData(uChar)`
- Produces: `GangOkSckMask`, `ChangeBits(x)`, error masks accepted by `GangCheckIfAllSocketError(uChar)`

- [ ] **Step 1: Run the failing polarity assertion**

```powershell
$p = 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.c'
$s = Get-Content -Raw -Encoding UTF8 -LiteralPath $p
if ($s -notmatch '#define\s+GangOkSckMask') { throw 'GangOkSckMask missing' }
if ($s -notmatch '#define\s+ChangeBits\(x\)') { throw 'ChangeBits missing' }
```

Expected: FAIL with `GangOkSckMask missing`.

- [ ] **Step 2: Add the new FPGA normalization macros**

After the driver state variables, add:

```c
/* New FPGA: GChkAllSckStatus bit=1 means PASS, bit=0 means FAIL. */
#define GangOkSckMask ((uChar)(~GangErrSckMask) & 0x0F)
#define ChangeBits(x) ((uChar)(~(x)) & GangOkSckMask)
```

- [ ] **Step 3: Convert all 15 decision points**

For Program, Verify, Erase, Blank Check, ID Check, `WriteStatusWELBit()`, `WriteStatusBPBits()`, and `DevSNProgAndVerify()`:

```c
if( ChangeBits(StatusData) == 0 )
    break ;

if( GangCheckIfAllSocketError(ChangeBits(StatusData)) )
```

Use the equivalent `ChangeBits(DevByteData)`, `ChangeBits(MftID)`, or `ChangeBits(DevIDLo)` for those variables. Do not change calls such as:

```c
StatusData = CheckAllStatusData(StatusWELBit) ;
StatusData = CheckAllStatusData(StatusWIPBit) ;
StatusData = CheckAllStatusData(BPData) ;
```

- [ ] **Step 4: Run complete polarity assertions**

```powershell
$p = 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.c'
$s = Get-Content -Raw -Encoding UTF8 -LiteralPath $p
$rawSites = [regex]::Matches($s, '(StatusData|DevByteData)\s*=\s*CheckAllStatusData\(').Count + [regex]::Matches($s, '=\s*PtrData\(GChkAllSckStatus\)').Count - 1
if ($rawSites -ne 15) { throw "Unexpected raw gang-result site count: $rawSites" }
if ($s -notmatch '#define\s+GangOkSckMask\s+\(\(uChar\)\(~GangErrSckMask\)\s*&\s*0x0F\)') { throw 'active-socket mask is wrong' }
if ($s -notmatch '#define\s+ChangeBits\(x\)') { throw 'ChangeBits missing' }
if ([regex]::Matches($s, 'ChangeBits\(').Count -lt 28) { throw 'not every gang-result decision was converted' }
if ($s -match 'GangCheckIfAllSocketError\((StatusData|DevByteData|MftID|DevIDLo)\)') { throw 'raw gang mask reaches error handler' }
if ($s -notmatch 'CheckAllStatusData\(StatusWELBit\)') { throw 'WEL expected data changed' }
```

Expected: PASS with no exception.

- [ ] **Step 5: Run whitespace review and commit**

```powershell
git diff --check -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c'
git diff -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c'
git add -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c'
git commit -m 'fix: adopt new FPGA gang result polarity'
```

Expected: one source-only commit; no header or unrelated working-tree changes are staged.

---

### Task 2: Service long erase waits and restore gang selection

**Files:**
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c:440-459,669-678`
- Create build outputs only: `WAx4_M25PE40-VMPxxx/build/new-fpga-gang-verification/`

**Interfaces:**
- Consumes: `PollingTimeCnt`, `HandleEvent_EP0(void)`, `GangErrSckMask`, `GEnSelSckNum`
- Produces: responsive Bulk Erase wait and a restored active-socket selection after `GetDeviceParameterInfo()`

- [ ] **Step 1: Run failing source assertions**

```powershell
$p = 'WAx4_M25PE40-VMPxxx\WAx4_M25PE40-VMPxxx.c'
$s = Get-Content -Raw -Encoding UTF8 -LiteralPath $p
$erase = $s.Substring($s.IndexOf('uChar EraseFun(void)'), $s.IndexOf('uChar BlankCheckFun(void)') - $s.IndexOf('uChar EraseFun(void)'))
$params = $s.Substring($s.IndexOf('void GetDeviceParameterInfo(void)'), $s.IndexOf('uChar ReadStatusData(void)', $s.IndexOf('void GetDeviceParameterInfo(void)')) - $s.IndexOf('void GetDeviceParameterInfo(void)'))
if ([regex]::Matches($erase, 'HandleEvent_EP0\(\)').Count -ne 1) { throw 'Bulk Erase EP0 service missing' }
if ($params -notmatch 'PtrData\(GEnSelSckNum\)\s*=\s*\(uChar\)\(~GangErrSckMask\)') { throw 'parameter load does not restore gang sockets' }
```

Expected: FAIL because both additions are absent.

- [ ] **Step 2: Add EP0 servicing to the Bulk Erase completion loop**

After its `Delay_1ms(10)` statement, add:

```c
if( !(PollingTimeCnt & 0x7F) )
{
    HandleEvent_EP0() ;
    Delay_1ms(10) ;
}
```

Do not add this to the short WIP-assertion loop.

- [ ] **Step 3: Restore all valid sockets after parameter loading**

At the end of `GetDeviceParameterInfo()` add:

```c
PtrData(GEnSelSckNum) = (uChar)(~GangErrSckMask) ;
Delay_10nsX(3) ;
```

- [ ] **Step 4: Re-run Task 2 assertions and source checks**

Run the Step 1 assertions again, followed by:

```powershell
git diff --check -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c'
```

Expected: all assertions pass and no whitespace errors are reported.

- [ ] **Step 5: Compile, link, and convert without deployment**

```powershell
$verifyDir = 'D:\HILO\ALL-1000\WAx4_M25PE40-VMPxxx\build\new-fpga-gang-verification'
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

- [ ] **Step 6: Commit the architecture integration unit**

```powershell
git add -- 'WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c'
git commit -m 'fix: service erase polling and restore gang sockets'
```

Expected: one source-only commit.
