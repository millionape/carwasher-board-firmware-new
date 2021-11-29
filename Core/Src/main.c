/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <max7219.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SW_DEBOUNCE_TIME 130
#define IOT_SEND_INTERVAL 5
#define CREDIT_HI_BYTE 0x06
#define CREDIT_LO_BYTE 0x07
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void init_display(void);
void do_1sec_tick(void);
void do_500ms_tick(void);
void do_400ms_tick(void);
void do_300ms_tick(void);
void do_200ms_tick(void);
uint8_t read_button(void);
void do_operation(void);
void segment_display_int(int number);
void decrease_credit(void);
uint8_t get_function_duration(uint8_t selected_mode);
void segment_display_standby(void);
void segment_display_off(void);
void reset_all_pins(void);
void setting_menu_loop(void);
void segment_display_function_settings(int func_number,int value);
void display_menu(uint8_t selected_menu);
/////  eeprom stuff
void eeprom_write(uint8_t addr, uint8_t data);
void store_credit_eeprom(uint16_t credit);
uint16_t read_credit_eeprom(void);
/////  eeprom stuff
void set_add_duration_of_function(uint8_t _selected_menu);
void set_substract_duration_of_function(uint8_t _selected_menu);
void read_settings_from_eeprom(void);
void reset_all_state(void);
void send_iot_status(uint8_t money_event);
void stop_acceptors(void);
void start_acceptors(void);

