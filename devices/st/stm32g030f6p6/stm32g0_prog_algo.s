; ================================================================
;  STM32G030F6P6 Flash 烧录算法 (Thumb-1, Cortex-M0+)
;  编译: arm-none-eabi-as -mthumb -mcpu=cortex-m0plus -o algo.o stm32g0_prog_algo.s
;        arm-none-eabi-objcopy -O binary algo.o algo.bin
; ================================================================
    .syntax unified
    .cpu    cortex-m0plus
    .thumb

; ---- 常量定义 ---------------------------------------------------
.equ  FLASH_SR,   0x40022010   ; Flash 状态寄存器
.equ  FLASH_CR,   0x40022014   ; Flash 控制寄存器
.equ  PARAM_BASE, 0x20000000   ; 参数块基址

; ---- 参数块结构 (0x20000000) ------------------------------------
;  [0x00]: flash_addr  (R0)
;  [0x04]: data_addr   (R1)
;  [0x08]: word_count  (R2)
;  [0x0C]: status      (输出: 0=成功, 非0=错误码)

; ---- 算法入口 (0x20000020) --------------------------------------

    ; --- 加载参数 ---
    ldr  r0, =PARAM_BASE
    ldr  r1, [r0, #0]       ; R1 = flash_addr
    ldr  r2, [r0, #4]       ; R2 = data_addr
    ldr  r3, [r0, #8]       ; R3 = word_count
    ldr  r4, =FLASH_SR      ; R4 = FLASH_SR 地址
    ldr  r5, =FLASH_CR      ; R5 = FLASH_CR 地址
    ldr  r6, =PARAM_BASE    ; R6 = 参数块基址 (输出用)

loop:
    ; --- 检查是否完成 ---
    cmp  r3, #0
    beq  done

wait_bsy:
    ; --- 等待 Flash 空闲 ---
    ldr  r7, [r4]
    lsls r7, r7, #15        ; 移位 bit16(BSY) 到 bit31(符号位)
    bmi  wait_bsy

    ; --- 置 PG 位 ---
    ldr  r7, [r5]
    movs r0, #1
    orrs r7, r7, r0
    str  r7, [r5]

    ; --- 写第一个字 ---
    ldr  r7, [r2]
    str  r7, [r1]
    adds r1, r1, #4
    adds r2, r2, #4

    ; --- 写第二个字 ---
    ldr  r7, [r2]
    str  r7, [r1]
    adds r1, r1, #4
    adds r2, r2, #4

wait_bsy2:
    ; --- 等待 Flash 空闲 ---
    ldr  r7, [r4]
    lsls r7, r7, #15
    bmi  wait_bsy2

    ; --- 检查错误 ---
    ldr  r7, [r4]
    movs r0, #0xF8
    ands r7, r7, r0         ; R7 = SR & 0xF8
    bne  error

    ; --- 清 PG 位 ---
    ldr  r7, [r5]
    movs r0, #1
    bics r7, r7, r0
    str  r7, [r5]

    ; --- 计数器减 2，继续循环 ---
    subs r3, r3, #2
    b    loop

done:
    ; --- 成功: status = 0 ---
    movs r0, #0
    str  r0, [r6, #12]      ; [PARAM_BASE+0x0C] = 0
    bkpt #0

error:
    ; --- 失败: 存错误码，清 SR，锁定 Flash ---
    str  r7, [r6, #12]      ; [PARAM_BASE+0x0C] = 错误码
    str  r7, [r4]           ; FLASH_SR = 错误码 (清标志)
    ldr  r7, [r5]           ; 读 CR
    ldr  r0, =0x80000000    ; LOCK 位
    orrs r7, r7, r0
    str  r7, [r5]           ; 写 LOCK=1
    bkpt #0

; ---- 字面量池 (PC-relative 加载目标) ----------------------------
    .align 2
    .ltorg

    .end
