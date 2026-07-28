/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G351X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G351X
#define CONFIG_MSPM0G3519

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     80000000
/* Defines for SYSPLL_ERR_01 Workaround */
/* Represent 1.000 as 1000 */
#define FLOAT_TO_INT_SCALE                                               (1000U)
#define FCC_EXPECTED_RATIO                                                  2500
#define FCC_UPPER_BOUND                       (FCC_EXPECTED_RATIO * (1 + 0.003))
#define FCC_LOWER_BOUND                       (FCC_EXPECTED_RATIO * (1 - 0.003))

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);


/* Defines for TB6612PWM */
#define TB6612PWM_INST                                                     TIMA1
#define TB6612PWM_INST_IRQHandler                               TIMA1_IRQHandler
#define TB6612PWM_INST_INT_IRQN                                 (TIMA1_INT_IRQn)
#define TB6612PWM_INST_CLK_FREQ                                         10000000
/* GPIO defines for channel 0 */
#define GPIO_TB6612PWM_C0_PORT                                             GPIOA
#define GPIO_TB6612PWM_C0_PIN                                     DL_GPIO_PIN_28
#define GPIO_TB6612PWM_C0_IOMUX                                   (IOMUX_PINCM3)
#define GPIO_TB6612PWM_C0_IOMUX_FUNC                  IOMUX_PINCM3_PF_TIMA1_CCP0
#define GPIO_TB6612PWM_C0_IDX                                DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_TB6612PWM_C1_PORT                                             GPIOA
#define GPIO_TB6612PWM_C1_PIN                                     DL_GPIO_PIN_16
#define GPIO_TB6612PWM_C1_IOMUX                                  (IOMUX_PINCM38)
#define GPIO_TB6612PWM_C1_IOMUX_FUNC                 IOMUX_PINCM38_PF_TIMA1_CCP1
#define GPIO_TB6612PWM_C1_IDX                                DL_TIMER_CC_1_INDEX

/* Defines for DRV8873 */
#define DRV8873_INST                                                      TIMG12
#define DRV8873_INST_IRQHandler                                TIMG12_IRQHandler
#define DRV8873_INST_INT_IRQN                                  (TIMG12_INT_IRQn)
#define DRV8873_INST_CLK_FREQ                                           80000000
/* GPIO defines for channel 0 */
#define GPIO_DRV8873_C0_PORT                                               GPIOB
#define GPIO_DRV8873_C0_PIN                                       DL_GPIO_PIN_20
#define GPIO_DRV8873_C0_IOMUX                                    (IOMUX_PINCM48)
#define GPIO_DRV8873_C0_IOMUX_FUNC                  IOMUX_PINCM48_PF_TIMG12_CCP0
#define GPIO_DRV8873_C0_IDX                                  DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_DRV8873_C1_PORT                                               GPIOA
#define GPIO_DRV8873_C1_PIN                                       DL_GPIO_PIN_25
#define GPIO_DRV8873_C1_IOMUX                                    (IOMUX_PINCM55)
#define GPIO_DRV8873_C1_IOMUX_FUNC                  IOMUX_PINCM55_PF_TIMG12_CCP1
#define GPIO_DRV8873_C1_IDX                                  DL_TIMER_CC_1_INDEX

