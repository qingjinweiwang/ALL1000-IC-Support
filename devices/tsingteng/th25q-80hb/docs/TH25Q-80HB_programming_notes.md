# TH25Q-80HB Programming Notes

## Document Information

- Device: TH25Q-80HB
- Manufacturer: TSINGTENG Microsystem Co., Ltd (TMS)
- Source Datasheet: TH25Q-80HB_datasheet_V1.6_TMS_20220326.pdf
- Datasheet Revision: V1.6 (Mar.26, 2022)
- Notes Version: 1.0
- Last Verified: 2026-08-12(按 Datasheet 文本逐项核对)
- Status: Unverified(未经硬件实测)

> 本文件只记录烧录驱动开发相关内容。未在 Datasheet 中明确确认的项目必须标记"未确认"。

---

## 1. Basic Information

- Device: TH25Q-80HB
- Manufacturer: TSINGTENG Microsystem
- Capacity: 8M-bit
- Main Flash Size: 1M-Byte (1024KB)
- Main Flash Address Range: 000000h - 0FFFFFh
- Voltage: 2.3V - 3.6V(Read/Erase/Program 全范围)
- Package: SOP8(150mil/208mil)、TSSOP8、USON8、WSON8
- Page Size: 256 Bytes
- Sector Size: 4K-Byte(uniform);另有 1KB Sector Erase 命令
- Block Size: 32K/64K-Byte

### Source

- Section: 1. FEATURES / 3. MEMORY ORGANIZATION
- Page: 4, 7

---

## 2. JEDEC ID

- Command: 9Fh
- Manufacturer ID: CDh
- Memory Type: 60h
- Capacity ID: 14h
- Additional ID Bytes: 无(9Fh 返回 3 字节)
- Expected Response: CDh - 60h - 14h
- 其他 ID 命令: REMS 90h 返回 CDh-13h; ABh 返回 Device ID 13h

### Required Precondition

- Power-up delay: tVSL = VCC(min) 到 CS# Low, min 50us / max 500us; tPWD = 300us
- Release from Deep Power Down: tRES1/tRES2 = 25us
- Reset requirement: 无硬件复位脚; 软件 Reset 66h+99h(tRST=30us, WRSR 后 4ms, CE 后 120us)

### Source

- Section: 7.27 / Table of ID Definitions / 8.1
- Page: 36, 15, 47

---

## 3. SPI Configuration

- Supported SPI Mode: Mode 0 和 Mode 3
- Default Mode: 未确认(两种均支持,平台用 Mode 0 即可)
- Standard SPI Max Clock: Read(03h) 55MHz; RDSR/RDID(05h/35h/9Fh/90h/ABh) 104MHz
- Read Max Clock: Fast Read(0Bh) 104MHz @2.7-3.6V, 85MHz @2.3-2.7V
- Program/Erase Clock Restriction: 无特殊限制
- CS Requirement: 命令必须以字节边界结束, CS# 在字节中间拉高则命令被拒绝
- Input Sampling Edge: SCLK 上升沿
- Output Change Edge: SCLK 下降沿

### Source

- Section: 4. DEVICE OPERATION / 8.6 AC CHARACTERISTICS
- Page: 8, 51-52

---

## 4. Basic Command Table

