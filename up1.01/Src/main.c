/* *
  ******************************************************************************
  * File Name          : main.c
  * Date               : 16/08/2015 14:36:40
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/*  Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/*  USER CODE BEGIN Includes */
#define _LOCAL_MAIN

#include "string.h"
#include "app_user.h"
#include "app_sensor.h"
#include "app_mmi.h"
#include "app_led.h"
#include "app_fram.h"
#include "app_audio.h"
#include "app_wave.h"
#include "app_network.h"
#include "app_mg2639d.h"
#include "app_sim7600ce.h"
#include "app_fifo.h"
#include "app_prtc.h"
#include "stdio.h"
#include "app_comser.h"
#include "app_finger.h"
#include "app_datrec.h"
#include "app_dht22.h"
#include "app_sds011.h"
#include "app_upsnr.h"
#include "app_key.h"
#include "math.h"

/*  USER CODE END Includes */

/*  Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2S_HandleTypeDef hi2s2;
DMA_HandleTypeDef hdma_spi2_tx;

IWDG_HandleTypeDef hiwdg;

RNG_HandleTypeDef hrng;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi3;
DMA_HandleTypeDef hdma_spi3_tx;
DMA_HandleTypeDef hdma_spi3_rx;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim12;
TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;

NAND_HandleTypeDef hnand1;

/*  USER CODE BEGIN PV */

uint32_t timer_zigbeecfg;  // timer of zigbee config

uint32_t timer_rotatsave;   // timer of rotat-data save
BOOL flag_rotatsave;        // flag of rotat-data save

BOOL flag_welcome = TRUE;   // flag of welcome audio play

uint32_t frq1_count = 0, frq2_count = 0;        // freqence counter, globle



/*  USER CODE END PV */

/*  Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_FSMC_Init(void);
static void MX_I2S2_Init(void);
static void MX_IWDG_Init(void);
static void MX_RNG_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM8_Init(void);
static void MX_TIM9_Init(void);
static void MX_TIM12_Init(void);
static void MX_TIM14_Init(void);
static void MX_UART4_Init(void);
static void MX_UART5_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USART6_UART_Init(void);

/*  USER CODE BEGIN PFP */

/*  USER CODE END PFP */

/*  USER CODE BEGIN 0 */

/*  USER CODE END 0 */

int main(void)
{

	/*  USER CODE BEGIN 1 */



	/** CAUTION! MX ERR, SHOULD BE MODIFIED **/
	/* 
		1. HES_ON
		2. RTC TIME DEINITIALIZATION
		3. FPU ENABLE
			// ,__FPU_PRESENT=1,__FPU_USED =1,ARM_MATH_CM4,__CC_ARM
	*/
	/* ********** END OF CAUTION! *************/
//  extern static __IO uint32_t uwTick;
//  uwTick = 0;

	/*  USER CODE END 1 */

	/*  MCU Configuration----------------------------------------------------------*/

	/*  Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/*  Configure the system clock */
	SystemClock_Config();

	/*  Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	MX_FSMC_Init();
	MX_I2S2_Init();
	MX_IWDG_Init();
	MX_RNG_Init();
	MX_RTC_Init();
	MX_SPI3_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM5_Init();
	MX_TIM6_Init();
	MX_TIM7_Init();
	MX_TIM8_Init();
	MX_TIM9_Init();
	MX_TIM12_Init();
	MX_TIM14_Init();
	MX_UART4_Init();
	MX_UART5_Init();
	MX_USART1_UART_Init();
	MX_USART3_UART_Init();
	MX_USART6_UART_Init();

	/*  USER CODE BEGIN 2 */

	tick_powerup = HAL_GetTick();

//      _WDOG_ON();

	_FM_NCS_DISABLE();      // initailize

	__HAL_RCC_RTC_ENABLE(); // effective only first-time after release

	_WDOG_KICK();

	gprs_rssi = 0;             // no signal
	DTU_rssi = 99;
	zigbee_rssi = 0;                // no signal
	gps_fixed = 0x0;    //FF;       // invalid

//  flag_torque_check = FALSE;  // flag of torque check
//  flag_zone_check = FALSE;        // flag of zone check
	flag_wklp_check = FALSE;        // flag of work-loop check
//  flag_collision_check = FALSE;   // flag of collision check
	flag_alarm_find = FALSE;        // flag of alarm find
//  flag_zigbee_cfg = FALSE;        // flag of zigbee config


	_WDOG_KICK();

	APP_fram_ini();
	APP_sensor_gpio_ini();

	_LED_TST2_OFF();
	_LED_TST3_OFF();
	_LED_TST4_OFF();
	_LED_RUN_ON();
	_LED_COM_OFF();
	_LED_ERR_OFF();

	//_ZIGBEE_CFG_HIGH();

	_GPRS_PWKEY_RELEASE();
	_VGPRS_OFF();   // TBD

	APP_ADC1_ini();     // use for MX error.

	_WDOG_KICK();

	APP_devver_ini();
	APP_devinfo_ini();
	APP_dustmoninfo_ini();
	APP_notice_ini();
	APP_limittbl_ini();
	APP_floortbl_ini();
	APP_calitbl_ini();
	APP_sensor_dat_ini();           // set default data
	APP_sensor_value_ini();         // set default value
	APP_sensor_valvestat_ini();     // set valve state

	_WDOG_KICK();


	/*  USER CODE END 2 */

	/*  USER CODE BEGIN 3 */

	HAL_TIM_Base_Start_IT(&htim6);
//  HAL_TIM_Base_Start_IT(&htim7);

    PWM1_START();   
    PWM2_START();   
    PWM3_START();   
    PWM4_START();   
    PWM5_START();   
                    
    FRQ_START_ALL();

	_WDOG_KICK();

	APP_sensor_alarm_ini();


	// novar init
	NETWORK_INIT();

	//DHT22_Init();

	APP_UPSNR_Init();

	APP_InitKey();

	// novar init end

	APP_audio_ini();

	APP_mmi_ini();

	APP_led_ini();

#ifdef  WDOG_ON
	_WDOG_ON();
#endif
				
	APP_UPSNR_OPENALARM();

	/*  Infinite loop */

	while (1)
	{

#ifdef _AUDIO_TIPS
		if (flag_welcome)
		{
			if (HAL_GetTick() - tick_powerup >= 2500)
			{
				flag_welcome = FALSE;				
				APP_UPSNR_CLOSEALARM();
				APP_mmi_ini();  // in case of error
			}
		}
#endif

#ifndef WDOG_ON
		_LED_TST2_TOGGLE();
#endif

		if (gprs_rssi == 99)
		{
			_LED_COM_OFF();
		} else
		{
			_LED_COM_ON();
		}

		_WDOG_KICK();


		{   // nvram management
			APP_devinfo_manage();
			APP_devver_manage();
			APP_dustmoninfo_manage();
			APP_notice_manage();
			APP_limittbl_manage();
			APP_floortbl_manage();
			APP_calitbl_manage();

			_WDOG_KICK();
			if (flag_datsave)
			{
				flag_datsave = FALSE;
				// prepare data to save
				APP_sensor_savprdvalue();

				// save real-time data into NAND-FLASH
				{
					//static uint32_t id;
					uint8_t cnt = 0;

					while (DATREC_EXC(&NAND_WPTR))
					{
						cnt++;
						if (cnt > 10) 
						{
							cnt = 0;
							APP_NAND_MDF(); 
							break;
						}
						//id = NAND_ReadID();
					}
				}
				// save write-pointer of NAND-FLASH into FRAM
				APP_fram_writedata(HDL_SPI_FRAM,\
						   NVADDR_NANDWR,\
						   (uint8_t *)&NAND_WPTR,\
						   sizeof(NAND_WPTR));
			}

			if (flag_datrpt)
			{
				flag_datrpt = FALSE;

				if (device_info.work_mod == WKMOD_CALI)
				{
					// prepare calibration data to send
					APP_sensor_sendrawdat();
				} else
//              if (device_info.work_mod == WKMOD_MIX || \
//                  device_info.work_mod == WKMOD_SELF || \
//                  device_info.work_mod == WKMOD_ASK)
				{
					// prepare period-value to send
					APP_sensor_sendprdvalue();
				}
			}

			_WDOG_KICK();

		}

		_WDOG_KICK();

		if (device_info.work_mod != WKMOD_CALI)
		{
			if (flag_wklp_check)
			{
				APP_sensor_wklpchk();
				flag_wklp_check = FALSE;
			}

			if (flag_alarm_find)
			{
				APP_sensor_findalarm();
				flag_alarm_find = FALSE;
			}
		}


		_WDOG_KICK();


		//HAL_UART_Receive_IT(HDL_UART_TFT, &mmi_rcvbyte, 1);
		if (mmi_rcvbuf.valid)
		{
			APP_mmi_input();
			mmi_rcvbuf.valid = FALSE;
		}

		_WDOG_KICK();

		if (!mmi_sendbuf.valid && mmi_stat.update)
		{
//_LED_TST2_ON();
			APP_mmi_display();
//_LED_TST2_OFF();
			mmi_stat.update = FALSE;
		}
		_WDOG_KICK();

		APP_led_task();

		_WDOG_KICK();

		APP_ADC1_convert();

		if (HAL_GetTick() - tick_powerup >= 10000)
		{
			APP_sensor_stateupdate();
		}
		_WDOG_KICK();
		// novar task

		NETWORK_TASK();

		_WDOG_KICK();

		APP_UPSNR_TASK();

		_WDOG_KICK();

		APP_KEY_TASK();
		// novar task end
	}

	/*  USER CODE END 3 */

}

/* * System Clock Configuration
*/
void SystemClock_Config(void)
{

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	__PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI
		| RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 320;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 8;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
		| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S | RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
	PeriphClkInitStruct.PLLI2S.PLLI2SR = 4;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);

	HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_HSE, RCC_MCODIV_4);

}

