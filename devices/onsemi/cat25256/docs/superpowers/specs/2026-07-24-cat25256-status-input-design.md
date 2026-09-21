# CAT25256 Status Register Input-Field Design

## Goal

Replace the BP0, BP1, and WPEN checkboxes in `WJ_CAT25256.spc` with one
hexadecimal byte input field while clearly documenting every supported
protection combination.

## Selected UI

Use the established ALL-1000 one-byte text-input syntax:

```text
t0000L01 Status Register Value (Default 0x00)
```

The field maps directly to `SpecialBitBuf(0)`. The existing driver masks the
value with `0x8C`, so only WPEN (bit 7), BP1 (bit 3), and BP0 (bit 2) are
applied.

## Operator-facing combination table

The `.spc` comments must list all eight meaningful combinations:

| Input | BP1:BP0 | WPEN | Effect |
| --- | --- | --- | --- |
| `00` | `00` | 0 | No array protection; WP pin function disabled |
| `04` | `01` | 0 | Protect `0x6000-0x7FFF`; WP pin function disabled |
| `08` | `10` | 0 | Protect `0x4000-0x7FFF`; WP pin function disabled |
| `0C` | `11` | 0 | Protect `0x0000-0x7FFF`; WP pin function disabled |
| `80` | `00` | 1 | No array protection; WP low locks the status register |
| `84` | `01` | 1 | Protect `0x6000-0x7FFF`; WP low locks the status register |
| `88` | `10` | 1 | Protect `0x4000-0x7FFF`; WP low locks the status register |
| `8C` | `11` | 1 | Protect `0x0000-0x7FFF`; WP low locks the status register |

The comments must also state:

- When WPEN is 1 and WP is high, the status register remains writable after
  WREN.
- Only bits 7, 3, and 2 are effective.
- Other entered bits are masked out by the driver.

## Scope

Only `WJ_CAT25256.spc` changes:

- Remove the `S0000L0100` checkbox group.
- Remove all `E<value>,<mask>` entries.
- Add the eight-row effect table.
- Add one `t0000L01` input field.

`WJ_CAT25256.c` and `WJ_CAT25256.h` already implement the required
`SpecialBitBuf(0)` and `0x8C` mask path and are not changed by this task.

## Validation

1. Confirm there is exactly one `t0000L01` entry.
2. Confirm no `E04,04`, `E08,08`, or `E80,80` checkbox entry remains.
3. Confirm all eight supported values and effects are documented.
4. Confirm the `0x8C` mask behavior and WP-high qualification are stated.
5. Run `git diff --check`.
6. Build the CodeWarrior Debug target and require `No Error. (Code 0)`.
