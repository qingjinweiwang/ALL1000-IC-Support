# N25Q256A Full Driver Conformance Design

Date: 2026-09-02

## Goal

Make `WAx4_N25Q256Ax3ESFxxx.c` and `WAx4_N25Q256Ax3ESFxxx.h` conform to the bundled Micron N25Q256A Rev. X datasheet while preserving the ALL-1000 13-callback ABI and four-socket gang behavior. Main-array access must remain in three-byte address mode and use Extended Address Register (EAR) bit 0 to select the lower or upper 16MB segment.

This specification supersedes conflicting four-byte-address and TH/GD security-register assumptions in the earlier adaptation and OTP documents. The existing `2026-09-02-n25q256a-ear-three-byte-addressing-design.md` remains compatible with this document for main-array addressing; this document is the authoritative design for the combined correction.

## Device Model

- Device family: Micron N25Q256Ax3ESFxxx, targeting the N25Q256A13 behavior in the bundled Rev. X datasheet.
- Main array: 32MB, logical addresses `00000000h` through `01FFFFFFh`.
- Main-array page size: 256 bytes.
- Erase geometry: 4KB subsectors and 64KB sectors; the active driver continues to use addressless `C7h` bulk erase.
- JEDEC ID: manufacturer `20h`, memory type `BAh`, capacity `19h`.
- Interface: extended SPI protocol with three-byte command addresses.
- OTP: 64 data bytes at offsets `00h` through `3Fh`, plus the OTP control byte at offset `40h`.

The driver will not use a second status register. N25Q256A has one eight-bit Status Register accessed with `05h` and `01h`, an eight-bit Flag Status Register accessed with `70h` and cleared with `50h`, and an eight-bit EAR accessed with `C8h` and `C5h`.

## Three-Byte Addressing and EAR

`PowerOnFun()` will select all currently passing sockets, invalidate the cached EAR state, and read Flag Status Register bit 0. Bit 0 must be zero, indicating three-byte address mode. A part that starts in four-byte mode fails initialization; the driver does not send `B7h` or `E9h` and does not rewrite the nonvolatile configuration register.

The driver will provide these internal helpers:

- `Send3ByteAddress(address)` sends A23-A16, A15-A8, and A7-A0.
- `CheckAllExtendedAddressData(expected)` reads `C8h` and compares EAR bit 0 across enabled sockets.
- `SetExtendedAddress(bank)` issues its own `06h`, writes `C5h`, reads back `C8h`, and marks the cached bank valid only after verification.
- `SelectAddressBank(address)` derives EAR bit 0 from logical address bit 24 and avoids redundant writes when the verified cached bank already matches.

Only EAR bit 0 is meaningful for this device. Reserved bits are written as zero and ignored during masked verification. Failed writes or readback mismatches invalidate the cache and prevent the caller from issuing its main-array operation.

All address-bearing main-array operations must select the bank before asserting CS for the array command. For modifying commands, EAR selection must precede the operation-specific WRITE ENABLE because writing EAR consumes WEL. This rule covers `03h`, `0Bh`, `02h`, and any active or future `20h`/`D8h` path. `C7h` bulk erase is addressless and does not depend on EAR.

Long `ReadFun()`, `VerifyFun()`, and `BlankCheckFun()` operations maintain a 32-bit logical address. When data remains after the address advances from `00FFFFFFh` to `01000000h`, the driver deasserts CS, selects EAR bank 1, and reissues the same read command with three address bytes. Buffer counters and remaining lengths advance exactly once. Page program and serial-number read-modify-write are limited to aligned 256-byte pages and cannot cross a bank boundary.

## Status Register and Array Protection

The Status Register model is one byte:

- bit 7: SRWD
- bit 6: BP3
- bit 5: TB
- bits 4:2: BP2-BP0
- bit 1: WEL
- bit 0: WIP

The software-protection mask is `0x7C`, covering BP3, TB, and BP2-BP0. `ProtectFun()` reads the current Status Register, replaces only masked protection bits with the host-provided target, preserves SRWD and the volatile WIP/WEL positions, issues `06h + 01h + one status byte`, polls WIP, and reads back the masked bits. `UnProtectFun()` follows the same flow with the default masked value `0x00`.

The driver will delete the copied TH/GD concepts and commands: `ReadStatus1Cmd`/`35h`, a two-byte `01h` write, CMP, QE, SRP1, LB1-LB3, and `CheckAllStatus1Data()`. `35h` is ENTER QUAD on N25Q256A and must never be used as a status-register read.

Before bulk erase, protection is considered disabled only when all masked BP/TB bits are zero. The driver does not apply TH/GD CMP-based protection inversion. If the array is protected, erase fails rather than reporting a false success.