/*  ADC1 init function */
void MX_ADC1_Init(void)
{

	ADC_ChannelConfTypeDef sConfig;

	/* *Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	*/
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV8;
	hadc1.Init.Resolution = ADC_RESOLUTION12b;
	hadc1.Init.ScanConvMode = ENABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 10;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = EOC_SEQ_CONV;
	HAL_ADC_Init(&hadc1);

	/* *Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Channel = ADC_CHANNEL_5;
	sConfig.Rank = 6;
	sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	/* *Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Rank = 9;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	/* *Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Rank = 7;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	/* *Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Rank = 8;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	/* *Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = 5;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	/* *Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Channel = ADC_CHANNEL_5;
	sConfig.Rank = 10;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	/* *Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = 3;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	/* *Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Rank = 1;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	/* *Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Rank = 4;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

	/* *Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Rank = 2;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);

}

/*  I2S2 init function */
void MX_I2S2_Init(void)
{

	hi2s2.Instance = SPI2;
	hi2s2.Init.Mode = I2S_MODE_MASTER_TX;
	hi2s2.Init.Standard = I2S_STANDARD_LSB;
	hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
	hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
	hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_8K;
	hi2s2.Init.CPOL = I2S_CPOL_LOW;
	hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
	hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
	HAL_I2S_Init(&hi2s2);

}

/*  IWDG init function */
void MX_IWDG_Init(void)
{

	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
	hiwdg.Init.Reload = 4000;
	HAL_IWDG_Init(&hiwdg);

}

/*  RNG init function */
void MX_RNG_Init(void)
{

	hrng.Instance = RNG;
	HAL_RNG_Init(&hrng);

}

/*  RTC init function */
void MX_RTC_Init(void)
{
/* 
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
*/
	/* *Initialize RTC and set the Time and Date
	*/
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	HAL_RTC_Init(&hrtc);
/* 
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.SubSeconds = 0;
  sTime.TimeFormat = RTC_HOURFORMAT12_AM;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  //HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BCD);

  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;
  //HAL_RTC_SetDate(&hrtc, &sDate, FORMAT_BCD);
*/
	/* *Enable Calibrartion
	*/
	HAL_RTCEx_SetCalibrationOutPut(&hrtc, RTC_CALIBOUTPUT_1HZ);

}

/*  SPI3 init function */
void MX_SPI3_Init(void)
{

	hspi3.Instance = SPI3;
	hspi3.Init.Mode = SPI_MODE_MASTER;
	hspi3.Init.Direction = SPI_DIRECTION_2LINES;
	hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi3.Init.NSS = SPI_NSS_SOFT;
	hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi3.Init.TIMode = SPI_TIMODE_DISABLED;
	hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
	HAL_SPI_Init(&hspi3);

}

/*  TIM2 init function */
void MX_TIM2_Init(void)
{

	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_IC_InitTypeDef sConfigIC;

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 799;
	htim2.Init.CounterMode = TIM_COUNTERMODE_DOWN;
	htim2.Init.Period = 49999;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&htim2);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

	HAL_TIM_IC_Init(&htim2);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 8;
	HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2);

	HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_4);
}

/*  TIM3 init function */
void MX_TIM3_Init(void)
{

	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_SlaveConfigTypeDef sSlaveConfig;
	TIM_IC_InitTypeDef sConfigIC;
	TIM_MasterConfigTypeDef sMasterConfig;

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 799;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 65535;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&htim3);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);

	HAL_TIM_IC_Init(&htim3);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI2FP2;
	sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
	sSlaveConfig.TriggerFilter = 3;
	HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 3;
	HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

}

/*  TIM5 init function */
void MX_TIM5_Init(void)
{

	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_SlaveConfigTypeDef sSlaveConfig;
	TIM_IC_InitTypeDef sConfigIC;
	TIM_MasterConfigTypeDef sMasterConfig;

	htim5.Instance = TIM5;
	htim5.Init.Prescaler = 799;
	htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim5.Init.Period = 110000;
	htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&htim5);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig);

	HAL_TIM_IC_Init(&htim5);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
	sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
	sSlaveConfig.TriggerFilter = 3;
	HAL_TIM_SlaveConfigSynchronization(&htim5, &sSlaveConfig);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 3;
	HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_1);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
	HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_2);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig);

}

/*  TIM6 init function */
void MX_TIM6_Init(void)
{

	TIM_MasterConfigTypeDef sMasterConfig;

	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 799;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 499;
	HAL_TIM_Base_Init(&htim6);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);

}

/*  TIM7 init function */
void MX_TIM7_Init(void)
{

	TIM_MasterConfigTypeDef sMasterConfig;

	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 79;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = 19;
	HAL_TIM_Base_Init(&htim7);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig);

}

/*  TIM8 init function */
void MX_TIM8_Init(void)
{

	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_SlaveConfigTypeDef sSlaveConfig;
	TIM_IC_InitTypeDef sConfigIC;
	TIM_MasterConfigTypeDef sMasterConfig;

	htim8.Instance = TIM8;
	htim8.Init.Prescaler = 799;
	htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim8.Init.Period = 65535;
	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim8.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&htim8);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig);

	HAL_TIM_IC_Init(&htim8);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
	sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
	sSlaveConfig.TriggerFilter = 3;
	HAL_TIM_SlaveConfigSynchronization(&htim8, &sSlaveConfig);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 3;
	HAL_TIM_IC_ConfigChannel(&htim8, &sConfigIC, TIM_CHANNEL_1);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
	HAL_TIM_IC_ConfigChannel(&htim8, &sConfigIC, TIM_CHANNEL_2);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig);

}

/*  TIM9 init function */
void MX_TIM9_Init(void)
{

	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_SlaveConfigTypeDef sSlaveConfig;
	TIM_IC_InitTypeDef sConfigIC;

	htim9.Instance = TIM9;
	htim9.Init.Prescaler = 7999;
	htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim9.Init.Period = 11000;
	htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&htim9);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig);

	HAL_TIM_IC_Init(&htim9);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
	sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
	sSlaveConfig.TriggerFilter = 03;
	HAL_TIM_SlaveConfigSynchronization(&htim9, &sSlaveConfig);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 03;
	HAL_TIM_IC_ConfigChannel(&htim9, &sConfigIC, TIM_CHANNEL_1);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
	HAL_TIM_IC_ConfigChannel(&htim9, &sConfigIC, TIM_CHANNEL_2);
}

/*  TIM12 init function */
void MX_TIM12_Init(void)
{

	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_SlaveConfigTypeDef sSlaveConfig;
	TIM_IC_InitTypeDef sConfigIC;

	htim12.Instance = TIM12;
	htim12.Init.Prescaler = 799;
	htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim12.Init.Period = 65535;
	htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&htim12);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig);

	HAL_TIM_IC_Init(&htim12);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
	sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
	sSlaveConfig.TriggerFilter = 3;
	HAL_TIM_SlaveConfigSynchronization(&htim12, &sSlaveConfig);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 3;
	HAL_TIM_IC_ConfigChannel(&htim12, &sConfigIC, TIM_CHANNEL_1);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
	HAL_TIM_IC_ConfigChannel(&htim12, &sConfigIC, TIM_CHANNEL_2);

}

/*  TIM14 init function */
void MX_TIM14_Init(void)
{

	htim14.Instance = TIM14;
	htim14.Init.Prescaler = 65535;
	htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim14.Init.Period = 50000;
	htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&htim14);

}

/*  UART4 init function */
void MX_UART4_Init(void)
{

	huart4.Instance = UART4;
	huart4.Init.BaudRate = 38400;
	huart4.Init.WordLength = UART_WORDLENGTH_8B;
	huart4.Init.StopBits = UART_STOPBITS_1;
	huart4.Init.Parity = UART_PARITY_NONE;
	huart4.Init.Mode = UART_MODE_TX_RX;
	huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart4.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart4);

}

/*  UART5 init function */
void MX_UART5_Init(void)
{

	huart5.Instance = UART5;
	huart5.Init.BaudRate = 115200;
	huart5.Init.WordLength = UART_WORDLENGTH_8B;
	huart5.Init.StopBits = UART_STOPBITS_1;
	huart5.Init.Parity = UART_PARITY_NONE;
	huart5.Init.Mode = UART_MODE_TX_RX;
	huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart5.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart5);

}

/*  USART1 init function */
void MX_USART1_UART_Init(void)
{

	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart1);

}

/*  USART3 init function */
void MX_USART3_UART_Init(void)
{

	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart3);

}

/*  USART6 init function */
void MX_USART6_UART_Init(void)
{

	huart6.Instance = USART6;
	huart6.Init.BaudRate = 115200;
	huart6.Init.WordLength = UART_WORDLENGTH_8B;
	huart6.Init.StopBits = UART_STOPBITS_1;
	huart6.Init.Parity = UART_PARITY_NONE;
	huart6.Init.Mode = UART_MODE_TX_RX;
	huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart6.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart6);

}

/* *
  * Enable DMA controller clock
  */
void MX_DMA_Init(void)
{
	/*  DMA controller clock enable */
	__DMA1_CLK_ENABLE();
	__DMA2_CLK_ENABLE();

	/*  DMA interrupt init */
	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
	HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

}

/* * Configure pins as
		* Analog
		* Input
		* Output
		* EVENT_OUT
		* EXTI
	 PC9   ------> RCC_MCO_2
	 PA8   ------> RCC_MCO_1
*/
void MX_GPIO_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct;

	/*  GPIO Ports Clock Enable */
	__GPIOE_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOH_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();
	__GPIOG_CLK_ENABLE();

	/* Configure GPIO pins : PE3 */
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/* Configure GPIO pins : PC0 PC1 PC2 PC7 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Configure GPIO pin : PB11 */
	GPIO_InitStruct.Pin = GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* Configure GPIO pin : PG2 PG3 PG4 */
	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/* Configure GPIO pins : PG5 PG7 PG8 */
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/* Configure GPIO pin : PC9 */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Configure GPIO pin : PA8 */
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Configure GPIO pins : PA11 */
	GPIO_InitStruct.Pin = GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Configure GPIO pin : PA12 PA15 */
	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Configure GPIO pin : PD3 */
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* Configure GPIO pin : PG15 */
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

}

