# CAT25256 WPEN Support Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make BP0, BP1, and WPEN configurable from `WJ_CAT25256.spc`, apply the selected byte in `ProtectFun`, and document the resulting array and hardware-lock effects.

**Architecture:** The `.spc` file remains the operator-facing source of the selected status byte at `SpecialBitBuf(0)`. The header expands the writable status mask to `0x8C`, while the C driver keeps the array-protection detection mask at `0x0C`, writes the masked `.spc` selection during Protect, and clears all three configurable bits during Unprotect.

**Tech Stack:** C89-style embedded C, ALL-1000 `.spc` syntax, PowerShell static assertions, CodeWarrior 5.6.1 command-line build

## Global Constraints

- Preserve the user's existing uncommitted changes in `WJ_CAT25256.c` and all unrelated files.
- Expose exactly BP0, BP1, and WPEN; do not expose IPL or LIP.
- Treat `BP1:BP0` as the EEPROM array-protection selector.
- Treat WPEN as the external WP pin's status-register lock enable, not as an array range.
- Keep array-protection detection at `0x0C`.
- Use `0x8C` as the writable/readable protection-configuration mask.
- Make `UnProtectFun` clear BP0, BP1, and WPEN.

---

### Task 1: Implement end-to-end BP/WPEN configuration

**Files:**
- Modify: `WJ_CAT25256.spc:7-19`
- Modify: `WJ_CAT25256.h:443-459`
- Modify: `WJ_CAT25256.c:56-60`
- Modify: `WJ_CAT25256.c:224-227`
- Modify: `WJ_CAT25256.c:470-483`
- Reference: `docs/superpowers/specs/2026-07-24-cat25256-wpen-support-design.md`

**Interfaces:**
- Consumes: `.spc` status byte at `SpecialBitBuf(0)`
- Consumes: `CAT25256WriteStatus(uChar StatusData, uChar StatusMask, uChar ErrorCode)`
- Produces: `CATStatusConfigMask = 0x008C` covering BP0, BP1, and WPEN
- Produces: Protect applies `SpecialBitBuf(0) & 0x8C`; Unprotect writes `0x00 & 0x8C`

- [ ] **Step 1: Run the pre-change assertion**

Run:

```powershell
$spc = Get-Content -LiteralPath '.\WJ_CAT25256.spc' -Raw
$header = Get-Content -LiteralPath '.\WJ_CAT25256.h' -Raw
$source = Get-Content -LiteralPath '.\WJ_CAT25256.c' -Raw
$ready =
    $spc.Contains('E80,80 WPEN (bit 7)') -and
    $spc.Contains('WPEN=1, WP=Low') -and
    $header.Contains('#define CATStatusConfigMask') -and
    $header.Contains('0x008C') -and
    $source.Contains('SpecialBitBuf(0)') -and
    $source.Contains('StatusArrayProtectMask')
if (-not $ready) {
    throw 'CAT25256 BP/WPEN configuration path is incomplete'
}
```

Expected: FAIL with `CAT25256 BP/WPEN configuration path is incomplete`.

- [ ] **Step 2: Add the WPEN control and effect text to `.spc`**

Replace the protection section with:

```text
GStatus Register - Protection Configuration:
G BP1:BP0  Array Protection  Protected Address Range
G    00    No Protection     None
G    01    Upper Quarter     0x6000-0x7FFF
G    10    Upper Half        0x4000-0x7FFF
G    11    Full Array        0x0000-0x7FFF
G WPEN and external WP pin effect:
G    WPEN=0, WP=Low/High  Status Register writable after WREN
G    WPEN=1, WP=High      Status Register writable after WREN
G    WPEN=1, WP=Low       Status Register write protected
G WPEN does not change the BP1:BP0 protected address range.
GSet BP1, BP0 and WPEN according to the tables above:
S0000L0100 Status Register (WPEN/BP1/BP0):
 E04,04 BP0 (bit 2)
 E08,08 BP1 (bit 3)
 E80,80 WPEN (bit 7)
```

- [ ] **Step 3: Define the full configuration mask in the header**

Immediately above `ProtectRegisterTable`, add:

```c
#define CATStatusConfigMask		0x008C
```

Replace the table entry and comments with:

```c
const uShort ProtectRegisterTable[ ] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
  /* Writable configuration: BP0(0x04) | BP1(0x08) | WPEN(0x80) */
	CATStatusConfigMask,//0

	/* here adding more... */
} ;

const uShort ProtectRegisterDefaultTable[] =
{ /* Table Sequence by Block/Sector Index Code Constant Define */
  /* Unprotect default: BP0=0, BP1=0, WPEN=0 */
	0x0000,//0

	/* here adding more... */
};
```

- [ ] **Step 4: Separate array protection detection from status configuration**

Replace:

