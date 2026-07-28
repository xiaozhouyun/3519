# ICM-42686-P Driver Identity Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 使现有 SPI 六轴驱动识别并初始化 ICM-42686-P，同时保持上层调用接口不变。

**Architecture:** 保留 `icm42688p.*` 的文件名和函数名，防止影响 `main.c` 与 Keil 工程成员；只把硬件身份常量、注释和静态检查期望改为 ICM-42686-P。采样地址、SPI 通讯和姿态算法均不变。

**Tech Stack:** C11、TI MSPM0 DriverLib、Keil、PowerShell 静态检查。

## Global Constraints

- ICM-42686-P 的 `WHO_AM_I` 固定值为 `0x44U`。
- 保留现有 ±16 g、±2000 dps 量程及 1 kHz 配置字 `0x06U`。
- 不修改 `main.c`、Keil 工程文件或已有用户改动。

---

### Task 1: 更新静态集成契约

**Files:**
- Modify: `tests/verify_icm42688p.ps1`

**Interfaces:**
- Consumes: `ICM42688P_WHO_AM_I_VALUE`
- Produces: ICM-42686-P 的静态集成断言

- [ ] **Step 1: 写入失败断言**

将测试中的身份值断言改为：

```powershell
if ($header -notmatch 'ICM42688P_WHO_AM_I_VALUE\s*\(0x44U\)') { throw 'WHO_AM_I 期望值不是 0x44' }
```

- [ ] **Step 2: 运行检查并确认失败**

Run: `powershell -ExecutionPolicy Bypass -File tests/verify_icm42688p.ps1`

Expected: FAIL，原因是现有头文件仍定义 `0x47U`。

### Task 2: 最小驱动身份切换

**Files:**
- Modify: `core/inc/icm42688p.h`
- Modify: `core/src/icm42688p.c`

**Interfaces:**
- Consumes: `icm42688p_init(void)`, `icm42688p_update(float)`
- Produces: 仍由原接口调用、但接受 ICM-42686-P 的驱动

- [ ] **Step 1: 修改芯片身份和注释**

```c
/* ICM-42686-P 的 WHO_AM_I 固定返回值。 */
#define ICM42688P_WHO_AM_I_VALUE (0x44U)
```

将源文件的芯片名称注释替换为 `ICM-42686-P`，不改寄存器地址和初始化配置字。

- [ ] **Step 2: 运行静态检查并确认通过**

Run: `powershell -ExecutionPolicy Bypass -File tests/verify_icm42688p.ps1`

Expected: `ICM-42686-P 静态集成检查通过`。

### Task 3: 编译与工程归属检查

**Files:**
- Verify: `keil/empty_LP_MSPM0G3519_nortos_keil.uvprojx`

**Interfaces:**
- Consumes: Keil 项目中的 `icm42688p.c` 成员
- Produces: 编译或明确的环境限制记录

- [ ] **Step 1: 确认项目继续引用驱动文件**

Run: `rg -n 'icm42688p\.c' keil/empty_LP_MSPM0G3519_nortos_keil.uvprojx`

Expected: 返回 `core/src/icm42688p.c` 条目。

- [ ] **Step 2: 使用 Keil 命令行重建**

Run: `D:\keil5\UV4\UV4.exe -b keil\empty_LP_MSPM0G3519_nortos_keil.uvprojx -j0`

Expected: Keil 编译日志中无错误，并产出更新后的目标文件。
