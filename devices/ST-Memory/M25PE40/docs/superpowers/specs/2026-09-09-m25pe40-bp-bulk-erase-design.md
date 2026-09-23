# M25PE40 BP Protection and Bulk Erase Design

## Goal

Correct the active M25PE40 VFQFPN8 driver so it keeps the existing ALL-1000
SPI/gang architecture while using only the nonvolatile Status Register
`BP2:BP0` bits for user-visible protection and `C7h` BULK ERASE for erase.

The supported silicon scope is **M25PE40 T9HX only**. The driver must not claim
compatibility with another M25PE40 process revision or reuse T7X-specific
behavior.

This design supersedes the Lock Register protection design dated 2026-09-09.
The M25PE40 Lock Registers are not exposed in the project SPC and are not used
by `ProtectFun()` or `UnProtectFun()`.

## Datasheet Basis

The command and timing requirements come from Micron M25PE40 Rev. D (January
2018). Process applicability comes from the earlier ST M25PE40 datasheet, which
states that W#, `BP2:BP0`, SRWD, WRSR, Lock Register commands, subsector erase,
and Bulk Erase are T9HX features. This project supports **T9HX only**:

- Main array: 512 KiB, addresses `000000h` through `07FFFFh`.
- Page size: 256 bytes.
- Status Register: `SRWD`, reserved bits, `BP2:BP0`, `WEL`, and `WIP`.
- `BP2:BP0` mask: `0x1C`.
- `BULK ERASE`: command `C7h`, no address or data bytes.
- `BULK ERASE` is accepted only when `BP2:BP0` are all zero and no sector is
  protected by a Lock Register.
- Maximum `tBE`: 10 seconds.
- Lock Register write-lock and lock-down bits exist on T9HX, are volatile, and
  reset to zero after power-up or hardware reset. They are not used as the
  user-visible protection mechanism in this driver.

## Protection Model

The SPC exposes one Status Register byte:

- `BP0 = 1` (`0x04`): protect sector 7, the upper eighth.
- `BP1 = 1` (`0x08`): participates in the datasheet BP truth table.
- `BP2 = 1` (`0x10`): all combinations with BP2 set protect the full array.

The exact protected ranges follow the datasheet truth table:

| BP2:BP0 | Protected area |
| --- | --- |
| `000` | None |
| `001` | Sector 7 |
| `010` | Sectors 6-7 |
| `011` | Sectors 4-7 |
| `1xx` | Sectors 0-7 |

`ProtectFun()` writes the requested `BP2:BP0` value. `UnProtectFun()` writes
`BP2:BP0 = 000`. Both operations write `SRWD = 0` and do not expose SRWD to the
user. Their required command order is:

1. Issue `WRITE ENABLE (06h)` and verify `WEL = 1` on every active socket.
2. Issue `WRITE STATUS REGISTER (01h)` with the BP target and `SRWD = 0`.
3. Poll `WIP` until it becomes 0, using a timeout greater than the 15 ms
   datasheet maximum `tW`.
4. Only after WIP is 0, read back and verify `BP2:BP0` on every active socket.

The fixture must hold `W#` HIGH during status-register changes. If a device
arrives with `SRWD = 1` while `W#` is LOW, the driver cannot clear protection;
that socket must fail the operation instead of being reported as successful.

## Metadata and Special Blocks

Keep the three entries that are represented by the active `.h` block table:

- `Noun_Block0`: the normal main-array block, device addresses `000000h`
  through `07FFFFh`, backed by DRAM from byte offset 0.
- `Noun_Block1`: a one-byte Status Register special block, with SRAM start/end
  offset 0 and device start/end address 0. Its virgin value is `FFh`, and its
  writable user field is `BP2:BP0`.
- `Noun_BlkPtStatus`: a one-byte virtual driver-operation control block, with
  SRAM start/end offset 3 and device start/end address 3. Its default value is
  `00h`; bit `0x80` enables Status Register Verify/Blank Check.

The `.h` block table and `.spc` SRAM offsets, lengths, names, and meanings must
agree exactly.

Remove all Sector Lock Register options from the active `.spc` file.

## Bulk Erase Flow

On the first `EraseFun()` entry:

1. Enable all currently passing gang sockets.
2. Call the BP-only unprotect operation.
3. Verify `BP2:BP0 = 000` on every active socket.
4. Issue `WRITE ENABLE (06h)` and verify `WEL = 1` on every active socket.
5. Send exactly one `BULK ERASE (C7h)` command byte and raise S# on a byte
   boundary.
