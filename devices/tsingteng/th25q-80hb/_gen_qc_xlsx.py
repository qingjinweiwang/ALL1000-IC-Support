# -*- coding: utf-8 -*-
"""Generate TH25Q-80HB QC execution sheet from the chip-qc-analyzer template.

Template facts (chip_qc_single_sheet_template.xlsx, sheet "测试执行表"):
- A1:L1 merged title; row 2 = 12 column headers; data rows 3-10 (8 sample rows)
- stage merges: A3:A4 基本功能 / A5:A7 NG测试 / A8:A10 特殊功能
- K column dropdown 未测试/Pass/Fail/N/A
This chip needs 12 rows (T01-T12): 基本功能 2 / NG测试 4 / 特殊功能 6,
so 4 rows are appended by copying the last template data row's style,
and stage merges become A3:A4 / A5:A8 / A9:A14.
"""
import shutil
from copy import copy

import openpyxl
from openpyxl.styles import Font

TPL = r"_tmp_qc/chip_qc_single_sheet_template.xlsx"
OUT = r"SYx4_TH25Q-80HB/SYx4_TH25Q-80HB_QC测试执行表.xlsx"

shutil.copyfile(TPL, OUT)
wb = openpyxl.load_workbook(OUT)
ws = wb["测试执行表"]

# ---- title ---------------------------------------------------------------
ws["A1"] = (
    "TH25Q-80HB 烧录 QC 测试执行表 · ALL-1000G"
    "(预计总耗时 32 min / 参考预算 ≤ 35 min · Skill v2)"
)

