# CAT25256 Status Input Field Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the CAT25256 BP0/BP1/WPEN checkboxes with one hexadecimal status-register input field and document all eight effective values.

**Architecture:** Use the established ALL-1000 `t0000L01` syntax so one byte is stored at `SpecialBitBuf(0)`. Keep the existing C/H `0x8C` mask path unchanged; the `.spc` comments explain how each valid byte affects array protection and the external WP lock.

**Tech Stack:** ALL-1000 `.spc` syntax, PowerShell assertions, CodeWarrior 5.6.1 command-line build

## Global Constraints

- Modify only `WJ_CAT25256.spc`.
- Use exactly one `t0000L01` input field.
- Remove the `S0000L0100` checkbox group and every `E<value>,<mask>` line.
- Document values `00`, `04`, `08`, `0C`, `80`, `84`, `88`, and `8C`.
- State that only bits 7, 3, and 2 are effective and other bits are masked.
- State that WPEN=1 with WP high leaves the status register writable after WREN.
- Preserve all existing user changes and do not commit the already-dirty implementation file.

---

### Task 1: Replace protection checkboxes with a byte input

**Files:**
- Modify: `WJ_CAT25256.spc:7-21`
- Reference: `docs/superpowers/specs/2026-07-24-cat25256-status-input-design.md`

**Interfaces:**
- Consumes: ALL-1000 input syntax `t<offset>L<length><label>`
- Produces: one byte at `SpecialBitBuf(0)` for the existing `0x8C` driver mask

- [ ] **Step 1: Run the pre-change assertion**

```powershell
$spc = Get-Content -LiteralPath '.\WJ_CAT25256.spc' -Raw
if ($spc -notmatch '(?m)^t0000L01\b' -or $spc -match '(?m)^\s*[SE][0-9A-Fa-f]') {
    throw 'CAT25256 status configuration is still checkbox-based'
}
```

Expected: FAIL with `CAT25256 status configuration is still checkbox-based`.

- [ ] **Step 2: Replace the protection UI section**

Use:

```text
GStatus Register Input (effective mask 0x8C):
G Value  BP1:BP0  WPEN  Effect
G  00       00      0   No array protection; WP pin function disabled
G  04       01      0   Protect 0x6000-0x7FFF; WP pin function disabled
G  08       10      0   Protect 0x4000-0x7FFF; WP pin function disabled
G  0C       11      0   Protect 0x0000-0x7FFF; WP pin function disabled
G  80       00      1   No array protection; WP Low locks Status Register
G  84       01      1   Protect 0x6000-0x7FFF; WP Low locks Status Register
G  88       10      1   Protect 0x4000-0x7FFF; WP Low locks Status Register
G  8C       11      1   Protect 0x0000-0x7FFF; WP Low locks Status Register
G When WPEN=1 and WP=High, Status Register remains writable after WREN.
G Only bits 7, 3 and 2 are effective; other input bits are masked out.
t0000L01 Status Register Value (Default 0x00)
```

- [ ] **Step 3: Run structural and semantic assertions**

```powershell
$spc = Get-Content -LiteralPath '.\WJ_CAT25256.spc' -Raw
$inputs = [regex]::Matches($spc, '(?m)^t0000L01\b')
if ($inputs.Count -ne 1) { throw "Expected one t0000L01 input, found $($inputs.Count)" }
if ($spc -match '(?m)^\s*S0000L0100\b' -or
    $spc -match '(?m)^\s*E[0-9A-Fa-f]{2},[0-9A-Fa-f]{2}\b') {
    throw 'Checkbox syntax remains'
}
foreach ($value in @('00','04','08','0C','80','84','88','8C')) {
    if ($spc -notmatch "(?m)^G\s+$value\s+") { throw "Missing effect row $value" }
}
foreach ($text in @(
    'effective mask 0x8C',
    'WPEN=1 and WP=High',
    'only bits 7, 3 and 2 are effective',
    'other input bits are masked out'
)) {
    if (-not $spc.ToLowerInvariant().Contains($text.ToLowerInvariant())) {
        throw "Missing note: $text"
    }
}
Write-Output 'CAT25256 status input validation passed'
```

Expected: PASS and print `CAT25256 status input validation passed`.

- [ ] **Step 4: Verify formatting and build**

```powershell
git diff --check -- WJ_CAT25256.spc
& 'C:\Program Files (x86)\ARM\IDEs\CodeWarrior\CodeWarrior\5.6.1\1592\win_32-pentium\bin\CmdIDE.exe' 'D:\ALL1000\ALL1000_Driver\WJ_CAT25256\RVDS\WJ_CAT25256.mcp' /t Debug /b
```

Expected: no whitespace errors and `No Error. (Code 0)`.

- [ ] **Step 5: Hand off without committing**

```powershell
git diff -- WJ_CAT25256.spc
git status --short
```

Expected: the `.spc` working-tree change remains uncommitted because it
already contains uncommitted WPEN support from the preceding task.
