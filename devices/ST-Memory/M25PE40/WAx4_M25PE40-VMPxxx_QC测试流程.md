M25PE40-VMPxxx driver 测试流程

平台：ALL-1000G（Gang4，A/B/C/D 四槽，3.3V VFQFPN8 座）
接口：Standard SPI（3 字节地址）

测试基本功能
1. 取两颗正常 IC 放入 A/B 槽，特殊位全部不勾，Buffer 载入 512KB 测试数据（00000000h-0007FFFFh），执行 AUTO（IDCHECK→Erase→Blank→Program→Verify→CheckSum），两槽全部 Pass，CheckSum 与上位机计算值一致。
2. A 槽执行 Read，读回数据与 Buffer 比对一致。

测试 NG
1. 取上面已烧录的 IC 直接执行 Blank，确认 Blank NG。
2. 修改 Buffer 任意 1 Byte 后执行 Verify，确认 Verify NG；恢复该 Byte 后再次 Verify，确认 Pass。
3. 一颗正常 IC 与空槽（或异常 IC）同时执行不含 Read 的 AUTO，确认正常槽 Pass、异常槽 NG，其他槽不受影响。
4. 放入一颗其他型号 IC 执行 IDCHECK，确认 ID NG（确认 Auto ID 确实在比对 20h-80h-13h）。

测试 BP 区域保护（Upper 1/8，保护区 070000h-07FFFFh）
前置：IC 已 Erase 为空白，特殊位全部不勾。
1. 在 STATUS REGISTER PROTECTION OPTIONS 中：勾选 BP0，BP1、BP2 不勾选。执行 Protect，确认 Pass；执行 Verify，确认 Pass（SR 的 BP0=1 已写入并与特殊位设置一致）。
2. Buffer 在 070000h-0700FFh 载入 1 Page 非 FF 数据（其余地址保持 FF），执行 Program，确认 Program NG（保护区内 Page Program 被芯片拒绝，驱动检测到写拒绝报 Program 失败）。
3. Buffer 改为在 000000h-0000FFh 载入 1 Page 数据（070000h 处恢复 FF），执行 Program→Verify，确认 Pass（非保护区可正常写入）。
4. A 槽执行 Read，确认 070000h-07FFFFh 读回全为 FF（保护区未被写入）。
5. 执行 UnProtect，确认 Pass；取消 BP0 勾选，执行 Erase→Blank→Program→Verify，确认全部 Pass（恢复）。

测试全片保护与 Erase 自动解除保护
前置：IC 已 Erase 为空白，特殊位全部不勾。
1. 在 STATUS REGISTER PROTECTION OPTIONS 中：勾选 BP2，BP0、BP1 不勾选（BP2-BP0=100 为全片保护）。执行 Protect，确认 Pass。
2. Buffer 任意地址载入 1 Page 非 FF 数据，执行 Program，确认 Program NG（全片保护，任何 Page Program 都被拒绝）。
3. 执行 Erase，确认 Pass（驱动先自动写入 BP=0 解除保护，再执行 Bulk Erase 整片擦除）；执行 Blank，确认 Pass（整片已擦除、保护已清除）。测试结束取消 BP2 勾选。

不可逆及未开放功能防呆确认
1. 确认特殊位设置界面只有 BP0/BP1/BP2 三项，无 SRWD 选项（驱动有意不开放：SRWD=1 且 W# 引脚为低时进入硬件保护模式，WRSR 被拒绝，保护将无法用 UnProtect 软件解除）。
2. 确认特殊位设置界面无 Lock Register（Write Lock / Lock Down）配置项，任务列表中无有效 Secure 任务（本驱动未开放 Lock Register 扇区锁定功能）。
3. 整个 QC 过程中不得使用其他工具对 IC 设置 SRWD、Lock Register。Lock Register 为易失性，意外置位后重新上电可恢复；SRWD 进入 HPM 后需 W# 引脚拉高才能解除。

注意事项
1. Read 仅 A 槽有效，多槽 AUTO 不要包含 Read。
2. Verify 只报 Pass/NG 和槽位，不报错误地址；单字节 Verify NG 不要求核对地址。
3. 保护态下 Program 报 NG 是预期行为（芯片拒绝写入后 WEL 不复位，驱动据此判定失败）；保护态下 Erase 报 Pass 也是预期行为（驱动先自动 UnProtect 再 Bulk Erase）。保护是否生效以 Program NG 和 Read 回读判定。
4. 修改 STATUS REGISTER PROTECTION OPTIONS 后必须执行 Protect 任务才写入芯片，单独执行 Program 不会修改状态寄存器；勾选特殊位后直接执行含 Verify 的 AUTO 会报 Verify NG（SR 与特殊位设置不一致）。
5. BP2-BP0 为非易失位，掉电不丢失；测试中断后 IC 可能仍带保护，每个测试项开始前先执行 UnProtect + Erase 恢复干净状态（本驱动 Erase 自带解除保护，单独执行 Erase 也可）。
6. Erase 为整片 Bulk Erase，典型 8s、最长 10s，耗时较长属正常；本驱动无扇区/页擦除任务。
7. Program 跳过全 FF 的页，属正常。
8. Gang 模式下状态寄存器逐槽比对；Read 任务回读的数据取自 A 槽。
9. 本驱动仅适用 T9HX（VFQFPN8，MP 封装）料号；CFD/UID 为出厂只读内容，驱动不开放修改。
