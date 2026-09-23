# CAT25256 `.spc` Protection Design

## Goal

Redesign the protection-related presentation in `WJ_CAT25256.spc` so it
matches the CAT25256 data sheet and does not present `WPEN` as an EEPROM
array protection type.

## Data-sheet model

The EEPROM array protection level is selected by the non-volatile
`BP1:BP0` status-register bits:

| BP1 | BP0 | Protected range | Protection type |
| --- | --- | --- | --- |
| 0 | 0 | None | No protection |
| 0 | 1 | `0x6000-0x7FFF` | Upper quarter |
| 1 | 0 | `0x4000-0x7FFF` | Upper half |
| 1 | 1 | `0x0000-0x7FFF` | Full array |

`WPEN` does not select an array range. When `WPEN` is set and the external
`WP` pin is low, writes to the status register are inhibited. It is therefore
a hardware lock control, not a fifth array protection type.

## Selected approach

Keep the existing bit-oriented `.spc` controls for `BP0` and `BP1`, because
the available `E<value>,<mask>` format represents independent masked bits and
does not provide a reliable mutually exclusive four-choice control.

Add a visible protection table before the editable status-register fields.
The table will state all four `BP1:BP0` combinations, their protected address
ranges, and their protection names. Rename the editable BP entries so the
operator understands that the final protection level is determined by their
combined value.

Remove the `WPEN=1` editable entry. The current driver protection mask is
`0x0C`, so the driver does not fully support editing or verifying bit 7.
Exposing it in `.spc` would imply a working hardware-lock feature and could
also make later status-register changes impossible when the external `WP`
pin is low.

## Scope

Only `WJ_CAT25256.spc` will change. The existing driver behavior remains:

- `ProtectFun` writes `BP1:BP0 = 11` for full-array protection.
- `UnProtectFun` writes `BP1:BP0 = 00` for no protection.
- The protection register mask remains `0x0C`.

No support for Identification Page bits (`IPL` and `LIP`) or hardware status
register locking (`WPEN`) is added.

## Validation

After editing:

1. Confirm the `.spc` section still uses the existing parser structure:
   device index, `.special.`, display lines, status-register declaration,
   and masked edit lines.
2. Confirm the four combinations and address ranges exactly match the data
   sheet.
3. Confirm only masks `0x04` and `0x08` remain editable.
4. Build the driver project to ensure packaging accepts the updated `.spc`.

