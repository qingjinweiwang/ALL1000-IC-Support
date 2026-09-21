# N25Q256A EAR Three-Byte Addressing Design

## Goal

Keep the N25Q256A13 in its default three-byte address mode while retaining access to the complete 32MB main array (`00000000h` through `01FFFFFFh`) through Extended Address Register (EAR) bit 0.

## Device Premise

The project datasheet, Micron N25Q256A Rev. X, defines nonvolatile configuration register bit 0 as the address-byte selection bit: `1` selects three-byte addressing and is the factory default. The actual power-on mode follows the nonvolatile configuration register, so this driver must not silently assume that an externally modified device still powers up in three-byte mode.

`PowerOnFun()` will not send ENTER 4-BYTE ADDRESS MODE (`B7h`) or EXIT 4-BYTE ADDRESS MODE (`E9h`). It will read the Flag Status Register and require the address-mode status bit to indicate three-byte mode. A device that powers up in four-byte mode fails initialization rather than receiving incorrectly framed commands. The driver does not rewrite the nonvolatile configuration register.

## Addressing Architecture

Main-array operations retain a 32-bit logical address. `SelectAddressBank(address)` derives the required EAR value from logical address bit 24:

- `0` selects `00000000h` through `00FFFFFFh`.
- `1` selects `01000000h` through `01FFFFFFh`.

`SetExtendedAddress(bank)` performs its own WRITE ENABLE (`06h`), writes EAR with `C5h`, reads it back with `C8h`, and accepts the new bank only after every enabled socket verifies the requested value. A cached bank value avoids redundant EAR writes. The cache starts invalid at power-on and is invalidated on failed EAR selection.

`Send3ByteAddress(address)` sends A23-A16, A15-A8, and A7-A0. The existing four-byte sender, four-byte mode entry helper, `B7h` command definition, and four-byte-mode success check are removed.

## Operation Routing

Every main-array command carrying an address must successfully call `SelectAddressBank(address)` before asserting CS for that command. This includes:

- `03h` READ
- `0Bh` FAST READ used by serial-number handling
- `02h` PAGE PROGRAM
- `20h` SUBSECTOR ERASE
- `D8h` SECTOR ERASE

The current `EraseFun()` uses only addressless `C7h` BULK ERASE. It does not select an EAR bank. Any current or future path that activates `20h` or `D8h` must follow the address-bearing erase rule above.

Bank selection occurs before the operation's WRITE ENABLE when the operation modifies the array. This prevents the EAR write sequence from consuming the WEL intended for PAGE PROGRAM or an address-bearing erase.

## Boundary Handling

PAGE PROGRAM remains limited to one 256-byte page, so a correctly aligned page cannot cross the 16MB EAR boundary. Serial-number read-modify-write similarly operates on one aligned 256-byte page.

READ, VERIFY, and BLANK CHECK may span `00FFFFFFh` to `01000000h`. At that boundary the driver must:

1. Finish the lower-bank bytes and deassert CS.
2. Advance the logical address to `01000000h`.
3. Select and verify EAR bank 1.
4. Reissue the original command with address `000000h`.
5. Continue without losing buffer or remaining-length accounting.

No active SPI transaction may change EAR.

## OTP Isolation

OTP commands are independent of the main-array EAR selection. READ OTP ARRAY (`4Bh`) and PROGRAM OTP ARRAY (`42h`) send exactly three address bytes: `00h`, `00h`, and the OTP byte offset. They do not call `SelectAddressBank()`.

## Error Handling

Failure to confirm three-byte mode at power-on returns `FAIL`. Failure to write or verify EAR uses the current operation's existing status code, releases CS if necessary, invalidates the cached bank, and follows existing gang-socket error handling. No operation proceeds with an unverified bank.

Bulk erase, program, protect, unprotect, and OTP Flag Status Register checks otherwise retain their current behavior.

## Compatibility and Scope

- Preserve the firmware's 13-callback ABI and the header's full 32MB block definition.
- Preserve the existing standard `03h`, `0Bh`, `02h`, `20h`, `D8h`, and `C7h` opcodes.
- Target the N25Q256A13 behavior described by the bundled datasheet. No A83-specific power-up or command exceptions are added.
- Do not add a fallback that switches address modes.
- Do not change unrelated protection, OTP-lock, pin-map, voltage, or gang-socket behavior.

## Verification

Verification must include:

- Source scan showing no active `B7h`, `Enter4ByteAddressMode()`, or `Send4ByteAddress()` implementation or call.
- Source inspection confirming every active `20h`/`D8h` address-bearing erase path selects EAR before its operation-specific WREN.
- Tests or targeted checks for bank selection at `00FFFFFFh` and `01000000h` and for read/verify/blank-check boundary restart behavior.
- Inspection confirming OTP transmits exactly three address bytes without EAR selection.
- Inspection confirming `C7h` bulk erase remains EAR-independent.
- A clean driver build through `iBuild.bat` with zero compiler errors.
