# N25Q256A OTP Secure Function Design

Date: 2026-09-01

## Scope

Adapt `WAx4_N25Q256Ax3ESFxxx` so the programmer's `SECURE_FUN` operation programs and optionally locks the Micron N25Q256A OTP array. Remove the unreachable TH/GD status-register locking implementation.

## Datasheet model

- OTP data area: 64 bytes, device OTP addresses `00h-3Fh`.
- OTP control byte: address `40h`; bit0 is `1` when unlocked and `0` when permanently locked.
- Read command: `4Bh`, current address width, followed by eight dummy clocks.
- Program command: `42h`, current address width, preceded by `06h`.
- The driver operates in 4-byte address mode, so OTP commands send four address bytes.
- Locking is irreversible. Programming control bit0 to zero must occur only after the 64 data bytes verify successfully.

## Block table

Expose one 65-byte DRAM-backed special block:

- Virtual/RAM range: `02000000h-02000040h`.
- OTP device range: `00000000h-00000040h`.
- Noun: `Noun_Block1`.
- Attribute: `0x0375`: special array, Secure enabled, and normal Read/Program/Verify/Erase/Blank/Checksum disabled.
- Byte `0-63`: desired OTP contents.
- Byte `64` bit0: `1` means leave unlocked; `0` requests permanent locking.

Move the Status Register pseudo block to `Noun_Block2` and set Secure-Off so `SECURE_FUN` is invoked only for the OTP block. Keep the operation-control block unchanged.

## Secure flow

1. Ignore non-OTP blocks.
2. Read 65 input bytes from the downloaded block data.
3. Clear Flag Status Register, issue WREN, and program bytes `0-63` with `42h` at OTP address zero.
4. Poll Status Register WIP until clear, then check Flag Status Register program/protection error bits.
5. Read back bytes `0-63` with `4Bh`, four address bytes, and one dummy byte; compare all active sockets through the FPGA gang checker.
6. If byte64 bit0 requests lock, enable only sockets still passing, program `FEh` at address `40h`, poll/check again, then read address `40h` and verify bit0 is zero.
7. On total failure return `FAIL` with `E_SecureFail`; otherwise preserve the existing gang behavior and continue with remaining passing sockets.

## Safety constraints

- Never issue the lock command before OTP data verification succeeds.
- Never attempt to set control bit0 back to one.
- Do not use Status Register 2, LB1-LB3, Security Register commands, or TH/GD command sequences.
- Register `SecureFun` in the 13-entry algorithm table only after the complete OTP path is implemented.
- Compile with the existing ARM RVDS toolchain and publish BIN/SPC to `D:\Hi-Lo\ALL1000\Algo1`.
