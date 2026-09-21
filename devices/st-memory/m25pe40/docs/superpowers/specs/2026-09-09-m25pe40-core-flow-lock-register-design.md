# M25PE40 Core Flow and Lock Register Design

## Goal

Correct the active M25PE40 VFQFPN8 driver so erase and sector protection follow
the Micron M25PE40 Rev. D command sequences while preserving the working ID
changes and current electrical/SPI settings.

## Memory Model

- Represent the 512 KiB main array as eight contiguous 64 KiB sectors.
- Use `Noun_Block0` through `Noun_Block7` for sectors 0 through 7.
- Add one eight-byte special block named `Noun_BlkLockStatus`. SRAM offsets
  0 through 7 represent the lock registers for sectors 0 through 7.
- Move the virtual driver-operation control byte to SRAM offset 8.

## Protection Model

- Sector protection uses READ LOCK REGISTER (`E8h`) and WRITE TO LOCK REGISTER
  (`E5h`) with an address inside the target sector.
- Only the reversible Write Lock bit, bit 0, is exposed in the SPC file.
- Lock Down bit 1 is always written as 0. The driver must not create a lock that
  requires reset or power cycling to remove.
- `ProtectFun()` clears legacy BP2-BP0 protection and then writes the eight
  requested sector Write Lock values.
- `UnProtectFun()` clears BP2-BP0 and writes `00h` to all eight lock registers.
- Read, verify, and blank-check operations for the special block use `E8h`, not
  READ STATUS REGISTER (`05h`).
- Lock register values are volatile and return to 0 after reset or power-down.

## Erase Flow

On the first erase entry, `EraseFun()` performs these steps:

1. Remove BP and sector-lock protection through `UnProtectFun()`.
2. Verify BP2-BP0 are 0 on every active socket.
3. Verify Write Lock bit 0 is 0 in all eight sector lock registers.
4. Send WRITE ENABLE (`06h`) and verify WEL is 1.
5. Send exactly one BULK ERASE (`C7h`) command byte and raise S#.
6. Verify WIP becomes 1 so a rejected command is not reported as completed.
7. Poll WIP until it returns to 0, using an 11-second timeout.

The existing framework blank check remains the authoritative content check
after erase.

## Power and Compatibility

- Drive S# HIGH before enabling device power and keep it HIGH during power-down.
- Retain the user's working IDCheck changes, voltage settings, and SPI delay.
- Do not drive `SetRSTPinCmd` until the programmer-to-VFQFPN pin mapping is
  independently confirmed. RESET# must be HIGH in the physical fixture during
  normal operation.
- Preserve the ALL-1000 driver ABI and gang-socket error handling.

## Verification

- Run static checks for eight 64 KiB sectors, `E5h`/`E8h` command framing,
  absence of BP options in the SPC file, and 11-second erase timeout.
- Compile, link, and convert the driver with ARMCC/ARMLINK/FROMELF into a local
  workspace build directory without deploying files to `Algo1`.
- Hardware validation must cover lock-register read/write, reversible sector
  protection, bulk erase after unprotect, WIP transition, blank check, and the
  existing ID path.