void add_bank_note_credit(void);
void add_coin_credit(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile int tim2_counter = 0;
volatile int tim2_300ms_counter = 0;
volatile int tim2_200ms_counter = 0;
volatile int tim2_400ms_counter = 0;
volatile int tim2_500ms_counter = 0;
volatile int iot_round_counter = 0;
volatile bool is_operation_running = false;
volatile uint16_t credit = 0;   // credit store
volatile uint8_t pressed_button = 0;
volatile uint8_t setting_mode = 0;
volatile uint8_t current_out_port = 0;
volatile bool is_standby = true;
volatile uint8_t standby_counter = 0;
volatile bool consume_credit = false;
volatile bool display_binking = false;
volatile bool front_button_reset_credit_press = false;
volatile uint16_t front_button_reset_credit_counter = 0;
/* duration settings */
volatile uint8_t F1_DURATION = 10;
volatile uint8_t F2_DURATION = 10;
volatile uint8_t F3_DURATION = 10;
volatile uint8_t F4_DURATION = 10;
volatile uint8_t F5_DURATION = 10;
volatile uint8_t credit_consume_counter = 0;

/// for timer capture
volatile uint32_t IC_Val1 = 0;
volatile uint32_t IC_Val2 = 0;
volatile uint32_t Difference = 0;
volatile uint8_t Is_First_Captured = 0;  // is the first value captured ?

volatile uint32_t coin_IC_Val1 = 0;
volatile uint32_t coin_IC_Val2 = 0;
volatile uint32_t coin_Difference = 0;
volatile uint8_t coin_Is_First_Captured = 0;  // is the first value captured ?

volatile const int8_t bank_acceptor_pulse_width = 45;
volatile const int8_t coin_acceptor_pulse_width = 50;
volatile const int8_t creditPulseOffset = 10;

volatile const uint8_t bank_credit_per_pulse = 10;
volatile const uint8_t coin_credit_per_pulse = 1;

volatile const int minimum_credit_to_start = 10;
volatile const uint8_t default_credit_duration = 10;

volatile uint8_t last_note_money = 0;
volatile uint8_t last_coin_money = 0;
////////////////////////////////////


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
  MX_RTC_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	reset_all_pins();
	read_settings_from_eeprom();
	init_display();
	//	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
	//	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
	reset_all_pins();
	HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		uint8_t btn_read = read_button();
		if(btn_read == 6){ // credit reset is press
			reset_all_state();
			store_credit_eeprom(credit); // save last credit value
			continue;
		}
		if(btn_read == 7){ // mode button is pressed
			setting_mode = 1;
			setting_menu_loop();
			continue;
		}
		if(btn_read != 0){
			if(btn_read == pressed_button){
				pressed_button = 0;
			}else{
				if(pressed_button == 0 ){
					pressed_button = btn_read;
				}
			}
		}
		if(is_operation_running){
			is_standby = false;
			do_operation();
		}else{
			pressed_button = 0;
			reset_all_pins();
		}
		HAL_Delay(120);
//		start_acceptors();

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void send_iot_status(uint8_t money_event){
	HAL_UART_Transmit(&huart1, (uint8_t*)">>>>>>>>> sending iot package\r\n", 31,HAL_MAX_DELAY);
	uint8_t iot_serial_package[8];
	iot_serial_package[0] = 0x02; // STX start flag
	iot_serial_package[1] = credit;  // credit hi bits
	iot_serial_package[2] = 0;  // credit lo bits
	iot_serial_package[3] = pressed_button;  // mode
	iot_serial_package[4] = money_event;  // money in event
	iot_serial_package[5] = 0x03; // ETX stop flag
	iot_serial_package[6] = 0x0D; // newline
	iot_serial_package[7] = 0x0A; // carriage return
	HAL_UART_Transmit(&huart1, (uint8_t *)iot_serial_package, 8, HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart3, (uint8_t *)iot_serial_package, 8, HAL_MAX_DELAY);
}
void display_menu(uint8_t selected_menu){
	switch (selected_menu) {
	case 1:
		segment_display_function_settings(selected_menu,F1_DURATION);
		break;
	case 2:
		segment_display_function_settings(selected_menu,F2_DURATION);
		break;
	case 3:
		max7219_PrintDigit(DIGIT_3,18,true);
		segment_display_function_settings(selected_menu,F3_DURATION);
		break;
	case 4:
		max7219_PrintDigit(DIGIT_3,19,true);
		segment_display_function_settings(selected_menu,F4_DURATION);
		break;
	case 5:
		segment_display_function_settings(selected_menu,F5_DURATION);
		break;
	}
}
void setting_menu_loop(){
	reset_all_state();
	is_standby = false;
	while(setting_mode != 0){
		uint8_t btn_read = read_button();
		if(btn_read == 7){
			setting_mode += 1;
			if(setting_mode >= 6){
				setting_mode = 0;
				reset_all_state();
				return;
			}
			continue;
		}else if(btn_read == 1){
			set_add_duration_of_function(setting_mode);
		}else if(btn_read == 2){
			set_substract_duration_of_function(setting_mode);
		}
	}
}
void store_credit_eeprom(uint16_t store_credit){
	uint8_t hbits=store_credit & 0xff;
	uint8_t lbits=(store_credit >> 8);
	eeprom_write(CREDIT_HI_BYTE, hbits);
	eeprom_write(CREDIT_LO_BYTE, lbits);
}
void eeprom_write(uint8_t addr, uint8_t data){
	/*
	 * EEPROM ADDRESS TABLE
	 * ------------------------
	 *  address | description
	 * ------------------------
	 *   0x01   |  F1_DURATION
	 *   0x02   |  F2_DURATION
	 *   0x03   |  F3_DURATION
	 *   0x04   |  F4_DURATION
	 *   0x05   |  F5_DURATION
	 *
	 *	 0x06   |  credit
	 *
	 * */
	switch(addr){
	case 0x01:
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, data);
		break;
	case 0x02:
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, data);
		break;
	case 0x03:
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, data);
		break;
	case 0x04:
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, data);
		break;
	case 0x05:
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, data);
		break;
	case 0x06:  /// credit hi bit
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR6, data);
		break;
	case 0x07:  /// credit lo bit
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR7, data);
		break;
	}

}
void set_add_duration_of_function(uint8_t _selected_menu){
	if(_selected_menu > 0){
		switch (_selected_menu) {
		case 1:
			if(F1_DURATION < 99){
				F1_DURATION += 1;
				eeprom_write(0x01,F1_DURATION);
			}
			break;
		case 2:
			if(F2_DURATION < 99){
				F2_DURATION += 1;
				eeprom_write(0x02,F2_DURATION);
			}
			break;
		case 3:
			if(F3_DURATION < 99){
				F3_DURATION += 1;
				eeprom_write(0x03,F3_DURATION);
			}
			break;
		case 4:
			if(F4_DURATION < 99){
				F4_DURATION += 1;
				eeprom_write(0x04,F4_DURATION);
			}
			break;
		case 5:
			if(F5_DURATION < 99){
				F5_DURATION += 1;
				eeprom_write(0x05,F5_DURATION);
			}
			break;
		}
	}
}

