# CAT25256 Reject-Protected-Program Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `ProgramFun` reject devices with BP0 or BP1 set instead of automatically clearing their protection.

**Architecture:** Keep the existing status-register read and `0x0C` array-protection mask. Replace the automatic `CAT25256WriteStatus` call with an immediate `E_ProtectStatusFail` return; explicit `UnProtectFun` remains the only path that clears protection.

**Tech Stack:** C89-style embedded C, PowerShell static assertions, CodeWarrior 5.6.1 command-line build

## Global Constraints

- Modify only the BP precheck inside `WJ_CAT25256.c::ProgramFun`.
- Keep `StatusArrayProtectMask` equal to `0x0C`.
- Set `SysStatusCode = E_ProtectStatusFail` when BP0 or BP1 is set.
- Return `FAIL` without sending WRSR or modifying BP0, BP1, or WPEN.
- Allow programming when the status is `0x00` or `0x80`.
- Preserve `UnProtectFun` as the explicit operation that clears the status configuration.
- Preserve all unrelated user changes and do not commit the already-dirty C file.

---

### Task 1: Reject programming when array protection is active

**Files:**
- Modify: `WJ_CAT25256.c:223-231`
- Reference: `docs/superpowers/specs/2026-07-24-cat25256-reject-protected-program-design.md`

**Interfaces:**
- Consumes: `CAT25256ReadStatus()` and `StatusArrayProtectMask`
- Produces: `FAIL` with `SysStatusCode == E_ProtectStatusFail` for any nonzero BP1:BP0

- [ ] **Step 1: Run the RED assertion**

```powershell
$source = Get-Content -LiteralPath '.\WJ_CAT25256.c' -Raw
$programStart = $source.IndexOf('uChar ProgramFun(void)')
$verifyStart = $source.IndexOf('uChar VerifyFun(void)')
$program = $source.Substring($programStart, $verifyStart - $programStart)
$rejectsProtected =
    $program.Contains('if (StatusData & StatusArrayProtectMask)') -and
    $program.Contains('SysStatusCode = E_ProtectStatusFail;') -and
    -not $program.Contains('CAT25256WriteStatus(')
if (-not $rejectsProtected) {
    throw 'ProgramFun still auto-unprotects protected devices'
}
```

Expected: FAIL with `ProgramFun still auto-unprotects protected devices`.

- [ ] **Step 2: Implement the minimal early rejection**

Replace:

```c
	StatusData = CAT25256ReadStatus();
	if (StatusData & StatusArrayProtectMask)
	{
		if (!CAT25256WriteStatus(LSB(PrtRegDefaultBit), LSB(PrtRegMaskBit), E_UnprotectFail))
		{
			SysStatusCode = E_UnprotectFail;
			return FAIL;
		}
	}
```

with:

```c
	StatusData = CAT25256ReadStatus();
	if (StatusData & StatusArrayProtectMask)
	{
		SysStatusCode = E_ProtectStatusFail;
		return FAIL;
	}
```

- [ ] **Step 3: Run the GREEN assertions**

```powershell
$source = Get-Content -LiteralPath '.\WJ_CAT25256.c' -Raw
$programStart = $source.IndexOf('uChar ProgramFun(void)')
$verifyStart = $source.IndexOf('uChar VerifyFun(void)')
$program = $source.Substring($programStart, $verifyStart - $programStart)
if (-not $program.Contains('StatusData = CAT25256ReadStatus();')) {
    throw 'ProgramFun no longer reads status'
}
if (-not $program.Contains('if (StatusData & StatusArrayProtectMask)')) {
    throw 'ProgramFun no longer tests BP0/BP1'
}
if (-not $program.Contains('SysStatusCode = E_ProtectStatusFail;')) {
    throw 'ProgramFun does not report protected status'
}
if ($program.Contains('CAT25256WriteStatus(') -or
    $program.Contains('E_UnprotectFail')) {
    throw 'ProgramFun still attempts automatic unprotect'
}
if (-not $source.Contains(\"#define`tStatusArrayProtectMask`t0x0C\")) {
    throw 'Array protection mask changed'
}
$unprotectStart = $source.IndexOf('uChar UnProtectFun(void)')
$unprotect = $source.Substring($unprotectStart, 220)
if (-not $unprotect.Contains('CAT25256WriteStatus(0x00,')) {
    throw 'Explicit UnProtectFun path changed'
}
Write-Output 'Protected-program rejection validation passed'
```

Expected: PASS and print `Protected-program rejection validation passed`.

- [ ] **Step 4: Check the focused diff**

```powershell
git diff --check -- WJ_CAT25256.c
git diff -U10 -G 'E_ProtectStatusFail|E_UnprotectFail' -- WJ_CAT25256.c
```

Expected: no whitespace errors; the new Program precheck has no WRSR call.

- [ ] **Step 5: Build the CodeWarrior Debug target**

```powershell
& 'C:\Program Files (x86)\ARM\IDEs\CodeWarrior\CodeWarrior\5.6.1\1592\win_32-pentium\bin\CmdIDE.exe' 'D:\ALL1000\ALL1000_Driver\WJ_CAT25256\RVDS\WJ_CAT25256.mcp' /t Debug /b
```

Expected: `No Error. (Code 0)`.

- [ ] **Step 6: Hand off without committing**

```powershell
git status --short
rg -n -C 5 'E_ProtectStatusFail' WJ_CAT25256.c
```

Expected: the C change remains uncommitted because the file contained
pre-existing user work. Hardware validation remains: Protect `0C` → Program
rejected → Unprotect → Program allowed.
