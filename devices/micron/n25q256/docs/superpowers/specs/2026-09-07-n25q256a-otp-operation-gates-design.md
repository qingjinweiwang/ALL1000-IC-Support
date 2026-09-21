# N25Q256A OTP Operation Gates Design

## Scope

Update only the N25Q256A OTP task controls and their disabled-path data-flow
handling. Preserve the existing Main Flash, Status Register, NVCR, protection,
four-byte addressing, Gang4, and OTP command implementations.

The design follows the PY25Q01GLC Security Register task organization, but uses
the N25Q256A OTP commands, address format, 65-byte logical size, and permanent
lock behavior.

## Customer Control Mapping

SRAM byte 3 contains four independent OTP task enables:

```c
#define OTPProgramOn       0x01
#define OTPVerifyOn        0x02
#define OTPBlankCheckOn    0x08
#define OTPChecksumOn      0x10
```

The customer SPC exposes exactly:

```text
S0003L0100 OTP OPERATION OPTIONS :
E01,01 Enable OTP Program
E02,02 Enable OTP Verify
E08,08 Enable OTP Blank Check
E10,10 Enable OTP Checksum Calculate
```

There is intentionally no OTP Read enable. As in the PY25Q01GLC Security
Register model, `ReadFun()` always reads an enabled-by-Attribute OTP block.

## OTP Logical and Transport Length

The OTP block has 65 logical bytes:

- addresses `00h-3Fh`: 64 data bytes;
- address `40h`: one control byte.

The CF/DRAM interface is word-addressed. Program, Verify, and Checksum consume
32 complete words plus one final word. Only the low byte of the final word is
logical OTP data; its high byte is transport padding.

Every enabled and disabled input-consuming branch must therefore:

- consume 33 physical transport words;
- decrement `DramBufCnt` by 65, not 66;
- decrement `DevCurrBlkLen.all` by 65;
- increment `DeviceAddress` by 65;
- call `GetChkBlkLenEnd()` before returning.

`CFCardReadFillSector()` must not be used for these OTP disabled paths because
it consumes an entire sector rather than one 65-byte logical block.

## Task Behavior

### ReadFun

OTP Read is unconditional and is not controlled by SRAM[3]. It continues to
read the 64-byte data area and control byte in separate `4Bh` transactions,
upload 65 logical bytes, update `ReadBufCnt`, `DeviceAddress`, and
`DevCurrBlkLen.all`, and restore the Gang socket selection.

### ProgramFun

Program tests `OTPProgramOn` before loading expected data. The disabled branch
uses the dedicated 65-byte skip helper so the CF stream is still consumed.

- OFF: perform no `42h`, advance the four logical data-flow counters/state
  items, and return success.
- ON: retain the existing 64-byte data Program, WREN/WEL check, WIP polling,
  FSR failure checks, data Verify, and separately controlled address-`40h`
  lock operation. Control bit0 equal to 1 must not issue a control-byte
  Program; bit0 equal to 0 programs `FEh` and verifies only bit0.

### VerifyFun

Verify tests `OTPVerifyOn` before loading expected data. The disabled branch
uses the same dedicated 65-byte skip helper.

- OFF: perform no `4Bh`, advance the logical data-flow state, and return
  success.
- ON: verify all 64 data bytes and control bit0 using the existing Gang4 OTP
  helper, then advance the same logical state.

### BlankCheckFun

BlankCheck has no CF input.

- OFF: perform no `4Bh`, advance `DeviceAddress` and `DevCurrBlkLen.all` by 65,
  and return success. BlankCheck has no CF input and therefore does not call
  the CF-input block-end helper.
- ON: retain the actual device check for 64 bytes equal to `FFh` and control
  bit0 equal to 1, then advance the same block state.

BlankCheck never erases or programs OTP.

### CheckSumFun

- OFF: consume 33 transport words, advance only 65 logical bytes, leave
  `StdChecksumHi/StdChecksumLo` unchanged, and return success.
- ON: retain the existing logical-byte checksum. The final word high byte is
  never included.

Checksum does not send an IC command.

## Unchanged Areas

- OTP Attribute remains `0x8028`.
- OTP Read remains unconditional.
- Status verification and protection behavior remain unchanged.
- NVCR remains hidden/disabled for the current customer SPC.
- Main Flash Read, Program, Verify, Erase, BlankCheck, ID, and checksum paths
  remain unchanged.
- Gang4 failure masks and socket-selection restoration remain unchanged.

## Validation

Static validation must confirm:

1. all four OTP control macros exist exactly once;
2. `StatusCheckOn` remains absent;
3. SPC exposes the four requested bits and no OTP Read switch;
4. OFF Program sends no `42h`;
5. OFF Verify and BlankCheck send no `4Bh`;
6. OFF Checksum sends no IC command and does not change the checksum;
7. every input-consuming OFF path consumes 33 words but accounts for 65
   logical bytes;
8. no OTP path uses `CFCardReadFillSector()`;
9. RVDS compilation, linking, and BIN conversion succeed;
10. `git diff --check` succeeds.

Hardware validation should cover every combination of the four bits, including
all-zero control. Permanent OTP lock validation must use a sacrificial device.
