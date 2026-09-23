# M25PE40 Template Cleanup Implementation Plan

**Goal:** Remove unsupported AGD25Q40C template features while retaining the driver architecture.

**Architecture:** Keep the .c/.h split, firmware entry tables and ARM SPI transfer implementation. Restrict special-block operations to the single status register.

**Tech stack:** C, ARM RVCT 2.2, ALL1000Code firmware objects.

- [x] Remove unsupported macros, declarations, functions and extended-memory branches from WAx4_M25PE40-VMPxxx.c; retain normal-array bodies.
- [x] Reduce status reads/protection to 05h/01h and BP mask 1Ch; use existing .spc byte 3 bit 7 control for Verify/BlankCheck.
- [x] Remove disabled OTP table and unused SecureFun declaration from WAx4_M25PE40-VMPxxx.h; clarify chip capabilities versus implemented driver operations.
- [x] Check diff, command inventory and ABI entry ordering. Compile and link using the existing RVCT toolchain; do not invoke iBuild.bat because it deploys outside the workspace.
- [x] Record validation and remaining hardware checks.

## Validation results

- ARMCC RVCT 2.2: 0 errors, 9 existing warnings (3 ABI constant initializer, 4 unreachable return, 2 inactive-path variable warnings).
- Added h/SimPinDef.h and h/hs0_mmc.h to resolve missing declarations already present in the original source.
- Source comparisons passed: 16 shared function bodies, 4 main-array branches (excluding removed 98h), and firmware entry/pin/specification tables retained.
- Unsupported symbol scan passed.
- Link attempted with the object list from iBuild.bat: blocked by USBCom.o requiring DevSNProgAndVerify, which is absent from both original and cleaned active source. No stub or new serial-number programming feature was added. No runnable BIN produced or deployed.
- Hardware checks still required: read/ID, erase/program/verify, BP protection/unprotection, and status Verify/BlankCheck with SRAM[3] bit7 enabled/disabled. WEL and interrupt-timeout behavior were retained.
