/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
osThreadId_t mainHandle;
uint32_t mainBuffer[ 128 ];
osStaticThreadDef_t mainControlBlock;
osThreadId_t lcdHandle;
uint32_t lcdTaskBuffer[ 128 ];
osStaticThreadDef_t lcdTaskControlBlock;
osMessageQueueId_t lcdQueueHandle;
uint8_t lcdBuffer[ 6 * sizeof( lcd_packet_t ) ];
osStaticMessageQDef_t lcdControlBlock;
/* USER CODE BEGIN PV */
osEventFlagsId_t modemTaskStartedEventHandle;

static const uint8_t custom_characters[] = {0x04, 0x0a, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
												0x0e, 0x06, 0x0a, 0x10, 0x10, 0x11, 0x0e, 0x00,
												0x04, 0x0e, 0x1f, 0x04, 0x04, 0x1f, 0x0e, 0x04,
												0x04, 0x0e, 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04,
												0x04, 0x04, 0x04, 0x04, 0x04, 0x1f, 0x0e, 0x04};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void mainTask(void *argument);
void lcdTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  lcd_init();
  for (uint8_t i = 0U; i < sizeof(custom_characters) / CG_CHARACTER_SIZE_BYTES; i++)
  {
  	lcd_set_cg_character(i + 1U, &(custom_characters[i * CG_CHARACTER_SIZE_BYTES]));
  }

  /* USER CODE END 2 */

  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  modemTaskStartedEventHandle = osEventFlagsNew(NULL);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of lcdQueue */
  const osMessageQueueAttr_t lcdQueue_attributes = {
    .name = "lcdQueue",
    .cb_mem = &lcdControlBlock,
    .cb_size = sizeof(lcdControlBlock),
    .mq_mem = &lcdBuffer,
    .mq_size = sizeof(lcdBuffer)
  };
  lcdQueueHandle = osMessageQueueNew (6, sizeof(lcd_packet_t), &lcdQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of main */
  const osThreadAttr_t main_attributes = {
    .name = "main",
    .stack_mem = &mainBuffer[0],
    .stack_size = sizeof(mainBuffer),
    .cb_mem = &mainControlBlock,
    .cb_size = sizeof(mainControlBlock),
    .priority = (osPriority_t) osPriorityNormal,
  };
  mainHandle = osThreadNew(mainTask, NULL, &main_attributes);

  /* definition and creation of lcd */
  const osThreadAttr_t lcd_attributes = {
    .name = "lcd",
    .stack_mem = &lcdTaskBuffer[0],
    .stack_size = sizeof(lcdTaskBuffer),
    .cb_mem = &lcdTaskControlBlock,
    .cb_size = sizeof(lcdTaskControlBlock),
    .priority = (osPriority_t) osPriorityLow,
  };
  lcdHandle = osThreadNew(lcdTask, NULL, &lcd_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_mainTask */
/**
  * @brief  Function implementing the main thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_mainTask */
void mainTask(void *argument)
{
  /* USER CODE BEGIN 5 */

  // wait for lcd task to start
  osEventFlagsWait(modemTaskStartedEventHandle, 0x00000001U, osFlagsWaitAny, osWaitForever);

  /* Infinite loop */
  for(;;)
  {
	lcd_clear();
	osDelay(1000U);

	lcd_puts(0U, 0U, "fred");
	lcd_puts(1U, 0U, "\x01\x02\x03\x04\x05");
	osDelay(1000U);
  }
  /* USER CODE END 5 */ 
}

/* USER CODE BEGIN Header_lcdTask */
/**
* @brief Function implementing the lcd thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_lcdTask */
void lcdTask(void *argument)
{
  /* USER CODE BEGIN lcdTask */
  lcd_packet_t lcd_packet;
  uint8_t i;

  // signal main task that this thread has started
  osEventFlagsSet(modemTaskStartedEventHandle, 0x00000001U);

  /* Infinite loop */
  for (;;)
  {
	osMessageQueueGet(lcdQueueHandle, &lcd_packet, NULL, osWaitForever);

	switch (lcd_packet.command)
	{
	case CLEAR_SCREEN:
		lcd_write_command(0x01U);
		break;

	case WRITE_TEXT:
		if (lcd_packet.row == 0U)
		{
			lcd_write_command(0x80U + lcd_packet.column);
		}
		else
		{
			lcd_write_command(0xc0U + lcd_packet.column);
		}

		for (i = 0U; i < lcd_packet.length; i++)
		{
			lcd_write_data(lcd_packet.text[i]);
		}
		break;
	}
  }
  /* USER CODE END lcdTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