void set_substract_duration_of_function(uint8_t _selected_menu){
	if(_selected_menu > 0){
		switch (_selected_menu) {
		case 1:
			if(F1_DURATION >0){
				F1_DURATION -= 1;
				eeprom_write(0x01,F1_DURATION);
			}
			break;
		case 2:
			if(F2_DURATION >0){
				F2_DURATION -= 1;
				eeprom_write(0x02,F2_DURATION);
			}
			break;
		case 3:
			if(F3_DURATION >0){
				F3_DURATION -= 1;
				eeprom_write(0x03,F3_DURATION);
			}
			break;
		case 4:
			if(F4_DURATION >0){
				F4_DURATION -= 1;
				eeprom_write(0x04,F4_DURATION);
			}
			break;
		case 5:
			if(F5_DURATION >0){
				F5_DURATION -= 1;
				eeprom_write(0x05,F5_DURATION);
			}
			break;
		}
	}
}
void segment_display_function_settings(int func_number,int value){
	max7219_Decode_On();
	max7219_Clean ();
	if(value < 10){
		max7219_PrintItos (DIGIT_1, value );
	}else if(value >= 10 && value < 100){
		max7219_PrintItos (DIGIT_2, value );
	}
	max7219_PrintDigit(DIGIT_3,func_number,true);
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2){
		//		HAL_UART_Transmit(&huart1, (uint8_t*)"timer2 tick!\r\n", 14,HAL_MAX_DELAY);
		tim2_counter += 1;
		tim2_500ms_counter += 1;
		tim2_300ms_counter += 1;
		tim2_400ms_counter += 1;
		tim2_200ms_counter += 1;
		if(tim2_counter % 10 == 0){ /// 100 ms tick
//			char tmp_msg[35];
//			sprintf(tmp_msg,"pressed button : %d \r\n",pressed_button);
//			HAL_UART_Transmit(&huart1, (uint8_t *)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);
			if(setting_mode != 0){
				is_standby = false;
				display_menu(setting_mode);
			}
		}

		if(tim2_counter >= 100){
			tim2_counter = 0;
			do_1sec_tick();
		}
		if(tim2_500ms_counter >= 50){
			tim2_500ms_counter = 0;
			do_500ms_tick();
		}
		if(tim2_200ms_counter >= 20){
			tim2_200ms_counter = 0;
			do_200ms_tick();
		}
		if(tim2_300ms_counter >= 30){
			tim2_300ms_counter = 0;
			do_300ms_tick();
		}
		if(tim2_400ms_counter >= 40){
			tim2_400ms_counter = 0;
			do_400ms_tick();
		}
	}
}
uint8_t get_function_duration(uint8_t selected_mode){
	uint8_t duration_per_1credit = 0;
	switch(selected_mode){
	case 1:
		duration_per_1credit = F1_DURATION;
		break;
	case 2:
		duration_per_1credit = F2_DURATION;
		break;
	case 3:
		duration_per_1credit = F3_DURATION;
		break;
	case 4:
		duration_per_1credit = F4_DURATION;
		break;
	case 5:
		duration_per_1credit = F5_DURATION;
		break;
	default:
		duration_per_1credit = 10;
		break;
	}
	return duration_per_1credit;
}
void reset_all_state(){
	//	HAL_TIM_Base_Stop_IT(&htim2);
	//	__HAL_TIM_SET_COUNTER(&htim2, 0);
	//	__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
	reset_all_pins();
	consume_credit = false;
	credit = 0;
	is_standby = true;
	is_operation_running = false;
	pressed_button = 0;
	max7219_Turn_On();
	max7219_Clean();
	tim2_counter = 0;
	tim2_500ms_counter = 0;
	tim2_300ms_counter = 0;
	tim2_400ms_counter = 0;
	tim2_200ms_counter = 0;
	//	HAL_TIM_Base_Start_IT(&htim2);
}
//void stop_acceptors(){
//	HAL_TIM_Base_Stop_IT(&htim3);
//	HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_1);
//	HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_2);
//	__HAL_TIM_SET_COUNTER(&htim3, 0);
//	__HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
//}
//
//void start_acceptors(){
//	HAL_TIM_Base_Start_IT(&htim3);
//	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
//	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
//}
void decrease_credit(){
	if(consume_credit){
		credit_consume_counter += 1;
		char tmp_msg[35];
		sprintf(tmp_msg,"Consuming counter: %d \r\n",credit_consume_counter);
		HAL_UART_Transmit(&huart1, (uint8_t *)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);
		uint8_t duration = get_function_duration(pressed_button);
		if(credit_consume_counter >= duration){
			credit_consume_counter = 0;
			HAL_UART_Transmit(&huart1, (uint8_t*)"Consuming 1 credit\r\n", 20,HAL_MAX_DELAY);
			credit -= 1; // decrease money
			if(credit <= 0){
				credit = 0;
				reset_all_state();
			}
		}
		store_credit_eeprom(credit);
	}
}
void do_1sec_tick(){
	HAL_UART_Transmit(&huart1, (uint8_t*)"1sec tick\r\n", 11,HAL_MAX_DELAY);
	char tmp_msg[35];
	sprintf(tmp_msg,"#### current credit: %d, mode: %d\r\n",credit,pressed_button);
	HAL_UART_Transmit(&huart1, (uint8_t *)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);
	decrease_credit(); // logic runner funtion
	iot_round_counter += 1;
	if(iot_round_counter >= IOT_SEND_INTERVAL){
		max7219_DisableDisplayTest(); /// keep disable display test to prevent 888
		iot_round_counter = 0;
		send_iot_status(0);
		if(last_coin_money != 0){
			send_iot_status(last_coin_money);
			last_coin_money = 0;
		}
		if(last_note_money != 0){
			send_iot_status(last_note_money);
			last_note_money = 0;
		}
	}
}
void do_500ms_tick(){

}
void do_200ms_tick(){
	if(front_button_reset_credit_press){
		front_button_reset_credit_counter += 1;
	}else{
		front_button_reset_credit_counter = 0;
	}
//	char tmp_msg[35];
//	sprintf(tmp_msg,"===== front button counter: %d\r\n",front_button_reset_credit_counter);
//	HAL_UART_Transmit(&huart1, (uint8_t *)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);
	if(is_standby){
		segment_display_standby();
	}
}
void do_300ms_tick(){
	store_credit_eeprom(credit);
	HAL_GPIO_TogglePin(ALIVE_LED_GPIO_Port, ALIVE_LED_Pin);
}

