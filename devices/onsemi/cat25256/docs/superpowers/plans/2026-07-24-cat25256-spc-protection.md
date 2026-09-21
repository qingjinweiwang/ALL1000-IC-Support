# CAT25256 `.spc` Protection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the CAT25256 `.spc` protection presentation match the data-sheet `BP1:BP0` array protection types and stop exposing unsupported `WPEN` editing.

**Architecture:** Keep the existing bit-oriented `.spc` parser structure and its independent `BP0`/`BP1` masked edit controls. Add operator-facing text that maps every combined bit value to its protected address range, and remove the misleading `WPEN` edit control without changing the C driver.

**Tech Stack:** ALL-1000 `.spc` configuration syntax, PowerShell validation, CodeWarrior command-line build

## Global Constraints

- Modify only `WJ_CAT25256.spc`.
- Preserve the `[0]`, `.special.`, `G...`, `S0000L0100...`, and `E<value>,<mask>...` parser structure.
- Keep the driver protection mask at `0x0C`; do not add `WPEN`, `IPL`, or `LIP` support.
- Show all four data-sheet protection combinations with exact address ranges.
- Preserve existing user changes in `WJ_CAT25256.c` and unrelated files.

---

### Task 1: Redesign the CAT25256 protection presentation

**Files:**
- Modify: `WJ_CAT25256.spc:2-11`
- Reference: `docs/superpowers/specs/2026-07-24-cat25256-spc-protection-design.md`

**Interfaces:**
- Consumes: ALL-1000 `.spc` bit edit syntax `E<set-value>,<mask> <label>`
- Produces: Two editable status bits, `BP0` with mask `0x04` and `BP1` with mask `0x08`; no editable `WPEN` bit

- [ ] **Step 1: Capture the pre-change validation failure**

Run:

```powershell
$spc = Get-Content -LiteralPath '.\WJ_CAT25256.spc' -Raw
if ($spc -notmatch '00\s+No Protection' -or
    $spc -notmatch '01\s+Upper Quarter.*0x6000-0x7FFF' -or
    $spc -notmatch '10\s+Upper Half.*0x4000-0x7FFF' -or
    $spc -notmatch '11\s+Full Array.*0x0000-0x7FFF' -or
    $spc -match '(?m)^\s*E80,80\s+WPEN=1\s*$') {
    throw 'CAT25256 protection presentation is not data-sheet aligned'
}
```

Expected: FAIL with `CAT25256 protection presentation is not data-sheet aligned`.

- [ ] **Step 2: Replace the protection section**

Set `WJ_CAT25256.spc` to:

```text

////////////////////////////////////////////////////////////////////////
// ON Semiconductor CAT25256VI-GT3(SOP8)
////////////////////////////////////////////////////////////////////////
[0]
.special.
GStatus Register - Array Protection:
G BP1:BP0  Protection Type  Protected Address Range
G    00    No Protection    None
G    01    Upper Quarter    0x6000-0x7FFF
G    10    Upper Half       0x4000-0x7FFF
G    11    Full Array       0x0000-0x7FFF
GSet BP1 and BP0 together according to the table above:
S0000L0100 Status Register (BP1:BP0):
 E04,04 BP0 (bit 2)
 E08,08 BP1 (bit 3)
```

- [ ] **Step 3: Run structural and semantic validation**

Run:

```powershell
$spc = Get-Content -LiteralPath '.\WJ_CAT25256.spc' -Raw
$required = @(
    '[0]',
    '.special.',
    '00    No Protection    None',
    '01    Upper Quarter    0x6000-0x7FFF',
    '10    Upper Half       0x4000-0x7FFF',
    '11    Full Array       0x0000-0x7FFF',
    'S0000L0100 Status Register (BP1:BP0):',
    'E04,04 BP0 (bit 2)',
    'E08,08 BP1 (bit 3)'
)
foreach ($text in $required) {
    if (-not $spc.Contains($text)) {
        throw "Missing required SPC text: $text"
    }
}
if ($spc -match '(?m)^\s*E80,80\b' -or $spc -match '(?m)^\s*E(?:10|40),') {
    throw 'Unsupported WPEN, LIP, or IPL edit entry remains'
}
$editLines = [regex]::Matches($spc, '(?m)^\s*E[0-9A-Fa-f]{2},[0-9A-Fa-f]{2}\b')
if ($editLines.Count -ne 2) {
    throw "Expected exactly 2 editable status bits; found $($editLines.Count)"
}
Write-Output 'CAT25256 SPC protection validation passed'
```

Expected: PASS and print `CAT25256 SPC protection validation passed`.

- [ ] **Step 4: Check formatting and scope**

Run:

```powershell
git diff --check -- WJ_CAT25256.spc
git diff --stat -- WJ_CAT25256.spc
git diff -- WJ_CAT25256.spc
```

Expected: no whitespace errors; the diff changes only the protection presentation in `WJ_CAT25256.spc`.

- [ ] **Step 5: Build the driver project**

Run:

```powershell
& 'C:\Program Files (x86)\ARM\IDEs\CodeWarrior\CodeWarrior\5.6.1\1592\win_32-pentium\bin\CmdIDE.exe' 'D:\ALL1000\ALL1000_Driver\WJ_CAT25256\RVDS\WJ_CAT25256.mcp' /t Debug /b
```

Expected: CodeWarrior completes the Debug build without a compiler or linker error.

- [ ] **Step 6: Commit only the `.spc` change**

Run:

```powershell
git add -- WJ_CAT25256.spc
git diff --cached --check
git diff --cached --stat
git commit -m "fix: clarify CAT25256 protection options"
```

Expected: one commit containing only `WJ_CAT25256.spc`.
