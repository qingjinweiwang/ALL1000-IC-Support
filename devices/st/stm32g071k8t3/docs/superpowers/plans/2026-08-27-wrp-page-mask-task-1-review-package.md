# Task 1 review package

## Change context

`WJ_STM32G071K8T3.c` is an untracked source file, so Git cannot construct a
commit range or diff for this task. The user directed that it must not be
staged or committed. Review the targeted function below against the attached
task brief and implementation report.

## Targeted source after implementation

```c
static uChar ValidateProtectConfig(uInt32 optr,uInt32 wrp1ar,uInt32 wrp1br,uInt32 *wrp1ar_reg,uInt32 *wrp1br_reg)
{
	uInt32 wrp1ar_start;
	uInt32 wrp1ar_end;
	uInt32 wrp1br_start;
	uInt32 wrp1br_end;

	if ((optr & OPTR_OTHER_MASK) != OPTR_DEFAULT_HIGH)
	{
		SysStatusCode = E_ProtectFail;
		return FAIL;
	}
	if ((optr & PROTECT_RDP_MASK) == PROTECT_RDP_LEVEL2)
	{
		SysStatusCode = E_ProtectFail;
		return FAIL;
	}
	*wrp1ar_reg = wrp1ar & WRP_FIELDS_MASK;
	wrp1ar_start = *wrp1ar_reg & PROTECT_WRP_START_MASK;
	wrp1ar_end = (*wrp1ar_reg >> PROTECT_WRP_END_SHIFT) & PROTECT_WRP_START_MASK;

	if ((wrp1ar_start == PROTECT_WRP_DISABLED_START) && (wrp1ar_end == 0))
	{
	}
	else
	{
		if ((wrp1ar_start > wrp1ar_end) || (wrp1ar_end >= TOTAL_PAGE))
		{
			SysStatusCode = E_ProtectFail;
			return FAIL;
		}
	}

	*wrp1br_reg = wrp1br & WRP_FIELDS_MASK;
	wrp1br_start = *wrp1br_reg & PROTECT_WRP_START_MASK;
	wrp1br_end = (*wrp1br_reg >> PROTECT_WRP_END_SHIFT) & PROTECT_WRP_START_MASK;

	if ((wrp1br_start == PROTECT_WRP_DISABLED_START) && (wrp1br_end == 0))
	{
	}
	else
	{
		if ((wrp1br_start > wrp1br_end) || (wrp1br_end >= TOTAL_PAGE))
		{
			SysStatusCode = E_ProtectFail;
			return FAIL;
		}
	}

	return SUCCESS;
}
```

## Verification evidence

- `wrp1ar_pages`, `wrp1br_pages`, and variable shift expressions no longer
  occur in the source.
- `PROTECT_ALL_PAGE_MASK` remains defined at file scope, but is unused; it was
  explicitly out of scope for this task.