void do_400ms_tick(){
	if(is_operation_running){
		display_binking = !display_binking;
		if(display_binking && (current_out_port != 0)){
			max7219_Turn_Off();
		}else{
			max7219_Turn_On();
			segment_display_int(credit);
		}

	}else{
		max7219_Turn_On();
	}
}

void init_display(){
	max7219_DisableDisplayTest();
	HAL_Delay(100);
	max7219_Init ( 5 );
	max7219_Decode_On ();
}
void reset_all_pins(){
	HAL_GPIO_WritePin(OUT_1_GPIO_Port, OUT_1_Pin, GPIO_PIN_RESET); // pin b0 --> out 1
	HAL_GPIO_WritePin(OUT_2_GPIO_Port, OUT_2_Pin, GPIO_PIN_RESET); // pin b1 --> out 2
	HAL_GPIO_WritePin(OUT_3_GPIO_Port, OUT_3_Pin, GPIO_PIN_RESET); // pin a8 --> out 3
	HAL_GPIO_WritePin(OUT_4_GPIO_Port, OUT_4_Pin, GPIO_PIN_RESET); // pin b8 --> out 4
	HAL_GPIO_WritePin(OUT_5_GPIO_Port, OUT_5_Pin, GPIO_PIN_RESET); // pin b9 --> out 5
	HAL_GPIO_WritePin(OUT_6_GPIO_Port, OUT_6_Pin, GPIO_PIN_RESET); // pin b12 --> out 6
	current_out_port = 0;
}

