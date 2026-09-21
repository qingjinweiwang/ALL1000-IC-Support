# Gang 错误掩码重复蜂鸣修复实施计划

> **目标：** 保留全部 SWD/IDCODE 检查，只让 `GangCheckIfAllSocketError()` 接收本次检查新产生的错误位，避免历史 `GangErrSckMask` 被重复当作新错误并再次触发长响。

## 修改文件

- `WJ_STM32G030F6P6.c`

## 实施步骤

### 1. 修改 `connecting()`

- 第一次 IDCODE 检查直接把 `SWD_Verify()` 的结果赋给 `sckStatus`。
- 第二次 CTRL/STAT 检查直接把 `SWD_Verify()` 的结果赋给 `sckStatus`。
- 删除两处用 `GangErrSckMask` 预装 `sckStatus` 的代码。
- 保留两次 `GangCheckIfAllSocketError()` 调用。

### 2. 修改 `ReadFun()`

- 直接把 `SWDIDcode_Verify(DPIDR)` 的结果赋给 `sckStatus`。
- 删除用 `GangErrSckMask` 预装 `sckStatus` 的代码。
- 保留 `GangCheckIfAllSocketError()` 调用。

### 3. 修改 `ProgramFun()`

- 直接把 `SWDIDcode_Verify(DPIDR)` 的结果赋给 `sckStatus`。
- 删除用 `GangErrSckMask` 预装 `sckStatus` 的代码。
- 保留 `GangCheckIfAllSocketError()` 调用。

### 4. 修改 `IDCheckFun()`

- 删除 `sckStatus |= GangErrSckMask`。
- 直接把 `SWDIDcode_Verify(g_DeviceID)` 的结果赋给 `sckStatus`。
- 保留 `GangCheckIfAllSocketError()` 调用。

### 5. 验证

- 搜索确认上述四个函数不再把 `GangErrSckMask` 混入本次检查结果。
- 确认所有原有 IDCODE、CTRL/STAT、Device ID 检查仍然存在。
- 使用项目现有 ARMCC 参数编译 `WJ_STM32G030F6P6.c`。
- 检查最终差异，确保没有修改蜂鸣器公共函数和其他业务逻辑。

## 预期行为

- 按键确认音仍为一次短响。
- 某插座在第一次检查中新失败时，公共函数仍可产生一次错误长响并记录到 `GangErrSckMask`。
- 后续检查若该插座没有产生新的错误，不会因为历史掩码再次长响。
- 后续检查发现新的插座错误时，仍会正常记录和提示。
