# STM32G030F6P6 Protect Safety Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Prevent `ProtectFun()` from programming irreversible or invalid STM32G030F6P6 Option Byte configurations and require verified register contents before reporting success.

**Architecture:** Add small static validation helpers inside `WJ_STM32G030F6P6.c`, with compile-time characterization assertions for the pure safety predicates. `ProtectFun()` validates before any power or SWD mutation, then uses one shared register-verification helper before `OPTSTRT`, after programming, and after reload when RDP remains Level 0.

**Tech Stack:** C89-style embedded C, ARM RVCT 2.2, existing FPGA-assisted SWD gang-programming framework.

## Global Constraints

- RDP `0xAA` is allowed as Level 0.
- RDP `0xCC` is always rejected with `E_ProtectFail`.
- Every other RDP byte is allowed as Level 1.
- OPTR reserved bits must exactly match the STM32G030F6P6 reset pattern; never silently normalize an invalid OPTR.
- WRP enabled ranges must be within pages `0..15`; disabled ranges must use only `START=0x1F, END=0`.
- A/B WRP overlap is allowed, but any single or combined configuration covering all 16 pages is rejected.
- All input validation must finish before power cycling, Flash unlock, or any SWD write.
- Verify OPTR/WRP registers before `OPTSTRT` and again after Option Byte programming but before `OBL_LAUNCH`.
- Level 0 requires final post-reload readback; Level 1 may return success only after both pre-reload readbacks and status checks pass.
- Preserve the existing per-socket failure exclusion behavior.
- Do not modify Program, Erase, Read, Verify, BlankCheck, CheckSum, or public function signatures.

---

### Task 1: Add testable Protect configuration predicates and validators

**Files:**
- Modify: `WJ_STM32G030F6P6.c:220-300`
- Modify: `WJ_STM32G030F6P6.c:900-940`

**Interfaces:**
- Produces: `static uChar BuildWrpPageMask(uInt32 wrp, uInt32 *normalized, uInt32 *pageMask)`.
- Produces: `static uChar ValidateProtectConfig(uInt32 optr, uInt32 wrp1ar, uInt32 wrp1br, uInt32 *wrp1ar_reg, uInt32 *wrp1br_reg)`.
- Consumes: `TOTAL_PAGE`, `WRP_FIELDS_MASK`, `SUCCESS`, `FAIL`, `SysStatusCode`, and `E_ProtectFail`.

- [ ] **Step 1: Add failing compile-time characterization assertions**

Add the following test block near the Protect constants before defining the referenced predicates:

```c
#ifdef PROTECT_CONFIG_SELF_TEST
typedef char ProtectTest_DefaultOptr[
	PROTECT_OPTR_RESERVED_VALID(0xDFFFE1AAUL) ? 1 : -1];
typedef char ProtectTest_ReservedBitRejected[
	!PROTECT_OPTR_RESERVED_VALID(0xDFFFF1AAUL) ? 1 : -1];
typedef char ProtectTest_RdpLevel0Allowed[
	!PROTECT_RDP_IS_LEVEL2(0x000000AAUL) ? 1 : -1];
typedef char ProtectTest_RdpLevel1Allowed[
	!PROTECT_RDP_IS_LEVEL2(0x000000BBUL) ? 1 : -1];
typedef char ProtectTest_RdpLevel2Rejected[
	PROTECT_RDP_IS_LEVEL2(0x000000CCUL) ? 1 : -1];
typedef char ProtectTest_DisabledWrp[
	PROTECT_WRP_IS_DISABLED(0x1FUL, 0x00UL) ? 1 : -1];
typedef char ProtectTest_FirstPage[
	PROTECT_WRP_RANGE_VALID(0x00UL, 0x00UL) ? 1 : -1];
typedef char ProtectTest_LastPage[
	PROTECT_WRP_RANGE_VALID(0x0FUL, 0x0FUL) ? 1 : -1];
typedef char ProtectTest_Page16Rejected[
	!PROTECT_WRP_RANGE_VALID(0x10UL, 0x10UL) ? 1 : -1];
typedef char ProtectTest_NonCanonicalDisabledRejected[
	!PROTECT_WRP_RANGE_VALID(0x02UL, 0x01UL) ? 1 : -1];
typedef char ProtectTest_SplitFullCoverage[
	((PROTECT_WRP_PAGE_MASK(0x00UL, 0x07UL) |
	  PROTECT_WRP_PAGE_MASK(0x08UL, 0x0FUL)) == 0x0000FFFFUL) ? 1 : -1];
typedef char ProtectTest_OverlapPartialCoverage[
	((PROTECT_WRP_PAGE_MASK(0x00UL, 0x07UL) |
	  PROTECT_WRP_PAGE_MASK(0x04UL, 0x0AUL)) != 0x0000FFFFUL) ? 1 : -1];
#endif
```

