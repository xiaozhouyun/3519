/**
 * @file Grayscale.c
 * @brief MSPM0G3519 八通道灰度循迹传感器驱动实现文件 (基于 TI DriverLib)
 * 
 * 主要功能:
 * 1. 控制 CD4051 多路开关 3 位地址引脚 (A0:PA7, A1:PA8, A2:PA9) 实现 8 通道选通
 * 2. 单次触发 ADC0 (PA27) 进行模拟量采样，每通道多次采样求均值滤波
 * 3. 基于黑白矫定阈值实现多通道二值化 (Bit0~Bit7) 与 0~4095 范围的模拟量归一化计算
 */

#include "../inc/Grayscale.h"

// --------------------------- 底层 ADC 采样与地址选通 ---------------------------

/**
 * @brief  切换 CD4051 多路开关 3 位地址引脚 (A0:PA7, A1:PA8, A2:PA9)
 * @param  ch 通道索引 (0 ~ 7)
 */
static inline void Grayscale_Set_Address(uint8_t ch)
{
    // A0 (Bit 0) -> PA7
    if (ch & 0x01) {
        DL_GPIO_setPins(xunjiGPIO_PORT, xunjiGPIO_PIN_1_PIN);
    } else {
        DL_GPIO_clearPins(xunjiGPIO_PORT, xunjiGPIO_PIN_1_PIN);
    }

    // A1 (Bit 1) -> PA8
    if (ch & 0x02) {
        DL_GPIO_setPins(xunjiGPIO_PORT, xunjiGPIO_PIN_2_PIN);
    } else {
        DL_GPIO_clearPins(xunjiGPIO_PORT, xunjiGPIO_PIN_2_PIN);
    }

    // A2 (Bit 2) -> PA9
    if (ch & 0x04) {
        DL_GPIO_setPins(xunjiGPIO_PORT, xunjiGPIO_PIN_3_PIN);
    } else {
        DL_GPIO_clearPins(xunjiGPIO_PORT, xunjiGPIO_PIN_3_PIN);
    }
}

/**
 * @brief  单次触发 ADC0 (PA27) 采样并获取最新 12 位转换结果
 * @return 12位 ADC 转换数字量 (0 ~ 4095)
 */
static uint16_t Grayscale_ADC_Read_Single(void)
{
    // 1. 清除上一次的 MEM0 转换完成中断标志，确保获取最新数据
    DL_ADC12_clearInterruptStatus(ADC0_xunji_INST, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);

    // 2. 使能并启动单次 ADC 转换
    DL_ADC12_enableConversions(ADC0_xunji_INST);
    DL_ADC12_startConversion(ADC0_xunji_INST);

    // 3. 等待本次 MEM0 结果转换加载完成
    uint32_t timeout = 100000U;
    while (!DL_ADC12_getRawInterruptStatus(ADC0_xunji_INST, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED) && --timeout);

    // 4. 读取并返回最新转换结果
    return DL_ADC12_getMemResult(ADC0_xunji_INST, ADC0_xunji_ADCMEM_0);
}

/**
 * @brief  轮询采集 8 个通道的原始模拟量 (每通道采样 8 次求均值滤波)
 * @param  result 长度为 8 的 16 位模拟量接收数组指针
 */
static void Grayscale_Read_All_Analog(uint16_t *result)
{
    uint8_t i, j;
    uint32_t sum = 0;

    for (i = 0; i < 8; i++)
    {
        // 1. 选通通道多路开关地址 (A0:PA7, A1:PA8, A2:PA9)
        Grayscale_Set_Address(i);
        delay_cycles(CPUCLK_FREQ / 100000 * 5); // 充放电与开关稳压延时 (~50us)

        // 2. 连续采样 8 次求均值
        sum = 0;
        for (j = 0; j < 8; j++)
        {
            sum += Grayscale_ADC_Read_Single();
        }

        // 3. 通道线序反转映射 (使通道 0 对应小车左侧，通道 7 对应右侧)
        result[7 - i] = (uint16_t)(sum / 8);
    }
}

// --------------------------- 核心算法: 二值化与归一化 ---------------------------

/**
 * @brief  模拟量转换为黑白开关二值化数字信号 (双门限滞后比较防抖)
 * @param  adc_val    8通道原始模拟量数组
 * @param  gray_white 8通道白色判定门限阈值数组
 * @param  gray_black 8通道黑色判定门限阈值数组
 * @param  digital    二值化结果输出指针 (按 Bit0~Bit7 存放)
 */
static void convertAnalogToDigital(const uint16_t *adc_val, const uint16_t *gray_white, const uint16_t *gray_black, uint8_t *digital)
{
    for (int i = 0; i < 8; i++)
    {
        if (adc_val[i] > gray_white[i])
        {
            *digital |= (1 << i);   // 大于白门限判为白 (对应 Bit 位置 1)
        }
        else if (adc_val[i] < gray_black[i])
        {
            *digital &= ~(1 << i);  // 小于黑门限判为黑 (对应 Bit 位置 0)
        }
    }
}

/**
 * @brief  模拟量归一化计算 (依据矫定的黑白边界线性映射至 0 ~ 4095)
 * @param  adc_val       8通道原始模拟量数组
 * @param  normal_factor 8通道归一化放大因子系数数组
 * @param  cali_black    8通道黑界矫定基准值数组
 * @param  result        归一化结果输出数组指针
 */
