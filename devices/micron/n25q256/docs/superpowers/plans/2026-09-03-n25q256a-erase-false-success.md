# N25Q256A EraseFun False-Success Fix Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `EraseFun()` return `FAIL` when any enabled socket fails protection, WEL, WIP timeout, or FSR checks.

**Architecture:** Keep the existing SPI commands, CS timing, polling helpers, and gang error handler. Change only local control flow inside `EraseFun()` so every per-socket erase error is recorded and propagated as an overall failure.

**Tech Stack:** ARM C firmware, ALL-1000 FPGA SPI register interface, CodeWarrior/RVDS build.

## Global Constraints

- Modify only `WAx4_N25Q256Ax3ESFxxx.c` function `EraseFun()`.
- Do not change the SPI framework or command timing.
- Do not add any function.
- Do not add a duplicate main-array blank check to `EraseFun()`.

---

### Task 1: Enforce strict erase failure propagation

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:486`
- Test: existing CodeWarrior target `WAx4_N25Q256Ax3ESFxxx/RVDS/WAx4_N25Q256Ax3ESFxxx.mcp`

**Interfaces:**
- Consumes: `CheckAllStatusData(uChar)`, `WriteStatusWELBit(void)`, `CheckAllFlagStatusData(uChar)`, `GangCheckIfAllSocketError(uChar)`, and `GangErrSckMask`.
- Produces: unchanged `uChar EraseFun(void)` interface with strict any-enabled-socket failure semantics.

- [ ] **Step 1: Capture the current gang error mask and gang-check protection bits**

Replace the local declaration and single-channel protection check with:

```c
uChar StatusData, EntryErrSckMask ;

/* ...existing socket selection and IfFirstTimeEntry()... */
EntryErrSckMask = GangErrSckMask ;
PtrData(GSetAllMaskDataLo) = LSB(PrtRegMaskBit) ;
StatusData = CheckAllStatusData(0x00) ;
if( StatusData )
{
    GangCheckIfAllSocketError(StatusData) ;
    SysStatusCode = E_EraseFail ;
    return FAIL ;
}
```

This requires BP3, TB, and BP2-BP0 to be zero on every enabled socket, as required for N25Q256A Bulk Erase.

- [ ] **Step 2: Reject partial WEL failure**

Keep the existing `WriteStatusWELBit()` call and extend its condition:

```c
if( !WriteStatusWELBit() || (GangErrSckMask != EntryErrSckMask) )
{
    SysStatusCode = E_EraseFail ;
    return FAIL ;
}
```

The shared helper remains unchanged; comparing the masks detects its existing partial-socket error path.

- [ ] **Step 3: Return failure for any WIP timeout**

Replace the timeout branch with:

```c
else if( !(--PollingTimeCnt) )
{
    GangCheckIfAllSocketError(StatusData) ;
    SysStatusCode = E_EraseFail ;
    return FAIL ;
}
```

- [ ] **Step 4: Return failure for any FSR erase/protection error**

Replace the final FSR condition with:

```c
if( StatusData )
{
    GangCheckIfAllSocketError(StatusData) ;
    SysStatusCode = E_EraseFail ;
    return FAIL ;
}
```

- [ ] **Step 5: Perform static checks**

Run:

```powershell
rg -n -A 70 -B 2 "uChar EraseFun\(void\)" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
git diff --check -- WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
```

Expected: the function contains no new SPI command path or function call except existing helpers; `git diff --check` has no output.

- [ ] **Step 6: Build the driver**

Run the existing `WAx4_N25Q256Ax3ESFxxx/iBuild.bat` build procedure.

Expected: build completes with zero errors and no new warnings.

- [ ] **Step 7: Review the final diff**

Run:

```powershell
git diff -- WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
```

Expected: only `EraseFun()` control flow is changed; command `C7h`, CS transitions, delays, WIP polling interval, and FSR mask remain unchanged.
