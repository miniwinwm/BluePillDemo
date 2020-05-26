/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "graphics.h"

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
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
extern const unsigned char pic[];
const int16_t triangleCornersXOrig[3] = {-5, 0, 5};
const int16_t triangleCornersYOrig[3] = {5, -5, 5};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
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
  int16_t triangleCornersX[3] = {-5, 0, 5};
  int16_t triangleCornersY[3] = {5, -5, 5};
  uint8_t i = 0U;

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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  GraphicsInit();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	i++;
	if (i == 5U)
	{
		memcpy(triangleCornersX, triangleCornersXOrig, sizeof(triangleCornersX));
		memcpy(triangleCornersY, triangleCornersYOrig, sizeof(triangleCornersY));
		i = 0U;
	}

	GraphicsClear(true);
	GraphicsUpdateDisplay();
	HAL_Delay(200UL);

	GraphicsClear(false);
	GraphicsUpdateDisplay();
	HAL_Delay(200UL);

	GraphicsVline(10, 20, 100, true);
	GraphicsUpdateDisplay();
	GraphicsHline(0U, SSD1306_OLED_WIDTH, 50, true);
	GraphicsUpdateDisplay();
	GraphicsRectangle(15, 10, 20, 18, true);
	GraphicsUpdateDisplay();
	GraphicsStandardString(20, 30, "Freddy", true);
	GraphicsUpdateDisplay();
	GraphicsStandardStringVert(60, 20, "Freddy", true);
	GraphicsUpdateDisplay();
	HAL_Delay(500UL);

	GraphicsClear(false);
	GraphicsLargeString(10, 10, "Freddy", true);
	GraphicsUpdateDisplay();
	GraphicsLargeStringVert(100, 0, "Freddy", true);
	GraphicsUpdateDisplay();
	HAL_Delay(500UL);

	GraphicsClear(false);
	GraphicsLine(0, 0, 127, 63, true);
	GraphicsUpdateDisplay();
	GraphicsLine(0, 63, 127, 0, true);
	GraphicsUpdateDisplay();
	GraphicsCircle(63, 32, 22, true);
	GraphicsUpdateDisplay();
	HAL_Delay(500UL);

	GraphicsClear(false);
	GraphicsMonochromeBitmap(0, 0, 128, 64, pic);
	GraphicsUpdateDisplay();
	HAL_Delay(500UL);

	GraphicsClear(false);
	GraphicsFilledCircle(64, 32, 30, true);
	GraphicsUpdateDisplay();
	HAL_Delay(500UL);

	GraphicsClear(false);
	GraphicsScaleShape(3U, triangleCornersX, triangleCornersY, 15, 13);
	GraphicsRotateShape(3U, triangleCornersX, triangleCornersY, 45);
	GraphicsDrawShape(3U, triangleCornersX, triangleCornersY, 42, 32, true);
	GraphicsDrawFilledShape(3U, triangleCornersX, triangleCornersY, 84, 32, true);
	GraphicsUpdateDisplay();
	HAL_Delay(500UL);

	GraphicsClear(false);
	GraphicsArc(42, 32, 15, 45, 270, true);
	GraphicsSegment(84, 32, 15, -90, 179, true);
	GraphicsUpdateDisplay();
	HAL_Delay(500UL);

	GraphicsClear(false);
	GraphicsFilledSegment(84, 32, 15, -90, 179, 20U, true);
	GraphicsUpdateDisplay();
	HAL_Delay(500UL);

	GraphicsClear(false);
	GraphicsRoundedRectangle(20, 10, 100U, 50U, 15, true);
	GraphicsUpdateDisplay();
    HAL_Delay(500UL);
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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