# ---- test rows ------------------------------------------------------------
# (编号, 测试项目, 前置准备, 测试步骤, 理论测试结果, 耗时, 槽位, 风险)
rows = [
    ("T01", "AUTO 基本烧录流程",
     "正式工程(SYx4_TH25Q-80HB-MSDx 已选入);覆盖全部 1MB(000000h-0FFFFFh)的测试数据文件(非全 FF,建议含递增/随机 pattern,首尾及页边界有特征数据);4 颗正常 TH25Q-80HB;SOP8 适配器。",
     "1. 确认上位机 Auto ID 配置为 CD-60-14(9Fh)。\n"
     "2. 打开\"特殊位设置\",确认所有选项均不勾选(出厂默认)。\n"
     "3. A/B/C/D 槽装入目标 IC。\n"
     "4. AUTO 勾选 IDCHECK、Erase、Blank、Program、Verify、CheckSum(不勾选 Read、Secure)。\n"
     "5. 执行 AUTO,记录 AUTO 总时间。\n"
     "6. 查看各槽位执行结果与 CheckSum。",
     "所有装片槽位依次完成 IDCHECK→Erase→Blank→Program→Verify 并显示 Pass;CheckSum 与测试文件一致;空槽按平台规则显示 NG;软件运行稳定,总时间被记录。",
     5, "A/B/C/D",
     "AUTO 不含 Read(Read 仅 A 槽,T02 单独测);Secure 为无效任务不要勾选;执行前确认特殊位全部不勾,防止带保护烧录。"),
    ("T02", "Main Flash Read CRC 比对",
     "T01 已通过;A 槽保留已烧录 IC;上位机可获取 Buffer CRC。",
     "1. 记录当前 Buffer(原始文件)CRC。\n"
     "2. 仅 A 槽执行 Read(完整 1MB)。\n"
     "3. 获取 Read 后 Buffer 的 CRC。\n"
     "4. 比较两个 CRC。",
     "A 槽 Read 显示 Pass;Read 后 CRC 与原始 CRC 一致。",
     2, "A",
     "Read 仅 A 槽有效(平台框架),不要用多槽 Read 代替。"),
    ("T03", "已烧录状态 Blank NG",
     "IC 保持 T01 烧录后的非空状态。",
     "不执行 Erase,直接执行 Blank Check。",
     "Blank Check 显示 NG;不得错误显示 Pass;NG 槽位显示正确。",
     1, "代表槽位",
     "只验证 Blank 的基本 NG 行为。"),
    ("T04", "单字节 Verify NG",
     "IC 与当前 Buffer 数据一致(T01 烧录后)。",
     "1. 修改 Buffer 中任意 1 个字节。\n"
     "2. 执行 Verify。\n"
     "3. 恢复该字节。\n"
     "4. 再次执行 Verify。",
     "首次 Verify 显示 NG;恢复后 Verify 显示 Pass。ALL-1000G 平台不反馈错误地址,不要求核对。",
     2, "代表槽位",
     "ALL-1000G Verify 只回报 Pass/NG 和槽位。"),
    ("T05", "多槽异常隔离",
     "4 槽中 1 槽留空(或放接触不良/异常 IC),其余槽放正常 IC。",
     "执行一次不含 Read 的代表性 AUTO(IDCHECK→Erase→Blank→Program→Verify),观察异常槽与正常槽结果。",
     "异常槽显示 NG(空槽即 ID NG);正常槽不受影响并显示 Pass;槽位显示与实际一致;软件不宕机。",
     4, "全槽",
     "多槽 AUTO 不得包含 Read。"),
    ("T06", "错型号 ID NG",
     "1 颗非 TH25Q-80HB 的其他 SPI Flash(ID 非 CD-60-14,如 W25Q80/GD25Q80);1 颗正常 TH25Q-80HB。",
     "1. 任一槽放错型号芯片,其余槽放正常 IC。\n"
     "2. 执行 IDCheck。",
     "错型号槽显示 NG(E_AutoIDFail);正常槽 Pass;槽位指示正确。",
     1, "代表槽位",
     "不改动正式工程的 Auto ID 配置;测试后确认放回正确型号芯片再走后续流程。"),
    ("T07", "BP 区域保护设置与 SR 校验",
     "可重复擦写样片;T01 测试文件。目的:验证 Protect 按 SPC 设置 BP 位并一次写满 16 位(不清 CMP/QE)。",
     "1. 特殊位设置全部不勾。执行 UnProtect(防止残存保护)→ Erase → Blank,确认空片。\n"
     "2. Program + Verify,全片数据 OK。\n"
     "3. 打开\"特殊位设置\":STATUS REGISTER 1 只勾选 \"BP0 = 1\";STATUS REGISTER 2 的 CMP 不勾;勾选 \"Enable Status Register Verify/Blank Check\"。确定。\n"
     "4. 执行 Protect。\n"
     "5. 执行 Verify(含状态寄存器校验)。",
     "Protect Pass;Verify Pass(读回 SR 低字节 BP0=1,证明 16 位 WRSR 与读回校验正确)。此时保护范围 = Upper 1/16:0F0000h-0FFFFFh(64KB)。",
     3, "A/B/C/D",
     "BP/CMP 勾选只通过 Protect 任务生效(状态位块 Program 为空操作);E80(Enable Status Register Verify/Blank Check)必须勾选,否则 Verify 不校验 SR。"),
    ("T08", "保护行为验证:保护区 Program 无效 / Erase NG / UnProtect 恢复",
     "接 T07 状态(BP0 保护生效,片内为 T01 测试数据)。",
     "1. 修改 Buffer 中 0F0000h 起始的 1 页(256B)数据(保护区内),执行 Program。\n"
     "2. 执行 Verify。\n"
     "3. 恢复 Buffer 该页数据。\n"
     "4. 执行 Erase(单独任务)。\n"
     "5. 执行 UnProtect,再执行 Erase → Blank。\n"
     "6. Program + Verify,确认完全恢复。",
     "步骤 1 Program 可能报 Pass(芯片静默忽略保护区写入,WIP 正常归零),不作为判据;步骤 2 Verify 必须 NG(保护区数据未被改写);步骤 4 Erase 必须 NG(E_EraseFail,驱动保护预检拦截,CE 未执行);步骤 5-6 全部 Pass,IC 恢复为无保护可烧录状态。",
     4, "A/B/C/D",
     "核心判据是 Verify NG + Erase NG,不要被 Program 的 Pass 误导;Erase NG 是预期行为,证明 CE 未被静默忽略造成假 Pass。"),
    ("T09", "CMP 互补保护与 CE 执行条件边界",
     "可重复擦写样片;E80 保持勾选。",
     "1. 特殊位设置:只勾选 \"BP1 = 1\" 和 \"BP2 = 1\"(CMP 不勾)。执行 Protect → 应 Pass。此时全片保护。\n"
     "2. 执行 Erase(单独任务)。\n"
     "3. 执行 UnProtect。\n"
     "4. 特殊位设置:勾选 \"BP0 = 1\"\"BP1 = 1\"\"BP2 = 1\" 并勾选 \"CMP = 1\"。执行 Protect → 应 Pass。此时 CMP=1 且 BP2-BP0=111,实际无保护。\n"
     "5. 执行 Erase → Blank。\n"
     "6. 执行 UnProtect(恢复 SR=0000h 出厂态)。\n"
     "7. 执行 Blank(E80 勾选状态下同时校验 SR 无保护)。",
     "步骤 2 Erase 必须 NG(BP2-0=110 不满足 CE 执行条件,预检拦截);步骤 5 Erase 必须 Pass 且 Blank Pass(证明 CMP=1 且 BP2-0=111 时 CE 真实执行);步骤 7 Blank Pass。",
     4, "A/B/C/D",
     "本项覆盖驱动 CE 预检的两个分支;每步 Protect/UnProtect 后可用 Verify(E80)确认 SR 值符合预期。"),
    ("T10", "Security Register 1-3 擦写读全流程",
     "含安全区数据的测试文件:虚拟地址 00100000h-001003FFh(Security Reg1)、00100400h-001007FFh(Reg2)、00100800h-00100BFFh(Reg3),三段 1KB 数据互不相同且非全 FF;正常样片(安全区未锁)。",
     "1. 特殊位设置:USER SECURITY REGISTER REGION OPTION 勾选 \"Enable Security Register Program\"\"Enable Security Register Verify\"\"Enable Security Register Erase\"\"Enable Security Register Blank Check\"(可选同时勾 Checksum);BP/CMP 全部不勾。\n"
     "2. 执行 AUTO:Erase → Blank → Program → Verify。\n"
     "3. (可选)仅 A 槽执行 Read,比对安全区虚拟地址段 CRC 与 Buffer 一致。",
     "AUTO 全部 Pass(主 Flash CE + 三个安全区分别擦除、空白检查、烧录、校验);可选 Read CRC 一致。",
     4, "A/B/C/D",
     "安全区 Program/Erase 前有 LB 预检;若报 FAIL 且其他功能正常,说明样片安全区已被永久锁定(来料异常),更换新片重测;锁定不可恢复,该片报废处理并记录。"),
    ("T11", "不可逆功能防呆检查(LB/Secure)",
     "不执行任何芯片写入;仅打开工程特殊位界面。",
     "1. 检查\"特殊位设置\"界面:确认无 LB1/LB2/LB3 或任何 Secure/永久锁定类选项(SPC 只有 BP0-BP4、CMP 和安全区操作开关)。\n"
     "2. 确认 AUTO/任务列表中 Secure 为无效任务(驱动算法表注册 UselessAlgoFun,SecureFun 直接返回)。\n"
     "3. 全程不对芯片执行任何写入。",
     "界面无 LB 选项;正常烧录流程不会触发任何永久锁定;未对芯片执行任何写入。",
     2, "-",
     "R-永久:LB1-LB3 为 OTP 位,一旦置 1 对应 Security Register 永久只读,不可恢复。本驱动第一阶段有意关闭该功能;禁止现场用其他工具/工程对样片置位 LB 或 SRP(1,1)。"),
    ("T12", "SN 烧录与校验(可选,量产用 SN 时执行)",
     "量产使用上位机 SN(序列号)功能时执行;IC 已 Erase 为空白。",
     "1. Erase → Blank 确认空片。\n"
     "2. 启用上位机 SN 烧录:SN 地址设在主 Flash 某空白页内非页首位置(如 00000110h),长度不超过页内剩余空间(≤256-偏移)。\n"
     "3. 执行 SN 烧录。\n"
     "4. 仅 A 槽 Read 该页,确认 SN 数据正确且页内其余字节保持 FF。",
     "SN 写入正确,页内其他数据不受影响;多槽逐槽独立写入;越界(SN 跨页)被驱动拒绝并报错。",
     3, "全槽",
     "SN 写入前不擦除,目标页必须为空白页;量产前确认 SN 地址不与固件数据区重叠。"),
]