/* Defines for servo */
#define servo_INST                                                         TIMA0
#define servo_INST_IRQHandler                                   TIMA0_IRQHandler
#define servo_INST_INT_IRQN                                     (TIMA0_INT_IRQn)
#define servo_INST_CLK_FREQ                                   1012658.2278481013
/* GPIO defines for channel 0 */
#define GPIO_servo_C0_PORT                                                 GPIOC
#define GPIO_servo_C0_PIN                                          DL_GPIO_PIN_2
#define GPIO_servo_C0_IOMUX                                      (IOMUX_PINCM76)
#define GPIO_servo_C0_IOMUX_FUNC                     IOMUX_PINCM76_PF_TIMA0_CCP0
#define GPIO_servo_C0_IDX                                    DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_servo_C1_PORT                                                 GPIOC
#define GPIO_servo_C1_PIN                                          DL_GPIO_PIN_4
#define GPIO_servo_C1_IOMUX                                      (IOMUX_PINCM78)
#define GPIO_servo_C1_IOMUX_FUNC                     IOMUX_PINCM78_PF_TIMA0_CCP1
#define GPIO_servo_C1_IDX                                    DL_TIMER_CC_1_INDEX
/* GPIO defines for channel 2 */
#define GPIO_servo_C2_PORT                                                 GPIOC
#define GPIO_servo_C2_PIN                                          DL_GPIO_PIN_0
#define GPIO_servo_C2_IOMUX                                      (IOMUX_PINCM74)
#define GPIO_servo_C2_IOMUX_FUNC                     IOMUX_PINCM74_PF_TIMA0_CCP2
#define GPIO_servo_C2_IDX                                    DL_TIMER_CC_2_INDEX
/* GPIO defines for channel 3 */
#define GPIO_servo_C3_PORT                                                 GPIOA
#define GPIO_servo_C3_PIN                                         DL_GPIO_PIN_17
#define GPIO_servo_C3_IOMUX                                      (IOMUX_PINCM39)
#define GPIO_servo_C3_IOMUX_FUNC                     IOMUX_PINCM39_PF_TIMA0_CCP3
#define GPIO_servo_C3_IDX                                    DL_TIMER_CC_3_INDEX




/* Defines for AB1 */
#define AB1_INST                                                           TIMG8
#define AB1_INST_IRQHandler                                     TIMG8_IRQHandler
#define AB1_INST_INT_IRQN                                       (TIMG8_INT_IRQn)
/* Pin configuration defines for AB1 PHA Pin */
#define GPIO_AB1_PHA_PORT                                                  GPIOA
#define GPIO_AB1_PHA_PIN                                          DL_GPIO_PIN_26
#define GPIO_AB1_PHA_IOMUX                                       (IOMUX_PINCM59)
#define GPIO_AB1_PHA_IOMUX_FUNC                      IOMUX_PINCM59_PF_TIMG8_CCP0
/* Pin configuration defines for AB1 PHB Pin */
#define GPIO_AB1_PHB_PORT                                                  GPIOB
#define GPIO_AB1_PHB_PIN                                          DL_GPIO_PIN_19
#define GPIO_AB1_PHB_IOMUX                                       (IOMUX_PINCM45)
#define GPIO_AB1_PHB_IOMUX_FUNC                      IOMUX_PINCM45_PF_TIMG8_CCP1

/* Defines for AB2 */
#define AB2_INST                                                           TIMG9
#define AB2_INST_IRQHandler                                     TIMG9_IRQHandler
#define AB2_INST_INT_IRQN                                       (TIMG9_INT_IRQn)
/* Pin configuration defines for AB2 PHA Pin */
#define GPIO_AB2_PHA_PORT                                                  GPIOB
#define GPIO_AB2_PHA_PIN                                           DL_GPIO_PIN_7
#define GPIO_AB2_PHA_IOMUX                                       (IOMUX_PINCM24)
#define GPIO_AB2_PHA_IOMUX_FUNC                      IOMUX_PINCM24_PF_TIMG9_CCP0
/* Pin configuration defines for AB2 PHB Pin */
#define GPIO_AB2_PHB_PORT                                                  GPIOB
#define GPIO_AB2_PHB_PIN                                           DL_GPIO_PIN_9
#define GPIO_AB2_PHB_IOMUX                                       (IOMUX_PINCM26)
#define GPIO_AB2_PHB_IOMUX_FUNC                      IOMUX_PINCM26_PF_TIMG9_CCP1



