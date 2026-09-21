# M25PE40 New-FPGA Gang Result Migration Design

## Goal

Adapt `WAx4_M25PE40-VMPxxx` exclusively to the new FPGA convention where each active bit returned by `GChkAllSckStatus` is `1` for PASS and `0` for FAIL.

This change also adopts two related driver-architecture details from `CXx4_GD251xxxx`: service EP0 during the long Bulk Erase completion wait, and restore all currently valid sockets after device parameters are loaded.

## Scope

Only `WAx4_M25PE40-VMPxxx/WAx4_M25PE40-VMPxxx.c` is modified.

The change must not alter SPI commands, WEL expectations, BP protection semantics, power sequencing, serial-number commands, timing-table values, block metadata, or `.spc` contents.

## Gang Result Contract

The new FPGA returns a PASS mask:

- active socket bit `1`: comparison passed;
- active socket bit `0`: comparison failed;
- inactive or already retired sockets must not reappear as failures.

Define:

```c
#define GangOkSckMask ((uChar)(~GangErrSckMask) & 0x0F)
#define ChangeBits(x) ((uChar)(~(x)) & GangOkSckMask)
```

`ChangeBits(rawPassMask)` is therefore the error mask accepted by `GangCheckIfAllSocketError()`.

All 15 existing gang-result decision points must normalize the raw result before testing it or passing it to `GangCheckIfAllSocketError()`. This includes Program, Verify, Erase, Blank Check, ID Check, WEL verification, WRSR completion/BP verification, and serial-number Page Write completion.

The expected device data is unchanged. In particular, `WriteStatusWELBit()` must continue to mask and compare `StatusWELBit`; only the returned socket mask is inverted.

## Bulk Erase Host Servicing

Keep the existing two-phase Bulk Erase check:

1. confirm WIP becomes `1` after C7h;
2. wait until WIP becomes `0`.

Only in the long second loop, call `HandleEvent_EP0()` every 128 polling iterations and retain the existing short delay after event handling. This prevents the up-to-11-second erase wait from starving host communication.

## Socket Restoration

At the end of `GetDeviceParameterInfo()`, write `~GangErrSckMask` to `GEnSelSckNum`, then wait three 10-ns units. This establishes all non-retired sockets as the default gang selection after parameter loading.

## Error Handling

No error codes or retry limits change. Every comparison failure is converted to an error mask before the existing gang-error handler is called. Error sockets continue to be retired through the existing framework behavior.

## Verification

Static checks must prove:

- both polarity macros exist;
- all 15 raw gang-result sites use `ChangeBits()` before success/failure decisions;
- `GangCheckIfAllSocketError()` never receives a raw `GChkAllSckStatus` or raw `CheckAllStatusData()` result;
- WEL comparison still expects `StatusWELBit`;
- the Bulk Erase completion loop contains EP0 servicing, while the short WIP-assertion loop does not;
- `GetDeviceParameterInfo()` restores all valid sockets.

Compile with ARMCC, link with the existing `All1000Code` object set, and convert the image with FROMELF in a local verification directory. Do not deploy the image.

Hardware verification must cover all-pass, one-socket failure, multiple-socket failure, and all-socket failure masks on the new FPGA, plus successful and rejected Bulk Erase operations.