START = 3
template_row = 10                     # last styled data row in template
need_rows = START + len(rows) - 1     # = 14

# extend rows beyond the template by copying the last data row's style
for r in range(template_row + 1, need_rows + 1):
    ws.row_dimensions[r].height = ws.row_dimensions[template_row].height
    for col in range(1, 13):
        src = ws.cell(row=template_row, column=col)
        dst = ws.cell(row=r, column=col)
        dst._style = copy(src._style)

# stage merges: 基本功能 A3:A4(保持) / NG测试 A5:A7→A5:A8 / 特殊功能 A8:A10→A9:A14
ws.unmerge_cells("A5:A7")
ws.unmerge_cells("A8:A10")
for r in range(5, need_rows + 1):
    ws.cell(row=r, column=1).value = None
ws.cell(row=5, column=1, value="NG测试")
ws.cell(row=9, column=1, value="特殊功能")
ws.merge_cells("A5:A8")
ws.merge_cells("A9:A14")

for i, (no, item, pre, steps, expect, cost, slots, risk) in enumerate(rows):
    r = START + i
    ws.cell(row=r, column=2, value=no)
    ws.cell(row=r, column=3, value=item)
    ws.cell(row=r, column=4, value=pre)
    ws.cell(row=r, column=5, value=steps)
    ws.cell(row=r, column=6, value=expect)
    ws.cell(row=r, column=7, value=cost)
    ws.cell(row=r, column=8, value=slots)
    ws.cell(row=r, column=9, value=risk)
    # 注意:ws.cell(..., value=None) 不会清空单元格,必须显式赋 .value = None
    ws.cell(row=r, column=10).value = None       # 实际测试结果,留给测试人员(清掉模板占位符)
    ws.cell(row=r, column=11, value="未测试")     # 结论
    ws.cell(row=r, column=12).value = None       # 备注(清掉模板占位符)

