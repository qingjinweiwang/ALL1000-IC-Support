# N25Q256A EAR Three-Byte Addressing Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep N25Q256A13 in its default three-byte mode and use EAR bit 0 to access the complete 32MB main array safely.

**Architecture:** Preserve 32-bit logical addresses, select the lower or upper 16MB through a verified and cached EAR value, and send only A23-A0 with address-bearing commands. Power-on only verifies that the device actually started in three-byte mode; it never sends `B7h` or `E9h`. Long read/verify/blank-check transactions restart after an EAR switch at the 16MB boundary.

**Tech Stack:** ARM RVCT C, ALL-1000 firmware ABI, Micron N25Q256A13 extended SPI commands, FPGA four-socket compare registers.

## Global Constraints

- Preserve logical main-array addresses `00000000h` through `01FFFFFFh`.
- Preserve the 13-callback ABI and all tables in `WAx4_N25Q256Ax3ESFxxx.h`.
- Do not enter or exit four-byte address mode and do not rewrite NVCR.
- Require Flag Status Register bit 0 to be clear at power-on.
- Write EAR with `06h + C5h` and verify it with `C8h` before accepting a bank.
- Every address-bearing main-array command, including `20h` and `D8h` erase, selects its bank before the command-specific WREN.
- Addressless `C7h` bulk erase never selects a bank.
- OTP always sends three address bytes and never selects an EAR bank.
- Preserve unrelated protection, pin-map, voltage, OTP-lock, and gang-socket behavior.

---

### Task 1: Replace Four-Byte Mode Setup with Verified EAR Infrastructure

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:1-1290`

**Interfaces:**
- Produces: `void Send3ByteAddress(uInt32 Address)`
- Produces: `uChar CheckAllExtendedAddressData(uChar DevData)`
- Produces: `uChar SetExtendedAddress(uChar Bank)`
- Produces: `uChar SelectAddressBank(uInt32 Address)`
- Produces: globals `uChar CurrentBank` and `uChar CurrentBankValid`

- [ ] **Step 1: Record the pre-change source assertions**

Run `rg -n "Enter4ByteAddrCmd|Enter4ByteAddressMode|Send4ByteAddress|FlagStatusAddr4ByteBit" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`.

Expected: active matches for `B7h`, the power-on call, four-byte address sends, and the mode-entry helper.

- [ ] **Step 2: Replace command definitions, state, and declarations**

Remove `Enter4ByteAddrCmd`. Keep `FlagStatusAddr4ByteBit` for the power-on check and add:

```c
#define ReadExtendedAddressCmd   0xC8  /* Read Extended Address Register */
#define WriteExtendedAddressCmd  0xC5  /* Write Extended Address Register */

uChar CurrentBank ;
uChar CurrentBankValid ;

void Send3ByteAddress(uInt32 Address) ;
uChar CheckAllExtendedAddressData(uChar DevData) ;
uChar SetExtendedAddress(uChar Bank) ;
uChar SelectAddressBank(uInt32 Address) ;
```

Delete declarations for `Send4ByteAddress()` and `Enter4ByteAddressMode()`.

- [ ] **Step 3: Make power-on verify the approved premise**

After selecting all passing sockets in `PowerOnFun()`, invalidate the cache, compare FSR bit 0 against zero, and initialize EAR bank 0:

```c
CurrentBankValid = 0 ;
PtrData(GSetAllMaskDataLo) = FlagStatusAddr4ByteBit ;
if( CheckAllFlagStatusData(0x00) )
{
    SysStatusCode = E_DeviceInitialFail ;
    return FAIL ;
}
if( !SetExtendedAddress(0) )
{
    SysStatusCode = E_DeviceInitialFail ;
    return FAIL ;
}
```

Do not send `B7h` or `E9h` anywhere.

- [ ] **Step 4: Implement the three-byte sender and EAR compare helper**

Replace `Send4ByteAddress()` with:

```c
void Send3ByteAddress(uInt32 Address)
{
    PtrData(OneDataCmd) = LSB(MSW(Address)) ;
    SPIRWDelay() ;
    PtrData(OneDataCmd) = MSB(LSW(Address)) ;
    SPIRWDelay() ;
    PtrData(OneDataCmd) = LSB(LSW(Address)) ;
    SPIRWDelay() ;
}
```

Add a `C8h` gang-compare helper matching `CheckAllStatusData()` but using `ReadExtendedAddressCmd` and comparing one byte through `GDevChkDataLoInc0`/`GChkAllSckStatus`.

- [ ] **Step 5: Implement verified and cached EAR selection**

`SetExtendedAddress()` masks `Bank` to bit 0, invalidates the cache before writing, calls `WriteStatusWELBit()`, sends `C5h` plus the one-byte bank value, and verifies via `C8h` with mask `0x01`. On mismatch it calls `GangCheckIfAllSocketError()` and fails if no usable socket remains. It sets `CurrentBank` and `CurrentBankValid` only after verification for the remaining enabled sockets.

```c
uChar SelectAddressBank(uInt32 Address)
{
    uChar Bank = (uChar)((Address >> 24) & 0x01) ;

    if( CurrentBankValid && (CurrentBank == Bank) )
        return SUCCESS ;
    return SetExtendedAddress(Bank) ;
}
```

- [ ] **Step 6: Verify Task 1 and commit**

Run:

```powershell
rg -n "Enter4ByteAddrCmd|Enter4ByteAddressMode|Send4ByteAddress|0xB7|0xE9" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
rg -n "ReadExtendedAddressCmd|WriteExtendedAddressCmd|Send3ByteAddress|SetExtendedAddress|SelectAddressBank" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
```

Expected: the first command has no active-code matches; the second lists definitions and calls. Commit only the driver source with message `feat: add N25Q256A EAR bank selection`.

---

### Task 2: Route Main-Array Operations and Restart at the 16MB Boundary

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:145-561`

