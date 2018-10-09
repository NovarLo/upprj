/* *
  ******************************************************************************
  * File Name          : stm32f4xx_hal_msp.c
  * Date               : 16/08/2015 14:36:38
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
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

extern DMA_HandleTypeDef hdma_adc1;

extern DMA_HandleTypeDef hdma_spi2_tx;

extern DMA_HandleTypeDef hdma_spi3_tx;

extern DMA_HandleTypeDef hdma_spi3_rx;

/*  USER CODE BEGIN 0 */

/*  USER CODE END 0 */

/* *
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /*  USER CODE BEGIN MspInit 0 */

  /*  USER CODE END MspInit 0 */

  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /*  System interrupt init*/
/*  SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

  /*  USER CODE BEGIN MspInit 1 */

  /*  USER CODE END MspInit 1 */
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hadc->Instance==ADC1)
  {
  /*  USER CODE BEGIN ADC1_MspInit 0 */

  /*  USER CODE END ADC1_MspInit 0 */
    /*  Peripheral clock enable */
    __ADC1_CLK_ENABLE();
  
    /* *ADC1 GPIO Configuration    
    PA4     ------> ADC1_IN4
    PA5     ------> ADC1_IN5 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*  Peripheral DMA init*/
  
    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_adc1.Init.Mode = DMA_NORMAL;
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_adc1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
    hdma_adc1.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_adc1.Init.PeriphBurst = DMA_PBURST_SINGLE;
    HAL_DMA_Init(&hdma_adc1);

    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc1);

  /*  USER CODE BEGIN ADC1_MspInit 1 */

  /*  USER CODE END ADC1_MspInit 1 */
  }

}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{

  if(hadc->Instance==ADC1)
  {
  /*  USER CODE BEGIN ADC1_MspDeInit 0 */

  /*  USER CODE END ADC1_MspDeInit 0 */
    /*  Peripheral clock disable */
    __ADC1_CLK_DISABLE();
  
    /* *ADC1 GPIO Configuration    
    PA4     ------> ADC1_IN4
    PA5     ------> ADC1_IN5 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_5);

    /*  Peripheral DMA DeInit*/
    HAL_DMA_DeInit(hadc->DMA_Handle);
  /*  USER CODE BEGIN ADC1_MspDeInit 1 */

  /*  USER CODE END ADC1_MspDeInit 1 */
  }

}

void HAL_I2S_MspInit(I2S_HandleTypeDef* hi2s)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hi2s->Instance==SPI2)
  {
  /*  USER CODE BEGIN SPI2_MspInit 0 */

  /*  USER CODE END SPI2_MspInit 0 */
    /*  Peripheral clock enable */
    __SPI2_CLK_ENABLE();
  
    /* *I2S2 GPIO Configuration    
    PB12     ------> I2S2_WS
    PB13     ------> I2S2_CK
    PB15     ------> I2S2_SD 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*  Peripheral DMA init*/
  
    hdma_spi2_tx.Instance = DMA1_Stream4;
    hdma_spi2_tx.Init.Channel = DMA_CHANNEL_0;
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_DISABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi2_tx.Init.Mode = DMA_NORMAL;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_spi2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_spi2_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
    hdma_spi2_tx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_spi2_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    HAL_DMA_Init(&hdma_spi2_tx);

    __HAL_LINKDMA(hi2s,hdmatx,hdma_spi2_tx);

    HAL_NVIC_SetPriority(SPI2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SPI2_IRQn);
  /*  USER CODE BEGIN SPI2_MspInit 1 */

  /*  USER CODE END SPI2_MspInit 1 */
  }

}

