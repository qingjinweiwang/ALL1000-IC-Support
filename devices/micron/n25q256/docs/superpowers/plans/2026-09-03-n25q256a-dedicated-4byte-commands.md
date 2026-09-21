# N25Q256A Dedicated 4-Byte Commands Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace all N25Q256A main-array EAR/3-byte accesses with Micron dedicated 4-byte commands.

**Architecture:** Preserve the existing four-socket FPGA SPI transport and task functions. Remove EAR state management, convert the existing address sender to four bytes, and use dedicated command codes for every main-array and serial-number access while leaving OTP and register operations unchanged.

**Tech Stack:** ARM C firmware, ALL-1000 FPGA SPI register interface, CodeWarrior/RVDS build.

## Global Constraints

- Modify only `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`.
- Do not change the FPGA SPI framework, gang socket mechanism, CS control, or delay APIs.
- Do not add a helper function; rename and extend the existing address helper.
- Keep N25Q256A OTP, status, FSR, protection, and Bulk Erase behavior unchanged.

---

### Task 1: Convert the complete main-array path atomically

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`
- Test: existing `WAx4_N25Q256Ax3ESFxxx/iBuild.bat`

**Interfaces:**
- Consumes: existing `PtrData`, `SPIRWDelay`, `DeviceAddress`, and gang checking interfaces.
- Produces: unchanged public task interfaces and `void Send4ByteAddress(uInt32 Address)` for internal main-array use.

- [ ] **Step 1: Replace command and helper declarations**

Define `ReadData4BCmd = 0x13`, `FastReadData4BCmd = 0x0C`, `PageProg4BCmd = 0x12`, `SubsectorErase4BCmd = 0x21`, and `SectorErase4BCmd = 0xDC`. Remove EAR command/state definitions and declarations. Rename `Send3ByteAddress` to `Send4ByteAddress`.

- [ ] **Step 2: Simplify PowerOnFun**

Remove `StatusData`, FSR address-mode checking, EAR cache initialization, and `SetExtendedAddress(0x00)`. Keep power sequencing, parameter loading, delays, and `SUCCESS` return unchanged.

- [ ] **Step 3: Implement the four-byte address sender**

Use the existing helper body and prepend the missing high byte:

```c
void Send4ByteAddress(uInt32 Address)
{
    PtrData(OneDataCmd) = MSB(MSW(Address)) ;
    SPIRWDelay() ;
    PtrData(OneDataCmd) = LSB(MSW(Address)) ;
    SPIRWDelay() ;
    PtrData(OneDataCmd) = MSB(LSW(Address)) ;
    SPIRWDelay() ;
    PtrData(OneDataCmd) = LSB(LSW(Address)) ;
    SPIRWDelay() ;
}
```

- [ ] **Step 4: Convert Read, Program, Verify, and Blank Check**

For main-array paths only, replace `03h` with `13h`, `02h` with `12h`, and every `Send3ByteAddress(DeviceAddress)` with `Send4ByteAddress(DeviceAddress)`. Remove all `SelectAddressBank()` calls and all `0x01000000` boundary restart branches. Preserve data loops, socket selection, status polling, error propagation, and block-length accounting.

- [ ] **Step 5: Convert serial-number main-array access**

Use `0Ch + four-byte address + one dummy byte` for both reads and `12h + four-byte address` for programming. Remove all `SelectAddressBank()` calls while preserving per-socket selection and verification.

- [ ] **Step 6: Delete EAR implementations**

Delete `CheckAllExtendedAddressData()`, `SetExtendedAddress()`, and `SelectAddressBank()` definitions. Confirm no EAR symbol remains.

- [ ] **Step 7: Run static validation**

Run:

```powershell
rg -n "EAR|ExtendedAddress|SelectAddressBank|Send3ByteAddress" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
rg -n "ReadData4BCmd|FastReadData4BCmd|PageProg4BCmd|Send4ByteAddress" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
git diff --check -- WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
```

Expected: the first search has no output; the second covers every main-array and SN path; whitespace validation passes.

- [ ] **Step 8: Build and inspect**

Run `cmd /c iBuild.bat` from `WAx4_N25Q256Ax3ESFxxx`. Expected: compilation, link, and BIN conversion succeed with zero errors and no new warnings. A denied copy to external `D:\Hi-Lo\ALL1000\Algo1` is a deployment limitation, not a compile failure.
