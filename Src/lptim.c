/**
  ******************************************************************************
  * File Name          : LPTIM.c
  * Description        : This file provides code for the configuration
  *                      of the LPTIM instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lptim.h"

/* USER CODE BEGIN 0 */
#include "stm32l0xx_hal.h"
#include "cmsis_os.h"


/* Set the Maximum value of the counter (Auto-Reload) that defines the Period */
#define PeriodValue (uint32_t)(32 - 1)

/* Set the Compare value that defines the duty cycle */
#define PulseValue (uint32_t)(15 - 1)


LPTIM_HandleTypeDef LptimHandle;

__IO uint32_t ulTickInit = 0;

/* Clocks structure declaration */
RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;


extern uint32_t ulTickFlag;



/* USER CODE END 0 */



/* LPTIM1 init function */
/*
void MX_LPTIM1_Init(void)
{

  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }

}
*/

//void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* lptimHandle)
//{
//
//  if(lptimHandle->Instance==LPTIM1)
//  {
//  /* USER CODE BEGIN LPTIM1_MspInit 0 */
//
//  /* USER CODE END LPTIM1_MspInit 0 */
//    /* LPTIM1 clock enable */
//    __HAL_RCC_LPTIM1_CLK_ENABLE();
//
//    /* LPTIM1 interrupt Init */
//    HAL_NVIC_SetPriority(LPTIM1_IRQn, 3, 0);
//    HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
//  /* USER CODE BEGIN LPTIM1_MspInit 1 */
//
//  /* USER CODE END LPTIM1_MspInit 1 */
//  }
//}
//
//void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* lptimHandle)
//{
//
//  if(lptimHandle->Instance==LPTIM1)
//  {
//  /* USER CODE BEGIN LPTIM1_MspDeInit 0 */
//
//  /* USER CODE END LPTIM1_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_LPTIM1_CLK_DISABLE();
//
//    /* LPTIM1 interrupt Deinit */
//    HAL_NVIC_DisableIRQ(LPTIM1_IRQn);
//  /* USER CODE BEGIN LPTIM1_MspDeInit 1 */
//
//  /* USER CODE END LPTIM1_MspDeInit 1 */
//  }
//}

/* USER CODE BEGIN 1 */

/**
* @brief  LPTIM MSP Init
* @param  hlptim : LPTIM handle
* @retval None
*/
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef *hlptim)
{
  GPIO_InitTypeDef     GPIO_InitStruct;

  /* ## - 1 - Enable LPTIM clock ############################################ */
  __HAL_RCC_LPTIM1_CLK_ENABLE();

  /* ## - 2 - Force & Release the LPTIM Periheral Clock Reset ############### */
  /* Force the LPTIM Periheral Clock Reset */
  __HAL_RCC_LPTIM1_FORCE_RESET();
  /* Release the LPTIM Periheral Clock Reset */
  __HAL_RCC_LPTIM1_RELEASE_RESET();

  /* ## - 3 - Enable & Configure LPTIM Output ############################### */
  /* Configure PC1 (LPTIM1_OUT) in alternate function (AF0), Low speed
  push-pull mode and pull-up enabled.
  Note: In order to reduce power consumption: GPIO Speed is configured in
  LowSpeed */

  /* Enable GPIO PORT C */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* Configure PC1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW     ;
  GPIO_InitStruct.Alternate = GPIO_AF0_LPTIM1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
* @brief  This function configures the LPTIM to generate an interrupt each 1ms.
  * @param  TickPriority: Tick interrupt priority.
  * @retval HAL status
  */
void InitTick(uint32_t TickPriority)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;

  if (ulTickInit == 0)
  {
    /* Enable LSE clock */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* ### - 1 - Re-target the LSE to Clock the LPTIM Counter ################# */
    /* Select the LSE clock as LPTIM peripheral clock */
    RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
    RCC_PeriphCLKInitStruct.LptimClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
    HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

    /* ### - 2 - Initialize LPTIM peripheral ################################## */
    /*
   *  Instance        = LPTIM1
   *  Clock Source    = APB or LowPowerOSCillator (in this example LSE is
   *                    already selected from the RCC stage)
   *  Clock prescaler = 1 (No division)
   *  Counter source  = Internal event.
   *  Counter Trigger = Software trigger
   *  Output Polarity = High
   *  Update mode     = Immediate (Registers are immediately updated after any
   *                    write access)
   */

    LptimHandle.Instance = LPTIM1;

    LptimHandle.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    LptimHandle.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
    LptimHandle.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
    LptimHandle.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    LptimHandle.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
    LptimHandle.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;

    /* Initialize LPTIM peripheral according to the passed parameters */
    if (HAL_LPTIM_Init(&LptimHandle) != HAL_OK)
    {
    	Error_Handler();
    }

    HAL_NVIC_SetPriority(LPTIM1_IRQn, TickPriority, 0U);
    HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
    /* ### - 3 - Start generating the PWM signal ############################## */
    /*
   *  Period = 99
   *  Pulse  = 49
   *  According to this configuration, the duty cycle will be equal to 50%
   */
    if (HAL_LPTIM_PWM_Start_IT(&LptimHandle, PeriodValue, PulseValue) != HAL_OK)
    {
    	Error_Handler();
    }

    ulTickInit = 1;
  }
}
/**
  * @brief This function configures the source of the time base.
  *        The time source is configured  to have 1ms time base with a dedicated
  *        Tick interrupt priority.
  * @param TickPriority: Tick interrupt priority.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  InitTick(TickPriority);

  return HAL_OK;
}
void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
  if (HAL_LPTIM_ReadAutoReload(hlptim) != PeriodValue)
  {
    HAL_LPTIM_PWM_Start_IT(&LptimHandle, PeriodValue, PulseValue);
  }

  HAL_IncTick();
  osSystickHandler();

#if (configUSE_TICKLESS_IDLE == 2)
  ulTickFlag = pdTRUE;
#endif
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
