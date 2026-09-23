# N25Q256A EraseFun 假成功修正设计

## 目标

修正 `EraseFun()` 在任一启用座擦除失败时仍可能返回 `SUCCESS` 的问题。修正后，只要任一启用座出现保护状态、WEL 设置失败、擦除超时或 Flag Status Register 擦除错误，整个 `EraseFun()` 都返回 `FAIL`。

## 修改范围

- 只修改 `WAx4_N25Q256Ax3ESFxxx.c` 中现有的 `EraseFun()`。
- 不修改原有 SPI 寄存器操作框架和命令时序。
- 不新增函数，不修改 `WriteStatusWELBit()` 等共用函数。
- 不在 `EraseFun()` 内增加完整主区 Blank Check；擦除后的逐地址空白检查仍由 `BlankCheckFun()` 负责。

## 设计

保留现有 `50h -> 06h -> C7h -> 05h WIP轮询 -> 70h FSR检查` 流程。

1. 进入首次擦除处理时，保存当时的 `GangErrSckMask`。
2. 将擦除前的保护检查由单一返回通道的 `ReadStatusData()` 改为现有 gang 检查路径：使用 `PrtRegMaskBit` 屏蔽 SR 的 BP3、TB、BP2-BP0，期望所有启用座均为零。任一座不为零时记录故障座并返回 `E_EraseFail`。
3. 继续调用现有 `WriteStatusWELBit()`。如果该函数返回失败，或调用后 `GangErrSckMask` 相比进入时发生变化，说明至少一个启用座未成功设置 WEL，立即返回 `E_EraseFail`。
4. 原样发送 Bulk Erase `C7h`，并按原周期轮询 Status Register 的 WIP 位。
5. WIP 轮询超时时，只要 `StatusData` 非零，就调用现有 gang 错误处理记录对应座，然后无条件返回 `E_EraseFail`，不再只在全部座失败时返回失败。
6. 擦除结束后检查 FSR 的 `0x22`，即 erase failure和protection failure。只要任一启用座命中，记录对应座并无条件返回 `E_EraseFail`。
7. 只有所有上述检查通过时才返回 `SUCCESS`。

## 正确性依据

Micron N25Q256A要求 Bulk Erase 前必须先设置 WEL；任一受保护区域会使 Bulk Erase 不执行，并在 FSR 中设置擦除/保护错误。器件执行期间 SR.WIP 为1，结束后清零。因此WEL、WIP超时和FSR均必须按“任一启用座失败即整体失败”传播。

## 验证

- 编译驱动，确认零编译错误并且没有新增警告。
- 静态检查 `EraseFun()` 的每个异常分支均设置 `SysStatusCode = E_EraseFail` 并返回 `FAIL`。
- 确认 Bulk Erase 命令仍为 `C7h`，且 SPI 命令顺序、CS控制和延时保持不变。
- 实机分别验证：正常未保护芯片擦除通过；任一座受保护、WEL失败、WIP超时或FSR报错时Erase显示失败；正常擦除后 `BlankCheckFun()` 通过。
