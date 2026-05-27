# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 提供在本仓库中工作的指导。

## 概述

U-Boot 是一款面向嵌入式板卡（ARM、RISC-V、PowerPC、x86、MIPS 等）的引导加载器。它采用 Linux 内核风格的 Kconfig 构建系统，支持 200 多个板卡厂商。

### 本项目特定信息（DFD SoC）

本仓库为 **DFD SoC** 的定制化 U-Boot，目标平台为 **ARM64**，主要在 QEMU 环境中运行验证。

关键平台参数：

| 参数 | 值 | 说明 |
|------|-----|------|
| `SPL_TEXT_BASE` | `0xFFF01000` | SPL 加载地址 |
| `TEXT_BASE` | `0x200000` | U-Boot proper 加载地址 |
| `SYS_LOAD_ADDR` | `0x880000` | 内核默认加载地址 |
| `DEBUG_UART_BASE` | `0xF1800000` | PL011 debug UART 基地址 |
| `GICD_BASE` | `0xF0001000` | GIC Distributor 基地址 |
| `GICC_BASE` | `0xF0002000` | GIC CPU Interface 基地址 |
| DDR 范围 | `0x0` ~ `0xF0000000` | 3.75GB 普通内存 |
| IO 范围 | `0xF0000000` ~ `0x100000000` | 设备内存区域 |

**重要设计决策**（来自最近两次 commit）：

- **GIC 在 U-Boot 中禁用**：`GICV2` 配置被注释掉。原因：QEMU 的 GICv2 如果在 U-Boot 中初始化，Linux 内核启动后将无法上报中断。因此 GIC 初始化留给内核自己处理。
- **U-Boot 切换到 EL1**：启用 `CONFIG_ARMV8_SWITCH_TO_EL1=y`，U-Boot 在启动 Linux 前从 EL3 切换到 EL1。
- **启动流程**：SPL 从 `0xFFF01000` 运行，初始化 UART 和 DRAM 后，加载 U-Boot proper 到 `0x200000`，然后跳转到 U-Boot。U-Boot 从 `spl_board_prepare_for_boot()` 切换到 EL1 后启动内核。
- **自定义工具**：`tools/dfd_image.c` 用于生成 DFD 平台专用镜像；`dfd-build.sh` 是项目构建脚本。

## 构建系统

### DFD SoC 构建

本项目当前只支持 `dfd` 板：

```bash
# 快速构建（项目根目录）
make dfd_defconfig
make -j$(nproc)

# 或使用项目脚本
./dfd-build.sh

# 清理
make clean        # 移除生成的文件，保留配置
make mrproper     # 移除所有生成的文件 + 配置
make distclean    # mrproper + 移除编辑器备份和补丁文件
```

构建产物：
- `spl/u-boot-spl.bin` — SPL 二进制，从 `0xFFF01000` 加载
- `u-boot.bin` / `u-boot-dtb.bin` — U-Boot proper，加载到 `0x200000`
- `u-boot.dtb` — DFD 设备树
- `tools/dfd_image` — 自定义镜像生成工具

### 关键构建目标

| 目标 | 说明 |
|--------|-------------|
| `all` / `u-boot` | 构建主 U-Boot 二进制文件 |
| `u-boot.bin` | 原始二进制输出 |
| `u-boot.img` | 用于 boot ROM 的包装镜像 |
| `u-boot.dtb` | 设备树 blob |
| `spl/u-boot-spl.bin` | SPL（Secondary Program Loader）二进制 |
| `envtools` | 仅主机端环境工具 |
| `dtbs` | 构建设备树 |

## 代码风格与检查

U-Boot 遵循 Linux 内核编码风格：

```bash
# Checkpatch（发送补丁前必须执行）
scripts/checkpatch.pl --strict patch.file
scripts/checkpatch.pl -f file.c          # 直接检查文件

# 使用 clang-format 自动格式化（项目有 .clang-format 配置）
clang-format -i file.c

# 栈使用分析
make checkstack
```

