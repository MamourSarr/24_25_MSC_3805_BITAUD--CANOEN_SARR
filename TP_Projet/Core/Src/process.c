/**
 * @file process.c
 * @brief Implémentation des fonctions pour la gestion des commandes via un shell UART sur STM32.
 *
 * Ce fichier contient le code permettant de gérer des commandes utilisateur
 * pour contrôler divers périphériques via un shell interactif.
 *
 * @date 27 novembre 2024
 * @author MAMOUR SARR
 */

#include "process.h"
#include "usart.h"
#include "tim.h"
#include "adc.h"

#define UART_TX_BUFFER_SIZE 64 /**< Taille du buffer de transmission UART */
#define UART_RX_BUFFER_SIZE 1  /**< Taille du buffer de réception UART */
#define CMD_BUFFER_SIZE 64     /**< Taille maximale du buffer de commande */
#define MAX_ARGS 9             /**< Nombre maximal d'arguments pour une commande */
#define ASCII_ENTER 0x0D       /**< Code ASCII pour la touche ENTER */
#define MAX_SPEED 1024         /**< Vitesse maximale pour le moteur */
#define ADC_BUF_SIZE 8         /**< Taille du buffer ADC */

extern uint16_t ADC_buffer[ADC_BUF_SIZE]; /**< Buffer pour les données ADC */
char cmdBuffer[CMD_BUFFER_SIZE];          /**< Buffer pour les commandes utilisateur */
int idx_cmd;                              /**< Index courant dans le buffer de commande */
char* argv[MAX_ARGS];                     /**< Tableau des arguments d'une commande */
int argc = 0;                             /**< Nombre d'arguments pour la commande */
int32_t ch_MCC = 0;                       /**< État du module MCC */

static const uint8_t prompt[] = "user@Nucleo-STM32G474>>"; /**< Invite de commande */
static const uint8_t started[] =
    "\r\n*-----------------------------*"
    "\r\n| Welcome on MCC SHELL |"
    "\r\n*-----------------------------*\r\n"; /**< Message de bienvenue */
static const uint8_t newline[] = "\r\n"; /**< Nouveau saut de ligne */
static const uint8_t cmdNotFound[] = "Command not found\r\n"; /**< Message d'erreur pour commande invalide */
static const uint8_t help[] = "Available commands: help, pinout, start, stop, speed, ADC\r\n"; /**< Aide utilisateur */
static const uint8_t pinout[] = "Pins used: PA0, PA1, PB3\r\n"; /**< Description des broches utilisées */
static const uint8_t powerOn[] = "Power ON\r\n"; /**< Message pour mise sous tension */
static const uint8_t powerOff[] = "Power OFF\r\n"; /**< Message pour arrêt */
static uint32_t uartRxReceived; /**< Indicateur de réception UART */
static uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE]; /**< Buffer de réception UART */
static uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE]; /**< Buffer de transmission UART */
static char Etat; /**< État courant du système */
static char Speed_buf[5]; /**< Buffer pour la vitesse */
uint8_t former_speed = 0; /**< Dernière vitesse définie */

/**
 * @brief Callback pour la réception UART.
 *
 * Cette fonction est appelée automatiquement lorsqu'une donnée
 * est reçue via UART.
 *
 * @param huart Pointeur vers la structure UART utilisée.
 */
void HAL_UART_RxCpltCallback (UART_HandleTypeDef * huart){
	uartRxReceived = 1;
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}


/**
 * @brief Fonction principale de gestion du shell UART.
 *
 * Cette fonction traite les entrées utilisateur, exécute les commandes correspondantes
 * et renvoie les résultats via UART.
 */
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


/**
 * @brief Calcule et affiche la vitesse actuelle.
 *
 * Cette fonction lit un encodeur pour déterminer la vitesse
 * du moteur et affiche les résultats via UART.
 */
void speed(){

	float speed_count = __HAL_TIM_GET_COUNTER(&htim3);
	float motor_speed = speed_count/(735/495);
	char speed_adc[50];
	int taille = snprintf(speed_adc, sizeof(speed_adc),"Speed_encoder: %f\r\n,", motor_speed);

	HAL_UART_Transmit(&huart2, (uint8_t*)speed_adc, taille, 100);

}

/**
 * @brief Lit les données du capteur ADC et calcule le courant correspondant.
 *
 * Cette fonction convertit les données brutes de l'ADC en tension, puis en courant
 * en utilisant les spécifications du capteur (sensibilité et offset).
 */
void ADC(){

	HAL_Delay(100);
	//HAL_ADC_Start(&hadc1);


	// Lecture de la valeur ADC
	//uint32_t ADC_value = HAL_ADC_GetValue(&hadc1);
	float ADC_value = ADC_buffer[0];
	HAL_Delay(10);
	float sensivity = 0.05;  // Sensibilité du capteur en V/A
	float V_offset = 1.65;
	float V_adc = (ADC_value * 3.3f / 4095.0f); // Tension mesurée
	float I_current = (V_adc - V_offset) / sensivity; // Courant calculé

	char buffer_adc[50];
	int taille = snprintf(buffer_adc, sizeof(buffer_adc),"ADC: %f, V: %.2f, I: %.2f\r\n", ADC_value, V_adc, I_current);

	HAL_UART_Transmit(&huart2, (uint8_t*)buffer_adc, taille, 100);

}


/**
 * @brief Traite une commande reçue via UART et exécute l'action correspondante.
 *
 * @param cmd La commande saisie par l'utilisateur sous forme de chaîne de caractères.
 */
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

		ADC();

	}

	else if (strcmp(cmd, "SPEED") == 0) {

		speed();

	}


	else {
		HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound) - 1, HAL_MAX_DELAY);
	}
}

