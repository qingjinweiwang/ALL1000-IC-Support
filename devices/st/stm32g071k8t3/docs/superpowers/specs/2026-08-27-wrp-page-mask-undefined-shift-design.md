# WRP page-mask undefined-shift fix

## Purpose

Remove undefined behavior in `ValidateProtectConfig` when a write-protection
range spans all 32 Flash pages of STM32G071K8T3.

## Scope

The function currently derives `wrp1ar_pages` and `wrp1br_pages` by shifting a
32-bit `1UL` by the range length. A page range of 0 through 31 attempts
`1UL << 32`, which is undefined in C. Neither derived value is read after it is
assigned.

The change removes the two unused local variables and all assignments to them.
It preserves the existing WRP start/end field extraction, disabled-area
representation, validation (`start <= end` and `end < TOTAL_PAGE`), and the
values written to `FLASH_WRP1AR` and `FLASH_WRP1BR`.

## Verification

Inspect the function to confirm no `wrp1ar_pages`, `wrp1br_pages`, or variable
shift expression remains. Confirm the source still validates a full 0--31
range without rejecting it when `TOTAL_PAGE` is 32.