| Function | Command | Address Bytes | Dummy | Notes |
|---|---:|---:|---:|---|
| Write Enable | 06h | 0 | 0 | |
| Write Disable | 04h | 0 | 0 | |
| Read Status 1 | 05h | 0 | 0 | 返回 S7-S0 |
| Read Status 2 | 35h | 0 | 0 | 返回 S15-S8(同一 16 位 SR 的高字节) |
| Read Status 3 | 不存在 | - | - | 本芯片只有 1 个 16 位 Status Register |
| Write Status | 01h | 0 | 0 | 写 1 或 2 字节; **只写 1 字节会将 CMP 和 QE 清 0** |
| Volatile SR Write Enable | 50h | 0 | 0 | 不置 WEL,仅供易失性写 SR |
| Read | 03h | 3 | 0 | 整片可单次连续读出 |
| Fast Read | 0Bh | 3 | 1 | |
| Page Program | 02h | 3 | 0 | 1-256 字节 |
| Sector Erase | 20h | 3 | 0 | 4KB |
| 1KB Sector Erase | 8Bh | 3 | 0 | 1KB |
| 32KB Block Erase | 52h | 3 | 0 | |
| 64KB Block Erase | D8h | 3 | 0 | |
| Chip Erase | C7h 或 60h | 0 | 0 | 两命令等价 |
| JEDEC ID | 9Fh | 0 | 0 | CD-60-14 |
| Reset Enable | 66h | 0 | 0 | |
| Reset | 99h | 0 | 0 | 需先 66h |
| Deep Power Down | B9h | 0 | 0 | tDP=25us |
| Release Power Down | ABh | 0 | 0/3 | tRES1=25us |
| Read Unique ID | 4Bh | 0 | 4 | 128-bit UID |
| SFDP Read | 5Ah | 3 | 1 | |
| Security Reg Erase | 44h | 3 | 0 | |
| Security Reg Program | 42h | 3 | 0 | |
| Security Reg Read | 48h | 3 | 1 | |
| Program/Erase Suspend | 75h/B0h | 0 | 0 | v0.1 未使用 |
| Program/Erase Resume | 7Ah/30h | 0 | 0 | v0.1 未使用 |
| Continuous Read Mode Reset | FFh | 0 | 0 | 仅 Dual/Quad 连续读模式后需要 |

### Source

- Section: 7. COMMANDS DESCRIPTION, Table2
- Page: 13-15

---

## 5. Status Registers

本芯片为**单一 16 位 Status Register**, 05h 读低字节(S7-S0), 35h 读高字节(S15-S8)。

### Status Register 低字节(05h, S7-S0)

| Bit | Name | R/W | Default | Description |
|---:|---|---|---|---|
| S0 | WIP | R | 0 | 1=Busy(Program/Erase/WRSR 进行中) |
| S1 | WEL | R | 0 | 写使能锁存 |
| S2 | BP0 | R/W | 0 | 块保护位(非易失) |
| S3 | BP1 | R/W | 0 | 块保护位 |
| S4 | BP2 | R/W | 0 | 块保护位 |
| S5 | BP3 | R/W | 0 | 块保护位 |
| S6 | BP4 | R/W | 0 | 块保护位 |
| S7 | SRP0 | R/W | 0 | 状态寄存器保护 0 |

### Status Register 高字节(35h, S15-S8)

| Bit | Name | R/W | Default | Description |
|---:|---|---|---|---|
| S8 | SRP1 | R/W | 0 | 状态寄存器保护 1(SRP1,SRP0=1,1 为 OTP 永久锁定!) |
| S9 | QE | R/W | 0 | Quad Enable(非易失) |
| S10 | SUS2 | R | 0 | Program Suspend 状态(只读) |
| S11 | LB1 | R/W(OTP) | 0 | Security Register 1 锁定位, **OTP, 置 1 永久锁定** |
| S12 | LB2 | R/W(OTP) | 0 | Security Register 2 锁定位, OTP |
| S13 | LB3 | R/W(OTP) | 0 | Security Register 3 锁定位, OTP |
| S14 | CMP | R/W | 0 | 保护互补位 |
| S15 | SUS1 | R | 0 | Erase Suspend 状态(只读) |

### Write Method

- Write command: 01h(WRSR)
- Number of bytes: 1 或 2; **CS# 在第 8 位后拉高 = 只写低字节, 且 CMP/QE 被清 0**; 需要设置/保持 CMP 时必须写满 16 位
- WREN required: 是(06h); 或 50h 易失性写
- Volatile/Non-volatile: BP/CMP/QE/SRP/LB 为非易失位
- BUSY after write: WIP=1, 持续 tW(typ 2.6ms / max 4ms), 完成后 WEL 自动清 0
- Special restriction: 01h 对 S15/S10/S1/S0 无效; SRP 进入硬件保护或 OTP 模式后 WRSR 被拒绝

