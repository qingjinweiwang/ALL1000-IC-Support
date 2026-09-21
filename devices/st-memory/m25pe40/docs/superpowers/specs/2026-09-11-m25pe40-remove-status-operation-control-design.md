# M25PE40 Status Operation Control Removal Design

## Goal

Remove the driver-only `S0003` operation-control option from the M25PE40 user interface so the `.spc` exposes only real device settings. Status Register BP2-BP0 verification becomes unconditional whenever the Status Register special block is processed.

## Scope

Modify only these coordinated areas:

- `WAx4_M25PE40-VMPxxx.spc`: remove the `STATUS REGISTER OPERATION OPTIONS` group and the `S0003L0100 DRIVER OPERATION CONTROL` byte.
- `WAx4_M25PE40-VMPxxx.h`: retain the existing one-byte `Noun_BlkPtStatus` virtual block at SRAM/device offset `0x0003` for driver metadata compatibility.
- `WAx4_M25PE40-VMPxxx.c`: remove `StatusCheckOn` and both SRAM[3] gates from `VerifyFun()` and `BlankCheckFun()`.

Existing uncommitted edits in the three driver files belong to the user and must be preserved. In particular, this change must not restore or alter FPGA gang-result polarity code, erase EP0 handling, socket-selection behavior, voltage values, or SPI timing values.

## Resulting Block Model

The driver metadata retains exactly three blocks:

1. Main Flash: `Noun_Block0`.
2. One-byte Status Register: `Noun_Block1`, mapped to SRAM/device offset `0x0000`.
3. The existing one-byte `Noun_BlkPtStatus` virtual block, retained for driver metadata compatibility but not exposed in `.spc`.

## Behavior

For `Noun_Block1`:

- `VerifyFun()` always reads the expected BP2-BP0 value from SRAM[0], masks it with `PrtRegMaskBit`, and compares every active socket.
- `BlankCheckFun()` always compares BP2-BP0 with the block virgin/default value, masked with `PrtRegMaskBit`.

There is no longer a user-selectable path that silently returns `SUCCESS` without checking the Status Register.

Protect, UnProtect, Bulk Erase, Page Program, serial-number programming, ID Check, power sequencing, error codes, timeouts, and all other device behavior remain unchanged.

## Verification

Static assertions must prove:

- `.spc` contains no `S0003`, `DRIVER OPERATION CONTROL`, or `STATUS REGISTER OPERATION OPTIONS`.
- `.h` still contains `Noun_BlkPtStatus` and the one-byte `Noun_Block1` Status Register block.
- `.c` contains no `StatusCheckOn` or `SRAMPointer = 0x03`.
- `VerifyFun()` and `BlankCheckFun()` still call `CheckAllStatusData()` for `Noun_Block1`.
- No unrelated user edit is overwritten.

Compile with ARMCC, link with the existing `All1000Code` objects, and convert with FROMELF in a local verification directory. Do not deploy the result.