/* Defines for OLED */
#define OLED_INST                                                           I2C0
#define OLED_INST_IRQHandler                                     I2C0_IRQHandler
#define OLED_INST_INT_IRQN                                         I2C0_INT_IRQn
#define OLED_BUS_SPEED_HZ                                                 100000
#define GPIO_OLED_SDA_PORT                                                 GPIOA
#define GPIO_OLED_SDA_PIN                                          DL_GPIO_PIN_0
#define GPIO_OLED_IOMUX_SDA                                       (IOMUX_PINCM1)
#define GPIO_OLED_IOMUX_SDA_FUNC                        IOMUX_PINCM1_PF_I2C0_SDA
#define GPIO_OLED_SCL_PORT                                                 GPIOA
#define GPIO_OLED_SCL_PIN                                          DL_GPIO_PIN_1
#define GPIO_OLED_IOMUX_SCL                                       (IOMUX_PINCM2)
#define GPIO_OLED_IOMUX_SCL_FUNC                        IOMUX_PINCM2_PF_I2C0_SCL


/* Defines for blue */
#define blue_INST                                                          UART1
#define blue_INST_FREQUENCY                                             40000000
#define blue_INST_IRQHandler                                    UART1_IRQHandler
#define blue_INST_INT_IRQN                                        UART1_INT_IRQn
#define GPIO_blue_RX_PORT                                                  GPIOB
#define GPIO_blue_TX_PORT                                                  GPIOB
#define GPIO_blue_RX_PIN                                           DL_GPIO_PIN_5
#define GPIO_blue_TX_PIN                                           DL_GPIO_PIN_4
#define GPIO_blue_IOMUX_RX                                       (IOMUX_PINCM18)
#define GPIO_blue_IOMUX_TX                                       (IOMUX_PINCM17)
#define GPIO_blue_IOMUX_RX_FUNC                        IOMUX_PINCM18_PF_UART1_RX
#define GPIO_blue_IOMUX_TX_FUNC                        IOMUX_PINCM17_PF_UART1_TX
#define blue_BAUD_RATE                                                    (9600)
#define blue_IBRD_40_MHZ_9600_BAUD                                         (260)
#define blue_FBRD_40_MHZ_9600_BAUD                                          (27)
/* Defines for UART_4 */
#define UART_4_INST                                                        UART4
#define UART_4_INST_FREQUENCY                                           80000000
#define UART_4_INST_IRQHandler                                  UART4_IRQHandler
#define UART_4_INST_INT_IRQN                                      UART4_INT_IRQn
#define GPIO_UART_4_RX_PORT                                                GPIOB
#define GPIO_UART_4_TX_PORT                                                GPIOB
#define GPIO_UART_4_RX_PIN                                        DL_GPIO_PIN_11
#define GPIO_UART_4_TX_PIN                                        DL_GPIO_PIN_10
#define GPIO_UART_4_IOMUX_RX                                     (IOMUX_PINCM28)
#define GPIO_UART_4_IOMUX_TX                                     (IOMUX_PINCM27)
#define GPIO_UART_4_IOMUX_RX_FUNC                      IOMUX_PINCM28_PF_UART4_RX
#define GPIO_UART_4_IOMUX_TX_FUNC                      IOMUX_PINCM27_PF_UART4_TX
#define UART_4_BAUD_RATE                                                (115200)
#define UART_4_IBRD_80_MHZ_115200_BAUD                                      (43)
#define UART_4_FBRD_80_MHZ_115200_BAUD                                      (26)




