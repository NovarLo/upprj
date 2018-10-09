/* *
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @date    16/08/2015 14:36:38
  * @brief   Interrupt Service Routines.
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
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
/*  USER CODE BEGIN 0 */

#include "app_user.h"
#include "app_sensor.h"
#include "app_fifo.h"
//#include "app_prtc.h"
//#include "app_ComSer.h"


/*  USER CODE END 0 */
/*  External variables --------------------------------------------------------*/

extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_spi2_tx;
extern I2S_HandleTypeDef hi2s2;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim9;
extern TIM_HandleTypeDef htim12;
extern TIM_HandleTypeDef htim14;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

/* *****************************************************************************/
/*             Cortex-M4 Processor Interruption and Exception Handlers         */
/* *****************************************************************************/

/* *
* @brief This function handles UART5 global interrupt.
*/
void UART5_IRQHandler(void)
{
	/*  USER CODE BEGIN UART5_IRQn 0 */
	uint32_t tmp1, tmp2;
	uint32_t pos = 0;

	tmp1 = __HAL_UART_GET_FLAG(&huart5, UART_FLAG_RXNE);
	tmp2 = __HAL_UART_GET_IT_SOURCE(&huart5, UART_IT_RXNE);
	/*  USER CODE END UART5_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(UART5_IRQn);
	HAL_UART_IRQHandler(&huart5);
	/*  USER CODE BEGIN UART5_IRQn 1 */


	if ((tmp1 != RESET) && (tmp2 != RESET))
	{
		//pos = (recvfifo_5.in + 1)&(QUEUE_REV_SIZE-1);
		pos = huart5.RxXferSize - huart5.RxXferCount;
		if (pos != recvfifo_5.out)
		{
			recvfifo_5.in = pos;
		}
	}
	/*  USER CODE END UART5_IRQn 1 */
}

/* *
* @brief This function handles TIM7 global interrupt.
*/
void TIM7_IRQHandler(void)
{
	/*  USER CODE BEGIN TIM7_IRQn 0 */

	/*  USER CODE END TIM7_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(TIM7_IRQn);
	HAL_TIM_IRQHandler(&htim7);
	/*  USER CODE BEGIN TIM7_IRQn 1 */

	/*  USER CODE END TIM7_IRQn 1 */
}

/* *
* @brief This function handles DMA2 Stream0 global interrupt.
*/
void DMA2_Stream0_IRQHandler(void)
{
	/*  USER CODE BEGIN DMA2_Stream0_IRQn 0 */

	/*  USER CODE END DMA2_Stream0_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(DMA2_Stream0_IRQn);
	HAL_DMA_IRQHandler(&hdma_adc1);
	/*  USER CODE BEGIN DMA2_Stream0_IRQn 1 */

	/*  USER CODE END DMA2_Stream0_IRQn 1 */
}

/* *
* @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
*/
void TIM6_DAC_IRQHandler(void)
{
	/*  USER CODE BEGIN TIM6_DAC_IRQn 0 */

//			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2,GPIO_PIN_RESET);

	/*  USER CODE END TIM6_DAC_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(TIM6_DAC_IRQn);
	HAL_TIM_IRQHandler(&htim6);
	/*  USER CODE BEGIN TIM6_DAC_IRQn 1 */


	/*  USER CODE END TIM6_DAC_IRQn 1 */
}

/* *
* @brief This function handles TIM2 global interrupt.
*/
void TIM2_IRQHandler(void)
{
	/*  USER CODE BEGIN TIM2_IRQn 0 */


	/*  USER CODE END TIM2_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(TIM2_IRQn);
	HAL_TIM_IRQHandler(&htim2);
	/*  USER CODE BEGIN TIM2_IRQn 1 */

	/*  USER CODE END TIM2_IRQn 1 */
}

/* *
* @brief This function handles UART4 global interrupt.
*/
void UART4_IRQHandler(void)
{
	/*  USER CODE BEGIN UART4_IRQn 0 */

	/*  USER CODE END UART4_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(UART4_IRQn);
	HAL_UART_IRQHandler(&huart4);
	/*  USER CODE BEGIN UART4_IRQn 1 */

	/*  USER CODE END UART4_IRQn 1 */
}

/* *
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
	/*  USER CODE BEGIN SysTick_IRQn 0 */

	/*  USER CODE END SysTick_IRQn 0 */
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
	/*  USER CODE BEGIN SysTick_IRQn 1 */

	/*  USER CODE END SysTick_IRQn 1 */
}

/* *
* @brief This function handles DMA1 Stream4 global interrupt.
*/
void DMA1_Stream4_IRQHandler(void)
{
	/*  USER CODE BEGIN DMA1_Stream4_IRQn 0 */

	/*  USER CODE END DMA1_Stream4_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(DMA1_Stream4_IRQn);
	HAL_DMA_IRQHandler(&hdma_spi2_tx);
	/*  USER CODE BEGIN DMA1_Stream4_IRQn 1 */

	/*  USER CODE END DMA1_Stream4_IRQn 1 */
}

/* *
* @brief This function handles TIM8 Capture Compare interrupt.
*/
void TIM8_CC_IRQHandler(void)
{
	/*  USER CODE BEGIN TIM8_CC_IRQn 0 */

	/*  USER CODE END TIM8_CC_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(TIM8_CC_IRQn);
	HAL_TIM_IRQHandler(&htim8);
	/*  USER CODE BEGIN TIM8_CC_IRQn 1 */

	/*  USER CODE END TIM8_CC_IRQn 1 */
}

