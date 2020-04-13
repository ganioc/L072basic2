/*
 * low_power_tick_management_LPTIM.c
 *
 *  Created on: 13 Apr 2020
 *      Author: yangjun
 */

#include "cmsis_os.h"
#include "stm32l0xx_hal.h"
#include "lptim.h"


#define portMAX_16_BIT_NUMBER				( 0xffffUL )

extern LPTIM_HandleTypeDef             LptimHandle;


void vPortSetupTimerInterrupt( void );

#if( configUSE_TICKLESS_IDLE == 2 )

/*
 * Override the default definition of vPortSuppressTicksAndSleep() that is
 * weakly defined in the FreeRTOS Cortex-M port layer with a version that
 * manages the LPTIM clock, as the tick is generated from the low power LPTIM
 * and not the SysTick as would normally be the case on a Cortex-M.
 */
void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime );


/*-----------------------------------------------------------*/

/* Calculate how many clock increments make up a single tick period. */
static const uint32_t ulReloadValueForOneTick = ( LSE_VALUE / configTICK_RATE_HZ);

/* Will hold the maximum number of ticks that can be suppressed. */
static uint32_t xMaximumPossibleSuppressedTicks = 0;

/* Flag set from the tick interrupt to allow the sleep processing to know if
sleep mode was exited because of a timer interrupt or a different interrupt. */
volatile uint32_t ulTickFlag = pdFALSE;

#endif