Compile with:

```powershell
$armcc = 'C:\Program Files (x86)\ARM\RVCT\Programs\2.2\349\win_32-pentium\armcc.exe'
$object = Join-Path $env:TEMP 'WJ_STM32G030F6P6_protect_red.o'
& $armcc -c --cpu=ARM926EJ-S -O0 -Otime --fpu=None --littleend `
  --diag_suppress=1296 --diag_suppress=111 -DPROTECT_CONFIG_SELF_TEST `
  '-I.' '-ID:\HILO\ALL-1000\All1000Code' '-ID:\HILO\ALL-1000\All1000Code\h' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows' `
  '-JC:\Program Files (x86)\ARM\RVCT\Data\2.2\349\include\windows\rw' `
  WJ_STM32G030F6P6.c -o $object
```

Expected: compile fails because the `PROTECT_*` predicates are not defined.

- [ ] **Step 2: Define the safety constants and predicates**

Add:

```c
#define PROTECT_OPTR_WRITABLE_MASK       0x074F60FFUL
#define PROTECT_OPTR_RESERVED_MASK       0xF8B09F00UL
#define PROTECT_OPTR_RESERVED_VALUE      0xD8B08100UL
#define PROTECT_RDP_MASK                 0x000000FFUL
#define PROTECT_RDP_LEVEL2               0x000000CCUL
#define PROTECT_WRP_START_MASK           0x0000001FUL
#define PROTECT_WRP_END_SHIFT            16
#define PROTECT_WRP_DISABLED_START       0x0000001FUL
#define PROTECT_ALL_PAGE_MASK            0x0000FFFFUL

#define PROTECT_OPTR_RESERVED_VALID(_value_) \
	(((_value_) & PROTECT_OPTR_RESERVED_MASK) == PROTECT_OPTR_RESERVED_VALUE)
#define PROTECT_RDP_IS_LEVEL2(_value_) \
	(((_value_) & PROTECT_RDP_MASK) == PROTECT_RDP_LEVEL2)
#define PROTECT_WRP_IS_DISABLED(_start_, _end_) \
	(((_start_) == PROTECT_WRP_DISABLED_START) && ((_end_) == 0))
#define PROTECT_WRP_RANGE_VALID(_start_, _end_) \
	(((_start_) <= (_end_)) && ((_end_) < TOTAL_PAGE))
#define PROTECT_WRP_PAGE_MASK(_start_, _end_) \
	((((1UL << (((_end_) - (_start_)) + 1)) - 1UL) << (_start_)) & \
	 PROTECT_ALL_PAGE_MASK)
```

The named writable mask is retained for auditability. Add a compile-time relationship assertion:

```c
#ifdef PROTECT_CONFIG_SELF_TEST
typedef char ProtectTest_WhitelistComplementsReserved[
	((PROTECT_OPTR_WRITABLE_MASK ^ 0xFFFFFFFFUL) ==
	 PROTECT_OPTR_RESERVED_MASK) ? 1 : -1];
#endif
```

- [ ] **Step 3: Implement WRP normalization and page-mask construction**

Add the prototype and implementation:

```c
static uChar BuildWrpPageMask(
	uInt32 wrp,
	uInt32 *normalized,
	uInt32 *pageMask)
{
	uInt32 start;
	uInt32 end;

	*normalized = wrp & WRP_FIELDS_MASK;
	start = *normalized & PROTECT_WRP_START_MASK;
	end = (*normalized >> PROTECT_WRP_END_SHIFT) &
		PROTECT_WRP_START_MASK;

	if (PROTECT_WRP_IS_DISABLED(start, end))
	{
		*pageMask = 0;
		return SUCCESS;
	}

	if (!PROTECT_WRP_RANGE_VALID(start, end))
	{
		return FAIL;
	}

	*pageMask = PROTECT_WRP_PAGE_MASK(start, end);
	return SUCCESS;
}
```

- [ ] **Step 4: Implement the complete common-input validator**

Add:

```c
static uChar ValidateProtectConfig(
	uInt32 optr,
	uInt32 wrp1ar,
	uInt32 wrp1br,
	uInt32 *wrp1ar_reg,
	uInt32 *wrp1br_reg)
{
	uInt32 wrp1ar_pages;
	uInt32 wrp1br_pages;

	if (!PROTECT_OPTR_RESERVED_VALID(optr) ||
		PROTECT_RDP_IS_LEVEL2(optr) ||
		BuildWrpPageMask(wrp1ar, wrp1ar_reg, &wrp1ar_pages) != SUCCESS ||
		BuildWrpPageMask(wrp1br, wrp1br_reg, &wrp1br_pages) != SUCCESS ||
		((wrp1ar_pages | wrp1br_pages) == PROTECT_ALL_PAGE_MASK))
	{
		SysStatusCode = E_ProtectFail;
		return FAIL;
	}

	return SUCCESS;
}
```

- [ ] **Step 5: Run GREEN compile-time tests and normal build**

Run the Step 1 command again.

Expected: compile succeeds with `PROTECT_CONFIG_SELF_TEST` enabled.

Run again without `-DPROTECT_CONFIG_SELF_TEST`.

Expected: normal driver compile succeeds with zero errors.

- [ ] **Step 6: Commit Task 1**

```powershell
git diff --check -- WJ_STM32G030F6P6.c
git add -- WJ_STM32G030F6P6.c
git commit -m "feat: validate STM32G030 protect configuration"
```

---

### Task 2: Validate before mutation and verify before OPTSTRT

**Files:**
- Modify: `WJ_STM32G030F6P6.c:300-315`
- Modify: `WJ_STM32G030F6P6.c:1436-1490`

**Interfaces:**
- Consumes: `ValidateProtectConfig(...)` from Task 1.
- Produces: `static uChar VerifyOptionRegistersAll(uInt32 optr, uInt32 wrp1ar, uInt32 wrp1br)`.

- [ ] **Step 1: Add the shared Option Byte register verification helper**

Add its prototype and implementation:

```c
static uChar VerifyOptionRegistersAll(
	uInt32 optr,
	uInt32 wrp1ar,
	uInt32 wrp1br)
{
	if (CheckFlashStatusAll(FLASH_OPTR, optr,
			ALL_BITS_32, E_ProtectFail) != SUCCESS ||
		CheckFlashStatusAll(FLASH_WRP1AR, wrp1ar,
			WRP_FIELDS_MASK, E_ProtectFail) != SUCCESS ||
		CheckFlashStatusAll(FLASH_WRP1BR, wrp1br,
			WRP_FIELDS_MASK, E_ProtectFail) != SUCCESS)
	{
		return FAIL;
	}

	return SUCCESS;
}
```

- [ ] **Step 2: Move validation ahead of every mutation**

Immediately after reading the three `.spc` values, replace direct WRP masking with:

```c
if (ValidateProtectConfig(optr, wrp1ar, wrp1br,
	&wrp1ar_reg, &wrp1br_reg) != SUCCESS)
{
	return FAIL;
}
```

Verify by inspection that this call appears before:

```c
Use1TimePowerOnMode = 0;
PowerOffFun();
```

- [ ] **Step 3: Add the first readback before OPTSTRT**

After writing the three option registers and before writing `0x00020000UL` to `FLASH_CR`, add:

```c
if (VerifyOptionRegistersAll(optr, wrp1ar_reg, wrp1br_reg) != SUCCESS)
{
	swd_writeAPResource(FLASH_CR,
		FLASH_CR_OPTLOCK | FLASH_CR_LOCK);
	return FAIL;
}
```

The `OPTSTRT` write must remain after this block.

- [ ] **Step 4: Add structural safety checks**

Run:

```powershell
rg -n -A 18 -B 8 'ValidateProtectConfig\\(' WJ_STM32G030F6P6.c
rg -n -A 16 -B 16 'OPTSTRT: program Option Bytes' WJ_STM32G030F6P6.c
```

Expected:

- `ProtectFun()` calls validation before `PowerOffFun()`.
- `VerifyOptionRegistersAll()` is called after the three data-register writes and before `OPTSTRT`.

- [ ] **Step 5: Build and commit**

Run normal and `PROTECT_CONFIG_SELF_TEST` RVCT compile commands from Task 1.

Expected: both succeed.

```powershell
git diff --check -- WJ_STM32G030F6P6.c
git add -- WJ_STM32G030F6P6.c
git commit -m "feat: verify option registers before programming"
```

