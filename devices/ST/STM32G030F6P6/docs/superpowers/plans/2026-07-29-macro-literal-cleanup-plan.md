# WJ_STM32G030F6P6.c Macro and Literal Cleanup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Simplify `WJ_STM32G030F6P6.c` by retaining frequent and hardware-significant macros while replacing low-frequency masks and fixed control values with commented hexadecimal literals.

**Architecture:** This is a behavior-preserving, single-file refactor. The work is divided into dead-definition removal, SWD/loop literal inlining, and Flash/Option Byte literal inlining; each stage must compile before the next stage starts.

**Tech Stack:** C89-style embedded C, ARM RVCT 2.2/CodeWarrior project, PowerShell and ripgrep for static checks.

## Global Constraints

- Modify only `WJ_STM32G030F6P6.c`; do not change `.h`, `.spc`, FPGA sources, or project configuration.
- Preserve SWD timing, register access order, multi-socket behavior, error handling, and all function interfaces.
- Retain register addresses, SWD DP/AP commands, unlock keys, device geometry, hardware identity, and frequently reused masks as named macros.
- Every inlined hexadecimal value must preserve the original type suffix and have a nearby comment explaining its meaning.
- Do not add Protect input validation in this refactor; that is a separate behavioral change.

---

### Task 1: Capture the baseline and remove dead explanatory definitions

**Files:**
- Modify: `WJ_STM32G030F6P6.c:113-259`

**Interfaces:**
- Consumes: ARM RVCT 2.2 compiler and the existing `D:\HILO\ALL-1000\All1000Code` headers.
- Produces: A compiling source file with no inactive component macros that are already represented by retained combined values.

- [ ] **Step 1: Capture the clean baseline**

Run:

```powershell
git status --short
git diff -- WJ_STM32G030F6P6.c
$armcc = 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium\armcc.exe'
$object = Join-Path $env:TEMP 'WJ_STM32G030F6P6_refactor.o'
& $armcc -c --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend `
  --diag_suppress=1296 --diag_suppress=111 `
  '-I.' '-ID:\HILO\ALL-1000\All1000Code' '-ID:\HILO\ALL-1000\All1000Code\h' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows\rw' `
  WJ_STM32G030F6P6.c -o $object
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
```

Expected: no pre-existing source diff and a successful baseline build.

- [ ] **Step 2: Remove definitions that have no active-code consumers**

Delete these inactive component definitions:

```c
#define OnlyUse1TimePowerCtrl        1

#define AP_CSW_32BIT_TRANSFER   0x00000002L
#define AP_CSW_AUTO_INCREMENT   0x00000010L
#define AP_CSW_HPROT_PRIV       0x01000000L
#define AP_CSW_MASTERTYPE_DEBUG 0x20000000L
#define AP_CSW_HPROT            0x02000000L

#define DP_ABORT_ORUNERRCLR     0x00000010UL
#define DP_ABORT_WDERRCLR       0x00000008UL
#define DP_ABORT_STKERRCLR      0x00000004UL
#define DP_ABORT_STKCMPCLR      0x00000002UL

#define DP_CTRL_CDBGPWRUPREQ    0x10000000UL
#define DP_CTRL_CDBGPWRUPACK    0x20000000UL
#define DP_CTRL_CSYSPWRUPREQ    0x40000000UL
#define DP_CTRL_CSYSPWRUPACK    0x80000000UL
#define DP_CTRL_MASKLANE_ALL    0x00000F00L
#define DP_CTRL_POWER_UP_ACK    0xA0000000UL
```

Also remove the two inactive `// #if OnlyUse1TimePowerCtrl` blocks from `PowerOnFun()` and `PowerOffFun()`. Do not alter the active statements in either function.

- [ ] **Step 3: Collapse Option Byte composite masks to their final values**

Replace the component-based definitions with:

```c
/* BSY | CFGBSY */
#define FLASH_OPTION_BUSY_MASK     0x00050000UL
/* Base program errors | MISSERR | FASTERR | RDERR | OPTVERR */
#define FLASH_OPTION_ERROR_MASK    0x0000C3FAUL
/* Option errors | EOP */
#define FLASH_OPTION_CLEAR_MASK    0x0000C3FBUL
```

Then delete the definitions that no longer have active consumers:

```c
#define FLASH_SR_OPERR
#define FLASH_SR_PROGERR
#define FLASH_SR_WRPERR
#define FLASH_SR_PGAERR
#define FLASH_SR_SIZERR
#define FLASH_SR_PGSERR
#define FLASH_SR_MISSERR
#define FLASH_SR_FASTERR
#define FLASH_SR_RDERR
#define FLASH_SR_OPTVERR
#define FLASH_SR_CFGBSY
```

