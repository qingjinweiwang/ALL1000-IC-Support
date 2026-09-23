TH25Q-80HB driver 测试流程

平台：ALL-1000G（Gang4，A/B/C/D 四槽，3.3V SOP8 座）
接口：Standard SPI（3 字节地址）

测试基本功能
1. 取两颗正常 IC 放入 A/B 槽，特殊位全部不勾，Buffer 载入 1MB 测试数据（00000000h-000FFFFFh），执行 AUTO（IDCHECK→Erase→Blank→Program→Verify→CheckSum），两槽全部 Pass，CheckSum 与上位机计算值一致。
2. A 槽执行 Read，读回数据与 Buffer 比对一致。

测试 NG
1. 取上面已烧录的 IC 直接执行 Blank，确认 Blank NG。
2. 修改 Buffer 任意 1 Byte 后执行 Verify，确认 Verify NG；恢复该 Byte 后再次 Verify，确认 Pass。
3. 一颗正常 IC 与空槽（或异常 IC）同时执行不含 Read 的 AUTO，确认正常槽 Pass、异常槽 NG，其他槽不受影响。
4. 放入一颗其他型号 IC 执行 IDCHECK，确认 ID NG（确认 Auto ID 确实在比对 CD-60-14）。

测试 BP 区域保护（Upper 1/16，保护区 0F0000h-0FFFFFh）
前置：IC 已 Erase 为空白。
1. 在 PROTECTION OPTIONS 中：勾选 BP0，BP1-BP4 不勾选，CMP 不勾选；勾选 USER SECURITY REGISTER OPTIONS 中的 Enable Status Register Verify/Blank Check。执行 Protect，确认 Pass；执行 Verify，确认 Pass（SR 低字节 BP0=1 已生效）。
2. 执行 Erase，确认 NG（保护态下驱动直接报 Erase 失败，CE 不会发出）。
3. Buffer 载入新数据，直接执行 Program 后执行 Verify，确认 Verify NG（保护区 0F0000h-0FFFFFh 未被写入）。可用 A 槽 Read 确认：000000h 起始数据与 Buffer 一致（非保护区可写），0F0000h 起始仍为 FF（保护区不可写）。
4. 执行 UnProtect，确认 Pass；再执行 Erase→Blank→Program→Verify，确认全部 Pass。测试结束取消 BP0 勾选。

测试全片保护与 CMP 互补
前置：IC 已 Erase 为空白，特殊位全部不勾。
1. 在 PROTECTION OPTIONS 中：勾选 BP1、BP2，BP0/BP3/BP4 不勾选，CMP 不勾选。执行 Protect，确认 Pass。
2. 执行 Erase，确认 NG；直接执行 Program 后执行 Verify，确认 Verify NG（全片保护，任何地址都写不进）。
3. 执行 UnProtect，确认 Pass；执行 Erase→Blank，确认 Pass（恢复）。
4. 在 PROTECTION OPTIONS 中：勾选 CMP、BP0、BP1、BP2（BP3/BP4 不勾选）。执行 Protect，确认 Pass；执行 Erase→Blank，确认 Pass（CMP=1 且 BP2-BP0=111 为无保护，CE 可执行，证明 CMP 已生效）。
5. 在 PROTECTION OPTIONS 中：只勾选 CMP，BP0-BP4 全不勾。执行 Protect，确认 Pass；执行 Erase，确认 NG（CMP=1 且 BP 全 0 为全片保护）。
6. 执行 UnProtect，确认 Pass；执行 Erase→Blank，确认 Pass。测试结束取消所有勾选。

测试 Security Register 1-3
前置：IC 已 Erase 为空白，PROTECTION OPTIONS 全部不勾。
1. 在 USER SECURITY REGISTER OPTIONS 中：勾选 Enable Security Register Program、Enable Security Register Verify、Enable Security Register Erase、Enable Security Register Blank Check、Enable Security Register Checksum Calculate。
2. Buffer 在 00100000h-00100BFFh 载入测试数据（3 个安全区各 1KB，对应设备地址 1000h/2000h/3000h）。
3. 执行 Erase→Blank→Program→Verify，确认全部 Pass（3 个安全区各自完成擦除、空白检查、写入、校验）。
4. A 槽执行 Read，确认 00100000h-00100BFFh 读回数据与 Buffer 一致。
5. 测试结束取消本组所有勾选。

不可逆功能防呆确认
1. 确认特殊位设置界面中无 LB1/LB2/LB3 选项，任务列表中无有效 Secure 任务（本驱动有意关闭 LB 永久锁定功能）。
2. 整个 QC 过程中不得使用其他工具对 IC 设置 LB1-LB3、SRP0、SRP1。

注意事项
1. Read 仅 A 槽有效，多槽 AUTO 不要包含 Read。
2. Verify 只报 Pass/NG 和槽位，不报错误地址；单字节 Verify NG 不要求核对地址。
3. 保护态下 Erase 报 NG 是预期行为（驱动预检状态寄存器，CE 根本不会发出）；需要擦除前必须先执行 UnProtect。
4. 保护区内 Program 可能报 Pass（芯片静默忽略写入，WIP 正常归零），保护是否生效一律以 Verify 结果判定。
5. 修改 PROTECTION OPTIONS 后必须执行 Protect 任务才写入芯片；单独执行 Program 不会修改状态寄存器。
6. BP/CMP 为非易失位，测试中断后 IC 可能仍带保护；每个测试项开始前先执行 UnProtect + Erase 恢复干净状态。
7. Gang 模式下状态寄存器读回值取自 A 槽，默认各槽出厂状态一致。
8. LB1-LB3 为 OTP 永久锁定位，一旦置 1 对应 Security Register 永久只读且不可恢复；本驱动已关闭该功能，禁止使用外部工具置位。
9. SRP0/SRP1 未开放，禁止置位（SRP1,SRP0=1,1 为 OTP 模式，永久锁定整个状态寄存器）。
10. 本驱动 SPI 时钟为保守值，QC 期间不要调整频率。
