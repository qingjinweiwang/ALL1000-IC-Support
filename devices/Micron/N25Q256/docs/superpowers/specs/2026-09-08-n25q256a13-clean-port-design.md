# N25Q256A13 Clean Driver Port Design

Date: 2026-09-08

## Goal

Reimplement `WAx4_N25Q256Ax3ESFxxx.c` for the Micron
N25Q256A13 family. Reuse the ALL-1000 Gang4 SPI transport structure and the 13
fixed task entry points from `SYx4_PY25Q01GLC-SMxxx.c`, but derive every device
command, register definition, address sequence, timeout, and memory property
from the bundled Micron N25Q256A Rev. X datasheet.

The current 32-line target C file is an intentional clean starting point.
`WAx4_N25Q256Ax3ESFxxx copy 2.c` may be inspected for known problems, but no
function body will be copied from it.

## Supported Device Scope

- Micron N25Q256A13 devices: feature-set digit 1, 3V family, 256Mb density.
- Extended SPI protocol only.
- VCC operating range: 2.7V to 3.6V.
- Main array: 32MB, logical byte addresses `00000000h` through `01FFFFFFh`.
- Page size: 256 bytes.
- OTP: 64 data bytes at offsets `00h` through `3Fh`, plus the control byte at
  offset `40h`.
- JEDEC ID: `20h BAh 19h`.

V1 does not support Dual I/O, Quad I/O, DTR, XIP, lock-register tasks, or an
abnormal startup configuration caused by an externally modified NVCR. It does
not read or write NVCR with `B5h` or `B1h` and does not attempt to recover a
device that powers up outside Extended SPI.

## Addressing Strategy

N25Q256A13 powers up in 3-byte address mode and supports the volatile ENTER
4-BYTE ADDRESS MODE command. `PowerOnFun()` will:

1. Apply the configured supply and I/O voltages and wait for them to stabilize.
2. Load the driver parameters and enable all currently passing Gang4 sockets.
3. Issue `WRITE ENABLE (06h)` and verify Status Register WEL bit 1.
4. Issue `ENTER 4-BYTE ADDRESS MODE (B7h)`.
5. Read `FLAG STATUS REGISTER (70h)` and verify only bit 0 equals one on all
   enabled sockets.

`B7h` is an explicit exception to the normal write state machine. It is not
preceded by `50h`, is not followed by WIP polling, and is checked only through
FSR addressing bit 0. No `WRITE DISABLE` is required because the datasheet
states that the mode command clears WEL.

After initialization, every address-bearing Main Flash command uses its
standard opcode with four address bytes in A31-A24 through A7-A0 order:

- READ: `03h`
- FAST READ: `0Bh`, followed by the configured eight dummy clocks
- PAGE PROGRAM: `02h`
- SUBSECTOR ERASE: `20h` if a future task enables it
- SECTOR ERASE: `D8h` if a future task enables it

The A83-only dedicated 4-byte opcodes `13h`, `12h`, `21h`, and `DCh` are
forbidden in the A13 implementation. V1 uses addressless `BULK ERASE (C7h)` for
the active erase task.

While the device is in 4-byte mode, OTP commands also send four address bytes.
`READ OTP ARRAY (4Bh)` then sends eight dummy clocks before receiving data.

## Driver Structure

The implementation preserves the existing ALL-1000 callback ABI, FPGA register
access pattern, CF/DRAM data flow, Gang4 socket-error mechanism, pin mapping,
and public table names. Device-specific behavior is isolated in small internal
helpers for:

- sending a four-byte address;
- reading and Gang4-comparing the Status Register;
- reading and Gang4-comparing the Flag Status Register;
- clearing FSR error bits;
- issuing WREN and checking WEL;
- waiting for WIP to clear with an operation-specific timeout;
- checking program or erase failure flags;
- entering and verifying 4-byte address mode;
- reading, programming, and verifying OTP bytes;
- consuming exactly one 65-byte logical OTP block from the word-based CF
  transport.

The code remains compatible with the existing ARM RVCT C dialect. It uses
fixed-size buffers only and declares variables in forms accepted by the legacy
compiler.

## Task Behavior

### Power and Identification

`PowerOnFun()` follows the addressing initialization above. `PowerOffFun()`
uses the template power-down sequence. `IDCheckFun()` issues `9Fh` and compares
the three JEDEC bytes against the host-provided expected values; mismatching
sockets are handled through the established Gang4 error path.

### Main Flash Read, Program, Verify, and Blank Check

Main Flash Read uses `03h` and four address bytes. Program is page-aligned,
never sends more than 256 bytes in one command, and skips pages whose complete
input data is `FFh`. Verify and Blank Check use the same four-byte READ sequence
and the FPGA comparison path. Buffer counters, logical addresses, remaining
block lengths, and host keep-alive events advance exactly once for each data
chunk.

The serial-number read-modify-program-verify path uses `0Bh` for reads and
`02h` for page programming with four-byte addresses. Its existing single-page
boundary rule remains in force.

### Bulk Erase

The active erase task uses addressless `C7h`. It first verifies that Status
Register protection bits covered by `0x7C` are all zero. It then executes the
normal write flow and uses a 500-second driver timeout, which exceeds the
datasheet's 480-second maximum bulk-erase time.

### Status Register Protection

The N25Q256A Status Register is one byte. The fixed protection mask is `0x7C`,
covering BP3, TB, and BP2-BP0. Both `ProtectFun()` and `UnProtectFun()` must use
read-modify-write:

```c
NewStatus = (CurrentStatus & (uChar)(~0x7C)) |
            (TargetStatus & 0x7C);
```

The driver reads the current Status Register immediately before writing,
changes only the protected field, writes one status byte with `01h`, and reads
back the masked field. All other bits, including SRWD, are preserved. Status
bits WIP and WEL are passed through the expression but are unaffected by the
WRITE STATUS REGISTER command as defined by the datasheet. `UnProtectFun()`
uses target protection value zero. `ProtectFun()` obtains `TargetStatus` from
the Status Register value downloaded to SRAM offset 0.

### OTP

The OTP logical block is always 65 bytes:

- logical bytes 0-63 map to OTP offsets `00h-3Fh`;
- logical byte 64 bit 0 is the permanent-lock request;
- the high byte of the final word read from the word-based CF transport is
  padding and is never programmed, verified, or included in a checksum.

The operation-control byte at SRAM offset 3 retains independent enables for
OTP Program (`0x01`), Verify (`0x02`), Blank Check (`0x08`), and Checksum
(`0x10`). OTP Read remains unconditional when the OTP block is selected.

OTP Program performs the irreversible operation in this order:

1. Consume the 65 logical bytes while discarding transport padding.
2. Program exactly 64 data bytes at offset zero with `42h`.
3. Wait for completion, check FSR program/protection errors, and verify all 64
   bytes.
4. Inspect logical byte 64 bit 0 only after successful data verification.
5. If bit 0 is one, issue no control-byte program command and leave OTP
   unlocked.
6. If bit 0 is zero, separately program `FEh` at offset `40h`, then poll, check
   FSR, and verify only bit 0 is zero.

OTP is never erased. The driver never attempts to change a locked control bit
from zero back to one. Disabled OTP input-consuming tasks still consume 33
physical CF words but advance all logical counters by 65 bytes. `SecureFun()`
remains a successful ABI no-op; the 65th OTP Program byte controls locking.

## Write State Machine and Error Handling

Except for `B7h`, every register, program, OTP-program, and erase write follows:

1. `CLEAR FLAG STATUS REGISTER (50h)`.
2. `WRITE ENABLE (06h)`.
3. Read Status Register and verify WEL bit 1.
4. Issue the modifying command and deassert CS to start it.
5. Poll Status Register WIP bit 0 until clear or timeout.
6. Read FSR and reject the operation-specific failure bits.
7. Read back data or writable register fields when verification is required.

Program and OTP Program reject FSR bits 4 and 1 (`0x12`). Erase rejects bits 5
and 1 (`0x22`). FSR bit 3, which reflects normal VPP-disabled operation, is not
treated as a failure.

The page-program timeout covers the datasheet's 5ms maximum. The status-write
timeout covers its 8ms maximum. The datasheet specifies only a typical 0.2ms
OTP program time, so the OTP timeout is documented as a conservative driver
guard rather than a datasheet maximum. Every error path that owns an active SPI
transaction deasserts CS before returning.

When one or more Gang4 sockets fail a compare or timeout, the driver passes the
socket mask to the existing Gang error handler. If usable sockets remain, it
restores `GEnSelSckNum` from the updated `GangErrSckMask` and continues. If all
sockets fail, it sets the existing task-specific `SysStatusCode` and returns
`FAIL`.

## Header and SPC Tables

The active header contains:

- one 32MB Main Flash block;
- one 65-byte DRAM-backed OTP block;
- one one-byte Status Register SRAM pseudo-block;
- one SRAM driver-operation-control byte at offset 3;
- the existing 13-entry callback order and Gang4 setting;
- page size 256 bytes;
- bulk erase timeout 500 seconds;
- protection mask `0x007C` and default protection value zero;
- an SPI setting of approximately 11MHz, safely below the 54MHz READ limit.

No active or `#if 0` NVCR block is retained in V1. The SPC continues to expose
BP0-BP3 and TB plus the four independent OTP operation enables. It explicitly
warns that clearing OTP control bit 0 is permanent.

## Verification

Static validation will confirm:

- no active A83-only `13h`, `12h`, `21h`, or `DCh` opcode;
- no `B5h` or `B1h` NVCR access;
- `B7h` has WREN/WEL and FSR bit-0 verification but no FSR clear or WIP poll;
- every active Main Flash and OTP address command sends four address bytes;
- every normal write path follows the common state sequence;
- Protect and Unprotect use the `0x7C` RMW expression and preserve other bits;
- OTP data is verified before a possible independent lock operation;
- all OTP paths account for 65 logical bytes and ignore the padding byte;
- all owned-CS failure exits deassert CS;
- `git diff --check` reports no whitespace errors.

The existing `iBuild.bat` procedure must compile with ARM RVCT, link, and
convert the image to BIN without errors. Failure to copy artifacts to the
external `D:\Hi-Lo\ALL1000\Algo1` deployment directory is reported separately
from compilation success.

Hardware validation will cover ID, Main Flash operations below and above the
16MB boundary, protection from both top and bottom, partial Gang4 socket
failure, OTP unlocked operation, each OTP task-control combination, and the
permanent lock flow on a sacrificial sample only.

## Out of Scope

- A83 feature-set parts and their dedicated 4-byte opcodes
- Three-byte plus Extended Address Register bank management
- NVCR read, write, validation, or startup recovery
- Dual, Quad, DTR, and XIP protocols
- Lock Register workflows
- Activating subsector or sector erase as separate host tasks
- Changes to unrelated ALL-1000 firmware or FPGA interfaces
