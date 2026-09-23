# M25PE40 Core Flow and Lock Register Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make M25PE40 erase and sector protection follow the Micron Rev. D Lock Register and Bulk Erase flows.

**Architecture:** Keep the ALL-1000 SPI/gang primitives and model each physical 64 KiB sector explicitly. Add small Lock Register helpers, make protection configuration an eight-byte virtual view, and make Bulk Erase reject protected or non-started operations instead of reporting false success.

**Tech Stack:** ARM C89, ALL-1000 firmware ABI, ARMCC RVCT 2.2, PowerShell static checks

## Global Constraints

- Preserve the current IDCheck behavior, voltage values, SPI delay, and VFQFPN8 package identity.
- Expose only reversible Lock Register Write Lock bit 0; never set Lock Down bit 1.
- Keep generated build output inside the workspace and do not deploy to `Algo1`.
- Do not modify unrelated dirty-worktree files.

---

### Task 1: Correct sector and protection metadata

**Files:**
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h`
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.spc`

**Interfaces:**
- Consumes: ALL-1000 block-table and SPC formats
- Produces: eight main sectors, `Noun_BlkLockStatus`, SRAM lock bytes 0-7, control byte 8

- [ ] Replace the single main block with eight contiguous 64 KiB blocks.
- [ ] Replace the status-register special block with an eight-byte Lock Register block.
- [ ] Move the driver control byte to SRAM offset 8.
- [ ] Replace BP options with eight sector Write Lock options.
- [ ] Change the protection mask to bit 0 and erase timeout to 11 seconds.
- [ ] Run source assertions for ranges, nouns, offsets, and SPC options.

### Task 2: Implement Lock Register helpers and special-block operations

**Files:**
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c`

**Interfaces:**
- Produces: `ReadLockRegisterData(uInt32)`, `CheckAllLockRegisterData(uInt32,uChar)`, `WriteLockRegisterData(uInt32,uChar)`

- [ ] Add `E5h` and `E8h` command constants and Lock Register bit masks.
- [ ] Implement single-socket read, gang compare, and gang write/verify helpers.
- [ ] Route special-block read, verify, and blank check through the Lock Register helpers.
- [ ] Run static command-sequence checks.

### Task 3: Correct protect, unprotect, and bulk erase

**Files:**
- Modify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c`

**Interfaces:**
- Consumes: Lock Register helpers from Task 2
- Produces: reversible sector protection and accepted/completed Bulk Erase detection

- [ ] Add a gang-safe BP2-BP0 clear-and-verify helper.
- [ ] Make `ProtectFun()` clear BP and write eight configured lock bytes.
- [ ] Make `UnProtectFun()` clear BP and all eight lock bytes.
- [ ] Make `EraseFun()` unprotect, verify all protection is clear, send C7h, verify WIP=1, and poll WIP=0 with an 11-second timeout.
- [ ] Drive S# HIGH around power transitions without changing RESET# control.
- [ ] Run static assertions for protection and erase ordering.

### Task 4: Build and review

**Files:**
- Verify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c`
- Verify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.h`
- Verify: `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.spc`

**Interfaces:**
- Produces: local object, AXF, BIN, and map files

- [ ] Run `git diff --check` and inspect the scoped diff.
- [ ] Compile with ARMCC using the project's include paths.
- [ ] Link with the existing ALL1000 object set and convert with FROMELF.
- [ ] Report compiler output and remaining hardware checks; do not commit source changes unless requested.
