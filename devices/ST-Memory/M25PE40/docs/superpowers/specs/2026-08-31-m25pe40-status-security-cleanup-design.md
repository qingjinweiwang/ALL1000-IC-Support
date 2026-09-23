# M25PE40 Status and Security Cleanup Design

## Goal

Remove TH25Q80HB/GD Security Register branches and convert every active status-register path to the M25PE40 one-byte Status Register protocol.

## Scope

- Delete all `bBlkAttri_Res15` branches from read, program, verify, erase, blank-check, and checksum flows.
- Remove Security Register commands and LB/CMP/SR2 handling.
- Read status only with command `05h`, returning one byte.
- Write status only with `01h` followed by one byte; protect and unprotect affect `BP2-BP0` only.
- Use `Noun_Block1` for the Status Register block.
- Keep `SecureFun()` as a successful no-op because the fixed algorithm table interface still declares it.
- Remove Status Register 2, CMP, BP3/BP4, and Security Register options from the SPC file.

## Non-goals

- Do not implement the M25PE40 sector Lock Register commands `E5h/E8h`.
- Do not change main-array read, program, erase, ID, power, or pin-map behavior.
- Do not reformat unrelated code or comments.

## Verification

- Scan active source and SPC for `Res15`, Security commands, `ReadStatus1`, `SR2`, CMP, LB1-LB3, BP3, and BP4.
- Build with `iBuild.bat` and require zero compiler errors.