`.checkpatch.conf` 为 checkpatch 配置了 U-Boot 特有的忽略项（例如 `--u-boot`、`--no-tree`）。

## 架构

### 启动阶段

U-Boot 可以构建为多个阶段：

| 阶段 | 用途 | 配置前缀 |
|-------|---------|---------------|
| TPL | Tertiary Program Loader（非常早期，体积很小） | `CONFIG_TPL_` |
| SPL | Secondary Program Loader（加载主 U-Boot） | `CONFIG_SPL_` |
| VPL | Verified Program Loader | `CONFIG_VPL_` |
| U-Boot proper | 主引导加载器 | （默认） |

SPL/TPL 从同一份源码树构建，定义了 `CONFIG_XPL_BUILD`。它们通过条件编译共享代码。`spl/` 目录存放构建输出，不是源码。

### 初始化序列

`common/` 中有两个主要的初始化路径：

1. **`board_init_f()`** (`common/board_f.c`)：在 DRAM 可用之前的早期初始化。运行 `initcall_run_f()`，这是一个硬编码的有序 init 调用列表（参见 `include/initcall.h` 中的 `INITCALL()` 宏）。关键步骤：`arch_cpu_init`、`board_early_init_f`、`dram_init`、`serial_init`、`fdtdec_setup`。

2. **`board_init_r()`** (`common/board_r.c`)：重定位到 RAM 后的后期初始化。运行 `initcall_run_r()`，步骤包括 `initr_reloc`、`initr_malloc`、`initr_dm`、`initr_env`、`initr_net`，然后进入 `run_main_loop()` → `main_loop()`。

### DFD SoC 启动流程

DFD 平台采用 SPL → U-Boot proper 两阶段启动：

```
ROM/SRAM (0xFFF01000)
    ↓
SPL: board_init_f()        [arch/arm/mach-dfd/spl.c]
    - check_cpu_boot_mode() 打印当前 Exception Level
    - spl_early_init()      初始化设备树和驱动模型
    - preloader_console_init()  初始化 PL011 debug UART
    - TODO: 时钟树和 DRAM 初始化（当前未完整实现）
    ↓
SPL: spl_board_init()      板级 SPL 初始化
    ↓
SPL: spl_board_prepare_for_boot()  [跳转到 U-Boot proper 前]
    - 再次打印 CPU boot mode
    ↓
U-Boot proper (0x200000)
    ↓
U-Boot: board_init_r()     标准 U-Boot 初始化序列
    - initr_dm()            驱动模型初始化
    - initr_env()           环境变量初始化
    - initr_net()           网络初始化
    ↓
U-Boot: board_cleanup_before_linux()
    - CONFIG_ARMV8_SWITCH_TO_EL1 生效，从 EL3 切换到 EL1
    - 打印最终 Exception Level
    ↓
启动 Linux 内核
```

**调试辅助**：`arch/arm/mach-dfd/common.c` 中 `check_cpu_boot_mode()` 会读取 `CurrentEL` 系统寄存器并打印当前 EL，在 SPL、U-Boot init、U-Boot cleanup 三个阶段都会调用，用于追踪 CPU 模式切换。

**内存映射** (`arch/arm/mach-dfd/board.c`)：
- DDR: `0x0` ~ `0xF0000000` (3.75GB, `MT_NORMAL | INNER_SHARE`)
- IO: `0xF0000000` ~ `0x100000000` (256MB, `MT_DEVICE_NGNRNE | NON_SHARE | PXN | UXN`)

### 驱动模型（DM）

U-Boot 使用统一的驱动模型（`include/dm/`）。三个核心概念：

- **uclass** (`struct uclass`, `include/dm/uclass.h`)：功能类别（例如 `UCLASS_GPIO`、`UCLASS_BLK`、`UCLASS_SERIAL`）。定义公共 API。
- **driver** (`struct driver`, `include/dm/device.h`)：具体的驱动实现。通过 `U_BOOT_DRIVER()` 宏注册。
- **udevice** (`struct udevice`)：绑定到驱动并附加到 uclass 的设备实例。

