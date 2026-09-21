# N25Q256A 专用4字节指令改造设计

## 目标

将N25Q256A主区访问从“EAR分区＋标准命令＋3字节地址”改为器件原生专用4字节指令。参考 `SYx4_PY25Q01GLC-SMxxx.c` 的四座FPGA SPI流程，但只采用N25Q256A规格书明确支持的命令和状态逻辑。

## 修改范围

- 只修改 `WAx4_N25Q256Ax3ESFxxx.c`。
- 不修改FPGA SPI框架、gang socket机制、CS控制方式或基础延时接口。
- 不新增辅助函数；把现有 `Send3ByteAddress()`改造为 `Send4ByteAddress()`。
- 不复制PY25Q的状态寄存器、安全寄存器或保护实现。

## 主区命令

- READ：`13h`
- FAST READ：`0Ch`
- PAGE PROGRAM：`12h`
- 4KB SUBSECTOR ERASE：`21h`（保留命令定义）
- 64KB SECTOR ERASE：`DCh`（保留命令定义）
- BULK ERASE：`C7h`（保持不变，无地址字段）

这些专用4字节指令始终携带A31-A24、A23-A16、A15-A8、A7-A0，不依赖EAR，也不要求用 `B7h/E9h`切换全局地址模式。

## 控制流

1. `PowerOnFun()`仅完成上电和参数初始化，不读取FSR地址模式，也不设置EAR。
2. Read、Program、Verify和Blank Check删除 `SelectAddressBank()`调用，直接发送专用4字节命令和完整4字节地址。
3. 删除16MB边界处关闭CS、切换EAR并重新发命令的分支；专用4字节READ可连续跨越该边界。
4. SN主区读、写、回读验证分别使用 `0Ch`、`12h`、`0Ch`及4字节地址。
5. Bulk Erase继续使用 `C7h`，保留任一启用座失败即返回 `FAIL` 的现有修正。
6. OTP继续使用 `4Bh/42h`及原有OTP地址序列，不使用主区4字节辅助函数。

## 删除内容

- `EARBankState`
- `WriteExtAddrRegCmd`、`ReadExtAddrRegCmd`、`ExtAddrBankMask`
- `FlagStatusAddrModeMask`
- `CheckAllExtendedAddressData()`、`SetExtendedAddress()`和 `SelectAddressBank()`的声明与定义
- 所有EAR选择调用和16MB边界切换代码

## 验证

- 静态搜索确认主区不再使用 `03h/0Bh/02h`或三字节地址。
- 静态搜索确认不再存在EAR变量、命令和函数。
- 确认OTP、状态寄存器、保护和Bulk Erase流程未被误改。
- 使用原 `iBuild.bat`完成编译、链接和BIN转换，要求零错误且无新增警告。
