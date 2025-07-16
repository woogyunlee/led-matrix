/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <stdio.h>
#define ROWS 10
#define COLS 10


/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
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
// ROW control GPIO pins (Active Low)
/*nt16_t PULL_ROW_PINS[ROWS] = {
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4,
    GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_8, GPIO_PIN_9
};*/
#define PULL_ROW_GPIO_Port 				GPIOA			//wg

// Shift Register pins
#define LATCH_PIN       					GPIO_PIN_0	//wg
#define CLK_PIN      						GPIO_PIN_1	//wg
#define SER_RED_Pin    						GPIO_PIN_13	//wg
#define SER_GREEN_Pin  						GPIO_PIN_14	//wg
#define SER_BLUE_Pin   						GPIO_PIN_15	//wg

uint8_t buffer[256];

// Arrow RGB pattern (Red head, green shaft, blue background)
uint8_t redMatrix[ROWS][COLS] = {
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}
};
uint8_t greenMatrix[ROWS][COLS] = {
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}
};
uint8_t blueMatrix[ROWS][COLS] = {  //red real wg
  {0,0,1,0,0,0,0,1,0,0},
  {0,1,1,0,0,0,1,1,0,0},
  {1,1,1,0,0,1,1,1,1,1},
  {1,1,1,0,1,1,1,1,1,0},
  {1,1,1,1,1,1,1,1,0,0},
  {1,1,1,1,1,1,1,0,0,0},
  {1,1,1,1,1,1,0,0,0,0},
  {1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,0},
  {1,1,1,1,1,1,1,1,0,0}
};  

void sendRGB(uint8_t r, uint8_t g, uint8_t b) {
	HAL_GPIO_WritePin(GPIOC, SER_RED_Pin, 		(r & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, SER_GREEN_Pin, 	(g & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, SER_BLUE_Pin, 	(b & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);		
	HAL_GPIO_WritePin(GPIOC, CLK_PIN, GPIO_PIN_SET);		// 시프트 클럭 ↑
	HAL_GPIO_WritePin(GPIOC, CLK_PIN, GPIO_PIN_RESET);		// 시프트 클럭 ↓				
}
void selectRow(uint8_t value){	// 3비트 주소를 74HC238에 설정하는 함수
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, (value & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, (value & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, (value & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, (value & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);	
}
void displayMatrix() {
    for (int row = 0; row < ROWS; row++) {
        selectRow(row);
		  HAL_GPIO_WritePin(GPIOC, LATCH_PIN, GPIO_PIN_RESET);	 // 래치 ↓
        for (int col = 0; col < COLS; col++)
            sendRGB(redMatrix[row][col], greenMatrix[row][col], blueMatrix[row][col]);
		  HAL_GPIO_WritePin(GPIOC, LATCH_PIN, GPIO_PIN_SET);		 // 래치 ↑        
		  //HAL_Delay(10);
    }
	 //HAL_Delay(10);
}
static uint8_t row_num = 0;
static uint8_t rotate = 0;
static uint8_t col_num = 0;

void displayMatrix_386us() {
	  HAL_GPIO_WritePin(GPIOC, LATCH_PIN, GPIO_PIN_RESET);	 // 래치 ↓
     for (int col = 0; col < COLS; col++)
         sendRGB(redMatrix[row_num%ROWS][col], greenMatrix[row_num%ROWS][col], blueMatrix[row_num%ROWS][col]);
	  HAL_GPIO_WritePin(GPIOC, LATCH_PIN, GPIO_PIN_SET);		 // 래치 ↑        
}

/*void clearMatrix() {		

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            redMatrix[r][c] = 0;
            greenMatrix[r][c] = 0;
            blueMatrix[r][c] = 0x01; // Blue background
        }
    }
}*/
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//HAL_UART
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	static int num = 0;
	
	if(htim->Instance == TIM2){		//368 usec
		displayMatrix_386us();
	}
	if(htim->Instance == TIM3){	//1second
		//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
		//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
		sprintf((char *)buffer, "%d \r\n", num++);
		HAL_UART_Transmit(&huart2, buffer, strlen((char *)buffer), 100);
	}
	if(htim->Instance == TIM4){		//368 * 10 usec
		selectRow(row_num%ROWS);
		row_num++;
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
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

	//HAL_StatusTypeDef serial_status;

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
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
//	test_74HC238();
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim4);
	//sprintf((char *)buffer, "Hello, World!\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //HAL_GPIO_WritePin()

  while (1){
//	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
//	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
//	sprintf((char *)buffer, "Hello, World!\r\n");
	//HAL_UART_Transmit(&huart2, buffer, strlen((char *)buffer), 100);
	//HAL_Delay(500);
	//delay(500);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