**Interfaces:**
- Consumes: `SelectAddressBank()` and `Send3ByteAddress()` from Task 1
- Preserves: `ReadFun()`, `ProgramFun()`, `VerifyFun()`, and `BlankCheckFun()` ABI

- [ ] **Step 1: Add bank selection to READ**

Before asserting CS in the main-array branch of `ReadFun()`, call `SelectAddressBank(DeviceAddress)` and return `E_ReadFail` on failure. Replace the address sender with `Send3ByteAddress()`.

After each `MaxPktByteSize` packet, increment `DeviceAddress`. If data remains and `(DeviceAddress & 0x00FFFFFF) == 0`, deassert CS, select the new bank, and reissue `03h + Send3ByteAddress(DeviceAddress)` before continuing. Keep `ReadBufCnt`, `DevCurrBlkLen`, and `SaveChkBlkLenEnd()` accounting unchanged.

- [ ] **Step 2: Add bank selection to PAGE PROGRAM**

For each nonblank page, call `SelectAddressBank(DeviceAddress)` before `ClearFlagStatusData()` and `WriteStatusWELBit()`. On failure set `E_ProgramNoDataFail`. Replace the page-program address sender with `Send3ByteAddress()`.

- [ ] **Step 3: Add bank selection and boundary restart to VERIFY**

Before opening `03h`, select the initial bank and use `Send3ByteAddress()`. After each 256-byte compare group, increment `DeviceAddress`. When more data remains and the low 24 bits become zero, close CS, select the next bank, and reopen `03h` at the new three-byte address. On selection failure close CS, set `E_VerifyFail`, and return `FAIL`.

- [ ] **Step 4: Add bank selection and boundary restart to BLANK CHECK**

Apply the same 256-byte boundary restart as VERIFY, using `E_BlankFail`. The bank switch happens only after CS is high; the reopened transaction sends `03h` and the new three-byte address.

- [ ] **Step 5: Inspect and commit**

Run `rg -n -A8 -B8 "SelectAddressBank|Send3ByteAddress|DeviceAddress \+=|0x00FFFFFF" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c`.

Expected: READ, PROGRAM, VERIFY, and BLANK CHECK select the bank; READ/VERIFY/BLANK CHECK update the logical address and restart after a boundary. Commit with message `feat: route N25Q256A array operations through EAR`.

---

### Task 3: Cover OTP, Serial Number, Erase Rules, and Final Verification

**Files:**
- Modify: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c:408-474,840-995,1324-1456`
- Inspect: `WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h`

**Interfaces:**
- Consumes: `SelectAddressBank()` and `Send3ByteAddress()`
- Preserves: `EraseFun()`, OTP helpers, `SecureFun()`, and `DevSNProgAndVerify()` ABI

- [ ] **Step 1: Isolate OTP three-byte addressing**

In both OTP helpers, replace the generic address call with exactly:

```c
PtrData(OneDataCmd) = 0x00 ;
SPIRWDelay() ;
PtrData(OneDataCmd) = 0x00 ;
SPIRWDelay() ;
PtrData(OneDataCmd) = Address ;
SPIRWDelay() ;
```

Do not call `SelectAddressBank()` in either OTP helper.

- [ ] **Step 2: Route serial-number page operations through EAR**

Before each serial-number page READ, PAGE PROGRAM, and verify READ, call `SelectAddressBank(DeviceAddress)`. For PAGE PROGRAM, select the bank before issuing its WREN. Replace all serial-number address sends with `Send3ByteAddress(DeviceAddress)`. Preserve the one-page overflow check and socket restoration paths.

- [ ] **Step 3: Enforce address-bearing erase ordering**

Keep active `C7h` bulk erase unchanged and EAR-independent. Add:

```c
/* Any 20h/ D8h address erase must call SelectAddressBank(DeviceAddress)
 * before its operation-specific WriteStatusWELBit(), then send A23-A0. */
```

If an active `20h` or `D8h` path exists, order it as `SelectAddressBank(DeviceAddress)`, operation-specific WREN, opcode, and `Send3ByteAddress(DeviceAddress)`.

- [ ] **Step 4: Perform static conformance checks**

Run:

```powershell
rg -n "0xB7|0xE9|Enter4ByteAddressMode|Send4ByteAddress" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
rg -n "SubsectorEraseCmd|SectorEraseCmd|BulkEraseCmd|SelectAddressBank" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c
rg -n "0x01FF,0xFFFF|Main Flash 32MB" WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h
git diff --check -- WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.c WAx4_N25Q256Ax3ESFxxx/WAx4_N25Q256Ax3ESFxxx.h
```

Expected: no active four-byte-mode matches; the header still ends main flash at `01FFFFFFh`; `git diff --check` is silent.

- [ ] **Step 5: Build the driver**

Run `cmd /c iBuild.bat` from `WAx4_N25Q256Ax3ESFxxx`.

Expected: ARM compiler, linker, and `fromelf` complete with `[SUCCESS] Build successful.` and zero compiler errors. A final copy failure to the external `D:\Hi-Lo\ALL1000\Algo1` destination is reported separately and does not invalidate compile/link/fromelf success.

- [ ] **Step 6: Commit the completed driver**

Commit the source with message `feat: use three-byte EAR addressing for N25Q256A`.