/* Defines for TFT_SPI0 */
#define TFT_SPI0_INST                                                      SPI0
#define TFT_SPI0_INST_IRQHandler                                SPI0_IRQHandler
#define TFT_SPI0_INST_INT_IRQN                                    SPI0_INT_IRQn
#define GPIO_TFT_SPI0_PICO_PORT                                           GPIOB
#define GPIO_TFT_SPI0_PICO_PIN                                    DL_GPIO_PIN_2
#define GPIO_TFT_SPI0_IOMUX_PICO                                (IOMUX_PINCM15)
#define GPIO_TFT_SPI0_IOMUX_PICO_FUNC                IOMUX_PINCM15_PF_SPI0_PICO
#define GPIO_TFT_SPI0_POCI_PORT                                           GPIOA
#define GPIO_TFT_SPI0_POCI_PIN                                   DL_GPIO_PIN_10
#define GPIO_TFT_SPI0_IOMUX_POCI                                (IOMUX_PINCM21)
#define GPIO_TFT_SPI0_IOMUX_POCI_FUNC                IOMUX_PINCM21_PF_SPI0_POCI
/* GPIO configuration for TFT_SPI0 */
#define GPIO_TFT_SPI0_SCLK_PORT                                           GPIOB
#define GPIO_TFT_SPI0_SCLK_PIN                                    DL_GPIO_PIN_3
#define GPIO_TFT_SPI0_IOMUX_SCLK                                (IOMUX_PINCM16)
#define GPIO_TFT_SPI0_IOMUX_SCLK_FUNC                IOMUX_PINCM16_PF_SPI0_SCLK
/* Defines for IMU660RC */
#define IMU660RC_INST                                                      SPI1
#define IMU660RC_INST_IRQHandler                                SPI1_IRQHandler
#define IMU660RC_INST_INT_IRQN                                    SPI1_INT_IRQn
#define GPIO_IMU660RC_PICO_PORT                                           GPIOB
#define GPIO_IMU660RC_PICO_PIN                                   DL_GPIO_PIN_15
#define GPIO_IMU660RC_IOMUX_PICO                                (IOMUX_PINCM32)
#define GPIO_IMU660RC_IOMUX_PICO_FUNC                IOMUX_PINCM32_PF_SPI1_PICO
#define GPIO_IMU660RC_POCI_PORT                                           GPIOB
#define GPIO_IMU660RC_POCI_PIN                                   DL_GPIO_PIN_14
#define GPIO_IMU660RC_IOMUX_POCI                                (IOMUX_PINCM31)
#define GPIO_IMU660RC_IOMUX_POCI_FUNC                IOMUX_PINCM31_PF_SPI1_POCI
/* GPIO configuration for IMU660RC */
#define GPIO_IMU660RC_SCLK_PORT                                           GPIOB
#define GPIO_IMU660RC_SCLK_PIN                                   DL_GPIO_PIN_16
#define GPIO_IMU660RC_IOMUX_SCLK                                (IOMUX_PINCM33)
#define GPIO_IMU660RC_IOMUX_SCLK_FUNC                IOMUX_PINCM33_PF_SPI1_SCLK



/* Defines for ADC1 */
#define ADC1_INST                                                           ADC1
#define ADC1_INST_IRQHandler                                     ADC1_IRQHandler
#define ADC1_INST_INT_IRQN                                       (ADC1_INT_IRQn)
#define ADC1_ADCMEM_DRC8873_ADC0                              DL_ADC12_MEM_IDX_0
#define ADC1_ADCMEM_DRC8873_ADC0_REF        DL_ADC12_REFERENCE_VOLTAGE_VDDA_VSSA
#define ADC1_ADCMEM_DRC8873_ADC1                              DL_ADC12_MEM_IDX_1
#define ADC1_ADCMEM_DRC8873_ADC1_REF        DL_ADC12_REFERENCE_VOLTAGE_VDDA_VSSA
#define GPIO_ADC1_C0_PORT                                                  GPIOA
#define GPIO_ADC1_C0_PIN                                          DL_GPIO_PIN_15
#define GPIO_ADC1_IOMUX_C0                                       (IOMUX_PINCM37)
#define GPIO_ADC1_IOMUX_C0_FUNC                   (IOMUX_PINCM37_PF_UNCONNECTED)
#define GPIO_ADC1_C4_PORT                                                  GPIOB
#define GPIO_ADC1_C4_PIN                                          DL_GPIO_PIN_17
#define GPIO_ADC1_IOMUX_C4                                       (IOMUX_PINCM43)
#define GPIO_ADC1_IOMUX_C4_FUNC                   (IOMUX_PINCM43_PF_UNCONNECTED)

