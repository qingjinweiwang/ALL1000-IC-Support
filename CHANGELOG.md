# Changelog

## 2026-09-21

- 初始化仓库目录结构(`common` / `protocols` / `devices` / `templates` / `docs`)。
- 将本地平铺的 IC 支撑目录按 `devices/<厂商>/<器件>` 归并,共 8 颗器件:
  stm32g030f6p6、stm32g071k8t3、s32k324、n25q256、m25pe40、cat25256、dp25q40sh、th25q-80hb。
- 抽取公共头文件到 `common/h/`(25 个,源自主固件 `All1000Code/h`)。
- 新增 `fpga/` 顶层目录,收纳烧录器本体 FPGA 固件(h3v3so16ug4a3 / h3v3so8ug4a3)。