/*  FSMC initialization function */
void MX_FSMC_Init(void)
{
	FSMC_NAND_PCC_TimingTypeDef ComSpaceTiming;
	FSMC_NAND_PCC_TimingTypeDef AttSpaceTiming;

	/* * Perform the NAND1 memory initialization sequence
	*/
	hnand1.Instance = FSMC_NAND_DEVICE;
	/*  hnand1.Init */
	hnand1.Init.NandBank = FSMC_NAND_BANK2;
	hnand1.Init.Waitfeature = FSMC_NAND_PCC_WAIT_FEATURE_ENABLE;
	hnand1.Init.MemoryDataWidth = FSMC_NAND_PCC_MEM_BUS_WIDTH_8;
	hnand1.Init.EccComputation = FSMC_NAND_ECC_DISABLE;
	hnand1.Init.ECCPageSize = FSMC_NAND_ECC_PAGE_SIZE_256BYTE;
	hnand1.Init.TCLRSetupTime = 1;
	hnand1.Init.TARSetupTime = 1;
	/*  hnand1.Info */
	/*  ComSpaceTiming */
	ComSpaceTiming.SetupTime = 1;
	ComSpaceTiming.WaitSetupTime = 2;
	ComSpaceTiming.HoldSetupTime = 3;
	ComSpaceTiming.HiZSetupTime = 2;
	/*  AttSpaceTiming */
	AttSpaceTiming.SetupTime = 2;
	AttSpaceTiming.WaitSetupTime = 2;
	AttSpaceTiming.HoldSetupTime = 3;
	AttSpaceTiming.HiZSetupTime = 3;

	HAL_NAND_Init(&hnand1, &ComSpaceTiming, &AttSpaceTiming);

}

/*  USER CODE BEGIN 4 */


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	static uint32_t pwm_high, pwm_prd;
	static float pwm_duty;



	if (htim->Instance == TIM2)
	{
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			frq1_count++;
		} else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
		{
			frq2_count++;
		}
	}

	if (htim->Instance == TIM5) // PWM1 capture
	{
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			pwm_prd = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_1);
			pwm_high = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_2);

			pwm_duty = 1.0f * pwm_high / pwm_prd;
			pwm_duty = (pwm_duty > 1.0f) ? 1.0f : pwm_duty;
			pwm_duty = (pwm_duty < 0.0f) ? 0.0f : pwm_duty;

			_APP_SET_SENSOR_DAT(CH_PWM1, pwm_duty);

		}
	}

	if (htim->Instance == TIM9) // PWM2 capture
	{
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			pwm_prd = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_1);
			pwm_high = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_2);

			pwm_duty = 1.0f * pwm_high / pwm_prd;
			pwm_duty = (pwm_duty > 1.0f) ? 1.0f : pwm_duty;
			pwm_duty = (pwm_duty < 0.0f) ? 0.0f : pwm_duty;

			_APP_SET_SENSOR_DAT(CH_PWM2, pwm_duty);
		}
	}

	if (htim->Instance == TIM3) // PWM3 capture
	{
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			pwm_prd = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_2);
			pwm_high = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_1);
			pwm_duty = 1.0f * pwm_high / pwm_prd;
			pwm_duty = (pwm_duty > 1.0f) ? 1.0f : pwm_duty;
			pwm_duty = (pwm_duty < 0.0f) ? 0.0f : pwm_duty;
//          pwm_duty = sensor_dat[PWM3].scale * pwm_duty + 0.5f;

			_APP_SET_SENSOR_DAT(CH_PWM3, pwm_duty);
		}
	}

	if (htim->Instance == TIM12)    // PWM4 capture
	{
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			pwm_prd = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_1);
			pwm_high = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_2);
			pwm_duty = 1.0f * pwm_high / pwm_prd;
			pwm_duty =  (pwm_duty > 1.0f) ? 1.0f : pwm_duty;
			pwm_duty =   (pwm_duty < 0.0f) ? 0.0f : pwm_duty;
//          pwm_duty = sensor_dat[PWM4].scale * pwm_duty + 0.5f;

			_APP_SET_SENSOR_DAT(CH_PWM4, pwm_duty);
		}
	}

	if (htim->Instance == TIM8) // PWM5 capture
	{
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			pwm_prd = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_1);
			pwm_high = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_2);
			pwm_duty = 1.0f * pwm_high / pwm_prd;
			pwm_duty =  (pwm_duty > 1.0f) ? 1.0f : pwm_duty;
			pwm_duty =   (pwm_duty < 0.0f) ? 0.0f : pwm_duty;
//          pwm_duty = sensor_dat[PWM5].scale * pwm_duty + 0.5f;

			_APP_SET_SENSOR_DAT(CH_PWM5, pwm_duty);
		}
	}

}




void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	if (htim->Instance == TIM2)
	{
		_APP_SET_SENSOR_DAT(CH_FRQ1, (uint32_t)(frq1_count << 1));
		_APP_SET_SENSOR_DAT(CH_FRQ2, (uint32_t)(frq2_count << 1));


		frq1_count = frq2_count = 0;
	}

	if (htim->Instance == TIM6)
	{
		// key scan
		{
			static uint8_t s_count = 0;
			if (++s_count >= 2)
			{
				s_count = 0;

				APP_KeyScan();		/* 按键扫描 */
									/* 每隔10ms调用一次此函数，此函数在 app_key.c */
			}
		}
		/* test uart6
		{
			static uint16_t cnt = 0;
			if (++cnt >= 400)
			{
				cnt = 0;
				APP_UPSNR_OPENALARM();
			}
		} */
		//  update timers 10 seconds after power-up 
		if (HAL_GetTick() - tick_powerup >= 10000)
		{
			static uint32_t mili_seconds = 120;

			mili_seconds += TIM6_STEP;
			if (mili_seconds >= 1000)
			{
				mili_seconds = 0;

				// process baseed-on seconds
				;
			}


			// update data-save timer
			timer_datsave += TIM6_STEP;
			if (timer_datsave >= device_info.datsave_time * 1000)
			{
				flag_datsave = TRUE;
				timer_datsave =  0;
			}
		}

		if (HAL_GetTick() - tick_powerup >= 20000)
		{
			// update data-report timer
			timer_datrpt += TIM6_STEP;
			if (timer_datrpt >= device_info.datrpt_time * 1000)
			{
				flag_datrpt = TRUE;
				timer_datrpt =  0;
			}

			// update alarm-find timer
			static uint32_t timer_findalarm = 50;
			if (timer_findalarm >= TIM_FINDALARM)
			{
				flag_alarm_find = TRUE;
				timer_findalarm =  0;
			} else if (!flag_alarm_find)
			{
				timer_findalarm += TIM6_STEP;
			}
		}

		// update configure-save delay timer
		{
			if (device_info.delay >= TIM6_STEP) device_info.delay -= TIM6_STEP;
			else device_info.delay = 0;

			if (device_ver.delay >= TIM6_STEP) device_ver.delay -= TIM6_STEP;
			else device_ver.delay = 0;

			if (dustmon_info.delay >= TIM6_STEP) dustmon_info.delay -= TIM6_STEP;
			else dustmon_info.delay = 0;

			if (dustmon_notice.delay >= TIM6_STEP) dustmon_notice.delay -= TIM6_STEP;
			else dustmon_notice.delay = 0;

			if (limit_tbl.delay >= TIM6_STEP) limit_tbl.delay -= TIM6_STEP;
			else limit_tbl.delay = 0;

			if (floor_tbl.delay >= TIM6_STEP) floor_tbl.delay -= TIM6_STEP;
			else floor_tbl.delay = 0;

			if (cali_tbl.delay >= TIM6_STEP) cali_tbl.delay -= TIM6_STEP;
			else cali_tbl.delay = 0;
		}


		// update sensor time-out counter
		{
			uint32_t index;

			for (index = 0; index < SENSOR_MAXCHANNEL; index++)
			{
				if (sensor_dat[index].timeout < ~0x100) sensor_dat[index].timeout += TIM6_STEP;
			}
		}

		// update TFT update counter
		{
			static uint32_t cnt_tftupdate = 0;

			cnt_tftupdate += TIM6_STEP;
			if (cnt_tftupdate >= 495)
			{
				cnt_tftupdate = 0;
				mmi_stat.update = TRUE;
			}

			switch (cnt_tftupdate % 100)
			{
			case 10:
				//flag_torque_check = TRUE;
				break;

			case 30:
				//flag_zone_check = TRUE;
				break;

			case 50:
				flag_wklp_check = TRUE;
				break;

			case 70:
				//flag_collision_check = TRUE;
				break;

			case 90:
				//flag_collision_check = TRUE;
				break;

			default:
				break;
			}
		}

		APP_sensor_raw2value();             // translate sensor raw-data into engineering quantity.

		APP_sensor_gpio_update(TIM6_STEP);      // update gpio state


		{   // let LED4 flash in 1 Hz with 10% duty.

			static unsigned int uiCnt_led_tim6 = 0;

			if (uiCnt_led_tim6 < 100) _LED_TST4_ON();
			else _LED_TST4_OFF();

			uiCnt_led_tim6 += TIM6_STEP;
			if (uiCnt_led_tim6 >= 1000) uiCnt_led_tim6 = 0;

		}
		// heart bag flag
		{
			static unsigned int uiCnt_heart_tim6 = 0;

			uiCnt_heart_tim6 += TIM6_STEP;
			if (uiCnt_heart_tim6 >= device_info.beat_time * 60 * 1000)
			{
				StatusFlag.STAT_HEART_OV = 1;
				uiCnt_heart_tim6 = 0;
			}
		}
		// 查询dtu信号强度
		// add "AT+CSQ? "
		{
			static unsigned int uiCnt_csq_tim7 = 0;

			uiCnt_csq_tim7 += TIM6_STEP;
			if (uiCnt_csq_tim7 >= 60 * 1000)	// 默认一分钟查询一次
			{
				StatusFlag.STAT_CSQ_CHK = 1;
				uiCnt_csq_tim7 = 0;
			}
		}

	}

	if (htim->Instance == TIM14)
	{
		RxOverDly();
	}
}



