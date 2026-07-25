/*
* MSPM0G3519 TFT180 LCD 直观中文字符串与全功能测试主程序
*/

#include "ti_msp_dl_config.h"
#include "core/inc/zf_device_tft180.h"
#include "FreeRTOS.h"
#include "task.h"

#define DISPLAY_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE * 2U)
#define DISPLAY_TASK_PRIORITY      (tskIDLE_PRIORITY + 1U)

static void DisplayTask(void *pvParameters)
{
  

    (void)pvParameters;

    while (1) {
     
        vTaskDelay(pdMS_TO_TICKS(100U));
    }
}

int main(void)
{
    // 1. 系统底层与屏驱动初始化
    SYSCFG_DL_init();
    tft180_init();

    // ------------------- 第一阶段：界面标题与分割线 -------------------
    tft180_clear();
    
    

    if (xTaskCreate(DisplayTask, "Display", DISPLAY_TASK_STACK_SIZE, NULL,
            DISPLAY_TASK_PRIORITY, NULL) != pdPASS) {
        while (1) {
        }
    }

    vTaskStartScheduler();

    while (1) {
    }
}
