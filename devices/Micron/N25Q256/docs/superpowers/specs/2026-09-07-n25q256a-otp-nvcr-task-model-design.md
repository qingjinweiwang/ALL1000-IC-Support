# N25Q256A OTP and NVCR Task Model Design

## Goal

Reorganize `WAx4_N25Q256Ax3ESFxxx` so its OTP and nonvolatile
configuration register (NVCR) operations follow the Programmer Framework
block attributes while retaining the existing N25Q256A main-flash and status
register behavior.

The PY25Q01GLC driver is an architectural reference only. Its Security
Register and Configure Register commands are not valid for N25Q256A.

## Source hierarchy

1. The Micron N25Q256A datasheet, `n25q_256mb_65nm.pdf`, Rev. X 06/18, is
   authoritative for commands, address length, byte order, register bits,
   defaults, timing, and irreversible behavior.
2. The Programmer Framework attribute definitions and buffer APIs are
   authoritative for task routing and data-flow accounting.
3. `SYx4_PY25Q01GLC-SMxxx.c/.h` provide examples of `Res15` DRAM blocks,
   Special SRAM blocks, Gang4 checking, and task organization.

The requested PY25Q01GLC `.spc` file is absent from the workspace. This does
not block implementation because its relevant SRAM organization is present in
the reference block table and task code. No PY-specific command will be used.

## Block model

The target block table keeps these attributes and mappings:

| Block | Attribute | Storage | Routing | Device range | SRAM/DRAM mapping |
| --- | --- | --- | --- | --- | --- |
| Main Flash | `0x0000` | DRAM | Normal | `00000000h-01FFFFFFh` | DRAM `00000000h` |
| OTP | `0x8028` | DRAM | `bBlkAttri_Res15` | `00h-40h` | DRAM `02000000h` |
| Status | `0x022B` | SRAM, Special | `IfSpecialBitBlk()` / `Noun_Block2` | one byte | SRAM[0] |
| NVCR | `0x0227` | SRAM, Special | `IfSpecialBitBlk()` / `Noun_Block3` | two bytes | SRAM[1:2] |
| Driver Control | `0x02FF` | SRAM, Special | `IfSpecialBitBlk()` / `Noun_BlkPtStatus` | one virtual byte | SRAM[3] |

`0x8028` enables Read, Program, Verify, BlankCheck, and Checksum while
disabling Erase and Secure. `0x0227` enables Read, Verify, BlankCheck, and
Secure while disabling Program, Erase, and Checksum.

The status block retains `0x022B` and the existing Protect/UnProtect design.
The Driver Control block remains virtual and never produces an SPI command.

`0x022B` is an intentional Programmer Framework exception. Its bit2 is 0, so
the ordinary Program task is enabled by the attribute, but the task performs
no device write for this Special block. Status Register writes remain owned by
Protect/UnProtect. The attribute must not be changed to "fix" this established
framework convention.

## Fixed SRAM layout

The SRAM layout is immutable for this change:

```text
SRAM[0] = Status Register
SRAM[1] = NVCR[7:0]  (low byte)
SRAM[2] = NVCR[15:8] (high byte)
SRAM[3] = Driver operation control
```

SRAM[3] bit7 (`StatusCheckOn`) gates Status and NVCR Verify/BlankCheck.
It does not gate OTP tasks and is never written to the device.

## Datasheet-derived protocol

### OTP

- READ OTP ARRAY is `4Bh`.
- PROGRAM OTP ARRAY is `42h` and requires WREN/WEL.
- The driver enters 4-byte address mode during `PowerOnFun()`, so OTP commands
  use four address bytes in this operating state.
- READ OTP requires eight dummy clocks after the address.
- OTP data occupies `00h-3Fh`; the control byte is at `40h`.
- Control bit0 is the only defined control bit: 1 means unlocked and 0 means
  permanently locked.
- Programming control bit0 to 0 is irreversible.
- Program completion is checked through Status.WIP and FSR program/protection
  error bits.

### NVCR

- READ NVCR is `B5h` and returns the low byte first, then the high byte.
- WRITE NVCR is `B1h`, requires WREN/WEL, and accepts the low byte first, then
  the high byte.
- Factory default is `FFFFh`.
- Bit5 is Reserved/Don't Care, reads as 1, and is not exposed in the SPC. A
  Gang write therefore forces target bit5 to 1 rather than pretending to
  preserve a possibly different per-socket value. Verify excludes bit5.