void HAL_SYSTICK_Callback(void)
{
	static unsigned int uiCnt_led_tick = 0;

	if (uiCnt_led_tick < 500) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);

	if (++uiCnt_led_tick >= 1000) uiCnt_led_tick = 0;
}



void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
/* 
	uint32_t    sum, max,min;
	uint32_t    dat;
	uint32_t    index;

	if (hadc->Instance == ADC1)
	{
		HAL_ADC_Stop_DMA(&hadc1);

		sum = 0;
		max = min = ADC1_DMAbuf[0];
		for (index = 0; index < 8; index++)
		{
			dat = ADC1_DMAbuf[index];
			sum += dat;
			max = (dat > max) ? dat : max;
			min = (dat < min) ? dat : min;
		}
		 sum = sum - max - min;
		 sum /= 6;
		 if (sum > (MAX_ADC_DAT / 10)) _APP_SET_SENSOR_DAT(CH_ANA1, sum);

		sum = 0;
		max = min = ADC1_DMAbuf[8];
		for (index = 8; index < 16; index++)
		{
			dat = ADC1_DMAbuf[index];
			sum += dat;
			max = (dat > max) ? dat : max;
			min = (dat < min) ? dat : min;
		}
		 sum = sum - max - min;
		 sum /= 6;
		 if (sum > (MAX_ADC_DAT / 10)) _APP_SET_SENSOR_DAT(CH_ANA2, sum);
	}

*/
}




void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == UART_TFT)
	{
		APP_mmi_rcvisr();
	}

	if (huart->Instance == UART_LED)
	{
		APP_led_rcvisr();
	}

	if (huart->Instance == UART_ZIGBEE)
	{
		;
	}

	if (huart->Instance == UART_GPRS)
	{
		recvfifo_5.in = 0;    // 初始化写指针
		HAL_UART_Receive_IT(HDL_UART_GPRS, recvbuf_5, QUEUE_REV_SIZE);
	}

	if (huart->Instance == UART_UP)
	{
        recvfifo_6.in = 0;    // 初始化写指针                       
        HAL_UART_Receive_IT(HDL_UART_UP, recvbuf_6, QUEUE_REV_SIZE);
	}
}



void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == UART_TFT)
	{
		mmi_sendbuf.valid = FALSE;
	}
	if (huart->Instance == UART_LED)
	{
		led_sendbuf.valid = FALSE;
	}
	if (huart->Instance == UART_ZIGBEE)
	{
		;
	}
	if (huart->Instance == UART_GPRS)
	{
		smflag.UART_TX_BUSY = 0;
		StatusFlag.STAT_GPRSSEND_BUSY = 0;
	}
	if (huart->Instance == UART_232)
	{
		StatusFlag.STAT_PRINT_BUSY = 0;
	}

}



void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	static char strbuf[20];
	__IO uint32_t stat_reg, data_reg;

	//huart->ErrorCode = HAL_UART_ERROR_NONE;
	//huart->State= HAL_UART_STATE_READY;

	{
		// Disable the UART Transmit Complete Interrupt
		__HAL_UART_DISABLE_IT(huart, UART_IT_TXE);

		// disable the UART Receive Interrupt
		__HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);

		// Disable the UART Parity Error Interrupt
		__HAL_UART_DISABLE_IT(huart, UART_IT_PE);

		// Disable the UART Error Interrupt: (Frame error, noise error, overrun error)
		__HAL_UART_DISABLE_IT(huart, UART_IT_ERR);

	}

	stat_reg = huart->Instance->SR;
	data_reg = huart->Instance->DR;


	if (huart->Instance == UART_TFT)
	{
		HAL_UART_Receive_IT(HDL_UART_TFT, &mmi_rcvbyte, 1);
		mmi_sendbuf.valid = FALSE;
	}

	if (huart->Instance == UART_LED)
	{
		HAL_UART_Receive_IT(HDL_UART_LED, &led_rcvbyte, 1);
		led_sendbuf.valid = FALSE;
	}

	if (huart->Instance == UART_ZIGBEE)
	{
		;
	}

	if (huart->Instance == UART_GPRS)
	{
		HAL_UART_Receive_IT(HDL_UART_GPRS, recvbuf_5, QUEUE_REV_SIZE);
		smflag.UART_TX_BUSY = 0;
		StatusFlag.STAT_GPRSSEND_BUSY = 0;

		sprintf((char *)strbuf, "error code:%X\r\n", huart->ErrorCode);
#ifdef PRINT_UART1
		novar_print((uint8_t *)strbuf, (uint16_t)strlen(strbuf));
#endif
	}

	if (huart->Instance == UART_232)
	{
		StatusFlag.STAT_PRINT_BUSY = 0;
	}

	if (huart->Instance == UART_UP)
	{
		HAL_UART_Receive_IT(HDL_UART_UP, recvbuf_6, QUEUE_REV_SIZE);
		//smflag.UART_TX_BUSY = 0;
		//StatusFlag.STAT_GPRSSEND_BUSY = 0;

		sprintf((char *)strbuf, "error code:%X\r\n", huart->ErrorCode);
	}

}


/* **************************************************
 fucntion:      HAL_I2S_TxCpltCallback
 input:
 output:
 describe:  IIS transmit ISR
***************************************************/

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s == HDL_IIS_SPK)
	{
		if (audio_play.flag == AUDIOSTAT_PLAYING)
		{
			if (++audio_play.idx < audio_play.size)
			{           // enable next play
#ifdef IIS_SPK_DMA
				HAL_I2S_Transmit_DMA(hi2s,\
										 audio_play.list[audio_play.idx].wave,\
										 audio_play.list[audio_play.idx].samples);
#else
				HAL_I2S_Transmit_IT(hi2s,\
										audio_play.list[audio_play.idx].wave,\
										audio_play.list[audio_play.idx].samples);
#endif
			} else
			{       // play finished
#ifdef IIS_SPK_DMA
				HAL_I2S_DMAStop(hi2s);
#endif
				__HAL_I2S_DISABLE(hi2s);
				//_AMP_OFF();

				audio_play.flag = AUDIOSTAT_IDLE;
				audio_play.size = 0;
				audio_play.idx = 0;
			}
		}
	}
}


void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
	if (hi2s == HDL_IIS_SPK)
	{
		APP_audio_stop();
	}
}


// delay 3FCLKs, 18.75ns @ FCLK = 160MHz
// addtional 12 FCLKs used in call-return.
void delay(uint32_t clk)
{
	while (clk--);  // cost 3 FCLKs
}

void delay_us(uint32_t us)
{
	uint32_t x;

	while (us--)
	{
		x = 54;     // 1.015us @ 80MHz
		while (x--);
	}
}

void delay_ms(uint32_t ms)
{
	while (ms--) delay_us(1000);
}


/* **************************************************
 fucntion:      APP_ADC1_ini
 input:
 output:
 describe:  initial ADC1, used for MX error only.
***************************************************/

void APP_ADC1_ini(void)
{
	ADC_ChannelConfTypeDef sConfig;

	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV8;
	hadc1.Init.Resolution = ADC_RESOLUTION12b;
	hadc1.Init.ScanConvMode = ENABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;             // 16
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = EOC_SEQ_CONV; // EOC_SINGLE_CONV;
	HAL_ADC_Init(&hadc1);

	sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	sConfig.Rank = 1;
	sConfig.Channel = ADC_CHANNEL_4;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}



/* **************************************************
 fucntion:      APP_ADC1_convert
 input:
 output:
 describe:  ADC1 convert manage
***************************************************/

void APP_ADC1_convert(void)
{
#define ADC_BUFSIZE 8
#define ADC_CUTOFF  0   //(MAX_ADC_DAT / 50)

	ADC_ChannelConfTypeDef sConfig;
	static uint32_t step = 0;
	static uint32_t buf[ADC_BUFSIZE];
	static uint32_t idx_buf = 0;
	static uint32_t max, min, dat, sum;

	switch (step)
	{
	case 0:
		// first time income, start CH_ANA1 convert
		//APP_ADC1_ini();
		sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
		sConfig.Rank = 1;
		sConfig.Channel = ADC_CHANNEL_4;
		HAL_ADC_ConfigChannel(&hadc1, &sConfig);
		HAL_ADC_Start(&hadc1);

		for (idx_buf = 0; idx_buf < ADC_BUFSIZE; idx_buf++)
		{
			buf[idx_buf] = 0;
		}
		idx_buf = 0;
		step = 1;

		break;

	case 1:
		// check CH_ANA1 convert finished
		if (HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK)
		{
			dat = HAL_ADC_GetValue(&hadc1);
			buf[idx_buf++] = dat;

			if (dat > ADC_CUTOFF) sensor_dat[CH_ANA1].timeout = 0;

			if (idx_buf >= ADC_BUFSIZE)
			{
				// set sensor data
				sum = 0;
				max = min = buf[0];
				for (idx_buf = 0; idx_buf < ADC_BUFSIZE; idx_buf++)
				{
					dat = buf[idx_buf];
					sum += dat;
					max = (dat > max) ? dat : max;
					min = (dat < min) ? dat : min;
				}
				sum = sum - max - min;
				sum /= (ADC_BUFSIZE - 2);
				//sum = 20*log10(sum/10);
				if (sum > ADC_CUTOFF) _APP_SET_SENSOR_DAT(CH_ANA1, sum);

				// reset buffer
				for (idx_buf = 0; idx_buf < ADC_BUFSIZE; idx_buf++)
				{
					buf[idx_buf] = 0;
				}
				idx_buf = 0;
				// update step for CH_ANA2
				step++;

				// update config for CH_ANA2
				sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
				sConfig.Rank = 1;
				sConfig.Channel = ADC_CHANNEL_5;
				HAL_ADC_ConfigChannel(&hadc1, &sConfig);
			}
			HAL_ADC_Start(&hadc1);
		}
		break;

	case 2:
		// check CH_ANA2 convert finished
		if (HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK)
		{
			dat = HAL_ADC_GetValue(&hadc1);
			buf[idx_buf++] = dat;

			if (dat > ADC_CUTOFF) sensor_dat[CH_ANA2].timeout = 0;

			if (idx_buf >= ADC_BUFSIZE)
			{
				// set sensor data
				sum = 0;
				max = min = buf[0];
				for (idx_buf = 0; idx_buf < ADC_BUFSIZE; idx_buf++)
				{
					dat = buf[idx_buf];
					sum += dat;
					max = (dat > max) ? dat : max;
					min = (dat < min) ? dat : min;
				}
				sum = sum - max - min;
				sum /= (ADC_BUFSIZE - 2);
				if (sum > ADC_CUTOFF) _APP_SET_SENSOR_DAT(CH_ANA2, sum);

				// reset buffer
				for (idx_buf = 0; idx_buf < ADC_BUFSIZE; idx_buf++)
				{
					buf[idx_buf] = 0;
				}
				idx_buf = 0;

				// update step for CH_ANA1
				step = 1;

				// set config for CH_ANA1
				sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
				sConfig.Rank = 1;
				sConfig.Channel = ADC_CHANNEL_4;
				HAL_ADC_ConfigChannel(&hadc1, &sConfig);
			}
			HAL_ADC_Start(&hadc1);
		}
		break;

	default:
		step = 0;
		break;
	}
}



