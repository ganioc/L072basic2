/**
 ******************************************************************************
 * File Name          : RTC.c
 * Description        : This file provides code for the configuration
 *                      of the RTC instances.
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
#include "rtc.h"

/* USER CODE BEGIN 0 */
#include "gpio.h"
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

	/** Initialize RTC Only
	 */
	/*##-1- Configure the RTC peripheral #######################################*/
	/* Configure RTC prescaler and RTC data registers */
	/* RTC configured as follows:
	 - Hour Format    = Format 24
	 - Asynch Prediv  = Value according to source clock
	 - Synch Prediv   = Value according to source clock
	 - OutPut         = Output Disable
	 - OutPutPolarity = High Polarity
	 - OutPutType     = Open Drain */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}

}

/**
  * @brief  Configure the current time and date.
  * @param  None
  * @retval None
  */
static void RTC_CalendarConfig(void)
{
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;

  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday February 18th 2014 */
  sdatestructure.Year = 0x20;
  sdatestructure.Month = RTC_MONTH_FEBRUARY;
  sdatestructure.Date = 0x18;
  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;

  if(HAL_RTC_SetDate(&hrtc,&sdatestructure,RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /*##-2- Configure the Time #################################################*/
  /* Set Time: 02:00:00 */
  stimestructure.Hours = 0x02;
  stimestructure.Minutes = 0x00;
  stimestructure.Seconds = 0x00;
  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(&hrtc, &stimestructure, RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /*##-3- Writes a data in a RTC Backup data Register1 #######################*/
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	if (rtcHandle->Instance == RTC)
	{
		/* USER CODE BEGIN RTC_MspInit 0 */
		/*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
		/* To change the source clock of the RTC feature (LSE, LSI), You have to:
		 - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
		 - Enable write access using HAL_PWR_EnableBkUpAccess() function before to
		 configure the RTC clock source (to be done once after reset).
		 - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and
		 __HAL_RCC_BACKUPRESET_RELEASE().
		 - Configure the needed RTc clock source */
		__HAL_RCC_PWR_CLK_ENABLE();
		HAL_PWR_EnableBkUpAccess();

		/*##-2- Configure LSE as RTC clock source ###################################*/
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
		RCC_OscInitStruct.LSEState = RCC_LSE_ON;
		if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		{
			Error_Handler();
		}

		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
		PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
		{
			Error_Handler();
		}
		/* USER CODE END RTC_MspInit 0 */
		/* RTC clock enable */
		__HAL_RCC_RTC_ENABLE();
		/* USER CODE BEGIN RTC_MspInit 1 */
		/*##-2- Check if Data stored in BackUp register1: No Need to reconfigure RTC#*/
		/* Read the Back Up Register 1 Data */
		if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2)
		{
			/* Configure RTC Calendar */
			RTC_CalendarConfig();
		}
		else
		{
			/* Check if the Power On Reset flag is set */
			if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
			{
				/* Turn on LED2: Power on reset occurred */
				onStateLED();
			}
			/* Check if Pin Reset flag is set */
			if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
			{
				/* Turn on LED4: External reset occurred */
				onStateLED();
			}
			/* Clear source Reset Flag */
			__HAL_RCC_CLEAR_RESET_FLAGS();
		}
		/* USER CODE END RTC_MspInit 1 */
	}
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

	if (rtcHandle->Instance == RTC)
	{
		/* USER CODE BEGIN RTC_MspDeInit 0 */

		/* USER CODE END RTC_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_RTC_DISABLE();
		/* USER CODE BEGIN RTC_MspDeInit 1 */
		/*##-2- Disables the PWR Clock and Disables access to the backup domain ###################################*/
		HAL_PWR_DisableBkUpAccess();
		__HAL_RCC_PWR_CLK_DISABLE();
		/* USER CODE END RTC_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
