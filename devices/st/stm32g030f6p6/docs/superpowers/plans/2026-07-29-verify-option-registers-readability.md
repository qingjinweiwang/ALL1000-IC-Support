# VerifyOptionRegistersAll Readability Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `VerifyOptionRegistersAll()` easy to read without changing its interface or runtime behavior.

**Architecture:** Replace one compound short-circuit condition with three ordered guard clauses. Each guard clause retains the original register, expected value, comparison mask, error code, and immediate-failure behavior.

**Tech Stack:** C, ARM RVCT 2.2, PowerShell, Git

## Global Constraints

- Modify only `VerifyOptionRegistersAll()` in `WJ_STM32G030F6P6.c`.
- Preserve the function signature and all callers.
- Preserve the check order: `FLASH_OPTR`, `FLASH_WRP1AR`, then `FLASH_WRP1BR`.
- Preserve short-circuit behavior: return `FAIL` immediately after the first failed check.
- Preserve every expected value, mask, and `E_ProtectFail` argument.
- Do not include the user's pre-existing `.c` or `.spc` changes in an automatic commit.

---

### Task 1: Split the option-register verification into readable guard clauses

**Files:**
- Modify: `WJ_STM32G030F6P6.c:915`
- Reference: `docs/superpowers/specs/2026-07-29-verify-option-registers-readability-design.md`

**Interfaces:**
- Consumes: `static uChar CheckFlashStatusAll(uInt32 addr, uInt32 expected, uInt32 mask, uChar errorCode)`
- Produces: unchanged `static uChar VerifyOptionRegistersAll(uInt32 optr, uInt32 wrp1ar, uInt32 wrp1br)`

- [ ] **Step 1: Record the current target function before editing**

Run:

```powershell
rg -n -A 16 -B 2 "static uChar VerifyOptionRegistersAll\(" WJ_STM32G030F6P6.c
```

Expected: one definition containing three `CheckFlashStatusAll()` calls joined by `||`, in the order `FLASH_OPTR`, `FLASH_WRP1AR`, `FLASH_WRP1BR`.

- [ ] **Step 2: Replace only the function body with ordered guard clauses**

Use this complete implementation:

```c
static uChar VerifyOptionRegistersAll(
	uInt32 optr,
	uInt32 wrp1ar,
	uInt32 wrp1br)
{
	/* OPTR: compare all 32 bits. */
	if (CheckFlashStatusAll(
		FLASH_OPTR, optr, ALL_BITS_32, E_ProtectFail) != SUCCESS)
	{
		return FAIL;
	}

	/* WRP1AR: compare only the write-protection fields. */
	if (CheckFlashStatusAll(
		FLASH_WRP1AR, wrp1ar, WRP_FIELDS_MASK, E_ProtectFail) != SUCCESS)
	{
		return FAIL;
	}

	/* WRP1BR: compare only the write-protection fields. */
	if (CheckFlashStatusAll(
		FLASH_WRP1BR, wrp1br, WRP_FIELDS_MASK, E_ProtectFail) != SUCCESS)
	{
		return FAIL;
	}

	return SUCCESS;
}
```

- [ ] **Step 3: Verify structural and parameter equivalence**

Run:

```powershell
rg -n -A 32 -B 2 "static uChar VerifyOptionRegistersAll\(" WJ_STM32G030F6P6.c
git diff --check -- WJ_STM32G030F6P6.c
```

Expected:

- Exactly three ordered calls remain.
- `FLASH_OPTR` uses `optr` and `ALL_BITS_32`.
- `FLASH_WRP1AR` uses `wrp1ar` and `WRP_FIELDS_MASK`.
- `FLASH_WRP1BR` uses `wrp1br` and `WRP_FIELDS_MASK`.
- Every call uses `E_ProtectFail`.
- `git diff --check` reports no whitespace errors.

- [ ] **Step 4: Build with the project's non-interactive build script**

Run:

```powershell
cmd /c iBuild.bat WJ_STM32G030F6P6
```

Expected: ARM compilation and linking finish with a success message and exit code `0`.

- [ ] **Step 5: Review the final source diff without committing unrelated work**

Run:

```powershell
git diff -- WJ_STM32G030F6P6.c
git status --short
```

Expected: the target function shows the guard-clause readability change. Existing unrelated modifications remain visible and uncommitted; do not stage or commit the dirty source file automatically.
