# ReadFun Special-Bit Refresh Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `ReadFun()` read OPTR, WRP1AR, and WRP1BR during the `.spc` special-block pass, immediately update their `SpecialBitBuf` fields, and fail with `E_ReadAHBFail` when any active socket disagrees.

**Architecture:** Add one file-local 32-bit-to-`SpecialBitBuf` writer that is the inverse of `GetSpecialBitDWord()`. Replace the current special-block no-op in `ReadFun()` with three ordered read/write/compare stages, using the existing SWD AHB read and FPGA gang-comparison helpers.

**Tech Stack:** Embedded C, ARMCC RVCT 2.2 build 349, STM32G030 Flash option registers, existing SWD AHB-AP and FPGA gang-socket interfaces.

## Global Constraints

- Modify only `WJ_STM32G030F6P6.c`.
- Preserve the user's existing uncommitted removal of the LCD display code.
- Do not add LCD output or display delays.
- Execute the new behavior only when `IfSpecialBitBlk()` is true.
- Preserve the ordinary Flash-read path.
- Process registers in this exact order: OPTR, WRP1AR, WRP1BR.
- Write each register to `SpecialBitBuf` immediately after reading it and before checking socket consistency.
- Do not roll back fields that were already written when a later comparison fails.
- On any active-socket mismatch, set `SysStatusCode = E_ReadAHBFail` and return `FAIL`.
- Do not modify `.spc`, the task table, USB commands, or Option Bytes programming behavior.
- Do not stage or commit `WJ_STM32G030F6P6.c` automatically because it contains overlapping user-owned uncommitted changes.

---

## File Structure

- Modify: `WJ_STM32G030F6P6.c`
  - Add the file-local `SetSpecialBitDWord()` declaration and implementation.
  - Replace only the `.spc` special-block branch in `ReadFun()`.
- No files are created for production code.
- No separate automated-test target exists for this hardware-bound driver; verification uses source characterization, the existing non-interactive ARM build, and a programmer hardware check.

---

### Task 1: Read and refresh the three .spc option-register fields

**Files:**
- Modify: `WJ_STM32G030F6P6.c:251`
- Modify: `WJ_STM32G030F6P6.c:925-936`
- Modify: `WJ_STM32G030F6P6.c:1051-1061`

**Interfaces:**
- Consumes: `DWORD swd_readAPResource(DWORD targetAddr)`.
- Consumes: `uChar ChkAllSckRead_AHB_Mask(uInt32 addr, uInt32 data, uInt32 mask)`.
- Consumes: `static void SelectActiveSockets(void)`.
- Consumes: `SpecialBitBuf(offset)`, `FLASH_OPTR`, `FLASH_WRP1AR`, `FLASH_WRP1BR`, `ALL_BITS_32`, and `E_ReadAHBFail`.
- Produces: `static void SetSpecialBitDWord(uShort offset, uInt32 value)`.
- Produces: a special-block `ReadFun()` path that updates offsets `0x00`, `0x08`, and `0x0C` and returns `SUCCESS` only when all three comparisons pass.

- [ ] **Step 1: Capture the dirty-source baseline and compile it**

Run:

```powershell
git diff -- WJ_STM32G030F6P6.c
cmd /c iBuild.bat WJ_STM32G030F6P6
```

Expected:

- The diff shows the user's LCD removal, including deletion of `Put_String()`, `Write_LcdRam()`, and `Delay_1ms(3000)`.
- The existing `IfSpecialBitBlk()` branch still returns `SUCCESS` without reading registers.
- The build prints `[SUCCESS] Build successful.` and produces no compiler error.

- [ ] **Step 2: Add the file-local setter declaration**

Immediately after the existing declaration:

```c
static uInt32 GetSpecialBitDWord(uShort offset);
```

add:

```c
static void SetSpecialBitDWord(uShort offset, uInt32 value);
```

- [ ] **Step 3: Implement the inverse byte conversion**

Immediately after `GetSpecialBitDWord()` add:

```c
/******************************************************/
static void SetSpecialBitDWord(uShort offset, uInt32 value)
{
	SpecialBitBuf(offset) = (uChar)(value >> 24);
	SpecialBitBuf(offset + 1) = (uChar)(value >> 16);
	SpecialBitBuf(offset + 2) = (uChar)(value >> 8);
	SpecialBitBuf(offset + 3) = (uChar)value;
}
```

This must remain the exact inverse of:

```c
	value = ((uInt32)SpecialBitBuf(offset)) << 24;
	value |= ((uInt32)SpecialBitBuf(offset + 1)) << 16;
	value |= ((uInt32)SpecialBitBuf(offset + 2)) << 8;
	value |= (uInt32)SpecialBitBuf(offset + 3);
```

