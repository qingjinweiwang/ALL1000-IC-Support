# N25Q256A 地址模式检查移至 IDCheck 设计

## 目标

使 `PowerOnFun()` 与参考驱动保持简洁，不在上电函数内读取或判断器件地址模式。N25Q256A的3字节模式检查和EAR初始值确认只在用户勾选ID Check时执行一次。

## 修改范围

- 只修改 `WAx4_N25Q256Ax3ESFxxx.c` 中的 `PowerOnFun()` 和 `IDCheckFun()`。
- 不修改SPI底层框架、现有命令实现或CS时序。
- 不新增函数。
- `SelectAddressBank()` 保持现有按需切换EAR的逻辑。

## 设计

### PowerOnFun

- 删除局部变量 `StatusData`。
- 删除FSR bit0地址模式检查及相关失败返回。
- 删除上电阶段的 `SetExtendedAddress(0x00)` 及相关失败返回。
- 保留 `EARBankState = 0xFF`，只让软件EAR缓存进入“未知”状态。

### IDCheckFun

在现有JEDEC ID检查全部通过后：

1. 用现有 `CheckAllFlagStatusData(0x00)` 和 `FlagStatusAddrModeMask` 检查FSR bit0。
2. 地址模式不是3字节时，调用现有gang错误处理，设置 `E_AutoIDFail` 并返回 `FAIL`。
3. 调用现有 `SetExtendedAddress(0x00)`，把所有当前有效座的EAR初始化为下半区并回读确认。
4. EAR设置失败时设置 `E_AutoIDFail` 并返回 `FAIL`。
5. 全部通过后返回 `SUCCESS`。

### 未勾选ID Check

- 不执行FSR地址模式检查，默认器件上电处于3字节地址模式。
- 因 `EARBankState` 在PowerOn中被置为 `0xFF`，首次主区访问调用现有 `SelectAddressBank()` 时仍会写入实际地址所需的EAR bank。
- 若器件NVCR曾被配置成4字节上电模式且未勾选ID Check，主区操作可能失败；这是本设计明确接受的使用约束。

## 验证

- 确认 `PowerOnFun()` 内没有地址模式读取、EAR写入或条件失败判断。
- 确认 `IDCheckFun()` 只在JEDEC ID通过后执行一次FSR bit0和EAR=0检查。
- 确认 `SelectAddressBank()` 未修改。
- 使用原工程编译，要求零错误且无新增警告。
