# TH25Q-80HB 芯片烧录 QC 分析报告

## 0. 分析范围

- 工程路径：`Driver_G/SYx4_TH25Q-80HB/`
- 目标芯片：TH25Q-80HB（TSINGTENG Microsystem，8M-bit SPI NOR Flash，SOP8）
- 烧录平台：ALL-1000G（Gang4，A/B/C/D 四槽）
- 编程接口：Standard SPI（3 字节地址，模式 0）
- 当前正式 C：`SYx4_TH25Q-80HB-MSDx.c`（DevDRV_Version 1.0，2026-08-12）
- 当前正式 H：`SYx4_TH25Q-80HB-MSDx.h`
- 当前正式 SPC：`SYx4_TH25Q-80HB-MSDx.spc`
- 资料：`docs/TH25Q-80HB_datasheet_V1.6_TMS_20220326.pdf`、`docs/TH25Q-80HB_programming_notes.md`（v1.0，已按 Datasheet 逐项核对）、`docs/qc-key.md`（本次生成）
- Skill：chip-qc-analyzer / Driver_QC_SKILL_v2 + spi-nor-flash-driver-dev v0.2

## 1. 芯片特性与 AUTO 流程

- 存储类型：SPI NOR Flash，1MB 主 Flash（000000h-0FFFFFh）+ 3×1KB Security Register
- Erase 是否需要：需要。驱动 Erase 首次进入执行 Chip Erase（C7h，tCE max 7.8ms），并带保护预检
- Blank 是否支持：支持（主 Flash FPGA 比对 FF；状态寄存器 Blank 受 E80 开关）
- Verify 是否支持：支持（主 Flash/安全区 FPGA 多槽比对；状态寄存器 Verify 受 E80 开关）
- 选择的 AUTO 流程：IDCHECK → Erase → Blank → Program → Verify（可挂 CheckSum）
- 选择依据：NOR Flash 烧录标准流程；Read 仅 A 槽，不放入多槽 AUTO

## 2. 平台限制

- Read 槽位：仅 A 槽（驱动 ReadFun 实际读数选 A 槽，读完恢复全部有效槽）
- 多槽 AUTO 是否包含 Read：否
- Verify 是否反馈错误地址：否，只报 Pass/NG 和槽位
- 单字节 Verify NG 预期：Verify 报 NG，恢复后 Pass，不要求核对地址
- Gang 模式 SR 读回取自 A 槽：Protect/UnProtect 的 RMW 读回值来自 A 槽，默认各槽出厂状态一致（驱动注释明确记录）

## 3. 芯片功能 × Datasheet × 驱动 × SPC 对应表

| 芯片功能 | Datasheet | C/H 实现 | SPC 开放 | 是否 QC |
|---|---|---|---|---|
| JEDEC ID 9Fh（CD-60-14） | ✓ | IDCheckFun，比对上位机下发 ID | 上位机 Auto ID 配置 | ✓ T01/T06 |
| 主 Flash Read 03h | ✓ | ReadFun（仅 A 槽） | — | ✓ T02 |
| 主 Flash Program 02h（跳全 FF 页） | ✓ | ProgramFun | — | ✓ T01 |
| 主 Flash Verify（FPGA 多槽） | ✓ | VerifyFun | — | ✓ T01/T04 |
| Chip Erase C7h + 保护预检 | ✓ | EraseFun（首次进入） | — | ✓ T01/T08/T09 |
| Blank Check 主 Flash | ✓ | BlankCheckFun | — | ✓ T01/T03 |
| CheckSum 主 Flash | ✓ | CheckSumFun（SysStdChecksum） | — | ✓（挂 T01） |
| Sector/Block Erase 20h/8Bh/52h/D8h | ✓ | 未使用（Erase 只走 CE） | — | 否（驱动未用） |
| BP4-BP0 + CMP 保护 | ✓ | ProtectFun：RMW + 01h 一次写 16 位 + 读回校验 | S0000 BP0-BP4 / S0001 CMP | ✓ T07-T09 |
| UnProtect | ✓ | UnProtectFun（目标 BP=0、CMP=0） | — | ✓ T08/T09 |
| 状态寄存器 Verify/Blank | ✓ | VerifyFun/BlankCheckFun 特殊位分支（mask 0x7C/0x40） | S0003 E80 | ✓ T07-T09 |
| Security Reg 1-3 Program/Erase/Read/Verify | ✓ | 42h/44h/48h，逐区独立，LB 预检 | S0003 E01/E02/E04/E08/E10 | ✓ T10 |
| LB1-LB3 OTP 永久锁 | ✓ | **有意关闭**：SecureFun 直接 return SUCCESS，算法表注册 UselessAlgoFun | 未开放 | 否（仅防呆 T11） |
| SRP0/SRP1 | ✓ | 不开放（ProtectRegisterTable mask 0x407C 不含） | 未开放 | 否（保证保护可解除） |
| QE | ✓ | RMW 保留，不开放 | 未开放 | 否（平台标准 SPI） |
| UID 4Bh / SFDP 5Ah | ✓ | 未实现 | — | 否 |
| Dual/Quad 读写 | ✓ | 未使用 | — | 否 |
| DPD B9h / Reset 66h+99h | ✓ | 未使用 | — | 否 |
| SN 烧录（平台 SN 功能） | 平台功能 | DevSNProgAndVerify（0Bh 读页合并 + 02h 写回 + 读回校验） | 上位机 SN 配置 | 可选 T12 |