/* **************************************************
 fucntion:      APP_devinof_default
 input:
 output:
 describe:  set default value of device version
***************************************************/

void APP_devinfo_default(void)
{
	device_info.delay = 0;
	device_info.flag = CFGSTAT_VALID;

	switch (dustmon_info.company_no)
	{
	case 1:
		device_info.addr[0] = EKYJ_DEV_MFR;       //0x01;
		device_info.addr[1] = EKYJ_DEV_MODEL;       //0x01;
		device_info.addr[2] = EKYJ_DEV_REGION;       //222;        //0xDE;
		break;
	case 2:
		device_info.addr[0] = ZZLY_DEV_MFR;       //0x01;
		device_info.addr[1] = ZZLY_DEV_MODEL;       //0x01;
		device_info.addr[2] = ZZLY_DEV_REGION;       //222;        //0xDE;
		break;
	case 3:
		device_info.addr[0] = SXRW_DEV_MFR;       //0x01;
		device_info.addr[1] = SXRW_DEV_MODEL;       //0x01;
		device_info.addr[2] = SXRW_DEV_REGION;       //222;        //0xDE;
		break;
	case 4:
		device_info.addr[0] = XAML_DEV_MFR;       //0x01;
		device_info.addr[1] = XAML_DEV_MODEL;       //0x01;
		device_info.addr[2] = XAML_DEV_REGION;       //222;        //0xDE;
		break;
	case 5:
		device_info.addr[0] = ZFMD_DEV_MFR;       //0x01;
		device_info.addr[1] = ZFMD_DEV_MODEL;       //0x01;
		device_info.addr[2] = ZFMD_DEV_REGION;       //222;        //0xDE;
		break;
	case 6:
		device_info.addr[0] = XMRS_DEV_MFR;       //0x01;
		device_info.addr[1] = XMRS_DEV_MODEL;       //0x01;
		device_info.addr[2] = XMRS_DEV_REGION;       //222;        //0xDE;
		break;
	case 7:
		device_info.addr[0] = SDWK_DEV_MFR;       //0x01;
		device_info.addr[1] = SDWK_DEV_MODEL;       //0x01;
		device_info.addr[2] = SDWK_DEV_REGION;       //222;        //0xDE;
		break;
	case 8:
		device_info.addr[0] = YRRD_DEV_MFR;       //0x01;
		device_info.addr[1] = YRRD_DEV_MODEL;       //0x01;
		device_info.addr[2] = YRRD_DEV_REGION;       //222;        //0xDE;
		break;
	case 9:
		device_info.addr[0] = GLD_DEV_MFR;       //0x01;
		device_info.addr[1] = GLD_DEV_MODEL;       //0x01;
		device_info.addr[2] = GLD_DEV_REGION;       //222;        //0xDE;
		break;
	case 10:
		device_info.addr[0] = LZXQ_DEV_MFR;       //0x01;
		device_info.addr[1] = LZXQ_DEV_MODEL;       //0x01;
		device_info.addr[2] = LZXQ_DEV_REGION;       //222;        //0xDE;
		break;
	case 11:
		device_info.addr[0] = SLX_DEV_MFR;       //0x01;
		device_info.addr[1] = SLX_DEV_MODEL;       //0x01;
		device_info.addr[2] = SLX_DEV_REGION;       //222;        //0xDE;
		break;
	default:
		device_info.addr[0] = TXBK_DEV_MFR;       //0x01;
		device_info.addr[1] = TXBK_DEV_MODEL;       //0x01;
		device_info.addr[2] = TXBK_DEV_REGION;       //222;        //0xDE;
		break;
	}
	device_info.addr[3] = 0;        //0xBC;
	device_info.addr[4] = 0;        //0x4A;

	device_info.work_mod = WKMOD_MIX;

	{
		uint32_t i;
		for (i = 0; i < SENSOR_MAXCHANNEL; i++) device_info.sensor_en[i] = TRUE;
	}
	//device_info.sensor_en[SENS_FINGER] = FALSE;

	device_info.height_offset = 1.0;
	device_info.weight_tare = 0.0;

	// IP: 122.114.22.87:8086
	// HEX: 7A.72.16.57:1F 96
	device_info.ip_port[0][0] = IPV4_01_BYTE1;      //0x7A;
	device_info.ip_port[0][1] = IPV4_01_BYTE2;      //0x72;
	device_info.ip_port[0][2] = IPV4_01_BYTE3;          //0x16;
	device_info.ip_port[0][3] = IPV4_01_BYTE4;          //0x57;
	device_info.ip_port[0][4] = IPV4_01_PORT / 256; //0x1F;
	device_info.ip_port[0][5] = IPV4_01_PORT % 256; //0x96;

	device_info.ip_port[1][0] = 0x00;
	device_info.ip_port[1][1] = 0x00;
	device_info.ip_port[1][2] = 0x00;
	device_info.ip_port[1][3] = 0x00;
	device_info.ip_port[1][4] = 0x00;
	device_info.ip_port[1][5] = 0x00;

	device_info.ip_port[2][0] = 0x00;
	device_info.ip_port[2][1] = 0x00;
	device_info.ip_port[2][2] = 0x00;
	device_info.ip_port[2][3] = 0x00;
	device_info.ip_port[2][4] = 0x00;
	device_info.ip_port[2][5] = 0x00;

	device_info.ip_port[3][0] = 0x00;
	device_info.ip_port[3][1] = 0x00;
	device_info.ip_port[3][2] = 0x00;
	device_info.ip_port[3][3] = 0x00;
	device_info.ip_port[3][4] = 0x00;
	device_info.ip_port[3][5] = 0x00;

	device_info.beat_time = 5;
	device_info.recon_time = 30;
	device_info.datrpt_time = 15;
	device_info.datsave_time = 15;
	device_info.link_timeout = 9;

	// password
	strcpy((char *)device_info.pswd[0], PSWD_USER);
#ifdef PSWD_DBG
	strcpy((char *)device_info.pswd[1], PSWD_DBG);
#else
	device_info.pswd[1][0] = '\0';
#endif
#ifdef PSWD_OPT
	strcpy((char *)device_info.pswd[2], PSWD_OPT);
#else
	device_info.pswd[2][0] = '\0';
#endif
/*
#ifdef PSWD_ADMIN
	strcpy((char*)device_info.pswd[2], PSWD_ADMIN);
#endif
*/
	NAND_WPTR = 0;
}


/* **************************************************
 fucntion:      device infomation initialize
 input:
 output:
 describe:  set default value of device infomation
***************************************************/

void APP_devinfo_ini(void)
{

#ifdef NVRAM_CNT
#define _STATIC static  // for test only
#else
#define _STATIC
#endif


	_STATIC uint32_t size;

	size = (uint32_t)&device_info.end - (uint32_t)&device_info + 4; // size in byte, including "end"
	APP_fram_readdata(HDL_SPI_FRAM,\
						  (uint32_t)NVADDR_DEVINFO,\
						  (uint8_t *)&device_info,\
						  size);

	// load write-pointer of NAND-FLASH from FRAM
	APP_fram_readdata(HDL_SPI_FRAM,\
						NVADDR_NANDWR,\
						(uint8_t *)&NAND_WPTR,\
						sizeof(NAND_WPTR));
/* 
#ifndef NVRAM_INI
	if (device_ver.ver_soft != DEV_VERSOFT)
#endif
*/
	{
		// soft version changed, update config


#ifndef NVRAM_INI
		if (device_info.flag != CFGSTAT_VALID || flag_framrst)
#endif
		{
			APP_devinfo_default();
			APP_fram_writedata(HDL_SPI_FRAM,\
								   (uint32_t)NVADDR_DEVINFO,\
								   (uint8_t *)&device_info,\
								   size);
			APP_fram_writedata(HDL_SPI_FRAM,\
					   NVADDR_NANDWR,\
					   (uint8_t *)&NAND_WPTR,\
					   sizeof(NAND_WPTR));

		}
	}



#undef _STATIC
}


/* **************************************************
 fucntion:      device infomation manage
 input:
 output:
 describe:  load/save device infomation
***************************************************/

