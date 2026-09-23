# N25Q256A UnProtect and Erase Start Verification Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make an already-unprotected device pass `UnProtectFun()` and prevent `EraseFun()` from succeeding unless Bulk Erase actually starts.

**Architecture:** Add one early status comparison inside `UnProtectFun()` and one post-command status comparison inside `EraseFun()`. Reuse the existing gang checker, status helper, SPI sequence, and error codes.

**Tech Stack:** ARM RVCT C, ALL-1000 firmware ABI, FPGA gang-SPI registers, Micron N25Q256A status protocol.

## Global Constraints

- Modify only `WAx4_N25Q256Ax3ESFxxx.c` function bodies `UnProtectFun()` and `EraseFun()`.
- Do not add functions or change shared SPI helpers.
- Do not add Blank Check logic to `EraseFun()`.
- Any enabled-socket erase-start mismatch returns `E_EraseFail`.

---

### Task 1: Correct no-op unprotect and verify erase start

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:413`
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:741`
- Test: existing `WAx4_N25Q256Ax3ESFxxx/iBuild.bat`

**Interfaces:**
- Consumes: `CheckAllStatusData(uChar)`, `GangCheckIfAllSocketError(uChar)`, `PrtRegMaskBit`, `StatusWIPBit`, and `StatusWELBit`.
- Produces: unchanged `uChar EraseFun(void)` and `uChar UnProtectFun(void)` callbacks.

- [ ] **Step 1: Return success when protection bits are already clear**

After computing `SR1Data`, set the gang comparison mask and return before `01h` when all enabled sockets already match:

```c
PtrData(GSetAllMaskDataLo) = LSB(PrtRegMaskBit) ;
StatusData = CheckAllStatusData(SR1Data) ;
if( !StatusData )
    return SUCCESS ;
```

- [ ] **Step 2: Reject a Bulk Erase command that did not start**

After the existing 50ms delay following `C7h`, require `WIP=1` and `WEL=0` on all enabled sockets:

```c
PtrData(GSetAllMaskDataLo) = StatusWIPBit | StatusWELBit ;
StatusData = CheckAllStatusData(StatusWIPBit) ;
if( StatusData )
{
    GangCheckIfAllSocketError(StatusData) ;
    SysStatusCode = E_EraseFail ;
    return FAIL ;
}
```

Then restore the existing WIP-only mask and completion loop.

- [ ] **Step 3: Perform static checks**

Run:

```powershell
rg -n -A 70 -B 2 "uChar EraseFun\(void\)|uChar UnProtectFun\(void\)" WAx4_N25Q256Ax3ESFxxx.c
```

Expected: the new checks appear inside the two existing functions; no helper is added.

- [ ] **Step 4: Build**

Run:

```powershell
cmd /c iBuild.bat
```

Expected: `[SUCCESS] Build successful.`