```c
#define	StatusProtectMask		0x0C
```

with:

```c
#define	StatusArrayProtectMask	0x0C
```

In `ProgramFun`, replace:

```c
	if (StatusData & StatusProtectMask)
```

with:

```c
	if (StatusData & StatusArrayProtectMask)
```

- [ ] **Step 5: Apply the `.spc` selection in Protect and clear all bits in Unprotect**

Replace `ProtectFun` and `UnProtectFun` with:

```c
uChar ProtectFun(void)
{
	uChar StatusData;

	/* Apply BP0, BP1 and WPEN selected in the .spc status byte. */
	StatusData = SpecialBitBuf(0) & LSB(PrtRegMaskBit);
	return CAT25256WriteStatus(StatusData, LSB(PrtRegMaskBit), E_ProtectFail);
}


/******************* Unprotect Subroutine ********************\
Notes: You can change this subroutine name to your definition name

\**********************************************************/
uChar UnProtectFun(void)
{
	/* Clear BP0, BP1 and WPEN so the array and status register are unlocked. */
	return CAT25256WriteStatus(LSB(PrtRegDefaultBit), LSB(PrtRegMaskBit), E_UnprotectFail);
}
```

- [ ] **Step 6: Run structural and semantic assertions**

Run:

```powershell
$spc = Get-Content -LiteralPath '.\WJ_CAT25256.spc' -Raw
$header = Get-Content -LiteralPath '.\WJ_CAT25256.h' -Raw
$source = Get-Content -LiteralPath '.\WJ_CAT25256.c' -Raw
$requiredSpc = @(
    '00    No Protection     None',
    '01    Upper Quarter     0x6000-0x7FFF',
    '10    Upper Half        0x4000-0x7FFF',
    '11    Full Array        0x0000-0x7FFF',
    'WPEN=0, WP=Low/High  Status Register writable after WREN',
    'WPEN=1, WP=High      Status Register writable after WREN',
    'WPEN=1, WP=Low       Status Register write protected',
    'E04,04 BP0 (bit 2)',
    'E08,08 BP1 (bit 3)',
    'E80,80 WPEN (bit 7)'
)
foreach ($text in $requiredSpc) {
    if (-not $spc.Contains($text)) {
        throw "Missing required SPC text: $text"
    }
}
$editLines = [regex]::Matches($spc, '(?m)^\s*E[0-9A-Fa-f]{2},[0-9A-Fa-f]{2}\b')
if ($editLines.Count -ne 3 -or $spc -match '(?m)^\s*E(?:10|40),') {
    throw 'SPC must expose exactly BP0, BP1 and WPEN'
}
if (-not $header.Contains('#define CATStatusConfigMask') -or
    -not $header.Contains('0x008C') -or
    -not $header.Contains('CATStatusConfigMask,//0')) {
    throw 'Header configuration mask is not 0x008C'
}
if (-not $source.Contains('#define' + "`t" + 'StatusArrayProtectMask') -or
    -not $source.Contains('if (StatusData & StatusArrayProtectMask)') -or
    -not $source.Contains('StatusData = SpecialBitBuf(0) & LSB(PrtRegMaskBit);')) {
    throw 'Driver does not separate array detection or consume SpecialBitBuf(0)'
}
Write-Output 'CAT25256 BP/WPEN static validation passed'
```

Expected: PASS and print `CAT25256 BP/WPEN static validation passed`.

- [ ] **Step 7: Check formatting and scope**

Run:

```powershell
git diff --check -- WJ_CAT25256.spc WJ_CAT25256.h WJ_CAT25256.c
git diff --stat -- WJ_CAT25256.spc WJ_CAT25256.h WJ_CAT25256.c
git diff -- WJ_CAT25256.spc WJ_CAT25256.h WJ_CAT25256.c
```

Expected: no whitespace errors; only the approved BP/WPEN configuration path changes.

- [ ] **Step 8: Build the CodeWarrior Debug target**

Run:

```powershell
& 'C:\Program Files (x86)\ARM\IDEs\CodeWarrior\CodeWarrior\5.6.1\1592\win_32-pentium\bin\CmdIDE.exe' 'D:\ALL1000\ALL1000_Driver\WJ_CAT25256\RVDS\WJ_CAT25256.mcp' /t Debug /b
```

Expected: `No Error. (Code 0)`.

- [ ] **Step 9: Verify the final working-tree handoff without committing**

Run:

```powershell
git status --short
git diff --check -- WJ_CAT25256.spc WJ_CAT25256.h WJ_CAT25256.c
git diff --stat -- WJ_CAT25256.spc WJ_CAT25256.h WJ_CAT25256.c
```

Expected: the three implementation files remain available in the working
tree. Do not stage or commit them because `WJ_CAT25256.c` contained user
changes before this task and a whole-file commit would include unrelated
work.
