# PB31 按键启动设计

## 目标

PB31 下降沿触发后，执行与蓝牙 `S` 命令相同的启动动作：清零运行计时并将 `g_bt_running_flag` 置为 1。

## 设计

- `blue.c/.h` 新增 `Key_Start()`；其实现只调用已有 `BT_Start()`。
- PB31 与现有 PB24 共用 `GPIOB_INT_IRQn`，因此只扩展已有的 `GROUP1_IRQHandler()`，不新增第二个同名中断函数。
- PB31 分支先清除 `key_user_key_PIN` 中断标志，再调用 `Key_Start()`。
- 保留 PB24 的 IMU 中断处理，不改蓝牙、循迹、电机或 FreeRTOS 任务。

## 边界与验证

- 本次按键只启动，不实现再次按下停止，也不增加软件去抖。
- 按键抖动最多重复调用幂等的启动动作；不会重复创建任务或直接写电机。
- 静态检查应确认 PB31 分支清中断并调用 `Key_Start()`，且 `Key_Start()` 复用 `BT_Start()`。
- Keil 编译与板上按键验证由用户执行：按下 PB31 后 `g_bt_running_flag` 应为 1，底盘进入循迹控制。
