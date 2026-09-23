# M25PE40 VFQFPN8 Targeted Fix Design

## Goal

Correct the active M25PE40 driver so it describes and operates the Micron
M25PE40 in the MP package (VFQFPN8, 6 x 5 mm) without retaining unsupported
SPI NOR template features.

## Scope

- Remove the active 512-byte secured-OTP block. M25PE40 has no
  user-programmable secured OTP array.
- Describe the M25PE40 status register as one special byte named
  `Noun_Block1` at SRAM offset 0.
- Keep the driver-operation control byte as a separate virtual block named
  `Noun_BlkPtStatus` at SRAM offset 3. It must not cause an IC transaction.
- Preserve the existing one-byte `05h`/`01h` status-register protocol and
  BP2:BP0 mask `1Ch`.
- Make serial-number read-modify-write use M25PE40 PAGE WRITE (`0Ah`) so an
  update can change bits in either direction. Poll WIP through the existing
  gang-status mechanism using a timeout that covers the datasheet maximum
  23 ms page-write time.
- Rename the local pin-map identifiers and comments from SO8 to VFQFPN8. The
  electrical pin numbers remain unchanged because the Micron SO and VFQFPN
  packages share the documented eight-pin signal assignment.
- Remove dead declarations, functions, macros, and copied comments that claim
  unsupported OTP/boot-lock behavior or unimplemented driver operations.

## Non-goals

- Do not implement sector Lock Register commands `E5h`/`E8h`.
- Do not add PAGE ERASE, SUBSECTOR ERASE, or SECTOR ERASE workflows.
- Do not change main-array read, bulk erase, program, verify, blank-check,
  checksum, JEDEC ID, voltage, or programmer ABI behavior.
- Do not deploy generated BIN or SPC files outside this workspace.

## Implementation

`WAx4_M25PE40-VMPxxx.h` will contain one main-array block, one status-register
block, and one virtual operation-control block. Local package-map symbols will
identify the MP/VFQFPN8 package while retaining the existing terminal values.

`WAx4_M25PE40-VMPxxx.c` will retain `Noun_Block1` as the status-register branch.
The serial-number write transaction will send `PageWriteCmd`, then poll WIP
instead of relying on a fixed delay. Unused template-only command declarations
and comments will be removed.

## Error Handling

PAGE WRITE first verifies WEL using `WriteStatusWELBit()`. A socket that does
not clear WIP before timeout is handled through `GangCheckIfAllSocketError()`;
if every active socket fails, the routine reports
`E_DeviceSerialCodeSetFail`, updates the serial-number result fields, restores
the active socket mask, and returns.

## Verification

- Scan the active header and source for OTP, boot-lock, SO8 package names, and
  unused unsupported operation macros.
- Check the block-table noun, SRAM offsets, lengths, and attributes.
- Check that the serial-number path sends `0Ah` and polls WIP for at least
  23 ms.
- Compile and link with the existing ARMCC project while keeping build outputs
  inside the workspace.
- Hardware validation remains required for ID, status read/verify, bulk erase,
  program/verify, serial-number rewrite, and BP protection.
