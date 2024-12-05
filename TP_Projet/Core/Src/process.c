/*
 * process.c
 *
 *  Created on: Nov 27, 2024
 *      Author: Utilisateur
 */

#include "process.h"
#include "usart.h"
#include "tim.h"
#include "adc.h"

#define UART_TX_BUFFER_SIZE 64
#define UART_RX_BUFFER_SIZE 1
#define CMD_BUFFER_SIZE 64
#define MAX_ARGS 9
#define ASCII_ENTER 0x0D
#define MAX_SPEED 1024  // Valeur maximale de la vitesse
#define ADC_BUF_SIZE 8

extern uint16_t ADC_buffer[ADC_BUF_SIZE];
char cmdBuffer[CMD_BUFFER_SIZE];
int idx_cmd;
char* argv[MAX_ARGS];
int argc = 0;
int32_t ch_MCC = 0;

static const uint8_t prompt[]="user@Nucleo-STM32G474>>";
static const uint8_t started[]=
		"\r\n*-----------------------------*"
		"\r\n| Welcome on MCC SHELL |"
		"\r\n*-----------------------------*"
		"\r\n";
static const uint8_t newline[]="\r\n";
static const uint8_t cmdNotFound[]="Command not found\r\n";
static const uint8_t help[] = "Available commands: help, pinout, start, stop, speed, ADC\r\n";
static const uint8_t pinout[] = "Pins used: PA0, PA1, PB3\r\n";
static const uint8_t powerOn[] = "Power ON\r\n";
static const uint8_t powerOff[] = "Power OFF\r\n";
static const uint8_t speed_msg[] = "Speed : \r\n";
static const uint8_t ADC_msg[] = "Current_value : \r\n";
static uint32_t uartRxReceived;
static uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];
static uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];
static char Etat;
static char Speed_buf[5];
uint8_t former_speed = 0;




void HAL_UART_RxCpltCallback (UART_HandleTypeDef * huart){
	uartRxReceived = 1;
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}

//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	//if (hadc->Instance == ADC1) {
		//ADC();
	//}
//}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//  if (htim->Instance == TIM1) {
//    ADC();
//}
//}

void shell(){

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

}


void ADC(){

	HAL_Delay(1000);
	//HAL_ADC_Start(&hadc1);


	// Lecture de la valeur ADC
	//uint32_t ADC_value = HAL_ADC_GetValue(&hadc1);
	float ADC_value = ADC_buffer[0];
	HAL_Delay(10);

	float sensivity = 0.05;  // Sensibilité du capteur en V/A
	float V_offset = 1.65;
	float V_adc = (ADC_value * 3.3f / 4096.0f); // Tension mesurée
	float I_current = (V_adc - V_offset) / sensivity; // Courant calculé

	char buffer_adc[50];
	int taille = snprintf(buffer_adc, sizeof(buffer_adc),"ADC: %f, V: %.2f, I: %.2f\r\n", ADC_value, V_adc, I_current);

	HAL_UART_Transmit(&huart2, (uint8_t*)buffer_adc, taille, 100);

}

void processCommand(char *cmd){

	if (strcmp(cmd, "help") == 0) {
		HAL_UART_Transmit(&huart2, help, sizeof(help) - 1, HAL_MAX_DELAY);
	}
	else if (strcmp(cmd, "pinout") == 0) {
		HAL_UART_Transmit(&huart2, pinout, sizeof(pinout) - 1, HAL_MAX_DELAY);
	}
	else if (strcmp(cmd, "start") == 0) {
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 512);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 512);
		HAL_UART_Transmit(&huart2, powerOn, sizeof(powerOn) - 1, HAL_MAX_DELAY);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

		former_speed = 50;

	}
	else if (strcmp(cmd, "stop") == 0) {
		HAL_UART_Transmit(&huart2, powerOff, sizeof(powerOff) - 1, HAL_MAX_DELAY);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

		former_speed = 0;


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
		uint32_t former_pwm_value = (former_speed * MAX_SPEED) / 100;
		if(pwm_value >= former_pwm_value){
			for(i = former_pwm_value; i < pwm_value; i++){
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, i);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 1024- i);
				HAL_Delay(10);
			}
		}
		else{

			for(i = former_pwm_value; i > pwm_value; i--){
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, i);
				__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 1024- i);
				HAL_Delay(10);
			}
		}
		former_speed = speed_value;
		//Afficher la vitesse actuelle
		char speedMsg[32];
		snprintf(speedMsg, sizeof(speedMsg), "Speed : %d\r\n", speed_value);
		HAL_UART_Transmit(&huart2, (uint8_t *)speedMsg, strlen(speedMsg), HAL_MAX_DELAY);
	}

	else if (strcmp(cmd, "ADC") == 0) {

		//if(htim->Instance == TIM1){
		ADC();
		//}

	}


	else {
		HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound) - 1, HAL_MAX_DELAY);
	}
}


