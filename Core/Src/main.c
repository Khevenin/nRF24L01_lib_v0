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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "settingModule.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEST_CONFIG 1
#define TEST_STATIC_LENGTH 1
#define TEST_DYNAMIC_LENGTH 0
#define	TESTS_ACK_PAYLOAD 1

#define TEST_RECEIVE 0

#define TAB_SIZE 5
#define BUF_SIZE 10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t testCounter = 0;
uint8_t regTmp = 0;

static uint8_t rxPayloadWidthPipe0 = 0;
uint8_t rxFifoStatus = 0;
uint8_t tmp = 0;
uint8_t txFifoStatus = 0;

uint8_t TransmitAddress[TAB_SIZE] = { 'A', 'B', 'A', 'B', 'A' };
uint8_t ReceiveAddress[TAB_SIZE] = { 'A', 'B', 'A', 'B', 'A' };

uint8_t ReceiveData[BUF_SIZE];
uint8_t TransmitData[BUF_SIZE];

char string;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void sendString(char *str, UART_HandleTypeDef *husart);
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t j;
	for (j = 0; j < BUF_SIZE; j++) {
		TransmitData[j] = ('A' + j);
	}
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
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start(&htim1);
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
#if  TEST_CONFIG
	/* 0. Create pointer and init structure. */
	sendString("nRF24L01+ struct init\r\n", &huart2);
	nrfStruct_t *testStruct;						// create pointer to struct
	testStruct = nRF_Init(&hspi1, &htim1, CSN_GPIO_Port, CSN_Pin, CE_GPIO_Port,
	CE_Pin);	// create struct
	regTmp = readReg(testStruct, CONFIG); 		// read value of CONFIG register
	sendString("nRF24L01+ struct init done\r\nStart init config\r\n", &huart2);
	/* 1.1  Set role as RX */
	modeRX(testStruct);
	regTmp = readReg(testStruct, CONFIG);
	/* 1.2 Enable CRC and set coding */
	enableCRC(testStruct);
	setCRC(testStruct, CRC_16_bits);
	regTmp = readReg(testStruct, CONFIG);
	/* 1.3 Enable/disable interrupts */
	enableRXinterrupt(testStruct);
	enableTXinterrupt(testStruct);
	regTmp = readReg(testStruct, CONFIG);
	/* 2. Set ACK for RX pipe  */
	enableAutoAckPipe(testStruct, 0);
	regTmp = readReg(testStruct, EN_AA);
	/* 3. Set RX pipe */
	enableRxAddr(testStruct, 0);
	regTmp = readReg(testStruct, EN_RXADDR);
	/* 4. Set RX/TX address width */
	setAddrWidth(testStruct, longWidth);
	regTmp = readReg(testStruct, SETUP_AW);
	/* 5. Set ARD and ARC */
	setAutoRetrCount(testStruct, 4);
	setAutoRetrDelay(testStruct, 3); //500us
	regTmp = readReg(testStruct, SETUP_RETR);
	/* 6. Set RF channel */
	setChannel(testStruct, 2);
	regTmp = readReg(testStruct, RF_CH);
	/* 7. Set RF power and Data Rate */
	setRFpower(testStruct, RF_PWR_6dBm);
	setDataRate(testStruct, RF_DataRate_250);
	regTmp = readReg(testStruct, RF_SETUP);
	/* 8 Set RX address */
	setReceivePipeAddress(testStruct, 0, ReceiveAddress,
			sizeof(ReceiveAddress));
	readRegExt(testStruct, RX_ADDR_P0, ReceiveData, 5);
	/* 9. Set TX address */
	setTransmitPipeAddress(testStruct, TransmitAddress,
			sizeof(TransmitAddress));
	readRegExt(testStruct, TX_ADDR, ReceiveData, 5);
	regTmp = readReg(testStruct, CONFIG);
#if TEST_STATIC_LENGTH
	setRxPayloadWidth(testStruct, 0, BUF_SIZE);
	regTmp = readReg(testStruct, RX_PW_P0);
	sendString("nRF24L01+ init done\r\n", &huart2);
	regTmp = readReg(testStruct, CONFIG);
#endif
#if TEST_DYNAMIC_LENGTH
	enableDynamicPayloadLength(testStruct);
	enableDynamicPayloadLengthPipe(testStruct, 0);
#endif
#if TESTS_ACK_PAYLOAD
	enableAckPayload(testStruct);
	writeTxPayloadAck(testStruct, TransmitData, sizeof(TransmitData));
#endif
#endif

	while (1) {
		HAL_Delay(1000);
		if (getRX_DR(testStruct)) {
			sendString(
					"\r\n\r\nRX_DS read as HIGH.  \r\nPayload to read.  \r\n",
					&huart2);
		}
		if (checkReceivedPayload(testStruct, 0) == 1) {
			rxFifoStatus = getRxStatusFIFO(testStruct);
			sendString("RX FIFO status BEFORE read: ", &huart2);
			tmp = rxFifoStatus + 48;
			HAL_UART_Transmit(&huart2, &tmp, 1, 1000);
			sendString("		\r\n", &huart2);


			readRxPayload(testStruct, ReceiveData, sizeof(ReceiveData));
			sendString("RX FIFO payload: ", &huart2);
			HAL_UART_Transmit(&huart2, ReceiveData, 10,
					1000);
			sendString("		\r\n", &huart2);

			rxFifoStatus = getRxStatusFIFO(testStruct);
			sendString("RX FIFO status AFTER read: ", &huart2);
			tmp = rxFifoStatus + 48;
			HAL_UART_Transmit(&huart2, &tmp, 1, 1000);
			sendString("		\r\n", &huart2);
			clearRX_DR(testStruct);
		}
#if TEST_RECEIVE
		rxFifoStatus = getRxStatusFIFO(testStruct);
		txFifoStatus = getTxStatusFIFO(testStruct);
		if (checkReceivedPayload(testStruct)) {
			rxPayloadWidthPipe0 = readDynamicPayloadWidth(testStruct);
			readRxPayload(testStruct, ReceiveData, sizeof(ReceiveData));//Read received data
			rxFifoStatus = getRxStatusFIFO(testStruct);
			txFifoStatus = getTxStatusFIFO(testStruct);
			flushTx(testStruct);								//Clear TX FIFO
			txFifoStatus = getTxStatusFIFO(testStruct);
			writeTxPayloadAck(testStruct, TransmitData, sizeof(TransmitData));//Write new ACK payload
			txFifoStatus = getTxStatusFIFO(testStruct);
		}
#endif
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
	/* End of main() */
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
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
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

/* USER CODE BEGIN 4 */
void sendString(char *str, UART_HandleTypeDef *husart) {
	HAL_UART_Transmit(husart, (uint8_t*) str, strlen(str),
			1000);
}



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