- XIP encodings 101b and 110b are reserved.
- Output-driver-strength encodings 000b and 100b are reserved.
- NVCR settings are copied into the internal configuration at power-on/reset.
  A completed NVCR write therefore does not immediately switch the active SPI
  protocol, XIP state, or address mode.
- `tWNVCR` has a 3-second maximum. Polling allows margin beyond that maximum.

## Task design

### ReadFun

Special blocks are dispatched by noun:

- `Noun_Block2`: select socket A, read `05h`, save SRAM[0], and restore all
  valid Gang sockets.
- `Noun_Block3`: select socket A, read `B5h` low then high, save SRAM[1] and
  SRAM[2], and restore all valid Gang sockets.
- `Noun_BlkPtStatus`: return without issuing an SPI command.

OTP is dispatched by `bBlkAttri_Res15`. It selects socket A and uses two
explicit transactions to avoid relying on the Command Set table's ambiguous
1-to-64-byte READ limit:

1. Send `4Bh`, address `00h` as four address bytes, and one dummy byte; read
   exactly 64 OTP data bytes.
2. Raise CS, then send a new `4Bh`, address `40h` as four address bytes, and one
   dummy byte; read exactly one control byte.

The first 64 bytes are packed as 32 complete words. The separately read
control byte is stored as the low byte of the last upload word, with a local
padding high byte. `ReadBufCnt`, `DevCurrBlkLen`, and `DeviceAddress` advance
by 65, not 66. CS is released and the valid Gang selection is restored on
every exit.

Main Flash Read remains unchanged.

### ProgramFun

Special blocks remain no-op in the ordinary Program task. This includes the
intentional Status `0x022B` exception described in the Block Model section.
The OTP `Res15` branch consumes exactly 65 logical bytes from the CF/DRAM
stream:

1. Load 64 OTP data bytes and the low byte of the final transport word as the
   requested control byte.
2. Program the 64-byte data array with `42h` after WREN/WEL validation.
3. Poll WIP, check FSR bits 4 and 1, and verify all 64 bytes with `4Bh`.
4. If requested control bit0 is 1, do not issue a control-byte Program command.
5. If requested control bit0 is 0, program `FEh` at address `40h`.
6. In both cases verify only control bit0, so reserved bits cannot trigger a
   lock or a false comparison.
7. Advance `DramBufCnt`, `DevCurrBlkLen`, and `DeviceAddress` by exactly 65 and
   complete the Programmer Framework buffer handoff.

OTP is never erased. Main Flash Program remains unchanged.

### VerifyFun

Status and NVCR Verify first read SRAM[3] bit7. When disabled, they return
without a device comparison. When enabled:

- Status compares SRAM[0] with `05h` using the existing protection mask.
- NVCR compares SRAM[1] to the first `B5h` byte and SRAM[2] to the second.
  NVCR bit5 is excluded from the user-data comparison.

OTP Verify loads 65 logical CF/DRAM bytes, compares all 64 data bytes, then
compares only control bit0 using a second `4Bh` transaction at address `40h`.
The existing `VerifyOTPArray()` is improved and reused. Partial socket failures
update `GangErrSckMask`, and remaining sockets continue. The task fails only
when no valid socket remains. CS and Gang selection are restored on all exits.

### BlankCheckFun

- Status retains the existing masked default comparison.
- NVCR compares against `FFFFh`, gated by SRAM[3] bit7.
- OTP uses one `4Bh` transaction to compare addresses `00h-3Fh` to `FFh`, then
  a separate `4Bh` transaction at address `40h` to compare control bit0 to 1.

BlankCheck only reports failures and never tries to erase OTP.

### CheckSumFun

Special SRAM blocks return without checksum work. OTP checksum is computed
from its CF/DRAM block data, not by an IC command. Thirty-two full words
contribute both bytes; the final transport word contributes only its low byte.
Counters advance by 65, so padding byte 66 is neither counted nor checksummed.
Main Flash continues to use `SysStdChecksum()`.

The framework transport is word-aligned: `CFCardReadWordData()` dereferences
and advances a `U16 *Rx_buffer_HSMMC_ch0`. Existing framework-compatible
checksum implementations process the low byte, test logical block completion,
and only then process the high byte. Therefore, for an odd-length DRAM block,
the 33rd word physically consumes a padded transport high byte while
`DramBufCnt`, `DevCurrBlkLen`, device addressing, and checksum account for only
65 logical bytes. If hardware integration disproves this padding convention,
implementation must stop and use a framework byte API; it must not reinterpret
an unaligned `U16 *` locally.

