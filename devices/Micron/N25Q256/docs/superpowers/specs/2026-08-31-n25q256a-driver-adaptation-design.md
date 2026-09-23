# N25Q256A Driver Adaptation Design

## Goal

Convert the copied TH25Q80HB driver into an N25Q256Ax3ESFxxx extended-SPI driver for the ALL-1000 platform.

## Supported Device Scope

- Device family: Micron N25Q256Ax3ESFxxx, 3V, 256Mb (32MB).
- Main-array byte address range: `00000000h` through `01FFFFFFh`.
- JEDEC ID: manufacturer `20h`, memory type `BAh`, capacity `19h`.
- Extended SPI only; dual, quad, DTR, and XIP modes are not enabled.
- Feature-set variants represented by `x` are supported through a common four-byte-address-mode initialization.

## Addressing

At power-on, issue `WREN (06h)`, `ENTER 4-BYTE ADDRESS MODE (B7h)`, then `WRDI (04h)`. Feature sets that power up in three-byte mode enter four-byte mode; feature-set 7 parts already in four-byte mode ignore the unsupported enter command and remain usable. Every main-array command sends `A31-A24`, `A23-A16`, `A15-A8`, and `A7-A0`.

Use standard opcodes in four-byte address mode:

- Read `03h`; fast read `0Bh` with one dummy byte.
- Page program `02h`, 1 to 256 bytes.
- Subsector erase `20h`; sector erase `D8h`; bulk erase `C7h`.

Dedicated four-byte opcodes such as `13h`, `12h`, `21h`, and `DCh` are not used because the datasheet limits them to specific A83 part numbers.

## Registers and Protection

- Status register is one byte, read with `05h` and written with `01h` plus one data byte.
- Writable protection fields are `BP3`, `TB`, and `BP2-BP0`; mask `7Ch`.
- Remove status-register-high-byte command `35h`, CMP, LB1-LB3, and all two-byte WRSR logic.
- Preserve SRWD and volatile WEL/WIP behavior outside the protection mask.
- Read Flag Status Register with `70h` after program/erase completion so protection, program, and erase failures are not reported as success; clear stale flags with `50h` before modifying operations.
- Bulk erase is rejected when `BP3`, `TB`, or `BP2-BP0` indicates protection, and its timeout is at least the datasheet maximum of 480 seconds.

## Memory and Programmer Tables

- Replace TH25Q names with N25Q256A names throughout the driver tables.
- Define one 32MB main-array block, one one-byte Status Register block, and one driver-operation-control pseudo-block.
- Keep the operation-control byte at SRAM offset 3 for compatibility with the existing driver callbacks.
- Set page size to 256 bytes and bulk-erase timeout to 500 seconds.
- Update the SPC options to expose BP0, BP1, BP2, TB, and BP3 plus Status Register verify/blank-check control.

## Removed Features

- Remove three TH25Q 1KB Security Register blocks and all `bBlkAttri_Res15` paths.
- Remove `48h/42h/44h` TH25Q Security Register handling.
- Keep `SecureFun()` as a successful no-op required by the fixed algorithm callback interface.
- Do not expose the N25Q256A 64-byte OTP array or its irreversible lock control in this change.
- Do not implement sector Lock Register UI or programming in this change.

## Error Handling

- A failure to set WEL causes program, erase, protect, or unprotect to fail.
- WIP polling timeouts return the existing operation-specific failure code.
- Flag Status Register program/erase/protection error bits cause the current gang sockets to fail.
- ID mismatch returns `E_AutoIDFail` using the existing host-supplied ID comparison mechanism.

## Verification

- Scan `.c`, `.h`, and `.spc` for TH25Q names, Security Register commands, SR2/CMP/LB fields, three-byte-only address sequences, and the old 1MB range.
- Build with `iBuild.bat WJx4_N25Q256Ax3ESFxxx`.
- Require zero compiler errors and updated BIN/SPC files in the shared `Algo1` directory.

