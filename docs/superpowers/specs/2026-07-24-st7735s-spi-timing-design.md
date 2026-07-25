# ST7735S SPI 时序稳健性设计

## 目标

在不改动 ST7735S 初始化命令、显示坐标偏移和接线定义的前提下，降低 SPI 时钟并让 DC 信号在命令发送前稳定，验证花屏是否由时序裕量不足导致。

## 修改范围

- 只修改 `core/src/zf_device_tft180.c`。
- 在 `tft180_init()` 的首次 SPI 传输前，将 SPI0 分频设置为 39，对应 80 MHz 主时钟下的约 1 MHz SPI。
- 在 `tft180_write_index()` 中，DC 拉低和拉高后各插入一个极短的空操作延迟。
- 不修改 `empty.syscfg`，避免手写改动 SysConfig 生成配置。

## 验证

1. Keil Rebuild All 必须显示 `0 Error(s), 0 Warning(s)`。
2. 烧录到同一块 ST7735S 硬件，观察整屏是否为白底、红色字符，且不再花屏。
3. 若仍花屏，保留本次改动作为 1 MHz 对照，再采集 DC、SCLK、MOSI 波形或执行四色整屏诊断；不混入另一套初始化表。
