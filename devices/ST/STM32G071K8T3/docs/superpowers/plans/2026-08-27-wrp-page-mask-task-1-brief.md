# Task 1: Remove unused WRP page-mask state

Modify only `WJ_STM32G071K8T3.c` within `ValidateProtectConfig`.

Remove the declarations `wrp1ar_pages` and `wrp1br_pages`. Remove their two
disabled-area zero assignments and their two variable-shift calculations plus
mask assignments. Do not change `FLASH_OPTION_ERROR_MASK`,
`FLASH_OPTION_CLEAR_MASK`, any other macro, WRP field extraction, disabled
area condition, `TOTAL_PAGE` validation, error returns, or output registers.

Do not stage or commit. Verify with:

```powershell
rg -n "wrp1(ar|br)_pages|1UL <<|PROTECT_ALL_PAGE_MASK" WJ_STM32G071K8T3.c
```

Expected: no matches. Then inspect the function to confirm both WRP areas
retain `start > end` and `end >= TOTAL_PAGE` rejection.
