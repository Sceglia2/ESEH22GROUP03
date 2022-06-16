/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "MAX32664.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define SCL GPIO_PIN_10
#define GPIO_SCL GPIOB
#define SDA GPIO_PIN_3
#define GPIO_SDA GPIOB
#define RESET  GPIO_PIN_2
#define GPIO_RESET	GPIOC
#define MFIO GPIO_PIN_3
#define GPIO_MFIO GPIOC
#define LD2 GPIO_PIN_5
#define GPIO_LD2 GPIOA

MAX32664 max;

char *ox = "Reading oxygen values:\r\n";
char *hr = "Reading heart rate value:\r\n";
char *conf = "Reading confidence:\r\n";
char *state = "state: \r\n";
char *status = "status: \r\n";
char *average_oxy = "average SpO2: \r\n";
char *average_hr = "average heart rate: \r\n";
char *uart_hr = "HEART RATE TO UART: \r\n";

char str[100];

 int avg_hr = 0;
 int avg_oxy = 0;
 int hr_array[60] = {0};
 int oxy_array[60] = {0};
 int len_hr = sizeof(hr_array) / sizeof(int);
 int len_oxy = sizeof(oxy_array) / sizeof(int);

int movingAverage(int *ptr_array, int len, int new_sample){
	// calculate moving average of array
	  for (int i = 0; i <= len-1; i++){
		  ptr_array[i] = ptr_array[i+1]; // shift array one sample
	  }
	  ptr_array[len -1] = new_sample; // and add new_sample to the end of the array

	int sum = 0; // init Sum
	int good_samples = 0; // number of samples that are usable

	for (int i = 0; i <= len-1; i++) {
	  if (ptr_array[i] != 0) {
		  sum = sum + ptr_array[i]; // sum of all usable values
		  good_samples++;
	  }
	}
	if (good_samples == 0) {
	  return 0; // if no good samples return 0
	}
	else {
	  return sum / (good_samples); // else return average of good samples
	}
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
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2); //initalize timer 2
  HAL_TIM_Base_Start_IT(&htim3); //initalize timer 3

  if(begin(&max,&hi2c2, GPIO_RESET,GPIO_MFIO, RESET, MFIO) != OK){
	  char * error = "errore nella begin";
	  HAL_UART_Transmit(&huart2, (uint8_t*) error, strlen(error), HAL_MAX_DELAY);
  }

  if(config_sensor(&max, 0x01) != OK){
	  char * error = "errore nella CONFIG\r\n";
	  	   HAL_UART_Transmit(&huart2, (uint8_t*) error, strlen(error), HAL_MAX_DELAY);
  }
  /* USER CODE END 2 */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// IF timer 2 (calculate moving average)
	if (htim->Instance == TIM2 ){
		HAL_GPIO_TogglePin(GPIO_LD2, LD2);
		read_sensor(&max);
		if (max.algorithm_state != 3){ // if state != 3 finger is not on sensor
			  char *cmd = "Place finger on sensor \r";
			  HAL_UART_Transmit(&huart2, (uint8_t*) cmd, strlen(cmd),HAL_MAX_DELAY);
			  avg_hr = movingAverage(hr_array, len_hr, 0);
			  avg_oxy = movingAverage(oxy_array, len_oxy, 0);
		 }
		 else if (max.confidence < 90) { // don't use values with confidence less than
			  avg_hr = movingAverage(hr_array, len_hr, 0);
			  avg_oxy = movingAverage(oxy_array, len_oxy, 0);
		 }
		 else { // if confidence and state are ok, print to OLED
			  avg_hr = movingAverage(hr_array, len_hr, max.heart_rate);
			  avg_oxy = movingAverage(oxy_array, len_oxy, max.oxygen);

			  //HAL_UART_Transmit(&huart2, (uint8_t*) hr, strlen(hr), HAL_MAX_DELAY);
			  //sprintf(str,"%u\r\n", max.heart_rate);
			  //HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

			  HAL_UART_Transmit(&huart2, (uint8_t*) average_hr, strlen(average_hr),HAL_MAX_DELAY);
			  sprintf(str,"%d\r\n", avg_hr);
			  HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);

			  HAL_UART_Transmit(&huart2, (uint8_t*) average_oxy, strlen(average_oxy),HAL_MAX_DELAY);
			  sprintf(str,"%d\r\n", avg_oxy);
			  HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);
			  if (avg_oxy >= 94 || avg_hr <= 110){
				  ; // print Normal
			  }
			  else if (avg_oxy <= 90){
				  ; // print critical warning
			  }
			  else {
				  ; // print warning if SpO2 levels are below X or HR is below Y
			  }

		  }
	}
	// else if timer 3 (print to UART)
	else if (htim->Instance == TIM3) {
		HAL_UART_Transmit(&huart2, (uint8_t*) uart_hr, strlen(uart_hr), HAL_MAX_DELAY);
		sprintf(str,"%u\r\n", avg_hr);
		HAL_UART_Transmit(&huart2, (uint8_t*) str, strlen(str),HAL_MAX_DELAY);
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
