# Eliminate ARMCC Build Warnings Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Produce the existing driver binary with ARMCC reporting zero warnings and zero errors.

**Architecture:** Preserve all firmware ABI tables and runtime branches. Locally acknowledge the required ARMCC address-integer initializer extension, remove four provably unreachable tail returns, and exclude the deliberately disabled OTP implementation through conditional compilation.

**Tech Stack:** C89-style embedded C, ARMCC RVCT 2.2, Windows batch build

## Global Constraints

- Do not change firmware table types, values, order, or sizes.
- Do not enable OTP security locking.
- Do not globally suppress diagnostic 111 or 1296 in `iBuild.bat`.
- Preserve BIN/SPC output paths and cleanup behavior.

---

### Task 1: Remove the eight ARMCC warnings

**Files:**
- Modify: `WJx4_M25PE40-VMPxxx.h`
- Modify: `WJx4_M25PE40-VMPxxx.c`
- Test: `iBuild.bat WJx4_M25PE40-VMPxxx`

**Interfaces:**
- Consumes: existing firmware ABI tables and driver entry functions.
- Produces: identical table data and driver behavior with warning-free compilation.

- [x] **Step 1: Confirm the warning baseline**

Run:

```powershell
.\iBuild.bat WJx4_M25PE40-VMPxxx
```

Expected: `8 warnings, 0 errors`.

- [x] **Step 2: Scope diagnostic 1296 to the required ABI tables**

Insert before `DevBlockIdx_Table`:

```c
/* These firmware ABI tables store ROM addresses in 32-bit integer slots. */
#pragma diag_suppress 1296
```

Insert after `DevPmpIdx_Table`:

```c
#pragma diag_default 1296
```

- [x] **Step 3: Remove unreachable tail returns**

Delete only the final `return SUCCESS ;` statements after the exhaustive branches in `ReadFun`, `ProgramFun`, `VerifyFun`, and `BlankCheckFun`.

- [x] **Step 4: Express the disabled OTP implementation at preprocessing time**

Wrap the retained `SecureFun` OTP implementation in `#if 0`, remove its early unconditional return, and compile this existing behavior instead:

```c
#else
    return SUCCESS ;
#endif
```

- [x] **Step 5: Build and verify outputs**

Run:

```powershell
.\iBuild.bat WJx4_M25PE40-VMPxxx
```

Expected: `0 warnings, 0 errors`, successful build, and successful BIN/SPC updates to `D:\HILO\ALL-1000\Algo1`.

- [x] **Step 6: Inspect scope and cleanup**

Run `git diff --check`, inspect the source diff, and confirm no `.o`, `.axf`, `.bin`, or `map.txt` remains in the driver directory.