void set_output_to(uint8_t pin){
	if(current_out_port == pin){
		return;
	}

	reset_all_pins();
	switch(pin){
	case 1:
		HAL_GPIO_WritePin(OUT_1_GPIO_Port, OUT_1_Pin, GPIO_PIN_SET); // pin b0 --> out 1
		current_out_port = 1;
		//		HAL_UART_Transmit(&huart1, (uint8_t*)"SET OUTPUT TO PORT1\r\n", 21,HAL_MAX_DELAY);
		break;
	case 2:
		HAL_GPIO_WritePin(OUT_2_GPIO_Port, OUT_2_Pin, GPIO_PIN_SET); // pin b1 --> out 2
		current_out_port = 2;
		//		HAL_UART_Transmit(&huart1, (uint8_t*)"SET OUTPUT TO PORT2\r\n", 21,HAL_MAX_DELAY);
		break;
	case 3:
		HAL_GPIO_WritePin(OUT_3_GPIO_Port, OUT_3_Pin, GPIO_PIN_SET); // pin a8 --> out 3
		current_out_port = 3;
		//		HAL_UART_Transmit(&huart1, (uint8_t*)"SET OUTPUT TO PORT3\r\n", 21,HAL_MAX_DELAY);
		break;
	case 4:
		HAL_GPIO_WritePin(OUT_4_GPIO_Port, OUT_4_Pin, GPIO_PIN_SET); // pin b8 --> out 4
		HAL_GPIO_WritePin(OUT_6_GPIO_Port, OUT_6_Pin, GPIO_PIN_SET); // pin b12 --> out 6
		current_out_port = 4;
		//		HAL_UART_Transmit(&huart1, (uint8_t*)"SET OUTPUT TO PORT4\r\n", 21,HAL_MAX_DELAY);
		break;
	case 5:
		HAL_GPIO_WritePin(OUT_5_GPIO_Port, OUT_5_Pin, GPIO_PIN_SET); // pin b9 --> out 5
		current_out_port = 5;
		//		HAL_UART_Transmit(&huart1, (uint8_t*)"SET OUTPUT TO PORT5\r\n", 21,HAL_MAX_DELAY);
		break;
	default:
		reset_all_pins();
		current_out_port = 0;
		break;
	}
//	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
//	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
}

void do_operation(){
	set_output_to(pressed_button);
	if(current_out_port != 0){
		consume_credit = true;
	}else{
		consume_credit = false;
	}
}

uint8_t read_button(){
	if(HAL_GPIO_ReadPin(FRONT_SW_1_GPIO_Port, FRONT_SW_1_Pin)){
		HAL_Delay(SW_DEBOUNCE_TIME);
		while(HAL_GPIO_ReadPin(FRONT_SW_1_GPIO_Port, FRONT_SW_1_Pin));
		HAL_UART_Transmit(&huart1, (uint8_t*)"pressed button1!\r\n", 18,HAL_MAX_DELAY);
		return 1;
	}else if(HAL_GPIO_ReadPin(FRONT_SW_2_GPIO_Port, FRONT_SW_2_Pin)){
		HAL_Delay(SW_DEBOUNCE_TIME);
		while(HAL_GPIO_ReadPin(FRONT_SW_2_GPIO_Port, FRONT_SW_2_Pin));
		HAL_UART_Transmit(&huart1, (uint8_t*)"pressed button2!\r\n", 18,HAL_MAX_DELAY);
		return 2;
	}else if(HAL_GPIO_ReadPin(FRONT_SW_3_GPIO_Port, FRONT_SW_3_Pin)){
		HAL_Delay(SW_DEBOUNCE_TIME);
		while(HAL_GPIO_ReadPin(FRONT_SW_3_GPIO_Port, FRONT_SW_3_Pin));
		HAL_UART_Transmit(&huart1, (uint8_t*)"pressed button3!\r\n", 18,HAL_MAX_DELAY);
		return 3;
	}else if(HAL_GPIO_ReadPin(FRONT_SW_4_GPIO_Port, FRONT_SW_4_Pin)){
		HAL_Delay(SW_DEBOUNCE_TIME);
		while(HAL_GPIO_ReadPin(FRONT_SW_4_GPIO_Port, FRONT_SW_4_Pin));
		HAL_UART_Transmit(&huart1, (uint8_t*)"pressed button4!\r\n", 18,HAL_MAX_DELAY);
		return 4;
	}else if(HAL_GPIO_ReadPin(FRONT_SW_5_GPIO_Port, FRONT_SW_5_Pin)){
		HAL_Delay(SW_DEBOUNCE_TIME);
		front_button_reset_credit_press = true;
		while(HAL_GPIO_ReadPin(FRONT_SW_5_GPIO_Port, FRONT_SW_5_Pin)){
			if(front_button_reset_credit_counter >= 60){
				reset_all_state();
				return 0;
			}
		}
		front_button_reset_credit_press = false;
		HAL_UART_Transmit(&huart1, (uint8_t*)"pressed button5!\r\n", 18,HAL_MAX_DELAY);
		return 5;
	}else if(HAL_GPIO_ReadPin(CREDIT_RESET_GPIO_Port, CREDIT_RESET_Pin)){
		HAL_Delay(SW_DEBOUNCE_TIME);
		while(HAL_GPIO_ReadPin(CREDIT_RESET_GPIO_Port, CREDIT_RESET_Pin));
		HAL_UART_Transmit(&huart1, (uint8_t*)"pressed button credit reset!\r\n", 30,HAL_MAX_DELAY);
		return 6;
	}else if(HAL_GPIO_ReadPin(MODE_SW_GPIO_Port, MODE_SW_Pin)){
		HAL_Delay(SW_DEBOUNCE_TIME);
		while(HAL_GPIO_ReadPin(MODE_SW_GPIO_Port, MODE_SW_Pin));
		HAL_UART_Transmit(&huart1, (uint8_t*)"pressed button mode!\r\n", 22,HAL_MAX_DELAY);
		return 7;
	}else{
		return 0;
	}
}