/* *
* @brief This function handles TIM8 Break interrupt and TIM12 global interrupt.
*/
void TIM8_BRK_TIM12_IRQHandler(void)
{
	/*  USER CODE BEGIN TIM8_BRK_TIM12_IRQn 0 */

	/*  USER CODE END TIM8_BRK_TIM12_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(TIM8_BRK_TIM12_IRQn);
	HAL_TIM_IRQHandler(&htim8);
	HAL_TIM_IRQHandler(&htim12);
	/*  USER CODE BEGIN TIM8_BRK_TIM12_IRQn 1 */

	/*  USER CODE END TIM8_BRK_TIM12_IRQn 1 */
}

/* *
* @brief This function handles TIM8 Trigger and Commutation interrupts and TIM14 global interrupt.
*/
void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
	/*  USER CODE BEGIN TIM8_TRG_COM_TIM14_IRQn 0 */

	/*  USER CODE END TIM8_TRG_COM_TIM14_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(TIM8_TRG_COM_TIM14_IRQn);
	HAL_TIM_IRQHandler(&htim8);
	HAL_TIM_IRQHandler(&htim14);
	/*  USER CODE BEGIN TIM8_TRG_COM_TIM14_IRQn 1 */

	/*  USER CODE END TIM8_TRG_COM_TIM14_IRQn 1 */
}

/* *
* @brief This function handles TIM5 global interrupt.
*/
void TIM5_IRQHandler(void)
{
	/*  USER CODE BEGIN TIM5_IRQn 0 */

	/*  USER CODE END TIM5_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(TIM5_IRQn);
	HAL_TIM_IRQHandler(&htim5);
	/*  USER CODE BEGIN TIM5_IRQn 1 */

	/*  USER CODE END TIM5_IRQn 1 */
}

/* *
* @brief This function handles USART1 global interrupt.
*/
void USART1_IRQHandler(void)
{
	/*  USER CODE BEGIN USART1_IRQn 0 */

	/*  USER CODE END USART1_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(USART1_IRQn);
	HAL_UART_IRQHandler(&huart1);
	/*  USER CODE BEGIN USART1_IRQn 1 */

	/*  USER CODE END USART1_IRQn 1 */
}

/* *
* @brief This function handles TIM3 global interrupt.
*/
void TIM3_IRQHandler(void)
{
	/*  USER CODE BEGIN TIM3_IRQn 0 */

	/*  USER CODE END TIM3_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(TIM3_IRQn);
	HAL_TIM_IRQHandler(&htim3);
	/*  USER CODE BEGIN TIM3_IRQn 1 */

	/*  USER CODE END TIM3_IRQn 1 */
}

/* *
* @brief This function handles TIM1 Break interrupt and TIM9 global interrupt.
*/
void TIM1_BRK_TIM9_IRQHandler(void)
{
	/*  USER CODE BEGIN TIM1_BRK_TIM9_IRQn 0 */

	/*  USER CODE END TIM1_BRK_TIM9_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(TIM1_BRK_TIM9_IRQn);
	HAL_TIM_IRQHandler(&htim9);
	/*  USER CODE BEGIN TIM1_BRK_TIM9_IRQn 1 */

	/*  USER CODE END TIM1_BRK_TIM9_IRQn 1 */
}

/* *
* @brief This function handles USART3 global interrupt.
*/
void USART3_IRQHandler(void)
{
	/*  USER CODE BEGIN USART3_IRQn 0 */

	/*  USER CODE END USART3_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(USART3_IRQn);
	HAL_UART_IRQHandler(&huart3);
	/*  USER CODE BEGIN USART3_IRQn 1 */

	/*  USER CODE END USART3_IRQn 1 */
}

/* *
* @brief This function handles USART6 global interrupt.
*/
void USART6_IRQHandler(void)
{
	/*  USER CODE BEGIN USART6_IRQn 0 */
	uint32_t tmp1, tmp2;
	uint32_t pos = 0;
	/*  USER CODE END USART6_IRQn 0 */
	tmp1 = __HAL_UART_GET_FLAG(&huart6, UART_FLAG_RXNE);
	tmp2 = __HAL_UART_GET_IT_SOURCE(&huart6, UART_IT_RXNE);
	HAL_NVIC_ClearPendingIRQ(USART6_IRQn);
	HAL_UART_IRQHandler(&huart6);
	/*  USER CODE BEGIN USART6_IRQn 1 */
	if ((tmp1 != RESET) && (tmp2 != RESET))
	{
		//pos = (recvfifo_5.in + 1)&(QUEUE_REV_SIZE-1);
		pos = huart6.RxXferSize - huart6.RxXferCount;
		if (pos != recvfifo_6.out)
		{
			recvfifo_6.in = pos;
		}
	}
	/*  USER CODE END USART6_IRQn 1 */
}

/* *
* @brief This function handles SPI2 global interrupt.
*/
void SPI2_IRQHandler(void)
{
	/*  USER CODE BEGIN SPI2_IRQn 0 */

	/*  USER CODE END SPI2_IRQn 0 */
	HAL_NVIC_ClearPendingIRQ(SPI2_IRQn);
	HAL_I2S_IRQHandler(&hi2s2);
	/*  USER CODE BEGIN SPI2_IRQn 1 */

	/*  USER CODE END SPI2_IRQn 1 */
}

/*  USER CODE BEGIN 1 */

/*  USER CODE END 1 */
/* *********************** (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
