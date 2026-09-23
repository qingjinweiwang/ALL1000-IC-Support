# N25Q256A UnProtect and Erase Start Verification Design

## Goal

Correct two observed behaviors without changing the existing ALL-1000 SPI framework or adding helper functions:

- `UnProtectFun()` fails even when every enabled device is already unprotected.
- `EraseFun()` can report success when `C7h` was ignored and the main array was not erased.

## UnProtectFun

At function entry, compare Status Register protection fields `BP3`, `TB`, and `BP2-BP0` with the unprotected value using the existing `CheckAllStatusData()` gang path and mask `PrtRegMaskBit` (`7Ch`). If all enabled sockets already match, return `SUCCESS` without issuing `WREN` or `01h`. If any protection bit is set, continue through the existing one-byte Status Register write, WIP polling, and read-back verification flow.

## EraseFun

Keep the existing `50h -> 06h -> C7h` sequence. After the existing 50ms post-command delay, compare Status Register bits `WIP|WEL` (`03h`) against `WIP=1, WEL=0` (`01h`). This is the required observable state for an accepted Bulk Erase at that point. A rejected or malformed command remains at `WIP=0`, and may leave `WEL` either clear or set; either condition must return `E_EraseFail`.

After the start check passes, retain the existing WIP completion polling, 500-second timeout, and Flag Status Register `22h` erase/protection error check.

## Constraints

- Modify only the existing bodies of `UnProtectFun()` and `EraseFun()` in the target C file.
- Do not add a function or change SPI register access helpers.
- Do not add a main-array blank check inside `EraseFun()`.
- Preserve strict failure propagation for any enabled socket.

## Verification

- Build with the existing `iBuild.bat`.
- Confirm an already-unprotected device makes `UnProtectFun()` return success without a Status Register write.
- Confirm an ignored `C7h` cannot reach the normal erase-completion path.
- Hardware-test successful erase followed by `BlankCheckFun()`.