# T11(不可逆防呆)风险列红色加粗强调
r_t11 = START + 10
f = ws.cell(row=r_t11, column=9).font
ws.cell(row=r_t11, column=9).font = Font(name=f.name, size=f.size, bold=True,
                                         color="FF0000")

# freeze column-header rows
ws.freeze_panes = "A3"

wb.save(OUT)
print("saved", OUT)

# ---- verify ---------------------------------------------------------------
wb2 = openpyxl.load_workbook(OUT)
ws2 = wb2["测试执行表"]
assert wb2.sheetnames == ["测试执行表"], wb2.sheetnames
print("sheets:", wb2.sheetnames)
print("merged:", sorted(str(x) for x in ws2.merged_cells.ranges))
print("freeze:", ws2.freeze_panes)
costs = [ws2.cell(row=r, column=7).value for r in range(START, need_rows + 1)]
print("costs:", costs, "sum:", sum(costs))
for r in range(START, need_rows + 1):
    print(r, "| A:", ws2.cell(row=r, column=1).value,
          "| B:", ws2.cell(row=r, column=2).value,
          "| C:", ws2.cell(row=r, column=3).value,
          "| J:", ws2.cell(row=r, column=10).value,
          "| K:", ws2.cell(row=r, column=11).value,
          "| L:", ws2.cell(row=r, column=12).value)
