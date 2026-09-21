# ReadFun Option Register LCD Display Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Read `FLASH_OPTR`, `FLASH_WRP1AR`, and `FLASH_WRP1BR` in `ReadFun()` and display their fixed-width hexadecimal values on LCD lines 5, 6, and 7.

**Architecture:** Add one file-local conversion helper that produces an eight-character uppercase hexadecimal string without `sprintf()`. After the existing DPIDR check succeeds, `ReadFun()` reads each option register through `swd_readAPResource()` and writes its label and value with `Put_String()`, then continues the existing Flash read loop unchanged.

**Tech Stack:** Embedded C, ARMCC RVCT 2.2 build 349, existing SWD AHB-AP helpers, existing LCD `Put_String()` API.

## Global Constraints

- Modify only `WJ_STM32G030F6P6.c`.
- Do not introduce `sprintf()` or another formatting library.
- Preserve leading zeroes and use uppercase hexadecimal digits.
- Use LCD lines 5, 6, and 7, starting labels at column 0 and values at column 8.
- Do not change existing Flash read behavior, return values, socket filtering, or `SysStatusCode`.
- Do not automatically commit the source file because it already contains user-owned uncommitted changes.

---

### Task 1: Add fixed-width hexadecimal conversion

**Files:**
- Modify: `WJ_STM32G030F6P6.c:235`
- Modify: `WJ_STM32G030F6P6.c:605`

**Interfaces:**
- Consumes: `uInt32` and `uChar` from the existing project type definitions.
- Produces: `static void DWordToHexString(uInt32 value, char *str)`, which writes exactly eight uppercase hexadecimal characters followed by `'\0'`.

- [ ] **Step 1: Add the file-local declaration**

Add beside the existing SWD helper declarations:

```c
static void DWordToHexString(uInt32 value, char *str);
```

- [ ] **Step 2: Add the conversion implementation**

Add after `swd_readAPResource()`:

```c
static void DWordToHexString(uInt32 value, char *str)
{
	static const char hexTable[] = "0123456789ABCDEF";
	uChar index;

	for (index = 0; index < 8; index++)
	{
		str[7 - index] = hexTable[value & 0x0FUL];
		value >>= 4;
	}
	str[8] = '\0';
}
```

- [ ] **Step 3: Compile the helper**

Run the existing ARMCC compile command for `WJ_STM32G030F6P6.c`.

Expected: compiler exit code `0`, with the object written only to the system temporary directory.

### Task 2: Display the option registers from ReadFun

**Files:**
- Modify: `WJ_STM32G030F6P6.c:1038-1080`

**Interfaces:**
- Consumes: `swd_readAPResource(DWORD targetAddr)`, `DWordToHexString(uInt32 value, char *str)`, and `Put_String(uChar LineNum, uChar addr, char *str)`.
- Produces: LCD lines `OPTR:   XXXXXXXX`, `WRP1AR: XXXXXXXX`, and `WRP1BR: XXXXXXXX`.

- [ ] **Step 1: Add the LCD value buffer**

Extend the local declarations in `ReadFun()`:

```c
	char hexString[9];
```

- [ ] **Step 2: Read and display all three registers**

Immediately after the existing successful DPIDR check, add:

```c
	Put_String(5, 0, "OPTR:   ");
	readData = swd_readAPResource(FLASH_OPTR);
	DWordToHexString(readData, hexString);
	Put_String(5, 8, hexString);

	Put_String(6, 0, "WRP1AR: ");
	readData = swd_readAPResource(FLASH_WRP1AR);
	DWordToHexString(readData, hexString);
	Put_String(6, 8, hexString);

	Put_String(7, 0, "WRP1BR: ");
	readData = swd_readAPResource(FLASH_WRP1BR);
	DWordToHexString(readData, hexString);
	Put_String(7, 8, hexString);
```

- [ ] **Step 3: Check the focused source diff**

Confirm:

- The three labels occupy eight characters each.
- `hexString` has nine bytes.
- The DPIDR failure path remains before all LCD register reads.
- The original `dev_addr` setup and Flash data loop are unchanged after the new display block.
- No unrelated user-owned code is modified.

- [ ] **Step 4: Compile the complete change**

Run the existing ARMCC compile command for `WJ_STM32G030F6P6.c`.

Expected: compiler exit code `0`, with the object written only to the system temporary directory.

- [ ] **Step 5: Perform the hardware check**

Execute Read on the programmer and confirm:

```text
OPTR:   XXXXXXXX
WRP1AR: XXXXXXXX
WRP1BR: XXXXXXXX
```

Expected:

- Lines 5, 6, and 7 contain eight uppercase hexadecimal digits.
- Values with leading zeroes retain all eight digits.
- The displayed values match the target's actual option registers.
- The existing Read operation still completes normally.
