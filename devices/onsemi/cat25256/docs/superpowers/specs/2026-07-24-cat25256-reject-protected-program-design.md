# CAT25256 Reject-Protected-Program Design

## Goal

Make `ProgramFun` refuse to program a CAT25256 whose BP0 or BP1 status bit
is set. The driver must preserve the protection state until the operator
explicitly runs `UnProtectFun`.

## Selected behavior

At the start of `ProgramFun`, after selecting sockets and excluding the
special block:

1. Read the CAT25256 status register.
2. Test only `StatusArrayProtectMask` (`0x0C`).
3. If BP0 or BP1 is set:
   - Set `SysStatusCode` to `E_ProtectStatusFail`.
   - Return `FAIL`.
   - Do not call `CAT25256WriteStatus`.
   - Do not modify BP0, BP1, or WPEN.
4. Continue with normal page programming only when BP1:BP0 is `00`.

The replacement logic is:

```c
StatusData = CAT25256ReadStatus();
if (StatusData & StatusArrayProtectMask)
{
	SysStatusCode = E_ProtectStatusFail;
	return FAIL;
}
```

## Resulting status-value behavior

| Status value | Array protection | Program behavior |
| --- | --- | --- |
| `00` | None | Allowed |
| `04` | Upper quarter | Rejected |
| `08` | Upper half | Rejected |
| `0C` | Full array | Rejected |
| `80` | None; WPEN enabled | Allowed |
| `84` | Upper quarter; WPEN enabled | Rejected |
| `88` | Upper half; WPEN enabled | Rejected |
| `8C` | Full array; WPEN enabled | Rejected |

WPEN alone is not an EEPROM array-protection bit, so `0x80` does not cause
`ProgramFun` to reject programming.

## Explicit unprotect workflow

`UnProtectFun` remains the only driver operation that clears BP0, BP1, and
WPEN. The intended operator flow is:

1. Protect with a value whose BP bits are nonzero.
2. A Program operation fails with `E_ProtectStatusFail`.
3. Run Unprotect explicitly.
4. Program is allowed after the status value becomes `0x00`.

## Scope

Only the protection precheck in `WJ_CAT25256.c::ProgramFun` changes. The
status-register input UI, Protect, Unprotect, page programming, SPI helpers,
and device tables remain unchanged.

## Validation

1. Before implementation, assert that `ProgramFun` still calls
   `CAT25256WriteStatus` inside its BP precheck.
2. After implementation, assert that the precheck sets
   `E_ProtectStatusFail`, returns `FAIL`, and contains no automatic status
   write.
3. Confirm `StatusArrayProtectMask` remains `0x0C`.
4. Confirm `UnProtectFun` still performs the explicit `0x00` status write.
5. Run `git diff --check`.
6. Build the CodeWarrior Debug target and require `No Error. (Code 0)`.
7. On hardware, verify Protect `0C` → Program rejected → Unprotect → Program
   allowed.