### Source

- Section: 6. STATUS REGISTER / 7.4
- Page: 11-12, 17

---

## 6. BUSY / WEL

- BUSY/WIP Register: Status Register 低字节(05h)
- BUSY/WIP Bit: S0 (0x01)
- Busy Value: 1
- Ready Value: 0
- WEL Register: Status Register 低字节(05h)
- WEL Bit: S1 (0x02)
- WEL Set Command: WREN(06h)
- WEL Clear Condition: Power-Up / WRDI / WRSR / PP / SE / SE1K / BE / CE 完成后自动清除

### Source

- Section: 5. DATA PROTECTION / 6
- Page: 9, 11

---

## 7. Program

- Page Program Command: 02h
- Address Bytes: 3
- Page Size: 256 Bytes
- Page Boundary Behavior: 写入超过当前页末尾的数据回卷到本页起始; 超过 256 字节只保证最后 256 字节正确
- Max Program Bytes: 256
- Typical Program Time: 1.1ms (tPP)
- Maximum Program Time: 1.6ms (tPP)
- WREN Required: 是
- BUSY Poll Required: 是(05h 读 WIP)

### Recommended Driver Sequence

```text
WREN(06h)
检查 WEL=1
Page Program(02h + 3 字节地址 + 数据)
CS High
轮询 WIP=0(超时按 max 1.6ms 以上)
```

### Source

- Section: 7.14 / 8.6
- Page: 25, 52

---

## 8. Erase

### Sector Erase

- Command: 20h
- Size: 4KB
- Address Alignment: 4KB 对齐(范围内任意地址定位到所在 sector)
- Typical Time: 2.6ms
- Maximum Time: 7.6ms (tSE)

### 1KB Sector Erase

- Command: 8Bh
- Typical/Maximum: 未单独列出, 未确认

### 32KB Block Erase

- Command: 52h
- Supported: 是
- Typical Time: 2.6ms
- Maximum Time: 7.6ms (tBE1)

### 64KB Block Erase

- Command: D8h
- Supported: 是
- Typical Time: 2.6ms
- Maximum Time: 7.6ms (tBE2)

### Chip Erase

- Command: C7h(或 60h, 等价)
- Typical Time: 5.2ms
- Maximum Time: **7.8ms (tCE)** — 整片擦除极快, 超时设 1s 已非常充裕
- Special Requirement: 仅在 (BP2,BP1,BP0)=000 且 CMP=0, 或 (BP2,BP1,BP0)=111 且 CMP=1 时执行; 有 sector 被保护则被忽略

### Source

- Section: 7.18-7.21 / 8.6
- Page: 28-30, 52

---

## 9. Address Mode

- Capacity requires >24-bit address: 否(1MB, 20 位地址足够)
- Power-up Default: 3-byte
- 3-byte Mode: 所有命令均为 3 字节地址
- 4-byte Mode: 不存在
- Enter/Exit 4-byte Command: 不存在
- Dedicated 4-byte Commands: 不存在

### Driver Decision

- Main Flash access method: 3 字节地址 Read(03h) / Page Program(02h)
- Reason: 容量 1MB, 无 4 字节地址需求; **GD55B02GF 模板中的 13h/0Ch/12h 命令本芯片不支持, 必须全部替换**

### Source

- Section: 7, Table2
- Page: 13-14

---

## 10. Protection

- BP Bits: BP4-BP0(S6-S2), 非易失
- TB: 无独立 TB 位(由 BP4=1 组合提供 Top/Bottom 4KB-32KB 小块保护, 见保护表)
- SEC: 无独立 SEC 位
- CMP: S14(35h 读出字节的 bit6 = 0x40)
- SRP: SRP0(S7), SRP1(S8); (1,1)=OTP 永久锁定, 驱动禁止使用
- WPS: 不存在
- WP# Pin Effect: SRP0=1 且 WP#=0 时状态寄存器锁定
- Default Protection: 出厂 SR=0000h, 无保护
- Status Register Write Restriction: 01h 单字节写会清 CMP/QE; 写 CMP 必须 16 位写

