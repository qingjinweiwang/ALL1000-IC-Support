# ReadFun 回填 Option Bytes 到 .spc 设计

## 目标

当系统通过 `ReadFun()` 读取 `.spc` 特殊区块时，从 STM32G030F6P6
读取以下三个寄存器，并更新 `.spc` 对应输入框的数据缓存：

- `FLASH_OPTR`
- `FLASH_WRP1AR`
- `FLASH_WRP1BR`

多工位模式下，每个寄存器都必须检查所有有效工位的值是否一致。
发现不一致时，读取任务失败并设置 `SysStatusCode = E_ReadAHBFail`。

## 范围

修改范围限于 `WJ_STM32G030F6P6.c` 中的 `ReadFun()` 及其文件内辅助函数。

不修改：

- `.spc` 字段定义和偏移
- 普通 Flash 数据读取流程
- Option Bytes 的写入、保护或解保护流程
- 任务表和上位机通信协议
- LCD 显示逻辑

## 触发条件

仅当 `ReadFun()` 检测到 `IfSpecialBitBlk()` 为真时执行 Option Bytes
读取和 `.spc` 缓存更新。

普通 Flash 区块继续沿用现有读取流程，不重复读取或更新 Option Bytes。

## 字段映射与字节序

读取值按 `.spc` 当前采用的显示顺序写入 `SpecialBitBuf`，最高有效字节在前：

| 寄存器 | SpecialBitBuf 偏移 | 字节布局 |
| --- | --- | --- |
| OPTR | `0x00`～`0x03` | `[31:24]`、`[23:16]`、`[15:8]`、`[7:0]` |
| WRP1AR | `0x08`～`0x0B` | `[31:24]`、`[23:16]`、`[15:8]`、`[7:0]` |
| WRP1BR | `0x0C`～`0x0F` | `[31:24]`、`[23:16]`、`[15:8]`、`[7:0]` |

该布局与现有 `GetSpecialBitDWord()` 的解码方式互为逆操作。

## 结构

新增文件内辅助函数，将一个 32 位值写入指定的 `SpecialBitBuf` 偏移。
辅助函数只负责字节拆分，不负责 SWD 访问或错误处理。

`ReadFun()` 的特殊区块分支按以下顺序处理：

1. 读取 OPTR 到局部变量。
2. 立即将 OPTR 写入 `SpecialBitBuf(0x00..0x03)`。
3. 比较所有有效工位的 OPTR。
4. 读取 WRP1AR 到局部变量。
5. 立即将 WRP1AR 写入 `SpecialBitBuf(0x08..0x0B)`。
6. 比较所有有效工位的 WRP1AR。
7. 读取 WRP1BR 到局部变量。
8. 立即将 WRP1BR 写入 `SpecialBitBuf(0x0C..0x0F)`。
9. 比较所有有效工位的 WRP1BR。
10. 三项均一致时返回 `SUCCESS`。

每次比较使用完整的 `0xFFFFFFFF` 掩码，并沿用现有 FPGA 多工位比较接口。

## 多工位一致性与错误处理

单次寄存器读取所得的局部值作为该寄存器的比较基准。所有当前有效工位
必须与基准值完全一致。

任一寄存器比较失败时：

1. 设置 `SysStatusCode = E_ReadAHBFail`。
2. `ReadFun()` 立即返回 `FAIL`。
3. 不回滚已经写入 `SpecialBitBuf` 的字段。
4. 尚未处理的字段保持原值。

因此允许出现部分更新。例如 OPTR 一致并已更新，随后 WRP1AR 不一致，
则 OPTR 和当前读取到的 WRP1AR 保留新值，WRP1BR 保持旧值。

该行为是有意设计，满足“读取后立即更新输入框数据”的要求。

## 上位机数据流

`SpecialBitBuf` 是 `.spc` 特殊字段的固件侧数据缓存。读取任务完成后，
现有 `VR_SpecialBit`/`UploadSpecialBit` 通路负责将缓存上传到上位机，
无需新增 USB 命令或界面协议。

## 验证

实现后执行以下检查：

1. 编译驱动，确认没有新增编译错误或警告。
2. 检查 OPTR、WRP1AR、WRP1BR 的偏移和大端字节拆分。
3. 确认特殊区块成功时三个字段均写入 `SpecialBitBuf`。
4. 确认任一寄存器多工位不一致时返回 `FAIL` 并设置
   `E_ReadAHBFail`。
5. 确认错误发生前已处理的字段不回滚，尚未处理的字段不被修改。
6. 确认普通 Flash 读取路径和返回行为保持不变。
7. 确认未重新加入 LCD 输出或显示延时。
