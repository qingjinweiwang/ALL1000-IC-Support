# VerifyOptionRegistersAll 可读性重构设计

## 目标

提高 `VerifyOptionRegistersAll()` 的人为可读性，同时保持现有功能和接口完全不变。

## 范围

仅修改 `WJ_STM32G030F6P6.c` 中 `VerifyOptionRegistersAll()` 的函数体。

不修改函数签名、调用方、`CheckFlashStatusAll()`、寄存器常量、掩码、错误码或其他文件。

## 设计

将当前包含三个 `CheckFlashStatusAll()` 调用的复合 `if` 条件拆成三个连续的失败即返回检查：

1. 使用 `ALL_BITS_32` 完整比对 `FLASH_OPTR`。
2. 使用 `WRP_FIELDS_MASK` 比对 `FLASH_WRP1AR` 的有效字段。
3. 使用 `WRP_FIELDS_MASK` 比对 `FLASH_WRP1BR` 的有效字段。

每个检查前添加一条简短注释，说明正在验证的寄存器和比对范围。全部检查成功后返回 `SUCCESS`。

## 行为保持

重构必须保持：

- `OPTR`、`WRP1AR`、`WRP1BR` 的检查顺序不变。
- 任一检查失败后立即返回 `FAIL`，不执行后续检查。
- 每个调用使用的期望值、掩码和 `E_ProtectFail` 错误码不变。
- 全部检查成功时返回 `SUCCESS`。

## 验证

- 检查 Git diff，确认仅目标函数发生预期变化。
- 检查三个 `CheckFlashStatusAll()` 调用的参数与原实现逐项一致。
- 若项目现有构建入口可用，运行构建并确认没有引入编译错误。