void APP_devinfo_manage(void)
{
	uint32_t size;

	if ((device_info.flag == CFGSTAT_SAVE) && (device_info.delay == 0))
	{
		size = (uint32_t)&device_info.end - (uint32_t)&device_info + 4; // size in byte, including "end"

		device_info.flag = CFGSTAT_VALID;
		APP_fram_writedata(HDL_SPI_FRAM,\
							   (uint32_t)NVADDR_DEVINFO,\
							   (uint8_t *)&device_info,\
							   size);
	} else if (device_info.flag == CFGSTAT_LOAD)
	{
		size = (uint32_t)&device_info.end - (uint32_t)&device_info + 4; // size in byte, including "end"

		APP_fram_readdata(HDL_SPI_FRAM,\
							  (uint32_t)NVADDR_DEVINFO,\
							  (uint8_t *)&device_info,\
							  size);
	}
}


/* **************************************************
 fucntion:      APP_devver_default
 input:
 output:
 describe:  set default value of device version
***************************************************/

void APP_devver_default(void)
{
	device_ver.delay = 0;
	device_ver.flag = CFGSTAT_VALID;

	switch (dustmon_info.company_no)
	{
	case 1:
		device_ver.mfr[0] = (uint8_t)(EKYJ_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(EKYJ_DEV_MFR);
		device_ver.model[0] = (uint8_t)(EKYJ_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(EKYJ_DEV_MODEL);
		break;
	case 2:
		device_ver.mfr[0] = (uint8_t)(ZZLY_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(ZZLY_DEV_MFR);
		device_ver.model[0] = (uint8_t)(ZZLY_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(ZZLY_DEV_MODEL);
		break;
	case 3:
		device_ver.mfr[0] = (uint8_t)(SXRW_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(SXRW_DEV_MFR);
		device_ver.model[0] = (uint8_t)(SXRW_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(SXRW_DEV_MODEL);
		break;
	case 4:
		device_ver.mfr[0] = (uint8_t)(XAML_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(XAML_DEV_MFR);
		device_ver.model[0] = (uint8_t)(XAML_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(XAML_DEV_MODEL);
		break;
	case 5:
		device_ver.mfr[0] = (uint8_t)(ZFMD_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(ZFMD_DEV_MFR);
		device_ver.model[0] = (uint8_t)(ZFMD_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(ZFMD_DEV_MODEL);
		break;
	case 6:
		device_ver.mfr[0] = (uint8_t)(XMRS_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(XMRS_DEV_MFR);
		device_ver.model[0] = (uint8_t)(XMRS_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(XMRS_DEV_MODEL);
		break;
	case 7:
		device_ver.mfr[0] = (uint8_t)(SDWK_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(SDWK_DEV_MFR);
		device_ver.model[0] = (uint8_t)(SDWK_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(SDWK_DEV_MODEL);
		break;
	case 8:
		device_ver.mfr[0] = (uint8_t)(YRRD_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(YRRD_DEV_MFR);
		device_ver.model[0] = (uint8_t)(YRRD_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(YRRD_DEV_MODEL);
		break;
	case 9:
		device_ver.mfr[0] = (uint8_t)(GLD_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(GLD_DEV_MFR);
		device_ver.model[0] = (uint8_t)(GLD_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(GLD_DEV_MODEL);
		break;
	case 10:
		device_ver.mfr[0] = (uint8_t)(LZXQ_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(LZXQ_DEV_MFR);
		device_ver.model[0] = (uint8_t)(LZXQ_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(LZXQ_DEV_MODEL);
		break;
	case 11:
		device_ver.mfr[0] = (uint8_t)(SLX_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(SLX_DEV_MFR);
		device_ver.model[0] = (uint8_t)(SLX_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(SLX_DEV_MODEL);
		break;

	default:
		device_ver.mfr[0] = (uint8_t)(TXBK_DEV_MFR >> 8); // see DEV_MFR.
		device_ver.mfr[1] = (uint8_t)(TXBK_DEV_MFR);
		device_ver.model[0] = (uint8_t)(TXBK_DEV_MODEL >> 8);  // 00 02: SPS20A
		device_ver.model[1] = (uint8_t)(TXBK_DEV_MODEL);
		break;
	}

	device_ver.ver_soft = DEV_VERSOFT;
	device_ver.ver_prtcl = DEV_VERPRTCL;
	device_ver.ver_mmi = DEV_VERMMI;
	device_ver.ver_dtu = DTU_MG2639;      // default :SPS32 B00
	device_ver.ver_pmsen = PMSENSOR_SDS011;     // defualt :YT-PM2510
}


/* **************************************************
 fucntion:      device version initialize
 input:
 output:
 describe:  initial value of device version
***************************************************/

void APP_devver_ini(void)
{
	uint8_t model[2];
#ifdef NVRAM_CNT
#define _STATIC static  // for test only
#else
#define _STATIC
#endif


	_STATIC uint32_t size;

	size = (uint32_t)&device_ver.end - (uint32_t)&device_ver + 4;   // size in byte, including "end"
	APP_fram_readdata(HDL_SPI_FRAM,\
						  (uint32_t)NVADDR_DEVVER,\
						  (uint8_t *)&device_ver,\
						  size);
	switch (dustmon_info.company_no)
	{
	case 1:
		model[0] = EKYJ_DEV_MODEL >> 8;
		model[1] = EKYJ_DEV_MODEL;
		break;
	case 2:
		model[0] = ZZLY_DEV_MODEL >> 8;
		model[1] = ZZLY_DEV_MODEL;
		break;
	case 3:
		model[0] = SXRW_DEV_MODEL >> 8;
		model[1] = SXRW_DEV_MODEL;
		break;
	case 4:
		model[0] = XAML_DEV_MODEL >> 8;
		model[1] = XAML_DEV_MODEL;
		break;
	case 5:
		model[0] = ZFMD_DEV_MODEL >> 8;
		model[1] = ZFMD_DEV_MODEL;
		break;
	case 6:
		model[0] = XMRS_DEV_MODEL >> 8;
		model[1] = XMRS_DEV_MODEL;
		break;
	case 7:
		model[0] = SDWK_DEV_MODEL >> 8;
		model[1] = SDWK_DEV_MODEL;
		break;
	case 8:
		model[0] = YRRD_DEV_MODEL >> 8;
		model[1] = YRRD_DEV_MODEL;
		break;
	case 9:
		model[0] = GLD_DEV_MODEL >> 8;
		model[1] = GLD_DEV_MODEL;
		break;
	case 10:
		model[0] = LZXQ_DEV_MODEL >> 8;
		model[1] = LZXQ_DEV_MODEL;
		break;
	case 11:
		model[0] = SLX_DEV_MODEL >> 8;
		model[1] = SLX_DEV_MODEL;
		break;

	default:
		model[0] = TXBK_DEV_MODEL >> 8;
		model[1] = TXBK_DEV_MODEL;
		break;
	}

	if ((device_ver.model[0] != model[0]) ||
		(device_ver.model[1] != model[1]))
	{
		flag_framrst = TRUE;
	}
	else flag_framrst = FALSE;
#ifndef NVRAM_INI
	if (device_ver.flag != CFGSTAT_VALID || flag_framrst)
#endif
	{
		APP_devver_default();
		APP_fram_writedata(HDL_SPI_FRAM,\
							   (uint32_t)NVADDR_DEVVER,\
							   (uint8_t *)&device_ver,\
							   size);
	}

#undef _STATIC
}


/* **************************************************
 fucntion:      device version manage
 input:
 output:
 describe:  load/save device version
***************************************************/

void APP_devver_manage(void)
{
	uint32_t size;

	if (device_ver.ver_soft != DEV_VERSOFT ||\
			device_ver.ver_prtcl != DEV_VERPRTCL)
	{
		// update version
		device_ver.ver_soft = DEV_VERSOFT;
		device_ver.ver_prtcl = DEV_VERPRTCL;

		device_ver.flag = CFGSTAT_SAVE;
		device_ver.delay = 0;
	}

	if ((device_ver.flag == CFGSTAT_SAVE) && (device_ver.delay == 0))
	{
		size = (uint32_t)&device_ver.end - (uint32_t)&device_ver + 4;   // size in byte, including "end"

		device_ver.flag = CFGSTAT_VALID;
		APP_fram_writedata(HDL_SPI_FRAM,\
							   (uint32_t)NVADDR_DEVVER,\
							   (uint8_t *)&device_ver,\
							   size);
	} else if (device_ver.flag == CFGSTAT_LOAD)
	{
		size = (uint32_t)&device_ver.end - (uint32_t)&device_ver + 4;   // size in byte, including "end"

		APP_fram_readdata(HDL_SPI_FRAM,\
							  (uint32_t)NVADDR_DEVVER,\
							  (uint8_t *)&device_ver,\
							  size);
	}
}


/* **************************************************
 fucntion:      APP_elivatorinof_default
 input:
 output:
 describe:  set default value of elivator info
***************************************************/

void APP_dustmoninfo_default(void)
{
	uint8_t i;
	dustmon_info.delay = 0;
	dustmon_info.flag = CFGSTAT_VALID;
/* 
	dustmon_info.name[0] = 'T';
	dustmon_info.name[1] = 'X';
	dustmon_info.name[2] = '\0';
*/
	dustmon_info.mfr_ID = 0x00;
	dustmon_info.model_ID = 0x00;

	for (i = 0; i < 8; i++)
	{
		dustmon_info.threshold[i] = 999.9f;
		dustmon_info.thrshdflag[i] = FALSE;
	}
	for (i = 0; i < 4; i++)
	{
		dustmon_info.manualflag[i] = FALSE;
	}

	dustmon_info.outdoorled_4lines_en = TRUE;
	dustmon_info.soundsensor_en = TRUE;
	dustmon_info.company_no = Default_manufacturer;
	NAND_WPTR = 0;
}


/* **************************************************
 fucntion:      APP_dustmoninfo_ini
 input:
 output:
 describe:  set default value of elivator infomation
***************************************************/

void APP_dustmoninfo_ini(void)
{
#ifdef NVRAM_CNT
#define _STATIC static  // for test only
#else
#define _STATIC
#endif


	_STATIC uint32_t size;

	size = (uint32_t)&dustmon_info.end - (uint32_t)&dustmon_info + 4;   // size in byte, including "end"
	APP_fram_readdata(HDL_SPI_FRAM,\
						  (uint32_t)NVADDR_DUSTMONINFO,\
						  (uint8_t *)&dustmon_info,\
						  size);

#ifndef NVRAM_INI
	if (dustmon_info.flag != CFGSTAT_VALID || flag_framrst)
#endif
	{
		APP_dustmoninfo_default();
		APP_fram_writedata(HDL_SPI_FRAM,\
							   (uint32_t)NVADDR_DUSTMONINFO,\
							   (uint8_t *)&dustmon_info,\
							   size);
	}

#undef _STATIC
}



/* **************************************************
 fucntion:      APP_dustmoninfo_manage
 input:
 output:
 describe:  load/save elivator infomation
***************************************************/

void APP_dustmoninfo_manage(void)
{
	uint32_t size;

	if ((dustmon_info.flag == CFGSTAT_SAVE) && (dustmon_info.delay == 0))
	{
		size = (uint32_t)&dustmon_info.end - (uint32_t)&dustmon_info + 4;   // size in byte, including "end"

		dustmon_info.flag = CFGSTAT_VALID;
		APP_fram_writedata(HDL_SPI_FRAM,\
							   (uint32_t)NVADDR_DUSTMONINFO,\
							   (uint8_t *)&dustmon_info,\
							   size);
	} else if (dustmon_info.flag == CFGSTAT_LOAD)
	{
		size = (uint32_t)&dustmon_info.end - (uint32_t)&dustmon_info + 4;   // size in byte, including "end"

		APP_fram_readdata(HDL_SPI_FRAM,\
							  (uint32_t)NVADDR_DUSTMONINFO,\
							  (uint8_t *)&dustmon_info,\
							  size);
	}
}


/* **************************************************
 fucntion:      APP_notice_default
 input:
 output:
 describe:  set default value of notice info
***************************************************/

void APP_notice_default(void)
{
	uint8_t i;
	dustmon_notice.delay = 0;
	dustmon_notice.flag = CFGSTAT_VALID;


	dustmon_notice.oled_dispchar.year_start = 2000;
	dustmon_notice.oled_dispchar.month_start = 1;
	dustmon_notice.oled_dispchar.day_start = 1;

	dustmon_notice.oled_dispchar.year_end = 2000;
	dustmon_notice.oled_dispchar.month_end = 1;
	dustmon_notice.oled_dispchar.day_end = 1;


	dustmon_notice.oled_dispchar.week = 0;
	dustmon_notice.oled_dispchar.timeslot_hour_start = 0;
	dustmon_notice.oled_dispchar.timeslot_min_start = 0;
	dustmon_notice.oled_dispchar.timeslot_hour_end = 0;
	dustmon_notice.oled_dispchar.timeslot_min_end = 0;

	dustmon_notice.oled_update = TRUE;

	for (i = 0; i < 234; i++)
	{
		dustmon_notice.oled_dispchar.display_char[i] = 0;
	}
}


/* **************************************************
 fucntion:      APP_notice_ini
 input:
 output:
 describe:  set default value of notice infomation
***************************************************/

void APP_notice_ini(void)
{
#ifdef NVRAM_CNT
#define _STATIC static  // for test only
#else
#define _STATIC
#endif


	_STATIC uint32_t size;

	size = (uint32_t)&dustmon_notice.end - (uint32_t)&dustmon_notice + 4;   // size in byte, including "end"
	APP_fram_readdata(HDL_SPI_FRAM,\
						  (uint32_t)NVADDR_NOTICE,\
						  (uint8_t *)&dustmon_notice,\
						  size);

#ifndef NVRAM_INI
	if (dustmon_notice.flag != CFGSTAT_VALID || flag_framrst)
#endif
	{
		APP_notice_default();
		APP_fram_writedata(HDL_SPI_FRAM,\
							   (uint32_t)NVADDR_NOTICE,\
							   (uint8_t *)&dustmon_notice,\
							   size);
	}

#undef _STATIC
}



/* **************************************************
 fucntion:      APP_notice_manage
 input:
 output:
 describe:  load/save oled notice infomation
***************************************************/

void APP_notice_manage(void)
{
	uint32_t size;

	if ((dustmon_notice.flag == CFGSTAT_SAVE) && (dustmon_notice.delay == 0))
	{
		size = (uint32_t)&dustmon_notice.end - (uint32_t)&dustmon_notice + 4;   // size in byte, including "end"

		dustmon_notice.flag = CFGSTAT_VALID;
		APP_fram_writedata(HDL_SPI_FRAM,\
							   (uint32_t)NVADDR_NOTICE,\
							   (uint8_t *)&dustmon_notice,\
							   size);
	} else if (dustmon_notice.flag == CFGSTAT_LOAD)
	{
		size = (uint32_t)&dustmon_notice.end - (uint32_t)&dustmon_notice + 4;   // size in byte, including "end"

		APP_fram_readdata(HDL_SPI_FRAM,\
							  (uint32_t)NVADDR_NOTICE,\
							  (uint8_t *)&dustmon_notice,\
							  size);
	}
}

/* **************************************************
 fucntion:      APP_limittbl_default
 input:
 output:
 describe:  set default value of limit-table
***************************************************/

void APP_limittbl_default(void)
{
	limit_tbl.delay = 0;
	limit_tbl.flag = CFGSTAT_VALID;

	limit_tbl.limit[SENS_UPWEIGHT].hilimit = 99999;        // offset of weight in kg
	limit_tbl.limit[SENS_UPWEIGHT].hiwarn = 88888;
	limit_tbl.limit[SENS_UPWEIGHT].lowarn = limit_tbl.limit[SENS_UPWEIGHT].hiwarn;
	limit_tbl.limit[SENS_UPWEIGHT].lolimit = 0;

	limit_tbl.limit[SENS_CABL].hilimit =  9999;       // offset of  in kg
	limit_tbl.limit[SENS_CABL].hiwarn = 8888;
	limit_tbl.limit[SENS_CABL].lowarn = limit_tbl.limit[SENS_CABL].hiwarn;
	limit_tbl.limit[SENS_CABL].lolimit = 0;

	limit_tbl.limit[SENS_CABR].hilimit =  9999;       // offset of  in kg
	limit_tbl.limit[SENS_CABR].hiwarn = 8888;
	limit_tbl.limit[SENS_CABR].lowarn = limit_tbl.limit[SENS_CABL].hiwarn;
	limit_tbl.limit[SENS_CABR].lolimit = 0;
}


/* **************************************************
 fucntion:      limit table initialize
 input:
 output:
 describe:  set default limit table
***************************************************/

void APP_limittbl_ini(void)
{
#ifdef NVRAM_CNT
#define _STATIC static  // for test only
#else
#define _STATIC
#endif


	_STATIC uint32_t size;

	size = (uint32_t)&limit_tbl.end - (uint32_t)&limit_tbl + 4; // size in byte, including "end"
	APP_fram_readdata(HDL_SPI_FRAM,\
						  (uint32_t)NVADDR_LIMITTBL,\
						  (uint8_t *)&limit_tbl,\
						  size);
/* 
		limit_tbl.limit[SENS_TILT].lowarn = limit_tbl.limit[SENS_TILT].hiwarn;
		limit_tbl.limit[SENS_WALK].lowarn = limit_tbl.limit[SENS_WALK].hiwarn;
		limit_tbl.limit[SENS_ROTAT].lowarn = limit_tbl.limit[SENS_ROTAT].hiwarn;
		limit_tbl.limit[SENS_MARGIN].lowarn = limit_tbl.limit[SENS_MARGIN].hiwarn;
		limit_tbl.limit[SENS_HEIGHT].lowarn = limit_tbl.limit[SENS_HEIGHT].hiwarn;
		limit_tbl.limit[SENS_WEIGHT].lowarn = limit_tbl.limit[SENS_WEIGHT].hiwarn;
*/
#ifndef NVRAM_INI
	if (limit_tbl.flag != CFGSTAT_VALID || flag_framrst)
#endif
	{
		APP_limittbl_default();
		APP_fram_writedata(HDL_SPI_FRAM,\
							   (uint32_t)NVADDR_LIMITTBL,\
							   (uint8_t *)&limit_tbl,\
							   size);
	}

#undef _STATIC
}


/* **************************************************
 fucntion:      limit table manage
 input:
 output:
 describe:  load/save limit table
***************************************************/

void APP_limittbl_manage(void)
{
	uint32_t size;

	if ((limit_tbl.flag == CFGSTAT_SAVE) && (limit_tbl.delay == 0))
	{
		size = (uint32_t)&limit_tbl.end - (uint32_t)&limit_tbl + 4; // size in byte, including "end"

		limit_tbl.flag = CFGSTAT_VALID;
		APP_fram_writedata(HDL_SPI_FRAM,\
							   (uint32_t)NVADDR_LIMITTBL,\
							   (uint8_t *)&limit_tbl,\
							   size);
	} else if (limit_tbl.flag == CFGSTAT_LOAD)
	{
		size = (uint32_t)&limit_tbl.end - (uint32_t)&limit_tbl + 4; // size in byte, including "end"

		APP_fram_readdata(HDL_SPI_FRAM,\
							  (uint32_t)NVADDR_LIMITTBL,\
							  (uint8_t *)&limit_tbl,\
							  size);
	}
}


/* **************************************************
 fucntion:      APP_floortbl_default
 input:
 output:
 describe:  set default value of floor-table
***************************************************/

void APP_floortbl_default(void)
{
	float height;
	uint32_t index, floor;

	floor_tbl.delay = 0;
	floor_tbl.flag = CFGSTAT_VALID;

	floor_tbl.tblsize = 3;

	floor_tbl.type[0].height = 5.0f;
	floor_tbl.type[0].number = 3;

	floor_tbl.type[1].height = 3.5f;
	floor_tbl.type[1].number = 2;

	floor_tbl.type[2].height = 3.0f;
	floor_tbl.type[2].number = 18;

	floor_tbl.type[3].height = 3.2f;
	floor_tbl.type[3].number = 3;

	height = 0;
	floor = 0;
	for (index = 0; index < floor_tbl.tblsize; index++)
	{
		floor += floor_tbl.type[index].number;
		height += floor_tbl.type[index].height * floor_tbl.type[index].number;
	}
	floor_tbl.height = height;
	floor_tbl.floor = floor;
}

/* **************************************************
 fucntion:      APP_floortbl_ini
 input:
 output:
 describe:  set default value for floor table
***************************************************/

void APP_floortbl_ini(void)
{
#ifdef NVRAM_CNT
#define _STATIC static  // for test only
#else
#define _STATIC
#endif


	_STATIC uint32_t size;

	size = (uint32_t)&floor_tbl.end - (uint32_t)&floor_tbl + 4; // size in byte, including "end"
	APP_fram_readdata(HDL_SPI_FRAM,\
						  (uint32_t)NVADDR_FLOORTBL,\
						  (uint8_t *)&floor_tbl,\
						  size);

#ifndef NVRAM_INI
	if (floor_tbl.flag != CFGSTAT_VALID || flag_framrst)
#endif
	{
		APP_floortbl_default();
		APP_fram_writedata(HDL_SPI_FRAM,\
							   (uint32_t)NVADDR_FLOORTBL,\
							   (uint8_t *)&floor_tbl,\
							   size);
	}

#undef _STATIC
}



/* **************************************************
 fucntion:      APP_floortbl_manage
 input:
 output:
 describe:  load/save floor table
***************************************************/

void APP_floortbl_manage(void)
{
	uint32_t size;

	{       // update floor & height
		uint32_t index, floor;
		float height;

		height = 0;
		floor = 0;
		index = 0;
		while (index < floor_tbl.tblsize)
		{
			floor += floor_tbl.type[index].number;
			height += (floor_tbl.type[index].height * floor_tbl.type[index].number);
			index++;
		}
		floor_tbl.floor = floor;
		floor_tbl.height = height;
	}


	if ((floor_tbl.flag == CFGSTAT_SAVE) && (floor_tbl.delay == 0))
	{
		size = (uint32_t)&floor_tbl.end - (uint32_t)&floor_tbl + 4; // size in byte, including "end"

		floor_tbl.flag = CFGSTAT_VALID;
		APP_fram_writedata(HDL_SPI_FRAM,\
							   (uint32_t)NVADDR_FLOORTBL,\
							   (uint8_t *)&floor_tbl,\
							   size);
	} else if (floor_tbl.flag == CFGSTAT_LOAD)
	{
		size = (uint32_t)&floor_tbl.end - (uint32_t)&floor_tbl + 4; // size in byte, including "end"

		APP_fram_readdata(HDL_SPI_FRAM,\
							  (uint32_t)NVADDR_FLOORTBL,\
							  (uint8_t *)&floor_tbl,\
							  size);
	}
}



/* **************************************************
 fucntion:      APP_calitbl_default
 input:
 output:
 describe:  set default value of cali-table
***************************************************/

void APP_calitbl_default(void)
{
	float dat;

	cali_tbl.delay = 0;
	cali_tbl.flag = CFGSTAT_VALID;

	// PM2.5 sensor cali-dat
	// x: raw data including scale
	// y: pm2.5 value, ug/m3
	cali_tbl.chdat[SENS_PM25].tblsize = 2;
	dat = MAX_PWM_DUTY * 0.002f * sensor_dat[SENS_PM25].scale;
	cali_tbl.chdat[SENS_PM25].dat[0].x = (uint32_t)dat;
	cali_tbl.chdat[SENS_PM25].dat[0].y = 0.0f;

	dat = MAX_PWM_DUTY * 0.997f * sensor_dat[SENS_PM25].scale;
	cali_tbl.chdat[SENS_PM25].dat[1].x = (uint32_t)dat;
	cali_tbl.chdat[SENS_PM25].dat[1].y = 999.0f;

	// PM10 sensor cali-dat
	// x: raw data including scale
	// y: PM10 value, ug/m3
	cali_tbl.chdat[SENS_PM10].tblsize = 2;
	dat = MAX_PWM_DUTY * 0.002f * sensor_dat[SENS_PM10].scale;
	cali_tbl.chdat[SENS_PM10].dat[0].x = (uint32_t)dat;
	cali_tbl.chdat[SENS_PM10].dat[0].y = 0.0f;

	dat = MAX_PWM_DUTY * 0.997f * sensor_dat[SENS_PM10].scale;
	cali_tbl.chdat[SENS_PM10].dat[1].x = (uint32_t)dat;
	cali_tbl.chdat[SENS_PM10].dat[1].y = 999.0f;

	// uploading platform weight sensor cali-dat 
	// x: raw data including scale 
	// y: uploading platform weight sensor value, kg
	cali_tbl.chdat[SENS_UPWEIGHT].tblsize = 2;
	dat = MAX_UPSNR_DUTY * 0.002f * sensor_dat[SENS_UPWEIGHT].scale;
	cali_tbl.chdat[SENS_UPWEIGHT].dat[0].x = (uint32_t)dat;
	cali_tbl.chdat[SENS_UPWEIGHT].dat[0].y = 0.0f;

	dat = MAX_UPSNR_DUTY * 0.997f * sensor_dat[SENS_UPWEIGHT].scale;
	cali_tbl.chdat[SENS_UPWEIGHT].dat[1].x = (uint32_t)dat;
	cali_tbl.chdat[SENS_UPWEIGHT].dat[1].y = 9999.0f;

	// uploading platform wire1 sensor cali-dat 
	// x: raw data including scale 
	// y: uploading platform wire1 sensor value, kg
	cali_tbl.chdat[SENS_CABL].tblsize = 2;
	dat = MAX_UPSNR_DUTY * 0.002f * sensor_dat[SENS_CABL].scale;
	cali_tbl.chdat[SENS_CABL].dat[0].x = (uint32_t)dat;
	cali_tbl.chdat[SENS_CABL].dat[0].y = 0.0f;

	dat = MAX_UPSNR_DUTY * 0.997f * sensor_dat[SENS_CABL].scale;
	cali_tbl.chdat[SENS_CABL].dat[1].x = (uint32_t)dat;
	cali_tbl.chdat[SENS_CABL].dat[1].y = 9999.0f;

	// uploading platform wire2 sensor cali-dat 
	// x: raw data including scale 
	// y: uploading platform wire2 sensor value, kg
	cali_tbl.chdat[SENS_CABR].tblsize = 2;
	dat = MAX_UPSNR_DUTY * 0.002f * sensor_dat[SENS_CABR].scale;
	cali_tbl.chdat[SENS_CABR].dat[0].x = (uint32_t)dat;
	cali_tbl.chdat[SENS_CABR].dat[0].y = 0.0f;

	dat = MAX_UPSNR_DUTY * 0.997f * sensor_dat[SENS_CABR].scale;
	cali_tbl.chdat[SENS_CABR].dat[1].x = (uint32_t)dat;
	cali_tbl.chdat[SENS_CABR].dat[1].y = 9999.0f;

	//
	for (uint8_t i = 0; i < UPSNR_NUM+2; i++)
	{
		for (uint8_t j = 0; j < UPSNR_NUM; j++)
		{
			cali_tbl.upsnr_code[i][j] = 0.0f;
		}
	}
	cali_tbl.upsnr_cornweight = 0.0f;
	cali_tbl.upsnr_dispweight = 0.0f;
	for (uint8_t i = 0; i < UPSNR_NUM; i++)
	{
		cali_tbl.alpha[i] = 1.0;
	}

	cali_tbl.algorithm_enable = TRUE;
}


/* **************************************************
 fucntion:      sensor calibration table initialize
 input:
 output:
 describe:  set default value for calibration tables
***************************************************/

void APP_calitbl_ini(void)
{
#ifdef NVRAM_CNT
#define _STATIC static  // for test only
#else
#define _STATIC
#endif

	_STATIC uint32_t size;

	size = (uint32_t)&cali_tbl.end - (uint32_t)&cali_tbl + 4;   // size in byte, including "end"
	APP_fram_readdata(HDL_SPI_FRAM,\
						  (uint32_t)NVADDR_CALITBL,\
						  (uint8_t *)&cali_tbl,\
						  size);

#ifndef NVRAM_INI
	if (cali_tbl.flag != CFGSTAT_VALID || flag_framrst)
#endif
	{
		APP_calitbl_default();
		APP_fram_writedata(&hspi3,\
							   (uint32_t)NVADDR_CALITBL,\
							   (uint8_t *)&cali_tbl,\
							   size);
	}

#undef _STATIC
}


/* **************************************************
 fucntion:      calibration table manage
 input:
 output:
 describe:  load/save calibration table
***************************************************/

void APP_calitbl_manage(void)
{
	uint32_t size;

	if ((cali_tbl.flag == CFGSTAT_SAVE) && (cali_tbl.delay == 0))
	{
		size = (uint32_t)&cali_tbl.end - (uint32_t)&cali_tbl + 4;   // size in byte, including "end"

		cali_tbl.flag = CFGSTAT_VALID;
		APP_fram_writedata(&hspi3,\
							   (uint32_t)NVADDR_CALITBL,\
							   (uint8_t *)&cali_tbl,\
							   size);
	} else if (cali_tbl.flag == CFGSTAT_LOAD)
	{
		size = (uint32_t)&cali_tbl.end - (uint32_t)&cali_tbl + 4;   // size in byte, including "end"

		APP_fram_readdata(HDL_SPI_FRAM,\
							  (uint32_t)NVADDR_CALITBL,\
							  (uint8_t *)&cali_tbl,\
							  size);
	}
}



#undef  _LOCAL_MAIN

/*  USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/* *
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t *file, uint32_t line)
{
	/*  USER CODE BEGIN 6 */
	/*  User can add his own implementation to report the file name and line number,
	  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	printf("Wrong: file %s on line %d\r\n", file, line);

	/*  USER CODE END 6 */

}

#endif

/* *
  * @}
  */

/* *
  * @}
*/

/* *********************** (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