### SecureFun

`SecureFun()` acts only on `Noun_Block3`:

1. Read requested NVCR low/high bytes from SRAM[1:2].
2. Assemble the 16-bit target and force Reserved bit5 to 1.
3. Reject reserved combinations before any destructive command:
   XIP `[11:9]` equal to 5 or 6, or driver strength `[8:6]` equal to 0 or 4.
4. Call `WriteNVCRData(TargetNVCR)` for the complete write lifecycle.
5. Call `CheckAllNVCRData(TargetNVCR)` for readback verification of all exposed
   bits on all remaining valid sockets.

Partial Gang failures are recorded and removed from subsequent commands; the
function returns failure only if all sockets fail. Status writes remain in
Protect/UnProtect and are not mixed into this function.

## Helper functions

The implementation adds or updates focused helpers:

- `ReadNVCRData(void)` reads two bytes low-first without changing socket
  selection.
- `CheckAllNVCRData(uShort Data)` compares both bytes through FPGA Gang check
  with bit5 excluded and returns the combined failing-socket mask.
- `WriteNVCRData(uShort Data)` exclusively owns the transaction lifecycle:
  clear FSR, issue WREN and verify WEL, send low-first `B1h`, raise CS, poll WIP
  with margin beyond 3 seconds, check FSR program/protection errors, and
  maintain the Gang error mask/selection. It does not perform final readback.
- `ProgramOTPArray()` retains the N25Q256A `42h` flow and is hardened for
  partial-socket recovery and error-path CS release.
- `VerifyOTPArray()` retains the `4Bh` Gang comparison and is hardened for
  partial-socket recovery and error-path CS release.

No helper silently selects socket A. Upload callers select A explicitly and
restore `~GangErrSckMask`; Gang callers enter with all valid sockets selected.

## Error handling

- OTP Program all-socket failure reports `E_HiddenROMProgramFail`.
- OTP Verify all-socket failure reports `E_HiddenROMVerifyFail`.
- OTP BlankCheck all-socket failure reports `E_HiddenROMBlankFail`.
- NVCR Secure failure reports `E_SecureFail`.
- Status verification retains `E_ProtectStatusFail`.
- NVCR invalid-combination preflight reports `E_UserPrtRegIllegalFail`.
- NVCR Verify and BlankCheck use `E_UserPrtRegVerifyFail` and
  `E_UserPrtRegBlankFail`, respectively.

All listed symbols have been confirmed in `All1000Code/h/ErrorCode.h`; no new
error code is introduced.

Every command path that drives CS low raises it before returning. After any
Gang check removes failed sockets, `GEnSelSckNum` is refreshed from
`~GangErrSckMask` before another command.

## Validation

Validation consists of:

1. Compile the driver with the configured RVDS `armcc` into a temporary output
   location, without running the batch step that copies artifacts externally.
2. Inspect the final diff for changes outside OTP, NVCR, block comments, SPC,
   and necessary helper declarations.
3. Statically audit every CS-low region for a matching CS-high on all exits.
4. Statically audit socket-A reads for restoration of `GEnSelSckNum`.
5. Statically audit OTP counters and loops for an exact logical length of 65.
6. Confirm the generated/read input transport supplies the padded high byte of
   the 33rd word for a 65-byte DRAM block. Static evidence is the `U16 *`
   transport and existing low-byte-before-end-check checksum pattern; final
   confirmation requires Programmer Framework or hardware integration.
7. Test NVCR preflight with valid values plus all four reserved encodings.
8. Verify block attributes against the final task matrix, including the
   documented Status `0x022B` Program exception.
9. Confirm Main Flash command paths are unchanged.

## Known operational risk

The SPC intentionally exposes valid NVCR fields that can select dual/quad SPI,
XIP, or 3-byte addressing. WRITE NVCR itself does not immediately change the
active protocol. After a power cycle or software/hardware reset, however, the
new NVCR is loaded into the internal configuration register and can place the
device in Dual/Quad/XIP/3-byte mode. That state can make this
extended-SPI-only driver unable to communicate. The driver rejects reserved
encodings but does not silently rewrite or block valid requested modes; this
risk is reported explicitly in the implementation handoff.
