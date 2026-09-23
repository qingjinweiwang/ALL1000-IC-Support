# M25PE40 Status and Security Cleanup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove obsolete TH25Q80HB/GD Security Register code and use only the M25PE40 one-byte Status Register protocol.

**Architecture:** Preserve the existing fixed programmer callbacks and main-array paths. Remove unreachable pseudo-block branches, then simplify status data flow from a two-byte `stuShort` representation to one `uChar` shared by read, verify, protect, unprotect, and erase protection checks.

**Tech Stack:** ARM C driver, ALL-1000 firmware ABI tables, SPC option configuration, PowerShell batch build.

## Global Constraints

- Keep unrelated comments and formatting unchanged.
- Do not implement M25PE40 Lock Register commands.
- `SecureFun()` remains a successful no-op.

---

### Task 1: Remove obsolete Security Register branches

**Files:**
- Modify: `WJx4_M25PE40-VMPxxx.c`

**Interfaces:**
- Consumes: existing main-array and Status Register block dispatch.
- Produces: read/program/verify/erase/blank/checksum functions without `bBlkAttri_Res15` paths.

- [ ] Delete the six `else if( DevBlkAttri.Bit.bBlkAttri_Res15 )` branches.
- [ ] Remove unused Security/LB helpers and declarations.
- [ ] Scan for obsolete Security command identifiers and require no matches.

### Task 2: Convert status handling to one byte

**Files:**
- Modify: `WJx4_M25PE40-VMPxxx.c`
- Modify: `WJx4_M25PE40-VMPxxx.h`

**Interfaces:**
- Produces: `uChar ReadStatusData(void)` using only `ReadStatusCmd` (`05h`).

- [ ] Change status block dispatch from `Noun_Block8` to `Noun_Block1`.
- [ ] Remove `CheckAllStatus1Data()`, SR2 variables, high-byte masks, and high-byte table access.
- [ ] Change protect/unprotect to send and verify one status byte containing `BP2-BP0`.
- [ ] Keep erase precheck based only on `BP2-BP0`.

### Task 3: Clean SPC options and verify

**Files:**
- Modify: `WJx4_M25PE40-VMPxxx.spc`

**Interfaces:**
- Produces: one Status Register option group and operation-control byte at SRAM offset 3.

- [ ] Remove BP3, BP4, Status Register 2, CMP, and Security Register options.
- [ ] Preserve BP0-BP2 and status verify/blank-check control.
- [ ] Run residual-text scans.
- [ ] Run `iBuild.bat` and require a successful compilation.