SPC 未开放但 C 已实现的项目：无（C 中实现的功能均有对应控制路径；安全区各操作由 S0003 开关门控）。
SPC 有选项但 C 无有效实现的项目：无（S0003 六个开关在 .c 中均有对应逻辑）。
模板遗留未真正实现：`SecureFun` 主体为死代码（首行 return SUCCESS），与算法表 UselessAlgoFun、SPC 无 LB 选项三处一致，属有意关闭，非矛盾。

## 4. 驱动关键行为（QC 判据来源）

1. **Erase 不解保护**：保护态下 Erase 在预检阶段直接报 E_EraseFail，CE 根本不会发出——保护态 Erase NG 是预期行为，且证明 CE 未被静默忽略造成假 Pass。
2. **Program 不做保护预检**：保护区内 Page Program 被芯片静默忽略，WIP 正常归零，Program 可能报 Pass——保护行为必须以 Verify 结果判定。
3. **状态位块 Program 是空操作**：SPC 勾选 BP/CMP 后必须执行 Protect 任务才生效；Verify/Blank 对 SR 的校验由 E80 开关使能，期望值取自当前 SPC 设置（mask 0x7C/0x40）。
4. **WRSR 必须 16 位**：01h 单字节写会把 CMP/QE 清 0（Datasheet 7.4）；驱动 Protect/UnProtect 均为读-改-写 + 一次写满 2 字节 + WIP 轮询 + 双字节读回校验，符合要求。
5. **安全区 LB 预检**：Program/Erase 安全区前先查对应 LBx，已锁（或来料异常锁定）报 FAIL，不会假 Pass。
6. **SPI 时钟**：`ChipSPIMaxClkTable=22`，注释标注为保守 bring-up 值，QC 首轮按此验证，不建议 QC 中调频。

## 5. 最终测试项目

| 编号 | 阶段 | 测试项目 | 预计耗时 | 测试槽位 |
|---|---|---|---:|---|
| T01 | 基本功能 | AUTO 基本烧录流程（IDCHECK→Erase→Blank→Program→Verify→CheckSum） | 5 min | A/B/C/D |
| T02 | 基本功能 | Main Flash Read CRC 比对 | 2 min | A |
| T03 | NG测试 | 已烧录状态 Blank NG | 1 min | 代表槽位 |
| T04 | NG测试 | 单字节 Verify NG | 2 min | 代表槽位 |
| T05 | NG测试 | 多槽异常隔离（含空槽 ID NG） | 4 min | 全槽 |
| T06 | NG测试 | 错型号 ID NG | 1 min | 代表槽位 |
| T07 | 特殊功能 | BP 区域保护设置与 SR 校验（Protect + E80 Verify） | 3 min | A/B/C/D |
| T08 | 特殊功能 | 保护行为验证：保护区 Program 无效 / Erase NG / UnProtect 恢复 | 4 min | A/B/C/D |
| T09 | 特殊功能 | CMP 互补保护与 CE 执行条件边界 | 4 min | A/B/C/D |
| T10 | 特殊功能 | Security Register 1-3 擦写读全流程 | 4 min | A/B/C/D |
| T11 | 特殊功能 | 不可逆功能防呆检查（LB/Secure 已关闭确认） | 2 min | - |
| T12 | 特殊功能 | SN 烧录与校验（可选，量产用 SN 时执行） | 3 min | 全槽 |

- 预计总耗时：32 min（不含可选项 T12；含 T12 为 35 min）
- 参考预算：≤ 35 min
- 是否符合预算：是

