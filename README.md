# ALL1000-IC-Support

用于支援过的 IC 记录,以及不同版本的代码管理。

## 目录结构

```text
ALL1000-IC-Support/
├─ common/          # 与具体 IC 无关、多颗 IC 共用的公共代码
│  └─ h/            #   器件驱动共同依赖的公共头(Type.h / Disp.h / ErrorCode.h …)
├─ protocols/       # 通信 / 烧录协议实现
│  ├─ swd/
│  ├─ spi/
│  ├─ i2c/
│  └─ jtag/
├─ devices/         # 具体某颗 IC 的支援代码,按 厂商/器件 组织
│  ├─ st/
│  │  ├─ stm32g030f6p6/
│  │  └─ stm32g071k8t3/
│  ├─ nxp/
│  │  └─ s32k324/
│  ├─ micron/
│  │  └─ n25q256/
│  ├─ st-memory/
│  │  └─ m25pe40/
│  ├─ onsemi/
│  │  └─ cat25256/
│  ├─ puya/
│  │  └─ dp25q40sh/
│  └─ tsingteng/
│     └─ th25q-80hb/
├─ fpga/            # 烧录器本体 FPGA 固件(ISE 工程)
│  ├─ h3v3so16ug4a3/
│  └─ h3v3so8ug4a3/
├─ templates/       # 以后新增 IC 时复制的标准模板
│  ├─ arm_swd_device/
│  └─ spi_flash_device/
├─ docs/            # 开发文档
├─ CHANGELOG.md
└─ README.md
```

## 器件目录约定

- 路径形如 `devices/<厂商>/<器件型号>`,厂商与型号目录名均使用小写。
- 每个器件目录包含该 IC 的驱动 `.c` / `.h` 与规格 `.spc` 文件,以及对应文档与构建脚本。
- 原目录名中的创建人前缀(`WA` / `WJ` / `AL` / `SY` 等)不参与归并,以器件型号为准。

## 公共代码

器件驱动的 `.c` 文件通过 `#include "h/Type.h"` 等引用公共代码,对应物理位置为
`common/h/`。编译时需把 `common/` 加入头文件搜索路径,使 `h/xxx.h` 解析到
`common/h/xxx.h`。