void HAL_I2S_MspDeInit(I2S_HandleTypeDef* hi2s)
{

  if(hi2s->Instance==SPI2)
  {
  /*  USER CODE BEGIN SPI2_MspDeInit 0 */

  /*  USER CODE END SPI2_MspDeInit 0 */
    /*  Peripheral clock disable */
    __SPI2_CLK_DISABLE();
  
    /* *I2S2 GPIO Configuration    
    PB12     ------> I2S2_WS
    PB13     ------> I2S2_CK
    PB15     ------> I2S2_SD 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_15);

    /*  Peripheral DMA DeInit*/
    HAL_DMA_DeInit(hi2s->hdmatx);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(SPI2_IRQn);
  /*  USER CODE BEGIN SPI2_MspDeInit 1 */

  /*  USER CODE END SPI2_MspDeInit 1 */
  }

}

void HAL_IWDG_MspInit(IWDG_HandleTypeDef* hiwdg)
{

}

void HAL_RNG_MspInit(RNG_HandleTypeDef* hrng)
{

  if(hrng->Instance==RNG)
  {
  /*  USER CODE BEGIN RNG_MspInit 0 */

  /*  USER CODE END RNG_MspInit 0 */
    /*  Peripheral clock enable */
    __RNG_CLK_ENABLE();
  /*  USER CODE BEGIN RNG_MspInit 1 */

  /*  USER CODE END RNG_MspInit 1 */
  }

}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef* hrng)
{

  if(hrng->Instance==RNG)
  {
  /*  USER CODE BEGIN RNG_MspDeInit 0 */

  /*  USER CODE END RNG_MspDeInit 0 */
    /*  Peripheral clock disable */
    __RNG_CLK_DISABLE();
  /*  USER CODE BEGIN RNG_MspDeInit 1 */

  /*  USER CODE END RNG_MspDeInit 1 */
  }

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{

}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hspi->Instance==SPI3)
  {
  /*  USER CODE BEGIN SPI3_MspInit 0 */

  /*  USER CODE END SPI3_MspInit 0 */
    /*  Peripheral clock enable */
    __SPI3_CLK_ENABLE();
  
    /* *SPI3 GPIO Configuration    
    PB3     ------> SPI3_SCK
    PB4     ------> SPI3_MISO
    PB5     ------> SPI3_MOSI 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*  Peripheral DMA init*/
  
    hdma_spi3_tx.Instance = DMA1_Stream5;
    hdma_spi3_tx.Init.Channel = DMA_CHANNEL_0;
    hdma_spi3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_spi3_tx.Init.Mode = DMA_NORMAL;
    hdma_spi3_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_spi3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_spi3_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
    hdma_spi3_tx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_spi3_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    HAL_DMA_Init(&hdma_spi3_tx);

    __HAL_LINKDMA(hspi,hdmatx,hdma_spi3_tx);

    hdma_spi3_rx.Instance = DMA1_Stream0;
    hdma_spi3_rx.Init.Channel = DMA_CHANNEL_0;
    hdma_spi3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_spi3_rx.Init.Mode = DMA_NORMAL;
    hdma_spi3_rx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_spi3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_spi3_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
    hdma_spi3_rx.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_spi3_rx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    HAL_DMA_Init(&hdma_spi3_rx);

    __HAL_LINKDMA(hspi,hdmarx,hdma_spi3_rx);

  /*  USER CODE BEGIN SPI3_MspInit 1 */

  /*  USER CODE END SPI3_MspInit 1 */
  }

}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{

  if(hspi->Instance==SPI3)
  {
  /*  USER CODE BEGIN SPI3_MspDeInit 0 */

  /*  USER CODE END SPI3_MspDeInit 0 */
    /*  Peripheral clock disable */
    __SPI3_CLK_DISABLE();
  
    /* *SPI3 GPIO Configuration    
    PB3     ------> SPI3_SCK
    PB4     ------> SPI3_MISO
    PB5     ------> SPI3_MOSI 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);

    /*  Peripheral DMA DeInit*/
    HAL_DMA_DeInit(hspi->hdmatx);
    HAL_DMA_DeInit(hspi->hdmarx);
  /*  USER CODE BEGIN SPI3_MspDeInit 1 */

  /*  USER CODE END SPI3_MspDeInit 1 */
  }

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(htim_base->Instance==TIM2)
  {
  /*  USER CODE BEGIN TIM2_MspInit 0 */

  /*  USER CODE END TIM2_MspInit 0 */
    /*  Peripheral clock enable */
    __TIM2_CLK_ENABLE();
  
    /* *TIM2 GPIO Configuration    
    PA1     ------> TIM2_CH2
    PA3     ------> TIM2_CH4 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
  }
  else if(htim_base->Instance==TIM3)
  {
  /*  USER CODE BEGIN TIM3_MspInit 0 */

  /*  USER CODE END TIM3_MspInit 0 */
    /*  Peripheral clock enable */
    __TIM3_CLK_ENABLE();
  
    /* *TIM3 GPIO Configuration    
    PA7     ------> TIM3_CH2 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  /*  USER CODE BEGIN TIM3_MspInit 1 */

  /*  USER CODE END TIM3_MspInit 1 */
  }
  else if(htim_base->Instance==TIM5)
  {
  /*  USER CODE BEGIN TIM5_MspInit 0 */

  /*  USER CODE END TIM5_MspInit 0 */
    /*  Peripheral clock enable */
    __TIM5_CLK_ENABLE();
  
    /* *TIM5 GPIO Configuration    
    PA0-WKUP     ------> TIM5_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM5_IRQn);
  /*  USER CODE BEGIN TIM5_MspInit 1 */

  /*  USER CODE END TIM5_MspInit 1 */
  }
  else if(htim_base->Instance==TIM6)
  {
  /*  USER CODE BEGIN TIM6_MspInit 0 */

  /*  USER CODE END TIM6_MspInit 0 */
    /*  Peripheral clock enable */
    __TIM6_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  /*  USER CODE BEGIN TIM6_MspInit 1 */

  /*  USER CODE END TIM6_MspInit 1 */
  }
  else if(htim_base->Instance==TIM7)
  {
  /*  USER CODE BEGIN TIM7_MspInit 0 */

  /*  USER CODE END TIM7_MspInit 0 */
    /*  Peripheral clock enable */
    __TIM7_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);
  /*  USER CODE BEGIN TIM7_MspInit 1 */

  /*  USER CODE END TIM7_MspInit 1 */
  }
  else if(htim_base->Instance==TIM8)
  {
  /*  USER CODE BEGIN TIM8_MspInit 0 */

  /*  USER CODE END TIM8_MspInit 0 */
    /*  Peripheral clock enable */
    __TIM8_CLK_ENABLE();
  
    /* *TIM8 GPIO Configuration    
    PC6     ------> TIM8_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(TIM8_CC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM8_CC_IRQn);
    HAL_NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
    HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
  /*  USER CODE BEGIN TIM8_MspInit 1 */

  /*  USER CODE END TIM8_MspInit 1 */
  }
  else if(htim_base->Instance==TIM9)
  {
  /*  USER CODE BEGIN TIM9_MspInit 0 */

  /*  USER CODE END TIM9_MspInit 0 */
    /*  Peripheral clock enable */
    __TIM9_CLK_ENABLE();
  
    /* *TIM9 GPIO Configuration    
    PA2     ------> TIM9_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM9;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
  /*  USER CODE BEGIN TIM9_MspInit 1 */

  /*  USER CODE END TIM9_MspInit 1 */
  }
  else if(htim_base->Instance==TIM12)
  {
  /*  USER CODE BEGIN TIM12_MspInit 0 */

  /*  USER CODE END TIM12_MspInit 0 */
    /*  Peripheral clock enable */
    __TIM12_CLK_ENABLE();
  
    /* *TIM12 GPIO Configuration    
    PB14     ------> TIM12_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
  /*  USER CODE BEGIN TIM12_MspInit 1 */

  /*  USER CODE END TIM12_MspInit 1 */
  }
  else if(htim_base->Instance==TIM14)
  {
  /*  USER CODE BEGIN TIM14_MspInit 0 */

  /*  USER CODE END TIM14_MspInit 0 */
    /*  Peripheral clock enable */
    __TIM14_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
  /*  USER CODE BEGIN TIM14_MspInit 1 */

  /*  USER CODE END TIM14_MspInit 1 */
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{

  if(htim_base->Instance==TIM2)
  {
  /*  USER CODE BEGIN TIM2_MspDeInit 0 */

  /*  USER CODE END TIM2_MspDeInit 0 */
    /*  Peripheral clock disable */
    __TIM2_CLK_DISABLE();
  
    /* *TIM2 GPIO Configuration    
    PA1     ------> TIM2_CH2
    PA3     ------> TIM2_CH4 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_3);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
  /*  USER CODE BEGIN TIM2_MspDeInit 1 */

  /*  USER CODE END TIM2_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM3)
  {
  /*  USER CODE BEGIN TIM3_MspDeInit 0 */

  /*  USER CODE END TIM3_MspDeInit 0 */
    /*  Peripheral clock disable */
    __TIM3_CLK_DISABLE();
  
    /* *TIM3 GPIO Configuration    
    PA7     ------> TIM3_CH2 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_7);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
  /*  USER CODE BEGIN TIM3_MspDeInit 1 */

  /*  USER CODE END TIM3_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM5)
  {
  /*  USER CODE BEGIN TIM5_MspDeInit 0 */

  /*  USER CODE END TIM5_MspDeInit 0 */
    /*  Peripheral clock disable */
    __TIM5_CLK_DISABLE();
  
    /* *TIM5 GPIO Configuration    
    PA0-WKUP     ------> TIM5_CH1 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(TIM5_IRQn);
  /*  USER CODE BEGIN TIM5_MspDeInit 1 */

  /*  USER CODE END TIM5_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM6)
  {
  /*  USER CODE BEGIN TIM6_MspDeInit 0 */

  /*  USER CODE END TIM6_MspDeInit 0 */
    /*  Peripheral clock disable */
    __TIM6_CLK_DISABLE();

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
  /*  USER CODE BEGIN TIM6_MspDeInit 1 */

  /*  USER CODE END TIM6_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM7)
  {
  /*  USER CODE BEGIN TIM7_MspDeInit 0 */

  /*  USER CODE END TIM7_MspDeInit 0 */
    /*  Peripheral clock disable */
    __TIM7_CLK_DISABLE();

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(TIM7_IRQn);
  /*  USER CODE BEGIN TIM7_MspDeInit 1 */

  /*  USER CODE END TIM7_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM8)
  {
  /*  USER CODE BEGIN TIM8_MspDeInit 0 */

  /*  USER CODE END TIM8_MspDeInit 0 */
    /*  Peripheral clock disable */
    __TIM8_CLK_DISABLE();
  
    /* *TIM8 GPIO Configuration    
    PC6     ------> TIM8_CH1 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(TIM8_CC_IRQn);
    HAL_NVIC_DisableIRQ(TIM8_BRK_TIM12_IRQn);
    HAL_NVIC_DisableIRQ(TIM8_TRG_COM_TIM14_IRQn);
  /*  USER CODE BEGIN TIM8_MspDeInit 1 */

  /*  USER CODE END TIM8_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM9)
  {
  /*  USER CODE BEGIN TIM9_MspDeInit 0 */

  /*  USER CODE END TIM9_MspDeInit 0 */
    /*  Peripheral clock disable */
    __TIM9_CLK_DISABLE();
  
    /* *TIM9 GPIO Configuration    
    PA2     ------> TIM9_CH1 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(TIM1_BRK_TIM9_IRQn);
  /*  USER CODE BEGIN TIM9_MspDeInit 1 */

  /*  USER CODE END TIM9_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM12)
  {
  /*  USER CODE BEGIN TIM12_MspDeInit 0 */

  /*  USER CODE END TIM12_MspDeInit 0 */
    /*  Peripheral clock disable */
    __TIM12_CLK_DISABLE();
  
    /* *TIM12 GPIO Configuration    
    PB14     ------> TIM12_CH1 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(TIM8_BRK_TIM12_IRQn);
  /*  USER CODE BEGIN TIM12_MspDeInit 1 */

  /*  USER CODE END TIM12_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM14)
  {
  /*  USER CODE BEGIN TIM14_MspDeInit 0 */

  /*  USER CODE END TIM14_MspDeInit 0 */
    /*  Peripheral clock disable */
    __TIM14_CLK_DISABLE();

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(TIM8_TRG_COM_TIM14_IRQn);
  /*  USER CODE BEGIN TIM14_MspDeInit 1 */

  /*  USER CODE END TIM14_MspDeInit 1 */
  }

}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(huart->Instance==UART4)
  {
  /*  USER CODE BEGIN UART4_MspInit 0 */

  /*  USER CODE END UART4_MspInit 0 */
    /*  Peripheral clock enable */
    __UART4_CLK_ENABLE();
  
    /* *UART4 GPIO Configuration    
    PC10     ------> UART4_TX
    PC11     ------> UART4_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(UART4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);
  }
  else if(huart->Instance==UART5)
  {
  /*  USER CODE BEGIN UART5_MspInit 0 */

  /*  USER CODE END UART5_MspInit 0 */
    /*  Peripheral clock enable */
    __UART5_CLK_ENABLE();
  
    /* *UART5 GPIO Configuration    
    PC12     ------> UART5_TX
    PD2     ------> UART5_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(UART5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(UART5_IRQn);
  /*  USER CODE BEGIN UART5_MspInit 1 */

  /*  USER CODE END UART5_MspInit 1 */
  }
  else if(huart->Instance==USART1)
  {
  /*  USER CODE BEGIN USART1_MspInit 0 */

  /*  USER CODE END USART1_MspInit 0 */
    /*  Peripheral clock enable */
    __USART1_CLK_ENABLE();
  
    /* *USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /*  USER CODE BEGIN USART1_MspInit 1 */

  /*  USER CODE END USART1_MspInit 1 */
  }
  else if(huart->Instance==USART3)
  {
  /*  USER CODE BEGIN USART3_MspInit 0 */

  /*  USER CODE END USART3_MspInit 0 */
    /*  Peripheral clock enable */
    __USART3_CLK_ENABLE();
  
    /* *USART3 GPIO Configuration    
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /*  USER CODE BEGIN USART3_MspInit 1 */

  /*  USER CODE END USART3_MspInit 1 */
  }
  else if(huart->Instance==USART6)
  {
  /*  USER CODE BEGIN USART6_MspInit 0 */

  /*  USER CODE END USART6_MspInit 0 */
    /*  Peripheral clock enable */
    __USART6_CLK_ENABLE();
  
    /* *USART6 GPIO Configuration    
    PG9     ------> USART6_RX
    PG14     ------> USART6_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);
  /*  USER CODE BEGIN USART6_MspInit 1 */

  /*  USER CODE END USART6_MspInit 1 */
  }

}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{

  if(huart->Instance==UART4)
  {
  /*  USER CODE BEGIN UART4_MspDeInit 0 */

  /*  USER CODE END UART4_MspDeInit 0 */
    /*  Peripheral clock disable */
    __UART4_CLK_DISABLE();
  
    /* *UART4 GPIO Configuration    
    PC10     ------> UART4_TX
    PC11     ------> UART4_RX 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(UART4_IRQn);
  /*  USER CODE BEGIN UART4_MspDeInit 1 */

  /*  USER CODE END UART4_MspDeInit 1 */
  }
  else if(huart->Instance==UART5)
  {
  /*  USER CODE BEGIN UART5_MspDeInit 0 */

  /*  USER CODE END UART5_MspDeInit 0 */
    /*  Peripheral clock disable */
    __UART5_CLK_DISABLE();
  
    /* *UART5 GPIO Configuration    
    PC12     ------> UART5_TX
    PD2     ------> UART5_RX 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(UART5_IRQn);
  /*  USER CODE BEGIN UART5_MspDeInit 1 */

  /*  USER CODE END UART5_MspDeInit 1 */
  }
  else if(huart->Instance==USART1)
  {
  /*  USER CODE BEGIN USART1_MspDeInit 0 */

  /*  USER CODE END USART1_MspDeInit 0 */
    /*  Peripheral clock disable */
    __USART1_CLK_DISABLE();
  
    /* *USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /*  USER CODE BEGIN USART1_MspDeInit 1 */

  /*  USER CODE END USART1_MspDeInit 1 */
  }
  else if(huart->Instance==USART3)
  {
  /*  USER CODE BEGIN USART3_MspDeInit 0 */

  /*  USER CODE END USART3_MspDeInit 0 */
    /*  Peripheral clock disable */
    __USART3_CLK_DISABLE();
  
    /* *USART3 GPIO Configuration    
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX 
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /*  USER CODE BEGIN USART3_MspDeInit 1 */

  /*  USER CODE END USART3_MspDeInit 1 */
  }
  else if(huart->Instance==USART6)
  {
  /*  USER CODE BEGIN USART6_MspDeInit 0 */

  /*  USER CODE END USART6_MspDeInit 0 */
    /*  Peripheral clock disable */
    __USART6_CLK_DISABLE();
  
    /* *USART6 GPIO Configuration    
    PG9     ------> USART6_RX
    PG14     ------> USART6_TX 
    */
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_9|GPIO_PIN_14);

    /*  Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(USART6_IRQn);
  /*  USER CODE BEGIN USART6_MspDeInit 1 */

  /*  USER CODE END USART6_MspDeInit 1 */
  }

}

