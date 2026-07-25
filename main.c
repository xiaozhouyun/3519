/*
* MSPM0G3519 TFT180 LCD 直观中文字符串与全功能测试主程序
*/

#include "ti_msp_dl_config.h"
#include "core/inc/zf_device_tft180.h"

int main(void)
{
    // 1. 系统底层与屏驱动初始化
    SYSCFG_DL_init();
    tft180_init();

    // ------------------- 第一阶段：界面标题与分割线 -------------------
    tft180_clear();
    
    // 英文标题 (居中)
    tft180_set_color(RGB565_BLUE, RGB565_WHITE);
    tft180_set_font(TFT180_8X16_FONT);
    tft180_show_string(4, 2, "MSPM0G3519 TEST");

    // 分割线
    tft180_draw_line(0, 20, 127, 20, RGB565_RED);
    tft180_draw_line(0, 21, 127, 21, RGB565_RED);

    // ------------------- 第二阶段：直观中文字符串 API 测试 -------------------
    // 1. 直接传递中文字符串 "中文显示"
    tft180_show_chinese_str(32, 26, "中文显示", RGB565_RED);

    // 2. 支持中英文、数字混合排列 "电赛测试: OK"
    tft180_show_chinese_str(16, 46, "电赛测试: OK", RGB565_BLUE);

    // ------------------- 第三阶段：数值与浮点显示测试 -------------------
    tft180_set_font(TFT180_8X16_FONT);

    // 浮点数显示
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    tft180_show_string(0, 68, "FLOAT:");
    tft180_set_color(RGB565_PURPLE, RGB565_WHITE);
    tft180_show_float(48, 68, 3.14159, 1, 5);

    // 分割线
    tft180_draw_line(0, 88, 127, 88, RGB565_GRAY);
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    tft180_show_string(0, 96, "CNT :");

    // ------------------- 第四阶段：主循环动态刷新 -------------------
    uint32_t count = 0;
    while (1) {
        // 动态刷新递增计数器
        tft180_set_color(RGB565_RED, RGB565_WHITE);
        tft180_set_font(TFT180_8X16_FONT);
        tft180_show_uint(48, 96, count, 6);

        // 右下角动态指示点
        uint16_t dot_color = (count % 2 == 0) ? RGB565_GREEN : RGB565_YELLOW;
        tft180_draw_point(110, 104, dot_color);
        tft180_draw_point(111, 104, dot_color);
        tft180_draw_point(110, 105, dot_color);
        tft180_draw_point(111, 105, dot_color);

        count++;
        
        // 延时刷新
        delay_cycles(CPUCLK_FREQ / 10);
    }
}