void segment_display_int(int number){
	max7219_Turn_Off();
	max7219_Init(5);
	max7219_Clean ();
	max7219_Decode_On();
	max7219_Clean ();
	if(number < 10){
		max7219_PrintItos (DIGIT_1, number );
	}else if(number >= 10 && number < 100){
		max7219_PrintItos (DIGIT_2, number );
	}else{
		max7219_PrintItos (DIGIT_3, number );
	}
}

void segment_display_off(){
	max7219_Clean ();
	max7219_Decode_On();
	max7219_Clean ();
	max7219_PrintDigit(DIGIT_1,BLANK,false);
	max7219_PrintDigit(DIGIT_2,BLANK,false);
	max7219_PrintDigit(DIGIT_3,BLANK,false);
}

void segment_display_standby(){
	max7219_Clean ();
	max7219_Decode_On();
	max7219_Clean ();

	switch(standby_counter){
	case 0:
		max7219_PrintDigit(DIGIT_1,MINUS,false);
		max7219_PrintDigit(DIGIT_2,BLANK,false);
		max7219_PrintDigit(DIGIT_3,BLANK,false);
		break;
	case 1:
		max7219_PrintDigit(DIGIT_1,BLANK,false);
		max7219_PrintDigit(DIGIT_2,MINUS,false);
		max7219_PrintDigit(DIGIT_3,BLANK,false);
		break;
	case 2:
		max7219_PrintDigit(DIGIT_1,BLANK,false);
		max7219_PrintDigit(DIGIT_2,BLANK,false);
		max7219_PrintDigit(DIGIT_3,MINUS,false);
		break;
	case 3:
		max7219_PrintDigit(DIGIT_1,BLANK,false);
		max7219_PrintDigit(DIGIT_2,MINUS,false);
		max7219_PrintDigit(DIGIT_3,BLANK,false);
		break;
	default:
		max7219_PrintDigit(DIGIT_1,MINUS,false);
		max7219_PrintDigit(DIGIT_2,MINUS,false);
		max7219_PrintDigit(DIGIT_3,MINUS,false);
		break;
	}
	if(standby_counter >= 3){
		standby_counter = 0;
	}else{
		standby_counter += 1;
	}

}

void add_bank_note_credit(){
	if (true) {
		char tmp_msg[40];
		sprintf(tmp_msg, "added credit : %d \r\n",
				bank_credit_per_pulse);
		HAL_UART_Transmit(&huart1, (uint8_t*)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);
		credit += bank_credit_per_pulse;
		iot_round_counter = 0;
		last_note_money += bank_credit_per_pulse;
		if(credit >= 999){
			credit = 999;
		}
		store_credit_eeprom(credit);
		max7219_Turn_On();
		segment_display_int(credit);
		is_standby = false;
		if (credit >= minimum_credit_to_start) {
			is_operation_running = true;
			HAL_UART_Transmit(&huart1, (uint8_t*)"front btn is now enable\r\n", 25,
					HAL_MAX_DELAY);
		}
		char tmp_msg2[35];
		sprintf(tmp_msg2, "current credit : %d \r\n", (int)credit);
		HAL_UART_Transmit(&huart1, (uint8_t*)tmp_msg2, strlen(tmp_msg2),
				HAL_MAX_DELAY);
	}
}

