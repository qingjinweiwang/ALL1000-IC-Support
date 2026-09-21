# CAT25256 WPEN Configuration Support Design

## Goal

Make `BP0`, `BP1`, and `WPEN` configurable from `WJ_CAT25256.spc`, show the
effect of every protection control, and ensure the driver writes and verifies
the selected status-register value.

## Data-sheet behavior

`BP1:BP0` selects the protected EEPROM array range:

| BP1 | BP0 | Protected range | Effect |
| --- | --- | --- | --- |
| 0 | 0 | None | No array protection |
| 0 | 1 | `0x6000-0x7FFF` | Upper quarter is read-only |
| 1 | 0 | `0x4000-0x7FFF` | Upper half is read-only |
| 1 | 1 | `0x0000-0x7FFF` | Entire array is read-only |

`WPEN` enables the external `WP` pin as a status-register hardware lock:

| WPEN | WP pin | Status-register effect |
| --- | --- | --- |
| 0 | Low or high | `WP` does not lock the status register |
| 1 | High | Status register remains writable after `WREN` |
| 1 | Low | Status-register writes are inhibited |

The `WEL` latch must still be set before any permitted memory or
status-register write.

## Selected architecture

### `.spc` presentation

Keep the existing array-protection table and add a WPEN effect table. Expose
exactly three editable status bits:

```text
 E04,04 BP0 (bit 2)
 E08,08 BP1 (bit 3)
 E80,80 WPEN (bit 7)
```

Do not expose `IPL` or `LIP`.

### Configuration data flow

The `.spc` status byte is stored at `SpecialBitBuf(0)`. `ProtectFun` reads
that byte, masks it with `0x8C`, and passes the result to
`CAT25256WriteStatus`. This makes the Protect operation apply the operator's
selected BP0, BP1, and WPEN values instead of always forcing full-array
protection.

`ProtectRegisterTable[0]` changes from `0x000C` to `0x008C`. The same mask is
used when reading the special status block and when writing/verifying the
selected status value.

The array-protection detection mask remains `0x0C`. `WPEN` alone does not
make the EEPROM array read-only and therefore must not trigger automatic
array unprotection.

### Protect and unprotect behavior

`ProtectFun`:

1. Read `SpecialBitBuf(0)`.
2. Keep only `BP0`, `BP1`, and `WPEN` with mask `0x8C`.
3. Write the selected value through `CAT25256WriteStatus`.
4. Rely on the existing masked read-back verification and report
   `E_ProtectFail` on failure.

`UnProtectFun` writes `0x00` with mask `0x8C`. This clears BP0, BP1, and WPEN,
restoring an unprotected array and disabling the external WP status-register
lock.

If `WPEN=1` and the hardware `WP` pin is already low, the chip inhibits
status-register writes. A later Protect, Unprotect, or automatic unprotect
operation must fail through the existing timeout/read-back path. Recovery
requires driving `WP` high; the driver must not report a false success.

## Files in scope

- `WJ_CAT25256.spc`: add WPEN editing and its hardware effect table.
- `WJ_CAT25256.h`: expand `ProtectRegisterTable[0]` to `0x008C` and update
  comments.
- `WJ_CAT25256.c`: define the writable status mask clearly and make
  `ProtectFun` consume `SpecialBitBuf(0)`.

No changes are made to EEPROM addressing, page programming, identification
page support, pin mapping, or power sequencing.

## Validation

1. Statically verify that `.spc` contains exactly the BP0, BP1, and WPEN edit
   entries, with no IPL or LIP entry.
2. Verify all BP and WPEN effect rows against the CAT25256 data sheet.
3. Verify the driver configuration mask is `0x8C` while the array-protection
   detection mask remains `0x0C`.
4. Verify `ProtectFun` reads `SpecialBitBuf(0)` and masks the selected value.
5. Verify `UnProtectFun` clears all three configurable bits.
6. Run `git diff --check` and inspect the scoped diff.
7. Build the CodeWarrior Debug target and require `No Error. (Code 0)`.