6. Confirm that `WIP` becomes `1`. A socket whose WIP never asserts is treated
   as a rejected-command failure rather than a completed erase.
7. Poll until `WIP = 0`, with an 11-second driver timeout to cover the 10-second
   datasheet maximum plus polling overhead.
8. Leave the framework Blank Check as the authoritative content verification.

Because this is a **T9HX-only BP-protection driver**, Lock Register commands are
not exposed as protection operations. The real device power cycle performed by
the programming flow must reset their volatile bits to zero before Bulk Erase.
After BP is cleared, the WIP-start check detects a rejected `C7h`, including a
rejection caused by an unexpectedly set Lock Register. The hardware validation
procedure must use confirmed T9HX samples and reject extending this behavior to
another process revision without a separate datasheet review.

## Other M25PE40-Specific Operations

- Main-array programming uses `PAGE PROGRAM (02h)` and assumes an earlier
  erase when any bit must change from 0 to 1.
- Serial-number updates use `PAGE WRITE (0Ah)` so bytes may change in both
  directions. The operation reads the complete aligned page, merges the new
  serial-number bytes, issues WREN and Page Write, then polls WIP until 0 with
  a timeout greater than the 23 ms maximum `tPW`. It must not rely on a fixed
  post-write delay. After WIP clears, it reads and verifies the complete page.
- Normal read uses `03h`; serial-number page read uses `0Bh` with one dummy
  byte.
- Identification uses `9Fh` and compares `20h-80h-13h` through the project ID
  fields.
- Page, subsector, sector erase, Deep Power-Down, and Lock Register commands
  are not exposed by this driver.

## Power and Pin Requirements

S# must be HIGH before VCC rises, remain HIGH through the power-up delay, and
be HIGH before VCC falls. This is an implementation requirement, not only a
fixture assumption: `PowerOnFun()` explicitly drives `SetCSPinCmd = 1` before
enabling device power, and `PowerOffFun()` explicitly drives
`SetCSPinCmd = 1` before lowering VPIO/VPP/VIH/VCC. The existing 50 ms
stabilization delays exceed the datasheet `tPUW` and `tVSL` requirements.

RESET# must be held HIGH during normal operation. This design does not add
software RESET# control until the programmer-to-VFQFPN8 pin mapping is
independently confirmed.

## Error Handling

Gang comparison helpers may retire individual failing sockets according to the
existing ALL-1000 behavior. A function returns `FAIL` when all enabled sockets
have failed. Error paths must release S# HIGH before returning.

Status verification must use the FPGA gang comparison path
`CheckAllStatusData(expected)`, which compares the masked Status Register result
for every active socket. `ReadStatusData()` returns only one byte through the
single-data path and therefore must not be used to approve BP state for a gang
operation. It remains valid only where the caller has explicitly selected one
socket, such as the Status Register read operation for socket A.

Protection failures use `E_ProtectFail` or `E_UnprotectFail`; erase rejection,
start failure, or timeout uses `E_EraseFail`. Read-back mismatch of the Status
Register uses `E_ProtectStatusFail` in Verify/Blank Check.

## Verification

Static verification must check:

- Status Register block and SPC offsets agree.
- SPC contains BP options and no Lock Register options.
- The target description explicitly says `T9HX only`.
- Header and SPC comments identify the target as `M25PE40 T9HX only` without
  renaming the firmware ABI symbols or files.
- Protection mask is `0x1C`, with SRWD excluded.
- WRSR completion is polled before BP read-back verification.
- Protection and erase BP checks use the gang comparison path, not a
  single-byte `ReadStatusData()` result.
- Erase order is unprotect, BP verify, WREN/WEL verify, `C7h`, WIP assertion,
  WIP completion.
- Erase timeout is at least 11 seconds.
- `PowerOnFun()` and `PowerOffFun()` explicitly drive S# HIGH at the required
  side of each power transition, and relevant error paths leave S# HIGH.
- Serial-number Page Write polls WIP to completion before verification.
- No device-specific command removed from the supported flow remains callable.

Build verification must compile, link, and convert the driver using the
existing ARMCC toolchain without deploying generated files to `Algo1`.

Hardware verification must cover all eight BP combinations, four-socket mixed
status behavior, rejected Bulk Erase detection, successful Bulk Erase followed
by Blank Check, Page Program, Page Write serial-number update, and JEDEC ID.