### Protection Table(CMP=0, 摘要)

| BP4 BP3 BP2 BP1 BP0 | Protected Range |
|---|---|
| X X 0 0 0 | 无 |
| 0 0 0 0 1 | Upper 1/16(0F0000h-0FFFFFh) |
| 0 0 0 1 0 | Upper 1/8 |
| 0 0 0 1 1 | Upper 1/4 |
| 0 0 1 0 0 | Upper 1/2 |
| 0 1 0 0 1 | Lower 1/16 |
| 0 1 0 1 0 | Lower 1/8 |
| 0 1 0 1 1 | Lower 1/4 |
| 0 1 1 0 0 | Lower 1/2 |
| 0 X 1 0 1 | ALL |
| X X 1 1 X | ALL |
| 1 0 0 0 1 - 1 0 1 0 X | Top 4KB-32KB |
| 1 1 0 0 1 - 1 1 1 0 X | Bottom 4KB-32KB |

CMP=1 时保护范围取反(完整表见 Datasheet Table1.0/1.1)。

### Unprotect Target

- Required register value/mask: SR1 低字节清 BP4-BP0(mask 0x7C); SR2 高字节清 CMP(mask 0x40)
- Bits that must be preserved: QE(S9), LB1-LB3(S11-S13, OTP), SRP1(S8); SUS 位只读不受 01h 影响

### Source

- Section: 5. DATA PROTECTION / 6
- Page: 9-12

---

## 11. Security / OTP

- Supported: 是
- Region Count: 3
- Region Size: 每个 1024 字节(含 4 个 256 字节页)
- Address: SR1: A15-A12=0001(1000h); SR2: 0010(2000h); SR3: 0011(3000h); A23-A16=00h, A11-A10=00, A9-A0=字节地址
- Read Command: 48h(3 字节地址 + 1 dummy)
- Program Command: 42h(3 字节地址, 按 256 字节页)
- Erase Command: 44h(3 字节地址)
- Lock Bits: LB1-LB3(S11-S13)
- Permanent Lock: 是, OTP, 一旦置 1 对应 Security Register 永久只读
- Notes: 烧录驱动不得写 LB 位

### Source

- Section: 7.32-7.34 / Table2 note8
- Page: 39-41, 15

---

## 12. UID / SFDP

### UID

- Supported: 是
- Command: 4Bh
- Address: 无
- Dummy: 4 字节
- Length: 128-bit(16 字节)

### SFDP

- Supported: 是
- Command: 5Ah
- Address: 3 字节
- Dummy: 1 字节

### Source

- Section: 7.29 / Table2
- Page: 37, 14

---

## 13. Reset / Power Management

- Deep Power Down: B9h, tDP=25us
- Release Command: ABh
- Release Delay: tRES1=25us(不读 ID)/ tRES2=25us(读 ID)
- Reset Enable: 66h
- Reset Command: 99h
- Reset Delay: tRST=30us(WRSR 后 4ms, Chip Erase 后 120us)
- Power-up Delay Before Read: tVSL max 500us
- Power-up Delay Before Program/Erase: 同上(VWI 1.5-2.5V 写抑制)

### Source

- Section: 7.22-7.23 / 8.1 / 8.6
- Page: 30-32, 47, 51-52

---

## 14. Timing Summary

