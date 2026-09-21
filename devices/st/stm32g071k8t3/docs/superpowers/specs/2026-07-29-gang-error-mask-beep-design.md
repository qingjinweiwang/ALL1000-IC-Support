# Gang 错误掩码重复蜂鸣修复设计

## 目标

避免调用者把已经记录在 `GangErrSckMask` 中的旧插座错误再次传给 `GangCheckIfAllSocketError()`，从而消除 ID Check、Read 和 Program 后续检查中的重复长响，同时保留新错误处理和全插座失败判断。

## 修改范围

只修改 `WJ_STM32G030F6P6.c` 中以下函数：

- `connecting()`
- `ReadFun()`
- `ProgramFun()`
- `IDCheckFun()`

不修改预编译公共函数 `GangCheckIfAllSocketError()`，不删除任何 IDCODE、CTRL/STAT 或 Device ID 检查。

## 数据流

调用者只把本次硬件检查返回的错误位传给 `GangCheckIfAllSocketError()`：

```c
sckStatus = SWD_Verify(...);
```

或者：

```c
sckStatus = SWDIDcode_Verify(...);
```

不再先执行：

```c
sckStatus = GangErrSckMask;
```

也不再执行：

```c
sckStatus |= GangErrSckMask;
```

`GangCheckIfAllSocketError()` 内部继续负责把本次新错误累加到 `GangErrSckMask`、关闭失败插座、发出一次新错误长响，并根据累计掩码判断是否所有插座均失败。

## 预期行为

- 没有新错误：不产生长响。
- 部分插座首次失败：只为本次新失败产生一次长响，其他插座继续工作。
- 已记录的失败插座：后续检查不重复产生长响。
- 所有插座累计失败：调用者仍返回对应的 `FAIL` 和错误码。

## 验证

- ARMCC 编译通过。
- 搜索四个目标函数，确认不存在把 `GangErrSckMask` 赋给或并入 `sckStatus` 的代码。
- 所有 `GangCheckIfAllSocketError()`、IDCODE、CTRL/STAT 和 Device ID 检查仍然存在。
- 不改动用户当前其他本地修改。