---

### Task 3: Verify after programming and tighten success paths

**Files:**
- Modify: `WJ_STM32G030F6P6.c:1490-1535`

**Interfaces:**
- Consumes: `VerifyOptionRegistersAll(...)` from Task 2.
- Produces: Confirmed Level 0 and Level 1 completion behavior.

- [ ] **Step 1: Add the second readback before OBL_LAUNCH**

After the existing Option Byte error-mask check and before clearing EOP, add:

```c
if (VerifyOptionRegistersAll(optr, wrp1ar_reg, wrp1br_reg) != SUCCESS)
{
	swd_writeAPResource(FLASH_SR, FLASH_OPTION_CLEAR_MASK);
	swd_writeAPResource(FLASH_CR,
		FLASH_CR_OPTLOCK | FLASH_CR_LOCK);
	return FAIL;
}
```

Keep EOP clearing and `OBL_LAUNCH` after this block.

- [ ] **Step 2: Preserve confirmed Level 1 completion**

After `OBL_LAUNCH`, delay, and power-off, retain this policy with an explicit comment:

```c
/*
 * Any non-0xAA/non-0xCC RDP byte is Level 1.  SWD cannot reconnect
 * after reload, so success relies on both pre-reload readbacks and the
 * completed Option Byte status checks above.
 */
if ((optr & PROTECT_RDP_MASK) != 0x000000AAUL)
{
	return SUCCESS;
}
```

No Level 2 value can reach this point because `ValidateProtectConfig()` rejects it.

- [ ] **Step 3: Reuse the verification helper for Level 0 final readback**

Replace the three long final `CheckFlashStatusAll()` calls with:

```c
if (VerifyOptionRegistersAll(optr, wrp1ar_reg, wrp1br_reg) != SUCCESS)
{
	return FAIL;
}
```

- [ ] **Step 4: Update `.spc` safety notes**

Append these user-facing notes:

```text
G  Driver rejects RDP 0xCC (irreversible Level 2).
G  RDP 0xAA is Level 0; all other accepted RDP values are Level 1.
G  WRP ranges must use pages 0~15, or disabled form START=0x1F END=0.
G  Full-chip WRP, including combined Area A + Area B coverage, is rejected.
```

- [ ] **Step 5: Build and commit**

Run normal and self-test RVCT compile commands.

Expected: both succeed.

```powershell
git diff --check -- WJ_STM32G030F6P6.c WJ_STM32G030F6P6.spc
git add -- WJ_STM32G030F6P6.c WJ_STM32G030F6P6.spc
git commit -m "feat: require confirmed option byte programming"
```

---

### Task 4: Final Protect safety audit

**Files:**
- Review: `WJ_STM32G030F6P6.c`
- Review: `WJ_STM32G030F6P6.spc`

**Interfaces:**
- Consumes: Completed Tasks 1–3.
- Produces: Evidence that invalid inputs cannot reach SWD mutation and valid inputs follow the required verification sequence.

- [ ] **Step 1: Run compile-time safety characterization**

Run the Task 1 self-test RVCT compile.

Expected: zero errors.

- [ ] **Step 2: Run the production compile**

Run the Task 1 normal RVCT compile.

Expected: zero errors.

- [ ] **Step 3: Audit Protect operation order**

Run:

```powershell
rg -n 'GetSpecialBitDWord|ValidateProtectConfig|PowerOffFun|FLASH_OPTR, optr|VerifyOptionRegistersAll|OPTSTRT|OBL_LAUNCH|return SUCCESS' WJ_STM32G030F6P6.c
```

Expected order inside `ProtectFun()`:

1. Read inputs.
2. Validate.
3. Power cycle/connect.
4. Write option registers.
5. First verification.
6. `OPTSTRT`.
7. Busy/error checks.
8. Second verification.
9. `OBL_LAUNCH`.
10. Level 1 confirmed return or Level 0 reconnect/final verification.

- [ ] **Step 4: Inspect the complete feature diff**

Run:

```powershell
git diff 427368a..HEAD -- WJ_STM32G030F6P6.c WJ_STM32G030F6P6.spc
git diff --check 427368a..HEAD
git status --short
```

Expected:

- Only the planned C/SPC safety changes appear after the design commit.
- No invalid-input path performs power or SWD mutations.
- No Level 2 configuration reaches an Option Byte register write.
- The worktree is clean.