驱动通过设备树节点（当启用 `CONFIG_OF_CONTROL` 时）或平台数据绑定到设备。`dm/` 子系统管理 probe/remove 生命周期。

关键文件：
- `drivers/core/` - DM 核心（root.c、device.c、uclass.c）
- `include/dm/uclass-id.h` - uclass ID 枚举
- `test/dm/` - DM 单元测试

### 命令系统

命令位于 `cmd/`，通过宏注册：

```c
U_BOOT_CMD(name, maxargs, repeatable, cmd_fn, "usage", "help")
U_BOOT_CMD_WITH_SUBCMDS(name, desc, help, subcmds)
```

命令在构建时收集到链接器列表中。`cmd_tbl_t` 是命令表结构。`common/command.c` 负责分发。

### 全局数据（`gd`）

`gd_t` (`include/asm-generic/global_data.h`) 是一个全局结构体，保存关键的启动时状态。它在非常早期就可用（在 `malloc` 之前），按架构存储在固定寄存器或内存位置中。板级代码通过 `gd` 宏访问它。关键字段：`gd->ram_size`、`gd->bd`、`gd->fdt_blob`、`gd->flags`。

### binman

`tools/binman/` 是一个 Python 工具，根据设备树描述将多个入口（U-Boot proper、SPL、设备树等）组装成 U-Boot 镜像。它在构建过程中被调用。关键模块：`tools/binman/control.py`。

### 设备树

U-Boot 大量使用设备树：
- `dts/` - 构建期间编译的设备树源码
- `include/dt-bindings/` - DT binding 头文件
- `fdtdec_setup()` 在 `board_init_f()` 早期解析 DT blob
- `CONFIG_OF_CONTROL` 启用 DT 支持
- `CONFIG_OF_LIVE` 构建活树（struct ofnode），除扁平 blob 之外

## 目录结构

| 目录 | 内容 |
|-----------|----------|
| `arch/` | 架构相关代码（arm/、riscv/、x86/、sandbox/ 等） |
| `board/` | 板卡相关代码，按厂商组织（`board/<vendor>/<board>/`） |
| `boot/` | 启动镜像格式和协议 |
| `cmd/` | Shell 命令（约 285 个命令） |
| `common/` | 核心初始化（`board_f.c`、`board_r.c`）、主循环、控制台 |
| `configs/` | 板卡默认 `*_defconfig` 文件 |
| `drivers/` | 设备驱动（约 79 个子系统：gpio、mmc、net、usb 等） |
| `dts/` | 设备树构建规则和内部 DTB |
| `fs/` | 文件系统实现 |
| `include/` | 头文件（`include/asm/` 为架构头文件，`include/dm/` 为 DM 头文件） |
| `lib/` | 通用工具函数（CRC、字符串、压缩、加密） |
| `net/` | 网络协议栈（TFTP、DHCP、NFS 等） |
| `scripts/` | 构建脚本，包括 `checkpatch.pl`、`dtc/`、Kconfig |
| `spl/` | SPL 的构建输出目录（不是源码） |
| `test/` | 单元测试：C 测试在子目录中，Python 测试在 `test/py/` |
| `tools/` | 主机工具：`mkimage`、`binman`、`fdtgrep`、`dumpimage`、`dfd_image` |

### DFD SoC 关键文件

| 文件 | 说明 |
|------|------|
| `arch/arm/mach-dfd/board.c` | 板级初始化：`board_init()`、`dram_init()`、`board_late_init()`、内存映射 `dfd_mem_map[]` |
| `arch/arm/mach-dfd/spl.c` | SPL 入口：`board_init_f()`、`spl_boot_device()`（固定返回 `BOOT_DEVICE_MMC1`） |
| `arch/arm/mach-dfd/common.c` | 公共辅助函数：`check_cpu_boot_mode()` 读取并打印当前 Exception Level |
| `arch/arm/dts/dfd.dts` | DFD 平台设备树：4x Cortex-A53、`memory@0` (2GB)、PL011 serial、dfd SDHCI |
| `drivers/mmc/dfd_sdhci.c` | DFD 平台自定义 SDHCI MMC 驱动 |
| `tools/dfd_image.c` | 自定义镜像打包工具，生成 DFD 平台专用启动镜像 |
| `configs/dfd_defconfig` | DFD 板默认配置 |
| `include/configs/dfd.h` | 板级头文件：GIC 基地址、额外环境变量（`dfd_boot` → `mmcboot`） |
| `dfd-build.sh` | 项目构建脚本（`-d` defconfig / `-b` build / `-m` menuconfig / `-c` clean） |