Retain `FLASH_SR_EOP`, `FLASH_SR_BSY`, `FLASH_ERROR_MASK`, and `FLASH_CLEAR_MASK`.

- [ ] **Step 4: Verify removed names have no active references**

Run:

```powershell
rg -n 'OnlyUse1TimePowerCtrl|AP_CSW_32BIT_TRANSFER|AP_CSW_AUTO_INCREMENT|AP_CSW_HPROT_PRIV|AP_CSW_MASTERTYPE_DEBUG|DP_ABORT_ORUNERRCLR|DP_ABORT_WDERRCLR|DP_ABORT_STKERRCLR|DP_ABORT_STKCMPCLR|DP_CTRL_CDBGPWRUPREQ|DP_CTRL_CDBGPWRUPACK|DP_CTRL_CSYSPWRUPREQ|DP_CTRL_CSYSPWRUPACK|DP_CTRL_MASKLANE_ALL|DP_CTRL_POWER_UP_ACK|FLASH_SR_OPERR|FLASH_SR_PROGERR|FLASH_SR_WRPERR|FLASH_SR_PGAERR|FLASH_SR_SIZERR|FLASH_SR_PGSERR|FLASH_SR_MISSERR|FLASH_SR_FASTERR|FLASH_SR_RDERR|FLASH_SR_OPTVERR|FLASH_SR_CFGBSY' WJ_STM32G030F6P6.c
```

Expected: no matches.

- [ ] **Step 5: Build and review the dead-definition cleanup**

Run:

```powershell
$armcc = 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium\armcc.exe'
$object = Join-Path $env:TEMP 'WJ_STM32G030F6P6_refactor.o'
& $armcc -c --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend `
  --diag_suppress=1296 --diag_suppress=111 `
  '-I.' '-ID:\HILO\ALL-1000\All1000Code' '-ID:\HILO\ALL-1000\All1000Code\h' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows\rw' `
  WJ_STM32G030F6P6.c -o $object
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
git diff --check -- WJ_STM32G030F6P6.c
git diff --stat -- WJ_STM32G030F6P6.c
git add -- WJ_STM32G030F6P6.c
git commit -m "refactor: remove unused STM32G030 driver macros"
```

Expected: build succeeds, the whitespace check is silent, and the commit contains only dead-definition and comment cleanup.

---

### Task 2: Inline low-frequency SWD and loop-control constants

**Files:**
- Modify: `WJ_STM32G030F6P6.c:95-108`
- Modify: `WJ_STM32G030F6P6.c:415-585`
- Modify: `WJ_STM32G030F6P6.c:763-819`
- Modify: `WJ_STM32G030F6P6.c:885-925`
- Modify: `WJ_STM32G030F6P6.c:1008-1278`

**Interfaces:**
- Consumes: Existing SWD helper functions and their exact command/data ordering.
- Produces: Identical SWD and event-polling behavior with local hexadecimal values documented at their use sites.

- [ ] **Step 1: Inline single-purpose data-path masks**

Replace:

```c
OutDataLo |= READ_SHIFT_IN_MASK;
OutDataHi |= READ_SHIFT_IN_MASK;
```

with:

```c
OutDataLo |= 0x8000; /* Shift sampled SWD bit into bit 15. */
OutDataHi |= 0x8000; /* Shift sampled SWD bit into bit 15. */
```

Replace:

```c
PtrData(GSetAllMaskDataLo) = ACK_FPGA_MASK;
```

with:

```c
PtrData(GSetAllMaskDataLo) = 0xE0; /* FPGA mask for the three SWD ACK bits. */
```

Delete `READ_SHIFT_IN_MASK` and `ACK_FPGA_MASK`.

- [ ] **Step 2: Inline SWD reset and JTAG-to-SWD sequence values**

Change the reset loop bound to:

```c
for (TempCnt = 0; TempCnt < 0x37; TempCnt++) /* At least 55 SWCLK cycles. */
```

Change the sequence to:

```c
Send_8Bit_DataCmd(0x9E); /* JTAG-to-SWD selection sequence, byte 0. */
Send_8Bit_DataCmd(0xE7); /* JTAG-to-SWD selection sequence, byte 1. */
Send_8Bit_DataCmd(0xAA); /* Trailing selection/idle byte. */
```

Delete `LINE_RESET_CYCLES` and `JTAG_TO_SWD_BYTE0/1/2`.

- [ ] **Step 3: Inline one-call DP control payloads**

In `connecting()`, replace the four low-frequency combined macros with:

```c
SWD_Write(DPACC_ABORT_WR, 0x0000001EUL); /* Clear sticky DP errors. */
SWD_Write(DPACC_CTRLSTAT_WR, 0x50000000UL); /* Request debug and system power-up. */

