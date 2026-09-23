# WRP Page-Mask Undefined-Shift Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove unused WRP page-mask calculations so a full 32-page range cannot perform an undefined 32-bit shift.

**Architecture:** `ValidateProtectConfig` remains the sole validator for WRP area-A and area-B start/end fields. Remove only derived mask state that has no consumer; retain input validation and register-field output.

**Tech Stack:** C for ARM/CodeWarrior embedded driver; PowerShell and ripgrep for source-level verification.

## Global Constraints

- Do not change `WRP_FIELDS_MASK`, WRP start/end extraction, disabled-area encoding, or `TOTAL_PAGE` validation.
- Do not add a replacement mask calculation because no caller consumes its result.
- Do not modify the generated programming binary in this task.

---

### Task 1: Remove unused WRP page-mask state

**Files:**

- Modify: `WJ_STM32G071K8T3.c:932-992`
- Test: source-level verification against `WJ_STM32G071K8T3.c`

**Interfaces:**

- Consumes: `ValidateProtectConfig(uInt32, uInt32, uInt32, uInt32 *, uInt32 *)`.
- Produces: The same function signature, failure conditions, `SysStatusCode`, and masked `wrp1ar_reg`/`wrp1br_reg` outputs as before.

- [x] **Step 1: Inspect the pre-change calculation**

Run `rg -n -C 3 "wrp1(ar|br)_pages|1UL <<" WJ_STM32G071K8T3.c`.

Expected: two local page-mask variables and two range-length shift calculations appear in `ValidateProtectConfig`.

- [x] **Step 2: Make the minimal source change**

Delete the two `wrp1*_pages` declarations and their six assignments. Do not alter WRP field extraction or validation.

- [x] **Step 3: Verify no undefined shift or dead state remains**

Run `rg -n "wrp1(ar|br)_pages|1UL <<|PROTECT_ALL_PAGE_MASK" WJ_STM32G071K8T3.c`.

Expected: no matches. Manually confirm each WRP area still rejects `start > end` and `end >= TOTAL_PAGE`.

- [x] **Step 4: Review the focused change**

Run source-level inspection because `WJ_STM32G071K8T3.c` is untracked and must
not be staged by user direction.

Expected: only the unused declarations and assignments are removed.

- [ ] **Step 5: Commit when Git author identity is configured**

Run `git add WJ_STM32G071K8T3.c` and `git commit -m "fix: remove undefined WRP page mask shift"`.

Expected: one commit containing only the source change. If Git author identity remains unset, report the blocker.