## 关键模式

### 添加新命令

1. 创建 `cmd/mycommand.c`
2. 实现 `do_mycommand()` 并用 `U_BOOT_CMD()` 注册
3. 在 `cmd/Makefile` 中添加 `obj-$(CONFIG_CMD_MY COMMAND) += mycommand.o`
4. 在 `cmd/Kconfig` 中添加 `config CMD_MY COMMAND`

### 添加新驱动

1. 在合适的 `drivers/<subsystem>/` 中创建驱动文件
2. 定义 `U_BOOT_DRIVER()`，设置 `.id = UCLASS_XXX`
3. 为 uclass 实现 ops 结构体
4. 添加 Kconfig 选项和 Makefile 条目
5. 如果使用设备树，添加 DT binding 文档

### 添加新板卡

1. 添加 `configs/<board>_defconfig`
2. 在 `board/<vendor>/<board>/` 中添加板卡文件
3. 在 `arch/<arch>/dts/<board>.dts`（或 dts/upstream/）中添加设备树
4. 更新 `arch/<arch>/Kconfig` 或板卡 Kconfig 以包含该板卡

## 重要注意事项

- **SPL 代码必须注意体积**：SPL 在有限的 SRAM 中运行。使用 `CONFIG_IS_ENABLED(FOO)` 进行跨阶段的条件编译。
- **不要在 `board_init_f()` 中使用 `malloc()`**：使用 `CONFIG_SYS_MALLOC_F_LEN` 作为早期 malloc，或使用静态缓冲区。
- **链接器列表**：U-Boot 使用链接器生成的列表（`include/linker_lists.h`）来注册命令、驱动、initcall 和其他模式。
- **重定位**：U-Boot 将自己从 flash 复制到 RAM。二进制中的地址会被修正。`gd->relocaddr` 保存目标地址。
- **环境变量**：存储在 flash/SD/eMMC 等介质中。通过 `env_get()`/`env_set()` 访问。源码在 `env/`。

### DFD SoC 注意事项

- **GIC 禁用**：`arch/arm/mach-dfd/Kconfig` 中 `GICV2` 被注释掉。QEMU 的 GICv2 如果在 U-Boot 中初始化，Linux 内核将无法接收中断。因此 GIC 初始化完全留给内核处理。
- **EL1 切换**：`CONFIG_ARMV8_SWITCH_TO_EL1=y` 启用。U-Boot 在 `board_cleanup_before_linux()` 中从 EL3 切换到 EL1，然后启动内核。可用 `check_cpu_boot_mode()` 追踪各阶段的 EL 状态。
- **SPL 启动设备固定**：`spl_boot_device()` 始终返回 `BOOT_DEVICE_MMC1`，从 MMC/SD 卡加载 U-Boot。
- **DRAM 初始化未完整实现**：`spl.c` 中的 `/* TODO intialize dram */` 注释表明 DRAM 初始化尚未完成。当前 `dram_init()` 通过 `fdtdec_setup_memory_banksize()` 和 `fdtdec_setup_mem_size_base()` 从设备树获取内存信息。
- **自定义 binman 配置**：`dfd.dts` 中的 `&binman` 节点定义了两个输出：
  - `dfd-u-boot-spl.bin`：SPL 镜像，使用 `mkimage -T dfd` 打包
  - `u-boot.itb`：FIT 格式镜像，包含 `u-boot-nodtb.bin` + `u-boot.dtb`
- **构建脚本**：`dfd-build.sh` 使用 `bear` 生成 `compile_commands.json`，便于 LSP/IDE 代码补全。