static int FSMC_Initialized = 0;

static void HAL_FSMC_MspInit(void){
  GPIO_InitTypeDef GPIO_InitStruct;
  if (FSMC_Initialized) {
    return;
  }
  FSMC_Initialized = 1;
  /*  Peripheral clock enable */
  __FSMC_CLK_ENABLE();
  
  /* * FSMC GPIO Configuration  
  PE7   ------> FSMC_D4
  PE8   ------> FSMC_D5
  PE9   ------> FSMC_D6
  PE10   ------> FSMC_D7
  PD11   ------> FSMC_CLE
  PD12   ------> FSMC_ALE
  PD14   ------> FSMC_D0
  PD15   ------> FSMC_D1
  PG6   ------> FSMC_INT2
  PD0   ------> FSMC_D2
  PD1   ------> FSMC_D3
  PD4   ------> FSMC_NOE
  PD5   ------> FSMC_NWE
  PD6   ------> FSMC_NWAIT
  PD7   ------> FSMC_NCE2
  */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15 
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

void HAL_NAND_MspInit(NAND_HandleTypeDef* hnand){
  HAL_FSMC_MspInit();
}

static int FSMC_DeInitialized = 0;

static void HAL_FSMC_MspDeInit(void){
  if (FSMC_DeInitialized) {
    return;
  }
  FSMC_DeInitialized = 1;
  /*  Peripheral clock enable */
  __FSMC_CLK_DISABLE();
  
  /* * FSMC GPIO Configuration  
  PE7   ------> FSMC_D4
  PE8   ------> FSMC_D5
  PE9   ------> FSMC_D6
  PE10   ------> FSMC_D7
  PD11   ------> FSMC_CLE
  PD12   ------> FSMC_ALE
  PD14   ------> FSMC_D0
  PD15   ------> FSMC_D1
  PG6   ------> FSMC_INT2
  PD0   ------> FSMC_D2
  PD1   ------> FSMC_D3
  PD4   ------> FSMC_NOE
  PD5   ------> FSMC_NWE
  PD6   ------> FSMC_NWAIT
  PD7   ------> FSMC_NCE2
  */
  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10);

  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15 
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_7);

  HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6);

}

void HAL_NAND_MspDeInit(NAND_HandleTypeDef* hnand){
  HAL_FSMC_MspDeInit();
}

/*  USER CODE BEGIN 1 */

/*  USER CODE END 1 */

/* *
  * @}
  */

/* *
  * @}
  */

/* *********************** (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