测试顺序设计依据：先基础功能 → 可恢复的 BP/CMP 保护 → 特殊区域（Security Register）→ 不可逆项仅做防呆不执行；T07-T09 每项结束都恢复为无保护状态，不破坏后续条件；T10 安全区操作前主 Flash 已完成全部保护测试。

## 6. 未纳入现场表的功能

| 功能 | 原因 |
|---|---|
| LB1-LB3 OTP 永久锁 | 驱动有意关闭（三处一致）；不可逆，仅做 T11 防呆确认 |
| SRP0/SRP1（含 (1,1) OTP 模式） | 驱动/SPC 均不开放，保证保护永远可解除 |
| QE / Dual / Quad 模式 | 平台走标准 SPI，驱动未使用 |
| Sector/Block Erase 20h/8Bh/52h/D8h | 驱动 Erase 只走 Chip Erase，未使用这些命令 |
| UID 4Bh / SFDP 5Ah | 驱动未实现 |
| DPD / 软件 Reset | 驱动未使用，与烧录流程无关 |
| Fast Read 0Bh | 仅 SN 功能内部使用，随 T12 覆盖 |

## 7. 高风险防呆

| 项目 | 风险 | 处理方式 |
|---|---|---|
| LB1-LB3 OTP 永久锁 | 一旦置 1 对应安全区永久只读，不可恢复；Datasheet 支持但驱动已关闭 | T11 确认界面无 LB 选项、Secure 为无效任务；禁止现场用其他工具置位 |
| SRP1,SRP0=(1,1) OTP 模式 | 永久锁定整个 Status Register | 驱动 mask 不开放 SRP 位，RMW 保留原值（出厂 0）；QC 不涉及 |
| 保护态 Erase 假 Pass | 芯片在保护态静默忽略 CE，WIP 正常归零 | 驱动 Erase 入口预检 SR，保护态直接 E_EraseFail；T08/T09 验证该行为 |
| 保护区 Program 假 Pass | 芯片静默忽略，WIP 正常归零 | T08 以 Verify NG 作为保护生效判据 |
| WRSR 单字节写清 CMP/QE | 01h 只写 1 字节会清 CMP/QE | 驱动已按 16 位 RMW 实现；T07/T09 的读回校验（E80 Verify）覆盖 |

## 8. 发布前建议

- 自动 Loop：T01 流程可作为量产 Loop 基准；保护/安全区测试不进量产 Loop
- 工程保存恢复：QC 用的 BP/CMP/E80/安全区开关组合较多，建议每个测试项执行前核对当前特殊位状态，T09 结束后保存一份"全部不勾"的干净工程
- 断电恢复：BP/CMP 为非易失位，测试中断后 IC 可能带保护，下一项测试前先执行 UnProtect + Erase 恢复（T07/T08 前置步骤已包含）
- 样片管理：T07-T10 使用可重复擦写样片即可；本驱动无不可逆测试项，无需消耗性样片
- 编程笔记：QC 实测通过后，将结果回填 `docs/TH25Q-80HB_programming_notes.md` 第 17 节（Engineering Test Notes），并把对应条目标记 Hardware Verified

## 9. 输出文件

- qc-key：`SYx4_TH25Q-80HB/docs/qc-key.md`
- Markdown：`SYx4_TH25Q-80HB/SYx4_TH25Q-80HB_QC分析报告.md`
- Excel：`SYx4_TH25Q-80HB/SYx4_TH25Q-80HB_QC测试执行表.xlsx`
- 生成脚本：`SYx4_TH25Q-80HB/_gen_qc_xlsx.py`

## 10. 跨文件一致性结论

- notes ↔ .c/.h：命令表、SR 布局、保护掩码 0x407C、CE 预检条件、安全区地址、LB 关闭策略一致
- .h ↔ .c：算法表 13 项与 .c 函数一一对应；Secure 槽位 = UselessAlgoFun 与 .c SecureFun 关闭一致；Block 表虚拟地址与 .c 各分支地址使用一致
- .spc ↔ .c/.h：S0000/S0001（SRAM[0]/[1]）对应 ProtectFun 的 BP/CMP 目标字节；S0003（SRAM[3]）六个开关与 .c 中 ProgramOn/VerifyOn/EraseOn/CheckOn/ChecksumOn/StatusCheckOn 位定义一一对应；SPC 无 LB 选项与 Secure 关闭一致
- 结论：未发现 P0 冲突；P1 说明项为 SecureFun 死代码保留（注释已说明待逐槽 RMW 后重开），不影响 QC
