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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "task.h"
#include "modem.h"
#include "buffered_serial.h"
#include "mqtt.h"
#include "seatalk.h"
#include "printf.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/**** SET YOUR OWN VALUES HERE ****/
#define ACCESS_POINT_NAME					"everywhere"			// SET YOUR OWN VALUE network settings
#define USER_NAME							"eesecure"				// SET YOUR OWN VALUE may be blank for some networks in which case change to NULL (not "NULL")
#define PASSWORD							"secure"				// SET YOUR OWN VALUE may be blank for some networks in which case change to NULL (not "NULL")
#define MQTT_PUBLISH_TOPIC_ROOT				"BluePillDemo"			// SET YOUR OWN VALUE topic root for all published values
#error "Remove this line when you have set your own values above"

#define PUBLISH_PERIOD_MS					5000UL					// time in ms that changed values are published
#define MINIMUM_REPUBLISH_TIME_MS			30000UL					// time in ms that values are republished even if unchanged
#define NETWORK_REGISTRATION_WAIT_TIME_MS	30000UL					// time to wait for network registration before giving up

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define MY_SNPRINTF snprintf	// newlib snprintf with float support is too heavy and not threadsafe so using local snprintf from printf.c
								// but this causes a compile warning that float support is off so use this macro to prevent it

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticMutexDef_t;
osThreadId_t mainHandle;
uint32_t mainBuffer[ 250 ];
osStaticThreadDef_t mainControlBlock;
osThreadId_t modemHandle;
uint32_t modemBuffer[ 350 ];
osStaticThreadDef_t modemControlBlock;
osMessageQueueId_t commandQueueHandle;
uint8_t commandQueueBuffer[ 1 * sizeof( AtCommandPacket_t ) ];
osStaticMessageQDef_t commandQueueControlBlock;
osMessageQueueId_t responseQueueHandle;
uint8_t responseQueueBuffer[ 1 * sizeof( AtResponsePacket_t ) ];
osStaticMessageQDef_t responseQueueControlBlock;
osMutexId_t modemMutexHandle;
osStaticMutexDef_t modemMutexControlBlock;
/* USER CODE BEGIN PV */
osEventFlagsId_t modemTaskStartedEventHandle;
static bool subscribeResponseReceived;
static uint16_t subscribeResponsePacketIdentifier;
static bool sogReceived;
static bool depthReceived;
static bool boatspeedReceived;
static bool awaReceived;
static bool awsReceived;
static bool sogReceived;
static bool cogReceived;
static bool heading1Received;
static bool heading2Received;
static bool tripReceived;
static bool logReceived;
static bool twasReceived;
static bool latReceived;
static bool longReceived;
static bool tempReceived;
static float newSog;
static float oldSog;
static uint32_t sogLastPubTime = UINT32_MAX / 2;
static float newDepth;
static float oldDepth;
static uint32_t depthLastPubTime = UINT32_MAX / 2;
static float newBoatspeed;
static float oldBoatspeed;
static uint32_t boatspeedLastPubTime = UINT32_MAX / 2;
static float newAwa;
static float oldAwa;
static uint32_t awaLastPubTime = UINT32_MAX / 2;
static float newAws;
static float oldAws;
static uint32_t awsLastPubTime = UINT32_MAX / 2;
static int16_t newCog;
static int16_t oldCog;
static uint32_t cogLastPubTime = UINT32_MAX / 2;
static float oldTrip;
static float newTrip;
static uint32_t tripLastPubTime = UINT32_MAX / 2;
static float newLog;
static float oldLog;
static uint32_t logLastPubTime = UINT32_MAX / 2;
static float newTemp;
static float oldTemp;
static uint32_t tempLastPubTime = UINT32_MAX / 2;
static float newTws;
static float oldTws;
static uint32_t twsLastPubTime = UINT32_MAX / 2;
static float newTwa;
static float oldTwa;
static uint32_t twaLastPubTime = UINT32_MAX / 2;
static float newLatitude;
static float oldLatitude;
static uint32_t latitudeLastPubTime = UINT32_MAX / 2;
static float newLongitude;
static float oldLongitude;
static uint32_t longitudeLastPubTime = UINT32_MAX / 2;
static float newHeading;
static float oldHeading;
static uint32_t headingLastPubTime = UINT32_MAX / 2;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_IWDG_Init(void);
void mainTask(void *argument);
void modemTask(void *argument);