void add_coin_credit() {
	if (true) {
		char tmp_msg[40];
		sprintf(tmp_msg, "added credit : %d \r\n",
				(int)coin_credit_per_pulse);
		HAL_UART_Transmit(&huart1, (uint8_t*)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);
		credit += coin_credit_per_pulse;
		iot_round_counter = 0;
		last_coin_money += coin_credit_per_pulse;
		if(credit >= 999){
			credit = 999;
		}
		store_credit_eeprom(credit);
		is_standby = false;
		max7219_Turn_On();
		segment_display_int(credit);

		if (credit >= minimum_credit_to_start) {
			is_operation_running = true;
			HAL_UART_Transmit(&huart1, (uint8_t*)"front btn is now enable\r\n", 25,
					HAL_MAX_DELAY);
		}
	}
}

uint16_t read_credit_eeprom(){
	uint8_t hbits = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR6);
	uint8_t lbits = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR7);
	u_int16_t converted_value = (lbits << 8) | hbits;
	return converted_value;
}
void read_settings_from_eeprom(void){
	F1_DURATION = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
	F2_DURATION = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
	F3_DURATION = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
	F4_DURATION = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);
	F5_DURATION = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR5);
	credit = read_credit_eeprom();
	//	if(HAL_GPIO_ReadPin(USER_SEL_GPIO_Port, USER_SEL_Pin) == GPIO_PIN_SET){
	//		credit = 999;
	//	}
	if(credit >= 2){
		is_operation_running = true;
		is_standby = false;
		consume_credit = false;
		credit_consume_counter = 0;

	}else{
		reset_all_state();
	}
	char tmp_msg[35];
	sprintf(tmp_msg,"eeprom read DURATION 1 is %d \r\n",F1_DURATION);
	HAL_UART_Transmit(&huart1, (uint8_t *)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);
	sprintf(tmp_msg,"eeprom read DURATION 2 is %d \r\n",F2_DURATION);
	HAL_UART_Transmit(&huart1, (uint8_t *)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);
	sprintf(tmp_msg,"eeprom read DURATION 3 is %d \r\n",F3_DURATION);
	HAL_UART_Transmit(&huart1, (uint8_t *)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);
	sprintf(tmp_msg,"eeprom read DURATION 4 is %d \r\n",F4_DURATION);
	HAL_UART_Transmit(&huart1, (uint8_t *)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);
	sprintf(tmp_msg,"eeprom read DURATION 5 is %d \r\n",F5_DURATION);
	HAL_UART_Transmit(&huart1, (uint8_t *)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);
	sprintf(tmp_msg,"eeprom read CREDIT is %d \r\n",(int)credit);
	HAL_UART_Transmit(&huart1, (uint8_t *)tmp_msg, strlen(tmp_msg), HAL_MAX_DELAY);

	if(F1_DURATION == 0 || F2_DURATION == 0 || F3_DURATION == 0 || F4_DURATION == 0 || F5_DURATION == 0){
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, default_credit_duration);
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, default_credit_duration);
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, default_credit_duration);
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, default_credit_duration);
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, default_credit_duration);
		F1_DURATION = default_credit_duration;
		F2_DURATION = default_credit_duration;
		F3_DURATION = default_credit_duration;
		F4_DURATION = default_credit_duration;
		F5_DURATION = default_credit_duration;
	}
	HAL_Delay(300);
}

void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin)
{
	char tmp[80];
	if(GPIO_Pin == COIN_IT_Pin){
		//This block will be triggered after pin activated.
		sprintf(tmp,"\n----------\ntriggered from  COIN_IT_Pin\n----------\r\n");
		HAL_UART_Transmit(&huart1, (uint8_t *)tmp, strlen(tmp), HAL_MAX_DELAY);
		add_coin_credit();
	}
	if(GPIO_Pin == BANK_IT_Pin){
		//This block will be triggered after pin activated.
		sprintf(tmp,"\n----------\ntriggered from  BANK_IT_Pin\n----------\r\n");
		HAL_UART_Transmit(&huart1, (uint8_t *)tmp, strlen(tmp), HAL_MAX_DELAY);
		add_bank_note_credit();
	}
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