/* Defines for ADC0_xunji */
#define ADC0_xunji_INST                                                     ADC0
#define ADC0_xunji_INST_IRQHandler                               ADC0_IRQHandler
#define ADC0_xunji_INST_INT_IRQN                                 (ADC0_INT_IRQn)
#define ADC0_xunji_ADCMEM_0                                   DL_ADC12_MEM_IDX_0
#define ADC0_xunji_ADCMEM_0_REF             DL_ADC12_REFERENCE_VOLTAGE_VDDA_VSSA
#define GPIO_ADC0_xunji_C0_PORT                                            GPIOA
#define GPIO_ADC0_xunji_C0_PIN                                    DL_GPIO_PIN_27
#define GPIO_ADC0_xunji_IOMUX_C0                                 (IOMUX_PINCM60)
#define GPIO_ADC0_xunji_IOMUX_C0_FUNC             (IOMUX_PINCM60_PF_UNCONNECTED)



/* Port definition for Pin Group imuInt */
#define imuInt_PORT                                                      (GPIOB)

/* Defines for CS: GPIOB.13 with pinCMx 30 on package pin 37 */
#define imuInt_CS_PIN                                           (DL_GPIO_PIN_13)
#define imuInt_CS_IOMUX                                          (IOMUX_PINCM30)
/* Port definition for Pin Group buzzer */
#define buzzer_PORT                                                      (GPIOB)

/* Defines for PIN_0: GPIOB.1 with pinCMx 13 on package pin 16 */
#define buzzer_PIN_0_PIN                                         (DL_GPIO_PIN_1)
#define buzzer_PIN_0_IOMUX                                       (IOMUX_PINCM13)
/* Defines for TFT_DC: GPIOC.8 with pinCMx 86 on package pin 65 */
#define TFT_TFT_DC_PORT                                                  (GPIOC)
#define TFT_TFT_DC_PIN                                           (DL_GPIO_PIN_8)
#define TFT_TFT_DC_IOMUX                                         (IOMUX_PINCM86)
/* Defines for TFT_CS: GPIOC.9 with pinCMx 87 on package pin 66 */
#define TFT_TFT_CS_PORT                                                  (GPIOC)
#define TFT_TFT_CS_PIN                                           (DL_GPIO_PIN_9)
#define TFT_TFT_CS_IOMUX                                         (IOMUX_PINCM87)
/* Defines for TFT_RES: GPIOB.23 with pinCMx 51 on package pin 70 */
#define TFT_TFT_RES_PORT                                                 (GPIOB)
#define TFT_TFT_RES_PIN                                         (DL_GPIO_PIN_23)
#define TFT_TFT_RES_IOMUX                                        (IOMUX_PINCM51)
/* Defines for TFT_BLK: GPIOA.30 with pinCMx 5 on package pin 5 */
#define TFT_TFT_BLK_PORT                                                 (GPIOA)
#define TFT_TFT_BLK_PIN                                         (DL_GPIO_PIN_30)
#define TFT_TFT_BLK_IOMUX                                         (IOMUX_PINCM5)
/* Defines for key0: GPIOB.29 with pinCMx 66 on package pin 25 */
#define key_key0_PORT                                                    (GPIOB)
#define key_key0_PIN                                            (DL_GPIO_PIN_29)
#define key_key0_IOMUX                                           (IOMUX_PINCM66)
/* Defines for key1: GPIOB.30 with pinCMx 67 on package pin 26 */
#define key_key1_PORT                                                    (GPIOB)
#define key_key1_PIN                                            (DL_GPIO_PIN_30)
#define key_key1_IOMUX                                           (IOMUX_PINCM67)
/* Defines for key2: GPIOC.6 with pinCMx 84 on package pin 63 */
#define key_key2_PORT                                                    (GPIOC)
#define key_key2_PIN                                             (DL_GPIO_PIN_6)
#define key_key2_IOMUX                                           (IOMUX_PINCM84)
/* Defines for key3: GPIOC.7 with pinCMx 85 on package pin 64 */
#define key_key3_PORT                                                    (GPIOC)
#define key_key3_PIN                                             (DL_GPIO_PIN_7)
#define key_key3_IOMUX                                           (IOMUX_PINCM85)
/* Defines for AIN2: GPIOB.22 with pinCMx 50 on package pin 69 */
#define TB6612_AIN2_PORT                                                 (GPIOB)
#define TB6612_AIN2_PIN                                         (DL_GPIO_PIN_22)
#define TB6612_AIN2_IOMUX                                        (IOMUX_PINCM50)
/* Defines for AIN1: GPIOB.28 with pinCMx 65 on package pin 24 */
#define TB6612_AIN1_PORT                                                 (GPIOB)
#define TB6612_AIN1_PIN                                         (DL_GPIO_PIN_28)
#define TB6612_AIN1_IOMUX                                        (IOMUX_PINCM65)
/* Defines for BIN1: GPIOA.24 with pinCMx 54 on package pin 73 */
#define TB6612_BIN1_PORT                                                 (GPIOA)
#define TB6612_BIN1_PIN                                         (DL_GPIO_PIN_24)
#define TB6612_BIN1_IOMUX                                        (IOMUX_PINCM54)
/* Defines for BIN2: GPIOA.13 with pinCMx 35 on package pin 42 */
#define TB6612_BIN2_PORT                                                 (GPIOA)
#define TB6612_BIN2_PIN                                         (DL_GPIO_PIN_13)
#define TB6612_BIN2_IOMUX                                        (IOMUX_PINCM35)
/* Port definition for Pin Group dianci */
#define dianci_PORT                                                      (GPIOB)

