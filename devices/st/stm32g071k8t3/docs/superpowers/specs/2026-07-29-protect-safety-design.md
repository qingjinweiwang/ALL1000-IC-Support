# STM32G030F6P6 Protect 防呆设计

## 目标

在 `ProtectFun()` 写入 STM32G030F6P6 Option Bytes 前验证 `.spc` 输入，阻止不可逆的 RDP Level 2、非法 OPTR 保留位、越界或非标准 WRP 配置，以及覆盖全部 32 KB Flash 的写保护配置。

本设计依据 ST RM0454（STM32G0x0 Reference Manual）：

- RDP `0xAA` 为 Level 0。
- RDP `0xCC` 为不可逆 Level 2。
- 其他 RDP 值为 Level 1。
- STM32G030F6P6 有 16 个 2 KB Flash 页。
- WRP 区域由包含端点的 START/END 页号定义。

参考：https://www.st.com/resource/en/reference_manual/rm0454-stm32g0b0-advanced-armbased-32bit-mcus-stmicroelectronics.pdf

## 范围

修改：

- `WJ_STM32G030F6P6.c`
- 必要时更新 `WJ_STM32G030F6P6.spc` 的说明文字，但不增加新的确认字段。

不修改：

- 外部烧录框架接口。
- `ProtectFun()` 的函数签名。
- 四工位失败剔除策略。
- Program、Erase、Verify 等其他任务的功能。

## OPTR 输入规则

`.spc` 提供完整 32 位 OPTR。驱动不得将任意 32 位值直接传给 `FLASH_OPTR`。

STM32G030F6P6 允许变化的字段为：

- RDP `[7:0]`
- nRST_STOP bit 13
- nRST_STDBY bit 14
- IWDG_SW bit 16
- IWDG_STOP bit 17
- IWDG_STDBY bit 18
- WWDG_SW bit 19
- RAM_PARITY_CHECK bit 22
- nBOOT_SEL bit 24
- nBOOT1 bit 25
- nBOOT0 bit 26

对应可写白名单为 `0x074F60FFUL`。其余位使用保留位掩码 `0xF8B09F00UL`。

ST 默认 OPTR 为 `0xDFFFE1AAUL`，所以允许的保留位图样为：

```c
0xDFFFE1AAUL & 0xF8B09F00UL = 0xD8B08100UL
```

校验条件：

```c
(optr & 0xF8B09F00UL) == 0xD8B08100UL
```

任何保留位不一致都返回 `E_ProtectFail`。驱动不得通过掩码静默修正错误输入。

## RDP 规则

从 `optr & 0xFF` 提取 RDP：

- `0xAA`：允许，按 Level 0 处理。
- `0xCC`：永久禁止，设置 `SysStatusCode = E_ProtectFail` 并返回 `FAIL`。
- 其他值：允许，按 Level 1 处理。

全部 RDP 校验必须发生在上电、Flash 解锁和寄存器写入之前。

## WRP 标准化和范围规则

`WRP1AR` 和 `WRP1BR` 只使用：

- START `[4:0]`
- END `[20:16]`

输入先通过 `WRP_FIELDS_MASK` 标准化。`.spc` 当前默认值在保留位中包含 `0xFF`，因此 WRP 保留位不作为输入错误；写硬件寄存器时只使用标准化后的有效字段。

每个 WRP 区域只允许以下两种形式：

1. 启用：`START <= END`，并且 START、END 都在 `0～15`。
2. 禁用：只能是标准形式 `START=0x1F, END=0`。

以下情况必须拒绝：

- START 或 END 越过第 15 页。
- 非标准的 `START > END` 组合。
- 单个区域覆盖页 `0～15`。
- A/B 两区域合并后覆盖页 `0～15`。

A/B 区域重叠但没有覆盖全部 16 页时允许。

实现时为每个启用区域建立 16 位页覆盖图，A/B 覆盖图按位或。最终值等于 `0xFFFF` 时返回 `E_ProtectFail`。

## 代码结构

新增静态校验助手，建议接口：

```c
static uChar ValidateProtectConfig(
    uInt32 optr,
    uInt32 wrp1ar,
    uInt32 wrp1br,
    uInt32 *wrp1ar_reg,
    uInt32 *wrp1br_reg);
```

职责：

- 校验 OPTR 保留位。
- 拒绝 RDP Level 2。
- 标准化 WRP 字段。
- 校验两个 WRP 区域。
- 检查合并后的全片覆盖。
- 在任何失败路径设置 `SysStatusCode = E_ProtectFail`。

允许增加一个更小的静态 WRP 解码助手，但不得改变外部接口或引入动态内存。

## ProtectFun 执行顺序

1. 从 `SpecialBitBuf` 读取 OPTR、WRP1AR 和 WRP1BR。
2. 调用 `ValidateProtectConfig()`。
3. 校验失败时立即返回；不得执行下电、上电、解锁或寄存器写入。
4. 按现有流程重新上电并连接 SWD。
5. 等待 Flash/Option Byte 控制器空闲。
6. 解锁主 Flash 和 Option Bytes。
7. 清除旧状态位。
8. 写入 OPTR 和标准化后的 WRP1AR/WRP1BR。
9. 在 `OPTSTRT` 前第一次回读三个寄存器；任一活动工位不匹配时按现有四工位策略剔除，全部失败则返回 `FAIL`。
10. 执行 `OPTSTRT`。
11. 等待 BSY/CFGBSY 清零并检查全部 Option Byte 错误位。
12. 在 `OBL_LAUNCH` 前第二次回读三个寄存器；失败处理与第一次相同。
13. 清除 EOP。
14. 执行 `OBL_LAUNCH` 并等待复位。
15. 下电。
16. RDP Level 0：重新上电连接并最终回读三个寄存器，匹配后返回 `SUCCESS`。
17. RDP Level 1：SWD 预期不可回连；只有两次加载前回读和编程状态检查全部成功后才返回 `SUCCESS`。

## 多工位行为

继续使用 `CheckFlashStatusAll()`、`WaitFlashStatusAll()` 和 `SelectActiveSockets()`：

- 单个工位失败时将其从后续 SWD 操作中剔除。
- 只要仍有有效工位，允许其余工位继续。
- 所有工位失败时设置 `E_ProtectFail` 并返回 `FAIL`。

输入校验针对公共 `.spc` 数据，失败时所有工位都不得开始 Option Byte 操作。

## 验证

至少覆盖以下静态或主机侧校验案例：

- 默认 OPTR `0xDFFFE1AA`、两个禁用 WRP：通过。
- RDP `0xCC`：失败，且未执行任何 SWD 写入。
- RDP `0xAA`：通过输入校验。
- 非 `0xAA/0xCC` 的多个 RDP 值：作为 Level 1 通过。
- OPTR 每一组保留位被翻转：失败。
- WRP 单页 `0～0`、末页 `15～15`：通过。
- WRP 越界到页 16：失败。
- 非标准禁用组合：失败。
- 标准禁用 `31～0`：通过。
- 单一区域 `0～15`：失败。
- A/B 分段后合并覆盖 `0～15`：失败。
- A/B 重叠但未覆盖全片：通过。
- `OPTSTRT` 前第一次回读失败：不得执行 `OPTSTRT`。
- `OPTSTRT` 后第二次回读失败：不得执行 `OBL_LAUNCH`。
- Level 0 最终回读失败：返回 `FAIL`。

最终必须通过 ARM RVCT 2.2 编译，并确认 Protect 寄存器写入和控制顺序只发生上述设计要求的变化。