## Flag Status and Failure Handling

`ClearFlagStatusData()` sends `50h`. `CheckAllFlagStatusData(expected)` reads `70h` through the existing FPGA gang comparison path.

The relevant failure bits are:

- bit 5: erase failure or protection error
- bit 4: program failure or protection error
- bit 1: protection failure, including access to locked OTP

Flag Status Register bit 3 reports whether the optional VPP acceleration input is disabled; its default value is one during normal VCC-only operation and it is not treated as an operation failure. Before program, erase, status-register write, and OTP program operations, stale error bits are cleared where required by the operation flow. After WIP clears, the driver checks the operation-specific error mask. Main-array and OTP program reject bits 4 and 1 (`0x12`); erase rejects bits 5 and 1 (`0x22`). A failed socket is passed to the existing gang error handler; the function returns `FAIL` only when no usable socket remains, preserving established partial-gang behavior.

Every error path that owns an active SPI transaction deasserts CS before returning. EAR cache validity is cleared when EAR selection fails. Existing operation-specific `SysStatusCode` values are retained.

## OTP and Secure Operation

The copied TH/GD security-register commands `48h` and `44h` are removed. N25Q256A OTP uses:

- `4Bh`: READ OTP ARRAY
- `42h`: PROGRAM OTP ARRAY

Because the approved operating mode is three-byte addressing, each OTP command sends exactly three address bytes: `00h`, `00h`, and the OTP offset. OTP does not use or change EAR. READ OTP sends one dummy byte after the address.

The OTP block remains a 65-byte DRAM-backed special block. Bytes 0-63 contain requested OTP data. Byte 64 bit 0 is the lock request: `1` leaves OTP unlocked and `0` requests permanent locking.

`SecureFun()` executes only for the OTP block:

1. Load all 65 bytes from the downloaded block data.
2. Clear FSR, issue WREN, and program exactly the 64 OTP data bytes at offset zero with `42h`.
3. Poll WIP and reject OTP program/protection errors.
4. Read the 64 data bytes with `4Bh` and verify every still-enabled socket.
5. Only after successful data verification, if byte 64 bit 0 requests locking, program a value with bit 0 cleared at offset `40h`.
6. Poll/check again, then read offset `40h` and verify bit 0 is zero.

The lock transition is irreversible. The driver never attempts to change control bit 0 from zero back to one, never locks before data verification, and does not expose TH/GD LB lock bits. Normal `ProgramFun()`, `EraseFun()`, `VerifyFun()`, and `BlankCheckFun()` do not treat OTP as a TH/GD security-register array.

## Block Table and ABI

The header preserves:

- the main-array range ending at `01FFFFFFh`
- the 65-byte OTP special block ending at virtual address `02000040h`
- the single-byte Status Register pseudo block
- the driver-operation control block
- the existing 13-entry callback ordering
- `SystemGang = Gang4Type`
- the existing voltage and pin-map tables

Status Register operations target the actual status pseudo block noun. `SecureFun` remains registered only for the OTP-capable special block. No new public callback or table format is introduced.

## Serial-Number Path

`DevSNProgAndVerify()` accesses the main array, so each page read, page program, and verify read calls `SelectAddressBank(DeviceAddress)` and uses `Send3ByteAddress(DeviceAddress)`. Bank selection occurs before the page-program WREN. The existing rule that a serial number cannot cross a 256-byte page remains unchanged.

## Verification

Static verification will confirm:

- no active `B7h`, `E9h`, four-byte address sender, or four-byte OTP sequence
- no active `35h` status read, `48h`, `44h`, CMP, or LB1-LB3 implementation
- every address-bearing main-array command uses EAR selection and three address bytes
- `C7h` remains addressless and EAR-independent
- OTP sends three address bytes and never selects EAR
- the header retains the 32MB main array and 65-byte OTP block
- the protection mask is `0x7C` and status writes contain one data byte
- no whitespace or patch-format errors are present

Build verification will run `iBuild.bat` in `WAx4_N25Q256Ax3ESFxxx` and require ARM compile, link, and binary conversion to complete without errors. Output-copy problems outside the repository are reported separately from compiler success. Hardware validation should exercise addresses on both sides of `00FFFFFFh/01000000h`, protection and unprotection, bulk erase error reporting, OTP read/program, and the irreversible OTP-lock path only on a sacrificial device.

## Out of Scope

- Changing the ALL-1000 firmware ABI or FPGA protocol
- Enabling dual, quad, DTR, or XIP modes
- Rewriting the N25Q256A nonvolatile configuration register
- Adding sector-lock-register workflows
- Activating subsector or sector erase when the current product flow uses bulk erase
- Changing unrelated pin mapping, voltage limits, or host `.spc` user-interface behavior
