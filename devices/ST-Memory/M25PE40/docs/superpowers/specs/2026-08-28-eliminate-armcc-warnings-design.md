# Eliminate ARMCC Build Warnings Design

## Goal

Build `WJx4_M25PE40-VMPxxx` with ARMCC using zero warnings and zero errors without changing driver behavior, table layout, or output-copy behavior.

## Changes

### Address-table initializers

`DevBlockIdx_Table`, `DevSpecIdx_Table`, and `DevPmpIdx_Table` intentionally store ROM object addresses in `uInt32` entries required by the existing firmware ABI. ARMCC accepts these address-to-integer static initializers as an extension and emits diagnostic `1296`.

Keep the table types, element order, addresses, and sizes unchanged. Suppress diagnostic `1296` only around these three tables, with a comment explaining the ABI requirement, and restore the previous diagnostic state immediately afterward. Do not add a global suppression flag to `iBuild.bat`.

### Unreachable function tails

Remove the final `return SUCCESS;` from `ReadFun`, `ProgramFun`, `VerifyFun`, and `BlankCheckFun`. Every preceding branch either returns directly or enters a loop that exits through a return, so these statements are unreachable and their removal does not alter runtime control flow.

### Disabled security-lock implementation

`SecureFun` deliberately returns success before its OTP-lock implementation because per-socket read-modify-write support is not ready. Preserve this behavior while expressing it through conditional compilation: compile the current `return SUCCESS;` path and retain the deferred implementation under `#if 0`. This prevents intentionally disabled code from producing an unreachable-statement warning and keeps the implementation available for later re-enablement.

## Verification

Run:

```powershell
.\iBuild.bat WJx4_M25PE40-VMPxxx
```

Acceptance criteria:

- ARMCC reports `0 warnings, 0 errors`.
- The build reports `[SUCCESS] Build successful.`
- The BIN and SPC updates to `D:\HILO\ALL-1000\Algo1` both succeed.
- No generated `.o`, `.axf`, `.bin`, or `map.txt` files remain in the driver directory after cleanup.