- [ ] **Step 4: Replace the special-block no-op with ordered read/write/compare stages**

Replace:

```c
	if (IfSpecialBitBlk())
	{
		return SUCCESS;
	}
```

with:

```c
	if (IfSpecialBitBlk())
	{
		SelectActiveSockets();

		readData = swd_readAPResource(FLASH_OPTR);
		SetSpecialBitDWord(0x00, readData);
		sckStatus = ChkAllSckRead_AHB_Mask(
			FLASH_OPTR, readData, ALL_BITS_32);
		if (sckStatus & STATUS_LOW_NIBBLE)
		{
			SysStatusCode = E_ReadAHBFail;
			return FAIL;
		}

		readData = swd_readAPResource(FLASH_WRP1AR);
		SetSpecialBitDWord(0x08, readData);
		sckStatus = ChkAllSckRead_AHB_Mask(
			FLASH_WRP1AR, readData, ALL_BITS_32);
		if (sckStatus & STATUS_LOW_NIBBLE)
		{
			SysStatusCode = E_ReadAHBFail;
			return FAIL;
		}

		readData = swd_readAPResource(FLASH_WRP1BR);
		SetSpecialBitDWord(0x0C, readData);
		sckStatus = ChkAllSckRead_AHB_Mask(
			FLASH_WRP1BR, readData, ALL_BITS_32);
		if (sckStatus & STATUS_LOW_NIBBLE)
		{
			SysStatusCode = E_ReadAHBFail;
			return FAIL;
		}

		return SUCCESS;
	}
```

The call to `SetSpecialBitDWord()` must precede the corresponding
`ChkAllSckRead_AHB_Mask()` call. This ordering intentionally preserves the
new field value even when that field's gang comparison fails.

- [ ] **Step 5: Run focused source-characterization checks**

Run:

```powershell
rg -n -A 65 -B 3 "if \(IfSpecialBitBlk\(\)\)" WJ_STM32G030F6P6.c
rg -n "SetSpecialBitDWord|Put_String|Write_LcdRam|Delay_1ms\(3000\)" WJ_STM32G030F6P6.c
git diff --check -- WJ_STM32G030F6P6.c
```

Expected:

- The first special-block branch contains three read/write/compare stages in OPTR, WRP1AR, WRP1BR order.
- Offsets are exactly `0x00`, `0x08`, and `0x0C`.
- Each comparison uses `ALL_BITS_32`.
- Each mismatch sets `E_ReadAHBFail` and returns `FAIL`.
- No `Put_String`, `Write_LcdRam`, or `Delay_1ms(3000)` call exists.
- `git diff --check` is silent.

- [ ] **Step 6: Build the modified driver**

Run:

```powershell
cmd /c iBuild.bat WJ_STM32G030F6P6
```

Expected:

- ARM compilation and linking print `[SUCCESS] Build successful.`.
- No new compiler warning or error refers to `SetSpecialBitDWord()` or `ReadFun()`.
- The script updates the driver binary and `.spc` copy in the configured `Algo1` output directory.

- [ ] **Step 7: Review the final diff without staging the dirty source**

Run:

```powershell
git diff -- WJ_STM32G030F6P6.c
git status --short
```

Expected:

- The diff retains the user's prior LCD removal.
- New changes are limited to the setter declaration, setter implementation, and special-block `ReadFun()` branch.
- `.spc`, USB protocol headers, task tables, and ordinary Flash-read code are unchanged.
- Leave `WJ_STM32G030F6P6.c` uncommitted for the user to review.

- [ ] **Step 8: Verify behavior on programmer hardware**

Run a Read operation that includes the `.spc` special block.

Success case:

- With all active sockets holding identical values, Read succeeds.
- The `.spc` input boxes show the exact 8-digit values read from OPTR,
  WRP1AR, and WRP1BR.
- The values remain in display order; for example `0xDFFFE1AA` appears as
  `DFFFE1AA`, not `AAE1FFDF`.

Mismatch cases:

1. Make OPTR differ on one active socket. Expected: OPTR is refreshed from
   the reference read, `ReadFun()` fails with `E_ReadAHBFail`, and WRP1AR
   and WRP1BR retain their previous input values.
2. Make WRP1AR differ while OPTR matches. Expected: OPTR and WRP1AR are
   refreshed, `ReadFun()` fails with `E_ReadAHBFail`, and WRP1BR retains
   its previous input value.
3. Make WRP1BR differ while OPTR and WRP1AR match. Expected: all three
   fields are refreshed and `ReadFun()` fails with `E_ReadAHBFail`.

Ordinary-read regression:

- Run a normal Flash Read without the special block.
- Confirm the Flash data and return status match the baseline behavior.
- Confirm no LCD Option Bytes output or three-second display delay returns.