| Item | Typical | Maximum | Driver Timeout |
|---|---:|---:|---:|
| Page Program (tPP) | 1.1ms | 1.6ms | 现有 400us 预延时 + 3000×5us 轮询 ≈15ms |
| Sector Erase (tSE, 4K) | 2.6ms | 7.6ms | - |
| 32KB Erase (tBE1) | 2.6ms | 7.6ms | - |
| 64KB Erase (tBE2) | 2.6ms | 7.6ms | - |
| Chip Erase (tCE) | 5.2ms | 7.8ms | ChipEraseTime×100×10ms, 取 1s |
| Status Register Write (tW) | 2.6ms | 4ms | 100×1ms 轮询 = 100ms |

---

## 15. Driver Development Notes

### IDCheck

- Recommended flow: 9Fh, 读 3 字节, 与上位机下发 ID 比较
- Expected ID: CDh-60h-14h
- Special requirement: 上电后等 tVSL(平台 PowerOn 已有 50ms 延时, 足够)

### Read

- Recommended command: 03h(主 Flash, 3 字节地址)
- Address width: 3
- Dummy: 0

### Program

- Page size: 256
- Polling: 05h 查 WIP

### Erase

- Preferred method: Chip Erase(C7h), 首次进入 Erase 时整片擦除
- Reason: tCE max 仅 7.8ms, 整片擦除代价极低, 与平台 Gang 框架一致
- 保护态处理: Erase 不解保护(UnProtect 为上位机独立任务); 擦除前检查 SR,
  仅 (BP2-BP0=000 且 CMP=0) 或 (BP2-BP0=111 且 CMP=1) 时发 CE, 否则报 E_EraseFail, 避免 CE 被芯片静默忽略造成假 PASS

### Protect / UnProtect

- Register: 单一 16 位 SR; BP4-BP0 在低字节(05h), CMP 在高字节 bit6(35h)
- Mask: 低字节 0x7C, 高字节 0x40
- Preserve bits: 写 01h 必须一次写满 2 字节; 驱动采用读-改-写, 高字节只改 CMP, 低字节只改 BP
- 出厂 SR=0000h; Gang 模式 RMW 读回值取自 A 槽, 默认各槽出厂状态一致

### Secure(LB 永久锁定)

- SRAM[0x01] bit3-bit5 = LB1/LB2/LB3 锁定选项(.spc S0001 区)
- 流程: 未选锁定项直接成功返回; 否则 RMW 读当前 SR, 01h 一次写 16 位(低字节原样写回, 高字节置位 LBx), 轮询 WIP, 35h 读回校验
- LB 为 OTP, 一旦置 1 对应 Security Register 永久只读; Program/Erase Security 前已有 LB 预检, 锁定后报 FAIL 而非假 PASS

### Multi-socket

- Known platform considerations: 沿用 GD55B02GF 模板的 ALL1000 Gang4 框架;
  Read 主 Flash 走 A 槽单读(平台原实现), Verify/BlankCheck 用 FPGA 多槽比较, 不改动

---

## 16. Datasheet Reference Index

| Topic | Section | Page |
|---|---|---:|
| JEDEC ID | 7.27 / ID Table | 36 / 15 |
| SPI Mode | 4 | 8 |
| Status Register | 6 | 11-12 |
| Protection | 5 | 9-10 |
| Page Program | 7.14 | 25 |
| Sector Erase | 7.18 | 28 |
| Block Erase | 7.19-7.20 | 29 |
| Chip Erase | 7.21 | 30 |
| Address Mode | 7 Table2 | 13-14 |
| Reset | 7.36(66h/99h) | Table2 p13 |
| AC Timing | 8.6 | 51-52 |
| Power-up | 8.1 | 47 |

---

## 17. Engineering Test Notes

> 此处只记录本项目实测,不将其等同于 Datasheet 规格。

- Test date: 未测
- Programmer: ALL1000
- Board/Socket: 未定
- SPI frequency: 未定
- Result: 未定
- Observation: 无

---

## 18. Unconfirmed Items

- 1KB Sector Erase(8Bh) 的独立擦除时间未在 AC 表单独列出(驱动未使用该命令)
- SPI 默认 Mode(Mode 0/3 均支持, 平台按 Mode 0 工作, 无影响)
