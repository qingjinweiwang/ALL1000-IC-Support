# Protect Validation Readability Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the unreadable function-like protection macros and compile-time typedef tests with direct, sequential validation code while preserving every existing lockout rule.

**Architecture:** Keep register and mask constants at file scope, but place the complete OPTR and WRP decision flow inside `ValidateProtectConfig()`. Validate every WRP range before calculating its page mask so no invalid shift expression can execute.

**Tech Stack:** ARM C, ARMCC 2.2 build tools, STM32G030 option-byte registers.

## Global Constraints

- Reject `RDP=0xCC`.
- Treat `RDP=0xAA` as Level 0 and every other allowed value as Level 1.
- Reject a single WRP range or combined WRP ranges that cover all 16 Flash pages.
- Accept only WRP page numbers 0 through 15, or the disabled encoding `START=31, END=0`.
- Preserve `ProtectFun()` hardware sequencing and return behavior.
- Delete `PROTECT_CONFIG_SELF_TEST` without replacing it.
- Do not include the user's existing `.spc` or `.claude/settings.json` changes in the implementation commit.

---

### Task 1: Make Protect configuration validation sequential and readable

**Files:**
- Modify: `ALL-1000/WJ_STM32G030F6P6/WJ_STM32G030F6P6.c:193-255`
- Modify: `ALL-1000/WJ_STM32G030F6P6/WJ_STM32G030F6P6.c:308`
- Modify: `ALL-1000/WJ_STM32G030F6P6/WJ_STM32G030F6P6.c:981-1029`

**Interfaces:**
- Consumes: `ValidateProtectConfig(uInt32 optr, uInt32 wrp1ar, uInt32 wrp1br, uInt32 *wrp1ar_reg, uInt32 *wrp1br_reg)`
- Produces: The same function signature and `SUCCESS`/`FAIL` behavior; normalized WRP register values are returned through `wrp1ar_reg` and `wrp1br_reg`.

- [ ] **Step 1: Preserve the user's existing source and SPC edits**

Run from the repository root:

```powershell
git stash push -m "preserve local WJ edits before Protect readability refactor" -- ALL-1000/WJ_STM32G030F6P6/WJ_STM32G030F6P6.c ALL-1000/WJ_STM32G030F6P6/WJ_STM32G030F6P6.spc
git status --short
```

Expected: `.claude/settings.json` remains modified; the two WJ files no longer appear in the working-tree status. Do not include `.claude/settings.json` in the stash.

- [ ] **Step 2: Compile the current source as a baseline**

Run from `ALL-1000/WJ_STM32G030F6P6` with the existing ARMCC include paths:

```powershell
& 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium\armcc.exe' -c --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend --diag_suppress=1295 --diag_suppress=1296 --diag_suppress=111 --diag_suppress=177 -I. -ID:\HILO\ALL-1000\All1000Code -ID:\HILO\ALL-1000\All1000Code\h '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows' '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows\rw' WJ_STM32G030F6P6.c -o "$env:TEMP\protect_before.o"
```

Expected: exit code 0 and no compiler diagnostics.

- [ ] **Step 3: Remove macro-based validation and compile-time typedef tests**

Delete these five function-like macros:

```c
PROTECT_OPTR_RESERVED_VALID
PROTECT_RDP_IS_LEVEL2
PROTECT_WRP_IS_DISABLED
PROTECT_WRP_RANGE_VALID
PROTECT_WRP_PAGE_MASK
```

Delete the complete block from:

```c
#ifdef PROTECT_CONFIG_SELF_TEST
```

through its matching:

```c
#endif
```

Also remove the now-unused `PROTECT_OPTR_WRITABLE_MASK` constant, the `BuildWrpPageMask()` prototype, and the complete `BuildWrpPageMask()` function.

- [ ] **Step 4: Replace `ValidateProtectConfig()` with direct validation**

Use this implementation:

```c
static uChar ValidateProtectConfig(
	uInt32 optr,
	uInt32 wrp1ar,
	uInt32 wrp1br,
	uInt32 *wrp1ar_reg,
	uInt32 *wrp1br_reg)
{
	uInt32 wrp1ar_start;
	uInt32 wrp1ar_end;
	uInt32 wrp1ar_pages;
	uInt32 wrp1br_start;
	uInt32 wrp1br_end;
	uInt32 wrp1br_pages;

	if ((optr & PROTECT_OPTR_RESERVED_MASK) !=
		PROTECT_OPTR_RESERVED_VALUE)
	{
		SysStatusCode = E_ProtectFail;
		return FAIL;
	}

	if ((optr & PROTECT_RDP_MASK) == PROTECT_RDP_LEVEL2)
	{
		SysStatusCode = E_ProtectFail;
		return FAIL;
	}

	*wrp1ar_reg = wrp1ar & WRP_FIELDS_MASK;
	wrp1ar_start = *wrp1ar_reg & PROTECT_WRP_START_MASK;
	wrp1ar_end = (*wrp1ar_reg >> PROTECT_WRP_END_SHIFT) &
		PROTECT_WRP_START_MASK;

	if ((wrp1ar_start == PROTECT_WRP_DISABLED_START) &&
		(wrp1ar_end == 0))
	{
		wrp1ar_pages = 0;
	}
	else
	{
		if ((wrp1ar_start > wrp1ar_end) ||
			(wrp1ar_end >= TOTAL_PAGE))
		{
			SysStatusCode = E_ProtectFail;
			return FAIL;
		}

		wrp1ar_pages =
			((1UL << ((wrp1ar_end - wrp1ar_start) + 1)) - 1UL)
			<< wrp1ar_start;
		wrp1ar_pages &= PROTECT_ALL_PAGE_MASK;
	}

	*wrp1br_reg = wrp1br & WRP_FIELDS_MASK;
	wrp1br_start = *wrp1br_reg & PROTECT_WRP_START_MASK;
	wrp1br_end = (*wrp1br_reg >> PROTECT_WRP_END_SHIFT) &
		PROTECT_WRP_START_MASK;

	if ((wrp1br_start == PROTECT_WRP_DISABLED_START) &&
		(wrp1br_end == 0))
	{
		wrp1br_pages = 0;
	}
	else
	{
		if ((wrp1br_start > wrp1br_end) ||
			(wrp1br_end >= TOTAL_PAGE))
		{
			SysStatusCode = E_ProtectFail;
			return FAIL;
		}

		wrp1br_pages =
			((1UL << ((wrp1br_end - wrp1br_start) + 1)) - 1UL)
			<< wrp1br_start;
		wrp1br_pages &= PROTECT_ALL_PAGE_MASK;
	}

	if ((wrp1ar_pages | wrp1br_pages) == PROTECT_ALL_PAGE_MASK)
	{
		SysStatusCode = E_ProtectFail;
		return FAIL;
	}

	return SUCCESS;
}
```

- [ ] **Step 5: Verify source structure**

Run:

```powershell
rg -n "PROTECT_CONFIG_SELF_TEST|PROTECT_OPTR_RESERVED_VALID|PROTECT_RDP_IS_LEVEL2|PROTECT_WRP_IS_DISABLED|PROTECT_WRP_RANGE_VALID|PROTECT_WRP_PAGE_MASK|BuildWrpPageMask" WJ_STM32G030F6P6.c
```

Expected: no matches.

- [ ] **Step 6: Compile the refactored source**

Repeat the ARMCC command from Step 2 with output:

```powershell
"$env:TEMP\protect_after.o"
```

Expected: exit code 0 and no compiler diagnostics.

- [ ] **Step 7: Review the refactor diff**

Run:

```powershell
git diff --check
git diff -- ALL-1000/WJ_STM32G030F6P6/WJ_STM32G030F6P6.c
git status --short
```

Expected:

- No whitespace errors.
- `ProtectFun()` hardware sequence is unchanged.
- Only macro tests, `BuildWrpPageMask()`, and validation structure change in the relevant source diff.
- `.claude/settings.json` remains uncommitted; the stashed WJ edits are restored after the refactor commit.

- [ ] **Step 8: Commit only the source refactor**

```powershell
git add -- ALL-1000/WJ_STM32G030F6P6/WJ_STM32G030F6P6.c
git commit -m "refactor: simplify Protect configuration validation"
```

Expected: the commit contains only `WJ_STM32G030F6P6.c`.

- [ ] **Step 9: Restore the user's existing WJ edits**

Run:

```powershell
git stash pop
git status --short
```

Expected: the user's prior `.c` and `.spc` edits return as uncommitted modifications. If the `.c` file conflicts, preserve the new sequential `ValidateProtectConfig()` and preserve the user's unrelated comment removals; then mark the conflict resolved and leave both files unstaged.

- [ ] **Step 10: Compile the combined working-tree source**

Repeat the ARMCC command from Step 2 with output:

```powershell
"$env:TEMP\protect_combined.o"
```

Expected: exit code 0 and no compiler diagnostics.

- [ ] **Step 11: Confirm final repository state**

Run:

```powershell
git log -1 --oneline
git status --short
git diff --check
```

Expected:

- The latest commit is `refactor: simplify Protect configuration validation`.
- `.claude/settings.json`, `WJ_STM32G030F6P6.c`, and `WJ_STM32G030F6P6.spc` remain as the user's uncommitted modifications.
- There are no unmerged paths or whitespace errors.
