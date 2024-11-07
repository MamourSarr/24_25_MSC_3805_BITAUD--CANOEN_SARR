/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_TX_BUFFER_SIZE 64
#define UART_RX_BUFFER_SIZE 1
#define CMD_BUFFER_SIZE 64
#define MAX_ARGS 9
#define ASCII_ENTER 0x0D
#define MAX_SPEED 1024  // Valeur maximale de la vitesse
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
const uint8_t prompt[]="user@Nucleo-STM32G474>>";
const uint8_t started[]=
		"\r\n*-----------------------------*"
		"\r\n| Welcome on MCC SHELL |"
		"\r\n*-----------------------------*"
		"\r\n";
const uint8_t newline[]="\r\n";
const uint8_t cmdNotFound[]="Command not found\r\n";
const uint8_t help[] = "Available commands: help, pinout, start, stop, speed\r\n";
const uint8_t pinout[] = "Pins used: PA0, PA1, PB3\r\n";
const uint8_t powerOn[] = "Power ON\r\n";
const uint8_t powerOff[] = "Power OFF\r\n";
const uint8_t speed_msg[] = "Speed : \r\n";
uint32_t uartRxReceived;
uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];
uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];
char Etat;
char Speed_buf[5];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
	char cmdBuffer[CMD_BUFFER_SIZE];
	int idx_cmd;
	char* argv[MAX_ARGS];
	int argc = 0;
	int32_t ch_MCC = 0;
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
	MX_ADC2_Init();
	MX_ADC1_Init();
	MX_TIM1_Init();
	MX_TIM3_Init();
	MX_USART2_UART_Init();
	/* USER CODE BEGIN 2 */
	memset(argv,NULL,MAX_ARGS*sizeof(char*));
	memset(cmdBuffer,NULL,CMD_BUFFER_SIZE*sizeof(char));
	memset(uartRxBuffer,NULL,UART_RX_BUFFER_SIZE*sizeof(char));
	memset(uartTxBuffer,NULL,UART_TX_BUFFER_SIZE*sizeof(char));

	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
	HAL_Delay(10);
	HAL_UART_Transmit(&huart2, started, sizeof(started), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);


	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		if (uartRxReceived == 1)
		{
			printf("OK \r\n");
			// Si caractère reçu est ENTER
			if (uartRxBuffer[0] == ASCII_ENTER)
			{
				HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
				cmdBuffer[idx_cmd] = '\0';  // Fin de la commande

				// Process the command
				processCommand(cmdBuffer);



				HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
				idx_cmd = 0;
				memset(cmdBuffer, 0, CMD_BUFFER_SIZE);
			}
			else
			{
				// Si ce n'est pas ENTER, stocker le caractère dans le buffer
				if (idx_cmd < CMD_BUFFER_SIZE - 1)
				{
					cmdBuffer[idx_cmd++] = uartRxBuffer[0];
				}
			}
			HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);

			uartRxReceived = 0;
		}
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

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
	RCC_OscInitStruct.PLL.PLLN = 85;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

void processCommand(char *cmd)
{

	if (strcmp(cmd, "help") == 0) {
		HAL_UART_Transmit(&huart2, help, sizeof(help) - 1, HAL_MAX_DELAY);
	}
	else if (strcmp(cmd, "pinout") == 0) {
		HAL_UART_Transmit(&huart2, pinout, sizeof(pinout) - 1, HAL_MAX_DELAY);
	}
	else if (strcmp(cmd, "start") == 0) {
		HAL_UART_Transmit(&huart2, powerOn, sizeof(powerOn) - 1, HAL_MAX_DELAY);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

	}
	else if (strcmp(cmd, "stop") == 0) {
		HAL_UART_Transmit(&huart2, powerOff, sizeof(powerOff) - 1, HAL_MAX_DELAY);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

	}
	else if (strncmp(cmd, "speed", 5) == 0) {

		int speed_value = atoi(cmd + 6);

		//Limiter la vitesse à MAX_SPEED
		if (speed_value > MAX_SPEED) {
			speed_value = MAX_SPEED;
			char maxSpeedMsg[] = "Max Speed\r\n";
			HAL_UART_Transmit(&huart2, (uint8_t *)maxSpeedMsg, sizeof(maxSpeedMsg) - 1, HAL_MAX_DELAY);
		}

		int i = 0;
		uint32_t pwm_value = (speed_value * MAX_SPEED) / 100;
		for(i = 0; i < pwm_value; i++){
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, i);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 1024- i);
			HAL_Delay(10);
		}
		//Afficher la vitesse actuelle
		char speedMsg[32];
		snprintf(speedMsg, sizeof(speedMsg), "Speed : %d\r\n", speed_value);
		HAL_UART_Transmit(&huart2, (uint8_t *)speedMsg, strlen(speedMsg), HAL_MAX_DELAY);
	}


	else {
		HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound) - 1, HAL_MAX_DELAY);
	}
}


void HAL_UART_RxCpltCallback (UART_HandleTypeDef * huart){
	uartRxReceived = 1;
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}
/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
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
	__disable_irq();
	while (1)
	{
	}
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