/* Defines for dian1: GPIOB.27 with pinCMx 58 on package pin 77 */
#define dianci_dian1_PIN                                        (DL_GPIO_PIN_27)
#define dianci_dian1_IOMUX                                       (IOMUX_PINCM58)
/* Defines for dian2: GPIOB.26 with pinCMx 57 on package pin 76 */
#define dianci_dian2_PIN                                        (DL_GPIO_PIN_26)
#define dianci_dian2_IOMUX                                       (IOMUX_PINCM57)
/* Defines for PH1: GPIOA.22 with pinCMx 47 on package pin 62 */
#define DRV8873HPWPT_PH1_PORT                                            (GPIOA)
#define DRV8873HPWPT_PH1_PIN                                    (DL_GPIO_PIN_22)
#define DRV8873HPWPT_PH1_IOMUX                                   (IOMUX_PINCM47)
/* Defines for PH2: GPIOB.18 with pinCMx 44 on package pin 59 */
#define DRV8873HPWPT_PH2_PORT                                            (GPIOB)
#define DRV8873HPWPT_PH2_PIN                                    (DL_GPIO_PIN_18)
#define DRV8873HPWPT_PH2_IOMUX                                   (IOMUX_PINCM44)
/* Port definition for Pin Group xunjiGPIO */
#define xunjiGPIO_PORT                                                   (GPIOA)

/* Defines for PIN_1: GPIOA.7 with pinCMx 14 on package pin 17 */
#define xunjiGPIO_PIN_1_PIN                                      (DL_GPIO_PIN_7)
#define xunjiGPIO_PIN_1_IOMUX                                    (IOMUX_PINCM14)
/* Defines for PIN_2: GPIOA.8 with pinCMx 19 on package pin 22 */
#define xunjiGPIO_PIN_2_PIN                                      (DL_GPIO_PIN_8)
#define xunjiGPIO_PIN_2_IOMUX                                    (IOMUX_PINCM19)
/* Defines for PIN_3: GPIOA.9 with pinCMx 20 on package pin 23 */
#define xunjiGPIO_PIN_3_PIN                                      (DL_GPIO_PIN_9)
#define xunjiGPIO_PIN_3_IOMUX                                    (IOMUX_PINCM20)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_SYSCTL_CLK_init(void);

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);
void SYSCFG_DL_TB6612PWM_init(void);
void SYSCFG_DL_DRV8873_init(void);
void SYSCFG_DL_servo_init(void);
void SYSCFG_DL_AB1_init(void);
void SYSCFG_DL_AB2_init(void);
void SYSCFG_DL_OLED_init(void);
void SYSCFG_DL_blue_init(void);
void SYSCFG_DL_UART_4_init(void);
void SYSCFG_DL_TFT_SPI0_init(void);
void SYSCFG_DL_IMU660RC_init(void);
void SYSCFG_DL_ADC1_init(void);
void SYSCFG_DL_ADC0_xunji_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
