# MSPM0G3519 FreeRTOS（Keil）移植设计

## 目标与范围

在现有 `empty_LP_MSPM0G3519_nortos_keil` Keil 工程中接入 SDK 提供的
FreeRTOS 内核。首版只将 `main.c` 中的 TFT180 周期刷新循环迁移到一个
`DisplayTask`，显示内容、刷新周期和现有 `core` 驱动均保持不变。

本次不迁移到 CCS，不引入 TI-Drivers/POSIX，不拆分额外业务任务，也不改变
`empty.syscfg` 的外设配置。

## 依据与边界

- 目标器件：MSPM0G3519，Cortex-M0+。
- 当前可构建目标：`keil/empty_LP_MSPM0G3519_nortos_keil.uvprojx`。
- 参考 SDK：`examples/rtos/LP_MSPM0G3519/kernel/blink_led`，以及
  `kernel/freertos/builds/LP_MSPM0G3519/release/keil`。
- 现有启动文件的 `SVC_Handler`、`PendSV_Handler`、`SysTick_Handler` 是弱定义；
  接入 FreeRTOS 库后由内核提供实现，启动文件不修改。

## 构建设计

在原 Keil 工程内操作：

1. 保留当前 `main.c`、`ti_msp_dl_config.c`、TFT 源文件、散装加载文件和启动文件。
2. 加入 SDK 的 FreeRTOS 预编译库
   `freertos_builds_LP_MSPM0G3519_release_keil.lib`。
3. 增加 FreeRTOS 内核、Cortex-M0 移植层和 G3519 `release` 配置目录的包含路径。
4. 继续链接当前 DriverLib 库；不加入 TI-Drivers 库，避免把 DriverLib 工程改造成
   POSIX/TI-Drivers 工程。

SDK 默认配置使用抢占式调度、1 kHz tick、`heap_4`、动态分配、3 KB FreeRTOS 堆
和二级栈溢出检查。首版关闭 tickless idle；最终任务栈和堆大小按链接 map、
`uxTaskGetStackHighWaterMark()` 与 `xPortGetFreeHeapSize()` 实测调整。

## 程序执行路径

```text
Reset_Handler
  -> main()
     -> SYSCFG_DL_init() / TFT180 初始化
     -> xTaskCreate(DisplayTask)
     -> vTaskStartScheduler()
        -> DisplayTask: 原显示更新逻辑 -> vTaskDelay(100 ms) -> 循环
```

`DisplayTask` 是唯一创建的应用任务。它独占 TFT180 的显示 API；本阶段不存在其他
任务访问屏幕，故不增加互斥锁。原先的 `delay_cycles(CPUCLK_FREQ / 10)` 被
`vTaskDelay(pdMS_TO_TICKS(100))` 替代，避免 CPU 忙等。

## 故障处理与可观测性

- 实现 `vApplicationMallocFailedHook()`：任务创建或内核对象分配失败时停机供断点定位。
- 实现 `vApplicationStackOverflowHook()`：检测到任务栈溢出时停机供断点定位。
- 在调试版本保留 `configASSERT`；验证稳定后再决定发布配置。
- 调试阶段读取剩余堆和 `DisplayTask` 的栈高水位，作为后续配置的依据。

## 验收标准

1. no-RTOS Target 仍可编译，FreeRTOS Target 也能 Rebuild 且 0 Error、0 Warning。
2. 进入 `vTaskStartScheduler()` 后，断点/运行状态证明 `DisplayTask` 每 100 ms 运行。
3. 板上 TFT 的显示内容和刷新节奏与迁移前一致，无死机、花屏或持续复位。
4. 动态任务创建后尚有可接受的剩余 FreeRTOS 堆，`DisplayTask` 栈高水位非零且留有余量。