/* USER CODE BEGIN PFP */
void ModemDebugPrintStatus(const char *text, ModemStatus_t modemStatus);
void MqttDebugPrintStatus(const char *text, MqttStatus_t mqttStatus);
void PublishCallback(const char *topic, uint8_t topicLength, const uint8_t *payload, uint32_t payloadLength);
void PingResponseCallback(void);
void SubscribeResponseCallback(uint16_t packetIdentifier, bool success);
void UnsubscribeResponseCallback(uint16_t packetIdentifier);
void SeatalkMessageHandler(uint8_t messageType);
static void DebugPrint(const char *text);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void SeatalkMessageHandler(uint8_t messageType)
{
	switch(messageType)
	{
	case SEATALK_DEPTH:
		depthReceived = true;
		break;

	case SEATALK_BOATSPEED:
		boatspeedReceived = true;
		break;

	case SEATALK_APPARENT_WIND_ANGLE:
		awaReceived = true;
		break;

	case SEATALK_APPARENT_WIND_SPEED:
		awsReceived = true;
		break;

	case SEATALK_SOG:
		sogReceived = true;
		break;

	case SEATALK_COG:
		cogReceived = true;
		break;

	case SEATALK_HEADING_MAGNETIC_1:
		heading1Received = true;
		break;

	case SEATALK_HEADING_MAGNETIC_2:
		heading2Received = true;
		break;

	case SEATALK_TRIPLOG:
		tripReceived = true;
		logReceived = true;
		break;

	case SEATALK_TRUE_WIND_ANGLE_SPEED:
		twasReceived = true;
		break;

	case SEATALK_LATITUDE:
		latReceived = true;
		break;

	case SEATALK_LONGITUDE:
		longReceived = true;
		break;

	case SEATALK_TEMPERATURE:
		tempReceived = true;
		break;

	default:
		break;
	}
}

void SubscribeResponseCallback(uint16_t packetIdentifier, bool success)
{
  subscribeResponseReceived = success;
  subscribeResponsePacketIdentifier = packetIdentifier;
}

void MqttDebugPrintStatus(const char *text, MqttStatus_t mqttStatus)
{
  DebugPrint(text);
  DebugPrint(": ");
  DebugPrint(MqttStatusToText(mqttStatus));
  DebugPrint("\r\n");
}

static void DebugPrint(const char *text)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)text, strlen(text), 100U);
}

