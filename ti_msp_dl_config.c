/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G351X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerA_backupConfig gTB6612PWMBackup;
DL_TimerA_backupConfig gservoBackup;
DL_TimerG_backupConfig gAB1Backup;
DL_SPI_backupConfig gTFT_SPI0Backup;
DL_SPI_backupConfig gIMU660RCBackup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_TB6612PWM_init();
    SYSCFG_DL_DRV8873_init();
    SYSCFG_DL_servo_init();
    SYSCFG_DL_AB1_init();
    SYSCFG_DL_AB2_init();
    SYSCFG_DL_OLED_init();
    SYSCFG_DL_blue_init();
    SYSCFG_DL_UART_4_init();
    SYSCFG_DL_TFT_SPI0_init();
    SYSCFG_DL_IMU660RC_init();
    SYSCFG_DL_ADC1_init();
    SYSCFG_DL_ADC0_xunji_init();
    SYSCFG_DL_SYSCTL_CLK_init();
    /* Ensure backup structures have no valid state */
	gTB6612PWMBackup.backupRdy 	= false;
	gservoBackup.backupRdy 	= false;
	gAB1Backup.backupRdy 	= false;

	gTFT_SPI0Backup.backupRdy 	= false;
	gIMU660RCBackup.backupRdy 	= false;

}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_saveConfiguration(TB6612PWM_INST, &gTB6612PWMBackup);
	retStatus &= DL_TimerA_saveConfiguration(servo_INST, &gservoBackup);
	retStatus &= DL_TimerG_saveConfiguration(AB1_INST, &gAB1Backup);
	retStatus &= DL_SPI_saveConfiguration(TFT_SPI0_INST, &gTFT_SPI0Backup);
	retStatus &= DL_SPI_saveConfiguration(IMU660RC_INST, &gIMU660RCBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_restoreConfiguration(TB6612PWM_INST, &gTB6612PWMBackup, false);
	retStatus &= DL_TimerA_restoreConfiguration(servo_INST, &gservoBackup, false);
	retStatus &= DL_TimerG_restoreConfiguration(AB1_INST, &gAB1Backup, false);
	retStatus &= DL_SPI_restoreConfiguration(TFT_SPI0_INST, &gTFT_SPI0Backup);
	retStatus &= DL_SPI_restoreConfiguration(IMU660RC_INST, &gIMU660RCBackup);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_GPIO_reset(GPIOC);
    DL_TimerA_reset(TB6612PWM_INST);
    DL_TimerG_reset(DRV8873_INST);
    DL_TimerA_reset(servo_INST);
    DL_TimerG_reset(AB1_INST);
    DL_TimerG_reset(AB2_INST);
    DL_I2C_reset(OLED_INST);
    DL_UART_Main_reset(blue_INST);
    DL_UART_Main_reset(UART_4_INST);
    DL_SPI_reset(TFT_SPI0_INST);
    DL_SPI_reset(IMU660RC_INST);
    DL_ADC12_reset(ADC1_INST);
    DL_ADC12_reset(ADC0_xunji_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_GPIO_enablePower(GPIOC);
    DL_TimerA_enablePower(TB6612PWM_INST);
    DL_TimerG_enablePower(DRV8873_INST);
    DL_TimerA_enablePower(servo_INST);
    DL_TimerG_enablePower(AB1_INST);
    DL_TimerG_enablePower(AB2_INST);
    DL_I2C_enablePower(OLED_INST);
    DL_UART_Main_enablePower(blue_INST);
    DL_UART_Main_enablePower(UART_4_INST);
    DL_SPI_enablePower(TFT_SPI0_INST);
    DL_SPI_enablePower(IMU660RC_INST);
    DL_ADC12_enablePower(ADC1_INST);
    DL_ADC12_enablePower(ADC0_xunji_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralOutputFunction(GPIO_TB6612PWM_C0_IOMUX,GPIO_TB6612PWM_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_TB6612PWM_C0_PORT, GPIO_TB6612PWM_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_TB6612PWM_C1_IOMUX,GPIO_TB6612PWM_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_TB6612PWM_C1_PORT, GPIO_TB6612PWM_C1_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_DRV8873_C0_IOMUX,GPIO_DRV8873_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_DRV8873_C0_PORT, GPIO_DRV8873_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_DRV8873_C1_IOMUX,GPIO_DRV8873_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_DRV8873_C1_PORT, GPIO_DRV8873_C1_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_servo_C0_IOMUX,GPIO_servo_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_servo_C0_PORT, GPIO_servo_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_servo_C1_IOMUX,GPIO_servo_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_servo_C1_PORT, GPIO_servo_C1_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_servo_C2_IOMUX,GPIO_servo_C2_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_servo_C2_PORT, GPIO_servo_C2_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_servo_C3_IOMUX,GPIO_servo_C3_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_servo_C3_PORT, GPIO_servo_C3_PIN);

    DL_GPIO_initPeripheralInputFunction(GPIO_AB1_PHA_IOMUX,GPIO_AB1_PHA_IOMUX_FUNC);
    DL_GPIO_initPeripheralInputFunction(GPIO_AB1_PHB_IOMUX,GPIO_AB1_PHB_IOMUX_FUNC);
    DL_GPIO_initPeripheralInputFunction(GPIO_AB2_PHA_IOMUX,GPIO_AB2_PHA_IOMUX_FUNC);
    DL_GPIO_initPeripheralInputFunction(GPIO_AB2_PHB_IOMUX,GPIO_AB2_PHB_IOMUX_FUNC);

    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_OLED_IOMUX_SDA,
        GPIO_OLED_IOMUX_SDA_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_OLED_IOMUX_SCL,
        GPIO_OLED_IOMUX_SCL_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(GPIO_OLED_IOMUX_SDA);
    DL_GPIO_enableHiZ(GPIO_OLED_IOMUX_SCL);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_blue_IOMUX_TX, GPIO_blue_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_blue_IOMUX_RX, GPIO_blue_IOMUX_RX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_4_IOMUX_TX, GPIO_UART_4_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_4_IOMUX_RX, GPIO_UART_4_IOMUX_RX_FUNC);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_TFT_SPI0_IOMUX_SCLK, GPIO_TFT_SPI0_IOMUX_SCLK_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_TFT_SPI0_IOMUX_PICO, GPIO_TFT_SPI0_IOMUX_PICO_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_TFT_SPI0_IOMUX_POCI, GPIO_TFT_SPI0_IOMUX_POCI_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_IMU660RC_IOMUX_SCLK, GPIO_IMU660RC_IOMUX_SCLK_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_IMU660RC_IOMUX_PICO, GPIO_IMU660RC_IOMUX_PICO_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_IMU660RC_IOMUX_POCI, GPIO_IMU660RC_IOMUX_POCI_FUNC);

    DL_GPIO_initDigitalOutput(buzzer_PIN_0_IOMUX);

    DL_GPIO_initDigitalOutput(TFT_TFT_DC_IOMUX);

    DL_GPIO_initDigitalOutput(TFT_TFT_CS_IOMUX);

    DL_GPIO_initDigitalOutput(TFT_TFT_RES_IOMUX);

    DL_GPIO_initDigitalOutput(TFT_TFT_BLK_IOMUX);

    DL_GPIO_initDigitalInputFeatures(key_key0_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(key_key1_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(key_key2_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(key_key3_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(imuInt_int2_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(imuInt_CS_IOMUX);

    DL_GPIO_initDigitalOutput(TB6612_AIN2_IOMUX);

    DL_GPIO_initDigitalOutput(TB6612_AIN1_IOMUX);

    DL_GPIO_initDigitalOutput(TB6612_BIN1_IOMUX);

    DL_GPIO_initDigitalOutput(TB6612_BIN2_IOMUX);

    DL_GPIO_initDigitalOutput(dianci_dian1_IOMUX);

    DL_GPIO_initDigitalOutput(dianci_dian2_IOMUX);

    DL_GPIO_initDigitalOutput(DRV8873HPWPT_PH1_IOMUX);

    DL_GPIO_initDigitalOutput(DRV8873HPWPT_PH2_IOMUX);

    DL_GPIO_initDigitalOutput(xunjiGPIO_PIN_1_IOMUX);

    DL_GPIO_initDigitalOutput(xunjiGPIO_PIN_2_IOMUX);

    DL_GPIO_initDigitalOutput(xunjiGPIO_PIN_3_IOMUX);

    DL_GPIO_clearPins(GPIOA, TB6612_BIN1_PIN |
		TB6612_BIN2_PIN |
		DRV8873HPWPT_PH1_PIN |
		xunjiGPIO_PIN_1_PIN |
		xunjiGPIO_PIN_2_PIN |
		xunjiGPIO_PIN_3_PIN);
    DL_GPIO_setPins(GPIOA, TFT_TFT_BLK_PIN);
    DL_GPIO_enableOutput(GPIOA, TFT_TFT_BLK_PIN |
		TB6612_BIN1_PIN |
		TB6612_BIN2_PIN |
		DRV8873HPWPT_PH1_PIN |
		xunjiGPIO_PIN_1_PIN |
		xunjiGPIO_PIN_2_PIN |
		xunjiGPIO_PIN_3_PIN);
    DL_GPIO_clearPins(GPIOB, buzzer_PIN_0_PIN |
		TB6612_AIN2_PIN |
		TB6612_AIN1_PIN |
		dianci_dian1_PIN |
		dianci_dian2_PIN |
		DRV8873HPWPT_PH2_PIN);
    DL_GPIO_setPins(GPIOB, TFT_TFT_RES_PIN |
		imuInt_CS_PIN);
    DL_GPIO_enableOutput(GPIOB, buzzer_PIN_0_PIN |
		TFT_TFT_RES_PIN |
		imuInt_CS_PIN |
		TB6612_AIN2_PIN |
		TB6612_AIN1_PIN |
		dianci_dian1_PIN |
		dianci_dian2_PIN |
		DRV8873HPWPT_PH2_PIN);
    DL_GPIO_setUpperPinsPolarity(GPIOB, DL_GPIO_PIN_24_EDGE_RISE);
    DL_GPIO_clearInterruptStatus(GPIOB, imuInt_int2_PIN);
    DL_GPIO_enableInterrupt(GPIOB, imuInt_int2_PIN);
    DL_GPIO_setPublisherChanID(GPIOB, DL_GPIO_PUBLISHER_INDEX_1, GPIOB_EVENT_PUBLISHER_1_CHANNEL);
    DL_GPIO_enableEvents(GPIOB, DL_GPIO_EVENT_ROUTE_2, imuInt_int2_PIN);
    DL_GPIO_setPins(GPIOC, TFT_TFT_DC_PIN |
		TFT_TFT_CS_PIN);
    DL_GPIO_enableOutput(GPIOC, TFT_TFT_DC_PIN |
		TFT_TFT_CS_PIN);

}


static const DL_SYSCTL_SYSPLLConfig gSYSPLLConfig = {
    .inputFreq              = DL_SYSCTL_SYSPLL_INPUT_FREQ_32_48_MHZ,
	.rDivClk2x              = 1,
	.rDivClk1               = 0,
	.rDivClk0               = 0,
	.enableCLK2x            = DL_SYSCTL_SYSPLL_CLK2X_DISABLE,
	.enableCLK1             = DL_SYSCTL_SYSPLL_CLK1_DISABLE,
	.enableCLK0             = DL_SYSCTL_SYSPLL_CLK0_ENABLE,
	.sysPLLMCLK             = DL_SYSCTL_SYSPLL_MCLK_CLK0,
	.sysPLLRef              = DL_SYSCTL_SYSPLL_REF_SYSOSC,
	.qDiv                   = 4,
	.pDiv                   = DL_SYSCTL_SYSPLL_PDIV_1
};

SYSCONFIG_WEAK bool SYSCFG_DL_SYSCTL_SYSPLL_init(void)
{
    bool fFCCRatioStatus = false;
    uint32_t fFCCSysoscCount;
    uint32_t fFCCPllCount;
    uint32_t fFCCRatio;
    uint32_t fccTimeOutCounter;

    DL_SYSCTL_setFCCPeriods( DL_SYSCTL_FCC_TRIG_CNT_01 );

    /* Measuring PLL. */
    DL_SYSCTL_configFCC(DL_SYSCTL_FCC_TRIG_TYPE_RISE_RISE,
                        DL_SYSCTL_FCC_TRIG_SOURCE_LFCLK,
                        DL_SYSCTL_FCC_CLOCK_SOURCE_SYSPLLCLK0);
    /* Get SYSPLL frequency using FCC */
    fccTimeOutCounter = 0;
    DL_SYSCTL_startFCC();
    while (DL_SYSCTL_isFCCDone() == 0) {
        delay_cycles(977);  /* 1x LFCLK cycle = 32MHz/32.768kHz = 977, 30.5us */
        fccTimeOutCounter++;
        if(fccTimeOutCounter > 65){
            /* Timeout set to approximately 2ms (user-customizable) */
            break;
        }
    }

    /* get measA= SYSPLLCLK0 freq wrt LFOSC*/
    fFCCPllCount = DL_SYSCTL_readFCC();

    /* Measuring SYSPLL Source */
    DL_SYSCTL_configFCC(DL_SYSCTL_FCC_TRIG_TYPE_RISE_RISE,
                        DL_SYSCTL_FCC_TRIG_SOURCE_LFCLK,
                        DL_SYSCTL_FCC_CLOCK_SOURCE_SYSOSC);
    /* Get SYSPLL frequency using FCC */
    fccTimeOutCounter = 0;
    DL_SYSCTL_startFCC();
    while (DL_SYSCTL_isFCCDone() == 0) {
        delay_cycles(977);  /* 1x LFCLK cycle = 32MHz/32.768kHz = 977, 30.5us */
        fccTimeOutCounter++;
        if(fccTimeOutCounter > 65){
            /* Timeout set to approximately 2ms (user-customizable) */
            break;
        }
    }

    /* get measB= SYSOSC freq wrt LFOSC*/
    fFCCSysoscCount = DL_SYSCTL_readFCC();

    /* Get ratio of both measurements*/
    fFCCRatio = (fFCCPllCount * FLOAT_TO_INT_SCALE) / fFCCSysoscCount;
    /* Check ratio is within bounds*/
    if ((FCC_LOWER_BOUND <  fFCCRatio) && (fFCCRatio < FCC_UPPER_BOUND))
    {
        /* ratio is good for proceeding into application code. */
        fFCCRatioStatus = true;
    }

    return fFCCRatioStatus;
}
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_1);
    DL_SYSCTL_setFlashWaitState(DL_SYSCTL_FLASH_WAIT_STATE_2);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_configSYSPLL((DL_SYSCTL_SYSPLLConfig *) &gSYSPLLConfig);

    /*
     * [SYSPLL_ERR_01]
     * PLL Incorrect locking WA start.
     * Insert after every PLL enable.
     * This can lead an infinite loop if the condition persists
     * and can block entry to the application code.
     */

    while (SYSCFG_DL_SYSCTL_SYSPLL_init() == false)
    {
        /* Toggle SYSPLL enable to re-enable SYSPLL and re-check incorrect locking */
        DL_SYSCTL_disableSYSPLL();
        DL_SYSCTL_enableSYSPLL();

        /* Wait until SYSPLL startup is stabilized*/
        while ((DL_SYSCTL_getClockStatus() & SYSCTL_CLKSTATUS_SYSPLLGOOD_MASK) != DL_SYSCTL_CLK_STATUS_SYSPLL_GOOD){}
    }
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_2);
    DL_SYSCTL_setMCLKSource(SYSOSC, HSCLK, DL_SYSCTL_HSCLK_SOURCE_SYSPLL);
    /* INT_GROUP1 Priority */
    NVIC_SetPriority(GPIOB_INT_IRQn, 2);

}
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_CLK_init(void) {
    while ((DL_SYSCTL_getClockStatus() & (DL_SYSCTL_CLK_STATUS_SYSPLL_GOOD
		 | DL_SYSCTL_CLK_STATUS_HSCLK_GOOD
		 | DL_SYSCTL_CLK_STATUS_LFOSC_GOOD))
	       != (DL_SYSCTL_CLK_STATUS_SYSPLL_GOOD
		 | DL_SYSCTL_CLK_STATUS_HSCLK_GOOD
		 | DL_SYSCTL_CLK_STATUS_LFOSC_GOOD))
	{
		/* Ensure that clocks are in default POR configuration before initialization.
		* Additionally once LFXT is enabled, the internal LFOSC is disabled, and cannot
		* be re-enabled other than by executing a BOOTRST. */
		;
	}
}



/*
 * Timer clock configuration to be sourced by  / 1 (80000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   10000000 Hz = 80000000 Hz / (1 * (7 + 1))
 */
static const DL_TimerA_ClockConfig gTB6612PWMClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 7U
};

static const DL_TimerA_PWMConfig gTB6612PWMConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 400,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_TB6612PWM_init(void) {

    DL_TimerA_setClockConfig(
        TB6612PWM_INST, (DL_TimerA_ClockConfig *) &gTB6612PWMClockConfig);

    DL_TimerA_initPWMMode(
        TB6612PWM_INST, (DL_TimerA_PWMConfig *) &gTB6612PWMConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerA_setCounterControl(TB6612PWM_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerA_setCaptureCompareOutCtl(TB6612PWM_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(TB6612PWM_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(TB6612PWM_INST, 400, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(TB6612PWM_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(TB6612PWM_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(TB6612PWM_INST, 400, DL_TIMER_CC_1_INDEX);

    DL_TimerA_enableClock(TB6612PWM_INST);


    
    DL_TimerA_setCCPDirection(TB6612PWM_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT );


}
/*
 * Timer clock configuration to be sourced by  / 1 (80000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   80000000 Hz = 80000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gDRV8873ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};

static const DL_TimerG_PWMConfig gDRV8873Config = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
    .period = 1000,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_DRV8873_init(void) {

    DL_TimerG_setClockConfig(
        DRV8873_INST, (DL_TimerG_ClockConfig *) &gDRV8873ClockConfig);

    DL_TimerG_initPWMMode(
        DRV8873_INST, (DL_TimerG_PWMConfig *) &gDRV8873Config);

    // Set Counter control to the smallest CC index being used
    DL_TimerG_setCounterControl(DRV8873_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerG_setCaptureCompareOutCtl(DRV8873_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_0_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(DRV8873_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareValue(DRV8873_INST, 0, DL_TIMER_CC_0_INDEX);

    DL_TimerG_setCaptureCompareOutCtl(DRV8873_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_1_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(DRV8873_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    DL_TimerG_setCaptureCompareValue(DRV8873_INST, 0, DL_TIMER_CC_1_INDEX);

    DL_TimerG_enableClock(DRV8873_INST);


    
    DL_TimerG_setCCPDirection(DRV8873_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT );


}
/*
 * Timer clock configuration to be sourced by  / 1 (80000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   1012658.2278481013 Hz = 80000000 Hz / (1 * (78 + 1))
 */
static const DL_TimerA_ClockConfig gservoClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 78U
};

static const DL_TimerA_PWMConfig gservoConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
    .period = 20000,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_servo_init(void) {

    DL_TimerA_setClockConfig(
        servo_INST, (DL_TimerA_ClockConfig *) &gservoClockConfig);

    DL_TimerA_initPWMMode(
        servo_INST, (DL_TimerA_PWMConfig *) &gservoConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerA_setCounterControl(servo_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerA_setCaptureCompareOutCtl(servo_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(servo_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(servo_INST, 0, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(servo_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(servo_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(servo_INST, 0, DL_TIMER_CC_1_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(servo_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_2_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(servo_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_2_INDEX);
    DL_TimerA_setCaptureCompareValue(servo_INST, 0, DL_TIMER_CC_2_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(servo_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_3_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(servo_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_3_INDEX);
    DL_TimerA_setCaptureCompareValue(servo_INST, 0, DL_TIMER_CC_3_INDEX);

    DL_TimerA_enableClock(servo_INST);


    
    DL_TimerA_setCCPDirection(servo_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT | DL_TIMER_CC2_OUTPUT | DL_TIMER_CC3_OUTPUT );


}


static const DL_TimerG_ClockConfig gAB1ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};


SYSCONFIG_WEAK void SYSCFG_DL_AB1_init(void) {

    DL_TimerG_setClockConfig(
        AB1_INST, (DL_TimerG_ClockConfig *) &gAB1ClockConfig);

    DL_TimerG_configQEI(AB1_INST, DL_TIMER_QEI_MODE_2_INPUT,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_0_INDEX);
    DL_TimerG_configQEI(AB1_INST, DL_TIMER_QEI_MODE_2_INPUT,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_1_INDEX);
    DL_TimerG_setLoadValue(AB1_INST, 65535);
    DL_TimerG_enableClock(AB1_INST);
}
static const DL_TimerG_ClockConfig gAB2ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};


SYSCONFIG_WEAK void SYSCFG_DL_AB2_init(void) {

    DL_TimerG_setClockConfig(
        AB2_INST, (DL_TimerG_ClockConfig *) &gAB2ClockConfig);

    DL_TimerG_configQEI(AB2_INST, DL_TIMER_QEI_MODE_2_INPUT,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_0_INDEX);
    DL_TimerG_configQEI(AB2_INST, DL_TIMER_QEI_MODE_2_INPUT,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_1_INDEX);
    DL_TimerG_setLoadValue(AB2_INST, 65535);
    DL_TimerG_enableClock(AB2_INST);
}


static const DL_I2C_ClockConfig gOLEDClockConfig = {
    .clockSel = DL_I2C_CLOCK_BUSCLK,
    .divideRatio = DL_I2C_CLOCK_DIVIDE_1,
};

SYSCONFIG_WEAK void SYSCFG_DL_OLED_init(void) {

    DL_I2C_setClockConfig(OLED_INST,
        (DL_I2C_ClockConfig *) &gOLEDClockConfig);
    DL_I2C_setAnalogGlitchFilterPulseWidth(OLED_INST,
        DL_I2C_ANALOG_GLITCH_FILTER_WIDTH_50NS);
    DL_I2C_enableAnalogGlitchFilter(OLED_INST);

    /* Configure Controller Mode */
    DL_I2C_resetControllerTransfer(OLED_INST);
    /* Set frequency to 100000 Hz*/
    DL_I2C_setTimerPeriod(OLED_INST, 39);
    DL_I2C_setControllerTXFIFOThreshold(OLED_INST, DL_I2C_TX_FIFO_LEVEL_EMPTY);
    DL_I2C_setControllerRXFIFOThreshold(OLED_INST, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(OLED_INST);


    /* Enable module */
    DL_I2C_enableController(OLED_INST);


}

static const DL_UART_Main_ClockConfig gblueClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gblueConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_blue_init(void)
{
    DL_UART_Main_setClockConfig(blue_INST, (DL_UART_Main_ClockConfig *) &gblueClockConfig);

    DL_UART_Main_init(blue_INST, (DL_UART_Main_Config *) &gblueConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(blue_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(blue_INST, blue_IBRD_40_MHZ_115200_BAUD, blue_FBRD_40_MHZ_115200_BAUD);


    /* Configure FIFOs */
    DL_UART_Main_enableFIFOs(blue_INST);
    DL_UART_Main_setRXFIFOThreshold(blue_INST, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_setTXFIFOThreshold(blue_INST, DL_UART_TX_FIFO_LEVEL_1_2_EMPTY);

    DL_UART_Main_enable(blue_INST);
}
static const DL_UART_Main_ClockConfig gUART_4ClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_4Config = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_4_init(void)
{
    DL_UART_Main_setClockConfig(UART_4_INST, (DL_UART_Main_ClockConfig *) &gUART_4ClockConfig);

    DL_UART_Main_init(UART_4_INST, (DL_UART_Main_Config *) &gUART_4Config);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(UART_4_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_4_INST, UART_4_IBRD_80_MHZ_115200_BAUD, UART_4_FBRD_80_MHZ_115200_BAUD);


    /* Configure FIFOs */
    DL_UART_Main_enableFIFOs(UART_4_INST);
    DL_UART_Main_setRXFIFOThreshold(UART_4_INST, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_setTXFIFOThreshold(UART_4_INST, DL_UART_TX_FIFO_LEVEL_1_2_EMPTY);

    DL_UART_Main_enable(UART_4_INST);
}

static const DL_SPI_Config gTFT_SPI0_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_8,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
};

static const DL_SPI_ClockConfig gTFT_SPI0_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_TFT_SPI0_init(void) {
    DL_SPI_setClockConfig(TFT_SPI0_INST, (DL_SPI_ClockConfig *) &gTFT_SPI0_clockConfig);

    DL_SPI_init(TFT_SPI0_INST, (DL_SPI_Config *) &gTFT_SPI0_config);

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     8000000 = (80000000)/((1 + 4) * 2)
     */
    DL_SPI_setBitRateSerialClockDivider(TFT_SPI0_INST, 4);
    /* Set RX and TX FIFO threshold levels */
    DL_SPI_setFIFOThreshold(TFT_SPI0_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);

    /* Enable module */
    DL_SPI_enable(TFT_SPI0_INST);
}
static const DL_SPI_Config gIMU660RC_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_8,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
};

static const DL_SPI_ClockConfig gIMU660RC_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_IMU660RC_init(void) {
    DL_SPI_setClockConfig(IMU660RC_INST, (DL_SPI_ClockConfig *) &gIMU660RC_clockConfig);

    DL_SPI_init(IMU660RC_INST, (DL_SPI_Config *) &gIMU660RC_config);

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     8000000 = (80000000)/((1 + 4) * 2)
     */
    DL_SPI_setBitRateSerialClockDivider(IMU660RC_INST, 4);
    /* Set RX and TX FIFO threshold levels */
    DL_SPI_setFIFOThreshold(IMU660RC_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);

    /* Enable module */
    DL_SPI_enable(IMU660RC_INST);
}

/* ADC1 Initialization */
static const DL_ADC12_ClockConfig gADC1ClockConfig = {
    .clockSel       = DL_ADC12_CLOCK_SYSOSC,
    .divideRatio    = DL_ADC12_CLOCK_DIVIDE_4,
    .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,
};
SYSCONFIG_WEAK void SYSCFG_DL_ADC1_init(void)
{
    DL_ADC12_setClockConfig(ADC1_INST, (DL_ADC12_ClockConfig *) &gADC1ClockConfig);

    DL_ADC12_initSeqSample(ADC1_INST,
        DL_ADC12_REPEAT_MODE_DISABLED, DL_ADC12_SAMPLING_SOURCE_MANUAL, DL_ADC12_TRIG_SRC_SOFTWARE,
        DL_ADC12_SEQ_START_ADDR_00, DL_ADC12_SEQ_END_ADDR_01, DL_ADC12_SAMP_CONV_RES_12_BIT,
        DL_ADC12_SAMP_CONV_DATA_FORMAT_UNSIGNED);
    DL_ADC12_configConversionMem(ADC1_INST, ADC1_ADCMEM_DRC8873_ADC0,
        DL_ADC12_INPUT_CHAN_0, DL_ADC12_REFERENCE_VOLTAGE_VDDA_VSSA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_configConversionMem(ADC1_INST, ADC1_ADCMEM_DRC8873_ADC1,
        DL_ADC12_INPUT_CHAN_4, DL_ADC12_REFERENCE_VOLTAGE_VDDA_VSSA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_enableConversions(ADC1_INST);
}
/* ADC0_xunji Initialization */
static const DL_ADC12_ClockConfig gADC0_xunjiClockConfig = {
    .clockSel       = DL_ADC12_CLOCK_SYSOSC,
    .divideRatio    = DL_ADC12_CLOCK_DIVIDE_1,
    .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,
};
SYSCONFIG_WEAK void SYSCFG_DL_ADC0_xunji_init(void)
{
    DL_ADC12_setClockConfig(ADC0_xunji_INST, (DL_ADC12_ClockConfig *) &gADC0_xunjiClockConfig);
    DL_ADC12_configConversionMem(ADC0_xunji_INST, ADC0_xunji_ADCMEM_0,
        DL_ADC12_INPUT_CHAN_0, DL_ADC12_REFERENCE_VOLTAGE_VDDA_VSSA, DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_enableConversions(ADC0_xunji_INST);
}

