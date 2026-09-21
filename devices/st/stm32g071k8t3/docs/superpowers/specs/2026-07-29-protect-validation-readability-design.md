# Protect 配置校验可读性重构设计

## 目标

提高 `WJ_STM32G030F6P6.c` 中保护配置校验代码的可读性，使维护人员不需要展开带参数宏即可理解校验顺序，同时保持现有防锁死行为不变。

## 修改范围

- 删除五个带参数的 `PROTECT_*` 判断或计算宏。
- 删除整个 `PROTECT_CONFIG_SELF_TEST` 条件编译区。
- 删除仅用于拆分 WRP 校验的 `BuildWrpPageMask()`。
- 将 OPTR、RDP、WRP1AR、WRP1BR 和全片保护校验集中写入 `ValidateProtectConfig()`。
- 保留寄存器地址、固定掩码、固定值等常量宏。

## 校验顺序

`ValidateProtectConfig()` 按以下顺序执行，每项使用独立的局部变量和 `if`：

1. 检查 OPTR 保留位是否保持规定值。
2. 拒绝 RDP 字节 `0xCC`。
3. 使用 `WRP_FIELDS_MASK` 标准化 WRP1AR。
4. 提取 WRP1AR 的 START 和 END；仅接受 `START=31, END=0` 的禁用形式，或页号为 0～15 且 START 不大于 END 的有效范围。
5. 对 WRP1BR 执行相同处理。
6. 仅在范围确认有效后计算两组页覆盖掩码，避免非法移位。
7. 合并两组页掩码；若覆盖全部 16 页，则返回 `E_ProtectFail`。

## 行为约束

- `RDP=0xAA` 仍按 Level 0 处理。
- `RDP=0xCC` 仍永久禁止。
- 其他 RDP 值仍按 Level 1 处理。
- 单区域或双区域组合覆盖全部 16 页时仍返回 `E_ProtectFail`。
- WRP 中 START/END 之外的位仍被标准化丢弃，不直接写入寄存器。
- `ProtectFun()` 的解锁、写入、状态检查、回读和 OBL_LAUNCH 流程不变。

## 验证

- 检查源码中不再存在带参数的 `PROTECT_*` 宏、`BuildWrpPageMask()` 或 `PROTECT_CONFIG_SELF_TEST`。
- 使用现有 ARMCC 配置编译普通版本。
- 检查 Git 差异，确认未夹带用户现有的 `.c`、`.spc` 和 `.claude/settings.json` 修改。