static void normalizeAnalogValues(const uint16_t *adc_val, const double *normal_factor, const uint16_t *cali_black, uint16_t *result)
{
    for (int i = 0; i < 8; i++)
    {
        if (adc_val[i] < cali_black[i])
        {
            result[i] = 0;
        }
        else
        {
            double n = (double)(adc_val[i] - cali_black[i]) * normal_factor[i];
            if (n > 4095.0)
            {
                n = 4095.0;
            }
            result[i] = (uint16_t)n;
        }
    }
}

// --------------------------- API 函数实现 ---------------------------

/**
 * @brief  使用默认预设阈值快速初始化灰度循迹传感器
 * @param  sensor 灰度传感器句柄结构体指针
 */
void Grayscale_Init_First(Grayscale_Sensor_t *sensor)
{
    uint16_t default_white[8];
    uint16_t default_black[8];

    for (int i = 0; i < 8; i++)
    {
        default_white[i] = GRAYSCALE_DEFAULT_WHITE;
        default_black[i] = GRAYSCALE_DEFAULT_BLACK;
    }

    Grayscale_Init(sensor, default_white, default_black);
}

/**
 * @brief  使用校准后的黑白边界阈值完整初始化传感器对象
 * @param  sensor           灰度传感器句柄结构体指针
 * @param  calibrated_white 8通道白色校准基准数组指针
 * @param  calibrated_black 8通道黑色校准基准数组指针
 */
void Grayscale_Init(Grayscale_Sensor_t *sensor, const uint16_t *calibrated_white, const uint16_t *calibrated_black)
{
    memset(sensor, 0, sizeof(Grayscale_Sensor_t));

    // 配置 ADC0 采样保持时间 (50个时钟周期)
    DL_ADC12_setSampleTime0(ADC0_xunji_INST, 50);

    for (int i = 0; i < 8; i++)
    {
        uint16_t white = calibrated_white[i];
        uint16_t black = calibrated_black[i];

        // 保障 white > black 的大小关系
        if (black >= white)
        {
            uint16_t tmp = white;
            white = black;
            black = tmp;
        }

        sensor->calibrated_white[i] = white;
        sensor->calibrated_black[i] = black;

        // 计算滞后比较阈值界限: 灰度白门限为 2/3 处，灰度黑门限为 1/3 处
        sensor->gray_white[i] = (white * 2 + black) / 3;
        sensor->gray_black[i] = (white + black * 2) / 3;

        // 计算归一化缩放因子: Factor = 4095.0 / (White - Black)
        double diff = (double)white - (double)black;
        if (diff > 0.0)
        {
            sensor->normal_factor[i] = 4095.0 / diff;
        }
        else
        {
            sensor->normal_factor[i] = 0.0;
        }
    }

    sensor->is_ok = 1;
}

/**
 * @brief  批量统一设置全通道的全局黑白基础阈值
 * @param  sensor 灰度传感器句柄结构体指针
 * @param  white  统一的白色边界基准值
 * @param  black  统一的黑色边界基准值
 */
void Grayscale_Set_Global_Thresholds(Grayscale_Sensor_t *sensor, uint16_t white, uint16_t black)
{
    uint16_t white_arr[8];
    uint16_t black_arr[8];

    for (int i = 0; i < 8; i++)
    {
        white_arr[i] = white;
        black_arr[i] = black;
    }

    Grayscale_Init(sensor, white_arr, black_arr);
}

/**
 * @brief  刷新传感器采集并更新二值化与归一化数据 (轮询采集周期调用)
 * @param  sensor 灰度传感器句柄结构体指针
 */
void Grayscale_Update(Grayscale_Sensor_t *sensor)
{
    // 1. 采集 8 通道原始模拟量
    Grayscale_Read_All_Analog(sensor->analog_val);

    // 2. 转换黑白二值化数字量
    convertAnalogToDigital(sensor->analog_val, sensor->gray_white, sensor->gray_black, &sensor->digital);

    // 3. 归一化计算
    normalizeAnalogValues(sensor->analog_val, sensor->normal_factor, sensor->calibrated_black, sensor->normal_val);
}

/**
 * @brief  获取 8 通道黑白二值化数字状态字节 (按位存储，1:白，0:黑)
 * @param  sensor 灰度传感器句柄结构体指针
 * @return 8 位二值化状态掩码字节
 */
uint8_t Grayscale_Get_Digital(Grayscale_Sensor_t *sensor)
{
    return sensor->digital;
}

/**
 * @brief  获取 8 通道归一化后的模拟量数值数组 (0 ~ 4095)
 * @param  sensor 灰度传感器句柄结构体指针
 * @param  result 接收 8 通道归一化数据的 16 位数组指针
 * @return 1 表示成功获取，0 表示传感器未处于正常初始化状态
 */
uint8_t Grayscale_Get_Normalized(Grayscale_Sensor_t *sensor, uint16_t *result)
{
    if (!sensor->is_ok) return 0;
    memcpy(result, sensor->normal_val, sizeof(uint16_t) * 8);
    return 1;
}

/**
 * @brief  获取 8 通道原始 ADC 模拟量数值数组 (0 ~ 4095)
 * @param  sensor 灰度传感器句柄结构体指针
 * @param  result 接收 8 通道原始模拟量的 16 位数组指针
 * @return 1 表示成功获取，0 表示传感器未正常初始化
 */
uint8_t Grayscale_Get_Analog(Grayscale_Sensor_t *sensor, uint16_t *result)
{
    memcpy(result, sensor->analog_val, sizeof(uint16_t) * 8);
    return sensor->is_ok;
}