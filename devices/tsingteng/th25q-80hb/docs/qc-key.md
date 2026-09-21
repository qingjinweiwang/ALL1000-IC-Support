# qc-key: TH25Q-80HB

> 资料缓存与索引，不替代官方手册。高风险或有疑问的结论必须回原文确认。

- 芯片型号：TH25Q-80HB（TSINGTENG Microsystem / TMS）
- 原始文档：TH25Q-80HB_datasheet_V1.6_TMS_20220326.pdf，Rev V1.6 (2022-03-26)
- 配套笔记：TH25Q-80HB_programming_notes.md（同目录，内容更全，v1.0 按 Datasheet 逐项核对）
- 关键章节：FEATURES/Memory Organization(p4/p7)、5. DATA PROTECTION(p9-12，Table1.0/1.1 保护表）、6. STATUS REGISTER(p11-12)、7. COMMANDS(p13-46，Table2 命令表）、7.4 WRSR(p17)、7.14 PP(p25)、7.18-7.21 Erase(p28-30)、7.27 ID(p36)、7.32-7.34 Security Register(p39-41)、8.1 上电(p47)、8.6 AC 时序(p51-52)
- 建立日期：2026-08-27（QC 分析用）

## 容量与地址

- 8M-bit = 1M-Byte，主 Flash 地址 000000h-0FFFFFh
- Page 256B；Sector 4KB（命令 20h，另有 1KB 擦除 8Bh）；Block 32KB(52h)/64KB(D8h)
- 3 字节地址，无 4 字节模式
- 上位机虚拟地址（驱动 .h Block 表定义）：
  - Main Flash：00000000h-000FFFFFh ↔ 设备 000000h-0FFFFFh
  - Security Reg1：00100000h-001003FFh ↔ 设备 1000h-13FFh
  - Security Reg2：00100400h-001007FFh ↔ 设备 2000h-23FFh
  - Security Reg3：00100800h-00100BFFh ↔ 设备 3000h-33FFh

## ID

- JEDEC 9Fh → CDh-60h-14h（REMS 90h → CDh-13h；ABh → 13h）
- 上位机 Auto ID 必须配置为 CD-60-14，与工程一致

## Read / Program / Erase 关键规则

- Read 03h（55MHz max）/ Fast Read 0Bh；整片可连续读出
- Page Program 02h，1-256B，页内回卷；tPP typ 1.1ms / max 1.6ms
- Chip Erase C7h（60h 等价），tCE typ 5.2ms / max 7.8ms
- **CE 执行条件（7.21，p30）：仅 (BP2,BP1,BP0)=000 且 CMP=0，或 (BP2,BP1,BP0)=111 且 CMP=1 时执行；有 sector 被保护则 CE 被静默忽略（WIP 也会正常归零，不能凭 WIP 判成功）**
- Sector/Block Erase 时间 tSE/tBE max 7.6ms（驱动未使用，Erase 只走 CE）
- WREN 06h 置 WEL；PP/SE/BE/CE/WRSR 完成后 WEL 自动清 0

## Status Register（单一 16 位）

- 05h 读低字节 S7-S0：SRP0, BP4, BP3, BP2, BP1, BP0, WEL, WIP
- 35h 读高字节 S15-S8：SUS1, CMP, LB3, LB2, LB1, SUS2, QE, SRP1
- WRSR 01h 可写 1 或 2 字节；**CS# 在第 8 位后拉高 = 只写低字节且 CMP/QE 被清 0（7.4，p17）；设 CMP 必须一次写满 16 位**
- 01h 对 S15/S10/S1/S0 无效；tW typ 2.6ms / max 4ms

## 保护（Table1.0/1.1，p9-10）

- BP4-BP0（S6-S2，非易失）；无独立 TB/SEC 位（BP4=1 组合提供 Top/Bottom 4KB-32KB 小块保护）
- CMP（S14）取反保护范围；SRP0/SRP1（S7/S8），(1,1)=OTP 永久锁定 SR，禁止使用
- 出厂 SR=0000h 无保护
- QC 用到的组合（CMP=0）：
  - BP 全 0 → 无保护
  - 仅 BP0=1 → Upper 1/16 = 0F0000h-0FFFFFh（64KB）
  - BP2=1,BP1=1（XX11X）→ 全片 ALL
  - CMP=1 且 BP2-BP0=111 → 无保护（CE 可执行）
  - CMP=1 且 BP 全 0 → 全片 ALL（CE 被忽略）

## Security Register / OTP

- 3 个独立区域，各 1024B；设备地址 1000h/2000h/3000h 起（A15-A12=0001/0010/0011）
- Read 48h（3 字节地址+1 dummy）/ Program 42h（按 256B 页）/ Erase 44h（整区，tSE）
- **LB1-LB3（S11-S13）为 OTP 位：置 1 后对应区域永久只读，Program/Erase 被芯片静默忽略，不可恢复**
- 本驱动第一阶段有意关闭 LB/Secure（.c SecureFun return SUCCESS + 算法表 UselessAlgoFun + .spc 无 LB 选项，三处一致）

## 需掉电/Reset 生效项

- 无硬件复位脚；软件 Reset 66h+99h（tRST=30us，WRSR 后 4ms，CE 后 120us）
- 上电 tVSL max 500us；平台 PowerOn 已延时 50ms，足够
- BP/CMP/LB 等非易失位写入后无需掉电，WRSR 完成即生效

## 与 QC 直接相关的限制

- 驱动 Erase 不解保护：保护态下 Erase 直接报 E_EraseFail（预检 SR），必须先 UnProtect
- 驱动 Program 不做保护预检：保护区内 Program 被芯片静默忽略，Program 可能报 Pass，必须以 Verify 判据为准
- SPC 状态位块的 Program 是空操作；BP/CMP 只通过 Protect 任务生效
- SR Verify/Blank Check 由 SPC「Enable Status Register Verify/Blank Check」(E80) 开关使能
- 安全区 Program/Erase 前有 LB 预检：已锁区域报 FAIL 而非假 PASS
- Read 仅 A 槽（平台框架），Verify/Blank 用 FPGA 多槽比对
- Gang 模式下 SR 读回值取自 A 槽，默认各槽出厂状态一致
