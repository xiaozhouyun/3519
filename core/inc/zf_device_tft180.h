/*********************************************************************************************************************
* MSPM0G3519 TFT180 LCD Driver (TI Official DriverLib)
* 
* 接线定义（基于 ti_msp_dl_config.h）：
*                   ------------------------------------
*                   模块管脚             单片机管脚 (ti_msp_dl_config.h)
*                   SCL                  PB3 (SPI0_SCLK)
*                   SDA                  PB2 (SPI0_PICO)
*                   RES                  PB23 (spi_0_OLED_RES)
*                   DC                   PC8  (spi_0_OLED_DC)
*                   CS                   PC9  (spi_0_OLED_CS)
*                   BL                   PA30 (spi_0_OLED_BLK)
*                   VCC                  3.3V电源
*                   GND                  电源地
*                   最大分辨率 160*128
*                   ------------------------------------
********************************************************************************************************************/

#ifndef _zf_device_tft180_h_
#define _zf_device_tft180_h_

#include "ti_msp_dl_config.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef uint8
typedef uint8_t uint8;
#endif
#ifndef uint16
typedef uint16_t uint16;
#endif
#ifndef uint32
typedef uint32_t uint32;
#endif
#ifndef int8
typedef int8_t int8;
#endif
#ifndef int16
typedef int16_t int16;
#endif
#ifndef int32
typedef int32_t int32;
#endif

// RGB565 颜色宏定义
#define RGB565_WHITE                    0xFFFF
#define RGB565_BLACK                    0x0000
#define RGB565_RED                      0xF800
#define RGB565_GREEN                    0x07E0
#define RGB565_BLUE                     0x001F
#define RGB565_YELLOW                   0xFFE0
#define RGB565_GRAY                     0x8410
#define RGB565_PURPLE                   0xF81F

// 控制引脚宏定义 (基于 ti_msp_dl_config.h)
#define TFT180_DC(x)                    ((x) ? DL_GPIO_setPins(TFT_TFT_DC_PORT, TFT_TFT_DC_PIN) : DL_GPIO_clearPins(TFT_TFT_DC_PORT, TFT_TFT_DC_PIN))
#define TFT180_RST(x)                   ((x) ? DL_GPIO_setPins(TFT_TFT_RES_PORT, TFT_TFT_RES_PIN) : DL_GPIO_clearPins(TFT_TFT_RES_PORT, TFT_TFT_RES_PIN))
#define TFT180_CS(x)                    do { \
                                            if (x) { \
                                                while (!DL_SPI_isTXFIFOEmpty(TFT_SPI0_INST) || DL_SPI_isBusy(TFT_SPI0_INST)); \
                                                DL_GPIO_setPins(TFT_TFT_CS_PORT, TFT_TFT_CS_PIN); \
                                            } else { \
                                                DL_GPIO_clearPins(TFT_TFT_CS_PORT, TFT_TFT_CS_PIN); \
                                            } \
                                        } while(0)
#define TFT180_BLK(x)                   ((x) ? DL_GPIO_setPins(TFT_TFT_BLK_PORT, TFT_TFT_BLK_PIN) : DL_GPIO_clearPins(TFT_TFT_BLK_PORT, TFT_TFT_BLK_PIN))

#define TFT180_DEFAULT_DISPLAY_DIR      ( TFT180_PORTAIT   )                    // 默认的显示方向
#define TFT180_DEFAULT_PENCOLOR         ( RGB565_RED       )                    // 默认的画笔颜色
#define TFT180_DEFAULT_BGCOLOR          ( RGB565_WHITE     )                    // 默认的背景颜色
#define TFT180_DEFAULT_DISPLAY_FONT     ( TFT180_8X16_FONT )                    // 默认的字体模式

typedef enum
{
    TFT180_PORTAIT                      = 0,                                    // 竖屏模式
    TFT180_PORTAIT_180                  = 1,                                    // 竖屏模式  旋转180
    TFT180_CROSSWISE                    = 2,                                    // 横屏模式
    TFT180_CROSSWISE_180                = 3,                                    // 横屏模式  旋转180
}tft180_dir_enum;

typedef enum
{
    TFT180_6X8_FONT                     = 0,                                    // 6x8      字体
    TFT180_8X16_FONT                    = 1,                                    // 8x16     字体
    TFT180_16X16_FONT                   = 2,                                    // 16x16    字体 目前不支持
}tft180_font_size_enum;

void    tft180_init                     (void);
void    tft180_clear                    (void);
void    tft180_full                     (const uint16 color);
void    tft180_set_dir                  (tft180_dir_enum dir);
void    tft180_set_font                 (tft180_font_size_enum font);
void    tft180_set_color                (const uint16 pen, const uint16 bgcolor);
void    tft180_draw_point               (uint16 x, uint16 y, const uint16 color);
void    tft180_draw_line                (uint16 x_start, uint16 y_start, uint16 x_end, uint16 y_end, const uint16 color);

void    tft180_show_char                (uint16 x, uint16 y, const char dat);
void    tft180_show_string              (uint16 x, uint16 y, const char dat[]);
void    tft180_show_int                 (uint16 x, uint16 y, const int32 dat, uint8 num);
void    tft180_show_uint                (uint16 x, uint16 y, const uint32 dat, uint8 num);
void    tft180_show_float               (uint16 x, uint16 y, const double dat, uint8 num, uint8 pointnum);

void    tft180_show_binary_image        (uint16 x, uint16 y, const uint8 *image, uint16 width, uint16 height, uint16 dis_width, uint16 dis_height);
void    tft180_show_gray_image          (uint16 x, uint16 y, const uint8 *image, uint16 width, uint16 height, uint16 dis_width, uint16 dis_height, uint8 threshold);
void    tft180_show_rgb565_image        (uint16 x, uint16 y, const uint16 *image, uint16 width, uint16 height, uint16 dis_width, uint16 dis_height, uint8 color_mode);

void    tft180_show_wave                (uint16 x, uint16 y, const uint16 *wave, uint16 width, uint16 value_max, uint16 dis_width, uint16 dis_value_max);
void    tft180_show_chinese             (uint16 x, uint16 y, uint8 size, const uint8 *chinese_buffer, uint8 number, const uint16 color);

// 新增：直接传递中文字符串字面量接口 (支持中英混排，例如: tft180_show_chinese_str(0, 0, "中文显示", RGB565_RED);)
void    tft180_show_chinese_str         (uint16 x, uint16 y, const char *str, uint16 color);

#endif
