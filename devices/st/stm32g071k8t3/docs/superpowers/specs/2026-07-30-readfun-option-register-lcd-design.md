# ReadFun 选项寄存器 LCD 显示设计

## 目标

在 `ReadFun()` 中读取 STM32G030 的三个 Flash 选项寄存器，并将其当前值以固定 8 位大写十六进制显示在 LCD 上：

```text
OPTR:   DFFFE1AA
WRP1AR: 001F001F
WRP1BR: 001F001F
```

## 修改范围

只修改 `WJ_STM32G030F6P6.c`：

- 增加一个 32 位数转固定 8 位十六进制字符串的小函数。
- 在 `ReadFun()` 的 IDCODE 检查成功后读取并显示三个寄存器。
- 不修改 Flash 数据读取循环、错误处理、插座筛选或其他功能。

## LCD 布局

LCD 每行最多 21 个字符。本功能使用第 5、6、7 行，起始列为 0：

| LCD 行 | 标签 | 数值起始列 | 完整格式 |
|---|---|---:|---|
| 5 | `OPTR:   ` | 8 | `OPTR:   XXXXXXXX` |
| 6 | `WRP1AR: ` | 8 | `WRP1AR: XXXXXXXX` |
| 7 | `WRP1BR: ` | 8 | `WRP1BR: XXXXXXXX` |

标签和数值分别通过 `Put_String()` 输出。每个标签固定占 8 列，数值固定占 8 列。

## 数据流程

`ReadFun()` 完成当前的 DPIDR 检查后，依次执行：

1. `swd_readAPResource(FLASH_OPTR)`
2. 将返回值转换成 8 位大写十六进制字符串
3. `Put_String(5, 0, "OPTR:   ")`
4. `Put_String(5, 8, hexString)`
5. 对 `FLASH_WRP1AR` 和 `FLASH_WRP1BR` 重复同样流程，分别输出到第 6、7 行
6. 继续执行原来的 Flash 数据读取流程

现有 `swd_readAPResource()` 返回公共的 `SWD_Data`，因此 LCD 显示的是当前公共 SWD 读取通道返回的值，不新增逐插座独立显示。

## 十六进制转换

新增内部辅助函数，使用固定字符表：

```c
"0123456789ABCDEF"
```

函数始终生成：

- 8 个十六进制字符
- 高位在前
- 大写字母
- 保留前导零
- 末尾添加字符串结束符

不使用 `sprintf()`，避免为旧 ARMCC 工程引入格式化库及额外代码体积。

## 错误处理

保持现有行为：

- DPIDR 检查失败时立即返回，不读取或刷新寄存器显示。
- 寄存器 LCD 显示仅用于观察，不新增寄存器值合法性判断。
- 不改变 `ReadFun()` 现有返回值和 `SysStatusCode` 逻辑。

## 验证

- 检查三个寄存器地址和 LCD 行号是否正确。
- 检查字符串长度不超过每行 21 字符。
- 检查转换结果包含前导零且为大写。
- 使用现有 ARMCC 参数编译 `WJ_STM32G030F6P6.c`。
- 在设备上执行 Read，确认 LCD 第 5～7 行显示的值与实际寄存器一致。