void ModemDebugPrintStatus(const char *text, ModemStatus_t modemStatus)
{
  DebugPrint(text);
  DebugPrint(": ");
  DebugPrint(ModemStatusToText(modemStatus));
  DebugPrint("\r\n");
}

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
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  seatalk_init(SeatalkMessageHandler);

  /* USER CODE END 2 */

  osKernelInitialize();

  /* Create the recursive mutex(es) */
  /* definition and creation of modemMutex */
  const osMutexAttr_t modemMutex_attributes = {
    .name = "modemMutex",
    .attr_bits = osMutexRecursive,
    .cb_mem = &modemMutexControlBlock,
    .cb_size = sizeof(modemMutexControlBlock),
  };
  modemMutexHandle = osMutexNew(&modemMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  modemTaskStartedEventHandle = osEventFlagsNew(NULL);

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of commandQueue */
  const osMessageQueueAttr_t commandQueue_attributes = {
    .name = "commandQueue",
    .cb_mem = &commandQueueControlBlock,
    .cb_size = sizeof(commandQueueControlBlock),
    .mq_mem = &commandQueueBuffer,
    .mq_size = sizeof(commandQueueBuffer)
  };
  commandQueueHandle = osMessageQueueNew (1, sizeof(AtCommandPacket_t), &commandQueue_attributes);

  /* definition and creation of responseQueue */
  const osMessageQueueAttr_t responseQueue_attributes = {
    .name = "responseQueue",
    .cb_mem = &responseQueueControlBlock,
    .cb_size = sizeof(responseQueueControlBlock),
    .mq_mem = &responseQueueBuffer,
    .mq_size = sizeof(responseQueueBuffer)
  };
  responseQueueHandle = osMessageQueueNew (1, sizeof(AtResponsePacket_t), &responseQueue_attributes);

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

  /* definition and creation of modem */
  const osThreadAttr_t modem_attributes = {
    .name = "modem",
    .stack_mem = &modemBuffer[0],
    .stack_size = sizeof(modemBuffer),
    .cb_mem = &modemControlBlock,
    .cb_size = sizeof(modemControlBlock),
    .priority = (osPriority_t) osPriorityLow,
  };
  modemHandle = osThreadNew(modemTask, NULL, &modem_attributes);

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 936;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
  ModemStatus_t modemStatus;
  MqttStatus_t mqttStatus;
  bool registrationStatus;
  char ipAddress[MODEM_MAX_IP_ADDRESS_LENGTH];
  uint32_t startTime;

  // kick watchdog
  HAL_IWDG_Refresh(&hiwdg);

  // wait for modem task to start
  osEventFlagsWait(modemTaskStartedEventHandle, 0x00000001UL, osFlagsWaitAny, osWaitForever);

  serial_init();

  // kick watchdog
  HAL_IWDG_Refresh(&hiwdg);

  modemStatus = ModemHello(250UL);
  ModemDebugPrintStatus("Hello", modemStatus);

  DebugPrint("Attempting to register on network\r\n");
  startTime = osKernelGetTickCount();
  do
  {
    // kick watchdog
    HAL_IWDG_Refresh(&hiwdg);

	ModemGetNetworkRegistrationStatus(&registrationStatus, 250UL);
	osDelay(1000UL);

	if (osKernelGetTickCount() > startTime + NETWORK_REGISTRATION_WAIT_TIME_MS)
	{
		DebugPrint("Could not register on network, rebooting\r\n");
		while (true)
		{
		  osDelay(1000UL);
		}
	}
  } while (!registrationStatus);
  ModemDebugPrintStatus("Register on network", modemStatus);

  modemStatus = ModemSetManualDataRead(250UL);
  ModemDebugPrintStatus("Set manual read", modemStatus);

  if (modemStatus >= MODEM_OK)
  {
    modemStatus = ModemConfigureDataConnection(ACCESS_POINT_NAME, USER_NAME, PASSWORD, 250UL);
    ModemDebugPrintStatus("Configure data connection", modemStatus);
  }

  if (modemStatus >= MODEM_OK)
  {
	modemStatus = ModemActivateDataConnection(10000UL);
    ModemDebugPrintStatus("Activate data connection", modemStatus);
  }

  if (modemStatus >= MODEM_OK)
  {
    modemStatus = ModemGetOwnIpAddress(ipAddress, MODEM_MAX_IP_ADDRESS_LENGTH, 250UL);
    ModemDebugPrintStatus("Get own IP address", modemStatus);
  }

  if (modemStatus < MODEM_OK)
  {
	  DebugPrint("Modem error in modem configuration, rebooting\r\n");
	  while (true)
	  {
	    osDelay(1000UL);
	  }
  }

  MqttSetSubscribeResponseCallback(SubscribeResponseCallback);

  // kick watchdog
  HAL_IWDG_Refresh(&hiwdg);

  // open TCP connection to broker
  modemStatus = ModemOpenTcpConnection("broker.hivemq.com", 1883U, 8000UL);
  ModemDebugPrintStatus("Open TCP connection", modemStatus);

  // kick watchdog
  HAL_IWDG_Refresh(&hiwdg);

  if (modemStatus < MODEM_OK)
  {
	  DebugPrint("Coule no open connection to broker, rebooting\r\n");
	  while (true)
	  {
	    osDelay(1000UL);
	  }
  }

  // connect to broker
  mqttStatus = MqttConnect("1234", NULL, NULL, 600U, 20000UL);
  MqttDebugPrintStatus("MQTT connect", mqttStatus);
  if (mqttStatus != MQTT_OK)
  {
	DebugPrint("Could not connect to broker, rebooting\r\n");
	while (true)
	{
	  osDelay(1000UL);
	}
  }

  // go into main loop
  while (true)
  {
	// kick watchdog
	HAL_IWDG_Refresh(&hiwdg);

	// go into wait loop checking for incoming seatalk messages and incoming MQTT responses
	startTime = osKernelGetTickCount();
	uint8_t i = 0U;
	while (true)
	{
	  	seatalk_parse_next_message();
	  	seatalk_send_next_message();
	    osDelay(10UL);

	    i++;
	    if (i == 25U)
	    {
	    	i = 0U;
	    	mqttStatus = MqttHandleResponse(5000UL);
	    }

		if (osKernelGetTickCount() > startTime + PUBLISH_PERIOD_MS)
		{
			break;
		}
	}

    char buf[20];

    if (sogReceived && mqttStatus >= MQTT_OK)
    {
      sogReceived = false;
      newSog = seatalk_speed_over_ground_data_retrieve();
      if (oldSog != newSog || (osKernelGetTickCount() - sogLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
      {
		  MY_SNPRINTF(buf, sizeof(buf), "%.1f", newSog);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/sog", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldSog = newSog;
		  sogLastPubTime = osKernelGetTickCount();
      }
    }

    if (depthReceived && mqttStatus >= MQTT_OK)
    {
	  depthReceived = false;
	  newDepth = seatalk_depth_data_retrieve();
	  if (oldDepth != newDepth || (osKernelGetTickCount() - depthLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
	  {
		  MY_SNPRINTF(buf, sizeof(buf), "%.1f", newDepth);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/depth", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldDepth = newDepth;
		  depthLastPubTime = osKernelGetTickCount();
	  }
    }

    if (boatspeedReceived && mqttStatus >= MQTT_OK)
    {
	  boatspeedReceived = false;
	  newBoatspeed = seatalk_boat_speed_data_retrieve();
	  if (newBoatspeed != oldBoatspeed || (osKernelGetTickCount() - boatspeedLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
	  {
		  MY_SNPRINTF(buf, sizeof(buf), "%.1f", newBoatspeed);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/boatspeed", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldBoatspeed = newBoatspeed;
		  boatspeedLastPubTime = osKernelGetTickCount();
	  }
    }

    if (awaReceived && mqttStatus >= MQTT_OK)
    {
	  awaReceived = false;
	  newAwa = seatalk_apparent_wind_angle_retrieve();
	  if (newAwa != oldAwa && (osKernelGetTickCount() - awaLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
	  {
		  MY_SNPRINTF(buf, sizeof(buf), "%.0f", newAwa);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/awa", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldAwa = newAwa;
		  awaLastPubTime = osKernelGetTickCount();
	  }
    }

    if (awsReceived && mqttStatus >= MQTT_OK)
    {
	  awsReceived = false;
	  newAws = seatalk_apparent_wind_speed_retrieve();
	  if (newAws != oldAws || (osKernelGetTickCount() - awsLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
	  {
		  MY_SNPRINTF(buf, sizeof(buf), "%.1f", newAws);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/aws", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldAws = newAws;
		  awsLastPubTime = osKernelGetTickCount();
	  }
    }

    if (cogReceived && mqttStatus >= MQTT_OK)
    {
	  cogReceived = false;
	  newCog = seatalk_course_over_ground_data_retrieve();
	  if (newCog != oldCog || (osKernelGetTickCount() - cogLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
	  {
		  MY_SNPRINTF(buf, sizeof(buf), "%hu", newCog);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/cog", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldCog = newCog;
		  cogLastPubTime = osKernelGetTickCount();
	  }
    }

    if ((heading1Received || heading2Received)  && mqttStatus >= MQTT_OK)
    {
	  heading1Received = false;
	  heading2Received = false;
	  newHeading = seatalk_heading_magnetic_retrieve();
	  if (newHeading != oldHeading || (osKernelGetTickCount() - headingLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
	  {
		  snprintf(buf, sizeof(buf), "%hu", newHeading);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/heading", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldHeading = newHeading;
		  headingLastPubTime = osKernelGetTickCount();
	  }
    }

    if (tripReceived && mqttStatus >= MQTT_OK)
    {
	  tripReceived = false;
	  newTrip = seatalk_trip_data_retrieve();
	  if (newTrip != oldTrip || (osKernelGetTickCount() - tripLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
	  {
		  MY_SNPRINTF(buf, sizeof(buf), "%.1f", newTrip);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/trip", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldTrip = newTrip;
		  tripLastPubTime = osKernelGetTickCount();
	  }
    }

    if (logReceived && mqttStatus >= MQTT_OK)
    {
      logReceived = false;
      newLog = seatalk_log_data_retrieve();
      if (newLog != oldLog || (osKernelGetTickCount() - logLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
      {
		  MY_SNPRINTF(buf, sizeof(buf), "%.1f", newLog);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/log", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldLog = newLog;
		  logLastPubTime = osKernelGetTickCount();
      }
    }

    if (twasReceived && mqttStatus >= MQTT_OK)
    {
	  twasReceived = false;

	  newTwa = seatalk_true_wind_angle_retrieve();
	  if (newTwa != oldTwa || (osKernelGetTickCount() - twaLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
	  {
		  MY_SNPRINTF(buf, sizeof(buf), "%.0f", newTwa);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/twa", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldTwa = newTwa;
		  twaLastPubTime = osKernelGetTickCount();
	  }

	  newTws = seatalk_true_wind_speed_retrieve();
	  if (newTws != oldTws || (osKernelGetTickCount() - twsLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
	  {
		  MY_SNPRINTF(buf, sizeof(buf), "%.1f", newTws);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/twa", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldTws = newTws;
		  twsLastPubTime = osKernelGetTickCount();
	  }
    }

    if (latReceived && mqttStatus >= MQTT_OK)
    {
  	  latReceived = false;

      newLatitude = (float)seatalk_latitude_degrees_retrieve();
      if (newLatitude > 0.0f)
      {
    	  newLatitude += seatalk_latitude_minutes_retrieve() / 60.0f;
      }
      else
      {
    	  newLatitude -= seatalk_latitude_minutes_retrieve() / 60.0f;
      }
      if (newLatitude != oldLatitude || (osKernelGetTickCount() - latitudeLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
      {
		  MY_SNPRINTF(buf, sizeof(buf), "%f", newLatitude);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/lat", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldLatitude = newLatitude;
		  latitudeLastPubTime = osKernelGetTickCount();
      }
    }

    if (longReceived && mqttStatus >= MQTT_OK)
    {
  	  longReceived = false;

	  newLongitude = (float)seatalk_longitude_degrees_retrieve();
	  if (newLongitude > 0.0f)
	  {
		  newLongitude += seatalk_longitude_minutes_retrieve() / 60.0f;
	   }
	  else
	  {
		  newLongitude -= seatalk_longitude_minutes_retrieve() / 60.0f;
	  }
	  if (newLongitude != oldLongitude || (osKernelGetTickCount() - longitudeLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
	  {
		  MY_SNPRINTF(buf, sizeof(buf), "%f", newLongitude);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/long", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldLongitude = newLongitude;
		  longitudeLastPubTime = osKernelGetTickCount();
	  }
    }

    if (tempReceived && mqttStatus >= MQTT_OK)
    {
	  tempReceived = false;
	  newTemp = seatalk_temperature_data_retrieve();
	  if (newTemp != oldTemp || (osKernelGetTickCount() - tempLastPubTime > MINIMUM_REPUBLISH_TIME_MS))
	  {
		  MY_SNPRINTF(buf, sizeof(buf), "%.1f", newTemp);
		  mqttStatus = MqttPublish(MQTT_PUBLISH_TOPIC_ROOT "/temp", (uint8_t *)buf, strlen(buf), false, 5000UL);
		  oldTemp = newTemp;
		  tempLastPubTime = osKernelGetTickCount();
	  }
    }

    if (mqttStatus < MQTT_OK)
    {
      MqttDebugPrintStatus("MQTT error, rebooting", mqttStatus);
	  while (true)
	  {
	    osDelay(1000UL);
	  }
    }
  }

  /* USER CODE END 5 */ 
}

/* USER CODE BEGIN Header_modemTask */
/**
* @brief Function implementing the modem thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_modemTask */
void modemTask(void *argument)
{
  /* USER CODE BEGIN modemTask */
  ModemReset();
  ModemInit();

  // signal main task that this thread has started
  osEventFlagsSet(modemTaskStartedEventHandle, 0x00000001U);

  /* Infinite loop */
  DoModemTask();

  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END modemTask */
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