sckStatus |= SWD_Verify(DPACC_DP_CTRLSTAT_RD,
                        0xF0000000UL, /* Power request/ack field. */
                        0xF0000000UL);

SWD_Write(DPACC_CTRLSTAT_WR, 0x50000F00UL); /* Keep power-up requests and enable all lanes. */
```

Delete `DP_ABORT_CLEAR_ERRORS`, `DP_CTRL_POWER_UP_REQ`, `DP_CTRL_POWER_UP_MASK`, and `DP_CTRL_DEFAULT`. Retain `DP_SELECT_AHB_AP`.

- [ ] **Step 4: Inline low-frequency polling cadence values**

Replace the `WaitFlashStatusAll()` USB cadence test with:

```c
if (!(waitLimit & 0x00FFUL)) /* Service USB once per 256 polls. */
```

Replace both block-processing event tests that use `USB_EVENT_MASK` with:

```c
if (!(dev_addr & 0x0000FFFFUL)) /* Service USB at each 64 KiB address boundary. */
```

and:

```c
if (!(DevCurrBlkLen.all & 0x0000FFFFUL)) /* Service USB every 64 KiB remaining. */
```

Delete `FLASH_USB_POLL_MASK` and `USB_EVENT_MASK`.

- [ ] **Step 5: Check, build, and review**

Run:

```powershell
rg -n 'READ_SHIFT_IN_MASK|ACK_FPGA_MASK|LINE_RESET_CYCLES|JTAG_TO_SWD_BYTE[012]|DP_ABORT_CLEAR_ERRORS|DP_CTRL_POWER_UP_REQ|DP_CTRL_POWER_UP_MASK|DP_CTRL_DEFAULT|FLASH_USB_POLL_MASK|USB_EVENT_MASK' WJ_STM32G030F6P6.c
```

Expected: no matches.

Then run:

```powershell
$armcc = 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium\armcc.exe'
$object = Join-Path $env:TEMP 'WJ_STM32G030F6P6_refactor.o'
& $armcc -c --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend `
  --diag_suppress=1296 --diag_suppress=111 `
  '-I.' '-ID:\HILO\ALL-1000\All1000Code' '-ID:\HILO\ALL-1000\All1000Code\h' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows\rw' `
  WJ_STM32G030F6P6.c -o $object
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
git diff --check -- WJ_STM32G030F6P6.c
git add -- WJ_STM32G030F6P6.c
git commit -m "refactor: inline local SWD control constants"
```

Expected: build succeeds and no macro name scheduled for deletion remains.

---

### Task 3: Inline low-frequency Flash and Option Byte fields

**Files:**
- Modify: `WJ_STM32G030F6P6.c:181-232`
- Modify: `WJ_STM32G030F6P6.c:929-940`
- Modify: `WJ_STM32G030F6P6.c:1073-1204`
- Modify: `WJ_STM32G030F6P6.c:1284-1351`
- Modify: `WJ_STM32G030F6P6.c:1455-1558`

**Interfaces:**
- Consumes: Existing `ProgramFun()`, `EraseFun()`, and `ProtectFun()` behavior.
- Produces: The same Flash register values and Option Byte data with local comments documenting each bit value.

- [ ] **Step 1: Inline one-operation Flash CR fields**

In `ProgramFun()`, replace:

```c
swd_writeAPResource(FLASH_CR, FLASH_CR_PG);
```

with:

```c
swd_writeAPResource(FLASH_CR, 0x00000001UL); /* PG: enable double-word programming. */
```

In `EraseFun()`, replace:

```c
cr = FLASH_CR_PER | (page << FLASH_CR_PNB_SHIFT);
...
cr |= FLASH_CR_STRT;
```

with:

```c
cr = 0x00000002UL | (page << 3); /* PER plus page number in PNB[7:3]. */
...
cr |= 0x00010000UL; /* STRT: start page erase. */
```

Replace:

```c
Delay_1ms(FLASH_PAGE_ERASE_DELAY_MS);
```

with:

```c
Delay_1ms(0x32); /* 50 ms page-erase settling delay. */
```

Delete `FLASH_CR_PG`, `FLASH_CR_PER`, `FLASH_CR_STRT`, `FLASH_CR_PNB_SHIFT`, and `FLASH_PAGE_ERASE_DELAY_MS`.

- [ ] **Step 2: Inline `.spc` offsets and RDP comparison values**

In `ProtectFun()`, replace:

```c
optr = GetSpecialBitDWord(SPC_OPTR_OFFSET);
wrp1ar = GetSpecialBitDWord(SPC_WRP1AR_OFFSET);
wrp1br = GetSpecialBitDWord(SPC_WRP1BR_OFFSET);
```

with:

```c
optr = GetSpecialBitDWord(0x00);   /* .spc OPTR field. */
wrp1ar = GetSpecialBitDWord(0x08); /* .spc WRP1AR field. */
wrp1br = GetSpecialBitDWord(0x0C); /* .spc WRP1BR field. */
```

Replace:

```c
if ((optr & RDP_MASK) != RDP_LEVEL0)
```

with:

```c
if ((optr & 0x000000FFUL) != 0x000000AAUL) /* RDP byte is not Level 0. */
```

Delete `SPC_OPTR_OFFSET`, `SPC_WRP1AR_OFFSET`, `SPC_WRP1BR_OFFSET`, `RDP_MASK`, and `RDP_LEVEL0`. Retain `WRP_FIELDS_MASK` because it has multiple active uses.

- [ ] **Step 3: Inline Option Byte start and reload commands**

Replace:

```c
swd_writeAPResource(FLASH_CR, FLASH_CR_OPTSTRT);
```

with:

```c
swd_writeAPResource(FLASH_CR, 0x00020000UL); /* OPTSTRT: program Option Bytes. */
```

Replace:

```c
swd_writeAPResource(FLASH_CR, FLASH_CR_OBL_LAUNCH);
```

with:

```c
swd_writeAPResource(FLASH_CR, 0x08000000UL); /* OBL_LAUNCH: reload Option Bytes and reset. */
```

Delete `FLASH_CR_OPTSTRT` and `FLASH_CR_OBL_LAUNCH`. Retain `FLASH_CR_LOCK`, `FLASH_CR_OPTLOCK`, `FLASH_CR_IDLE`, and all register/key macros.

- [ ] **Step 4: Check, build, and review**

Run:

```powershell
rg -n 'FLASH_CR_PG|FLASH_CR_PER|FLASH_CR_STRT|FLASH_CR_PNB_SHIFT|FLASH_PAGE_ERASE_DELAY_MS|SPC_OPTR_OFFSET|SPC_WRP1AR_OFFSET|SPC_WRP1BR_OFFSET|RDP_MASK|RDP_LEVEL0|FLASH_CR_OPTSTRT|FLASH_CR_OBL_LAUNCH' WJ_STM32G030F6P6.c
```

Expected: no matches.

Then run:

```powershell
$armcc = 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium\armcc.exe'
$object = Join-Path $env:TEMP 'WJ_STM32G030F6P6_refactor.o'
& $armcc -c --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend `
  --diag_suppress=1296 --diag_suppress=111 `
  '-I.' '-ID:\HILO\ALL-1000\All1000Code' '-ID:\HILO\ALL-1000\All1000Code\h' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows\rw' `
  WJ_STM32G030F6P6.c -o $object
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
git diff --check -- WJ_STM32G030F6P6.c
git add -- WJ_STM32G030F6P6.c
git commit -m "refactor: inline local Flash control constants"
```

Expected: build succeeds with no scheduled macro name left in the file.

---

### Task 4: Final semantic audit

**Files:**
- Review: `WJ_STM32G030F6P6.c`

**Interfaces:**
- Consumes: Refactored source from Tasks 1–3.
- Produces: Evidence that the cleanup changed representation only, not behavior.

- [ ] **Step 1: Confirm important macros remain**

Run:

```powershell
rg -n '#define (FLASH_CR|FLASH_SR|FLASH_KEYR|FLASH_OPTKEYR|FLASH_OPTR|FLASH_WRP1AR|FLASH_WRP1BR|FLASH_KEY1|FLASH_KEY2|FLASH_OPTKEY1|FLASH_OPTKEY2|DPACC_|APACC_|FLASH_SIZE|FLASH_PAGE_SIZE|TOTAL_PAGE|DPIDR)' WJ_STM32G030F6P6.c
```

Expected: register addresses, SWD commands, unlock keys, device geometry, and DPIDR remain named.

- [ ] **Step 2: Review the complete functional diff**

Run:

```powershell
git diff b78cc4c..HEAD -- WJ_STM32G030F6P6.c
```

Expected: every changed active expression is a constant substitution with the same numeric value; no branch, loop, return path, register access, or delay call has been added, removed, or reordered.

- [ ] **Step 3: Run the final build and inspect repository state**

Run:

```powershell
$armcc = 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium\armcc.exe'
$object = Join-Path $env:TEMP 'WJ_STM32G030F6P6_refactor.o'
& $armcc -c --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend `
  --diag_suppress=1296 --diag_suppress=111 `
  '-I.' '-ID:\HILO\ALL-1000\All1000Code' '-ID:\HILO\ALL-1000\All1000Code\h' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows\rw' `
  WJ_STM32G030F6P6.c -o $object
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
git status --short
git log -5 --oneline
```

Expected: final build succeeds; only expected generated artifacts, if any, are present; all planned refactor commits follow the tracked-source baseline.
