/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hc595.h"
#include "key.h"
#include "temp_setting.h"
#include "temperature.h"
#include <stdio.h>

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

#define UART2_BUF_SIZE 64
uint8_t uart2_rx_buf[UART2_BUF_SIZE];
uint16_t uart2_rx_len = 0;

/* 全局温度阈值变量 - 两组独立阈值 */
uint16_t g_temp_threshold1 = 30;  /* 第一组阈值 (CH1-CH4) */
uint16_t g_temp_threshold2 = 30;  /* 第二组阈值 (CH5-CH8) */

/* 外部温度变量声明 */
extern float temperature[8];
extern uint8_t temp_valid[8];

/* 格式化输出温度到 UART1 */
void Print_Temperature(void) {
  char buf[128];
  int len = 0;

  len += sprintf(&buf[len], "Temp: ");
  for (int i = 0; i < 8; i++) {
    if (temp_valid[i]) {
      /* 使用整数方式输出温度，避免浮点 printf 问题 */
      int temp_int = (int)temperature[i];
      int temp_frac = (int)((temperature[i] - temp_int) * 10);
      if (temp_frac < 0)
        temp_frac = -temp_frac;
      len += sprintf(&buf[len], "CH%d=%d.%d ", i + 1, temp_int, temp_frac);
    } else {
      len += sprintf(&buf[len], "CH%d=ERR ", i + 1);
    }
  }
  len += sprintf(&buf[len], "\r\n");

  HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, 100);
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* 初始化按键和数码管 */
  KEY_Init();
  HC595_1_Init();  /* 初始化第一组数码管 */
  HC595_2_Init();  /* 初始化第二组数码管 */
  TEMP_SETTING_Init();

  /* 上电初始化继电器状态：默认正常状态（照明灯开，蜂鸣器关） */
  HAL_GPIO_WritePin(Relay_24_GPIO_Port, Relay_24_Pin, GPIO_PIN_RESET);   /* 24V 蜂鸣器关闭 */
  HAL_GPIO_WritePin(Relay_220_GPIO_Port, Relay_220_Pin, GPIO_PIN_SET);   /* 220V 照明灯开启 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* 处理按键和显示 - 先处理按键，让复位请求优先响应 */
    /* 注意：button_ticks() 已在 SysTick 中断中每 5ms 调用 */
    TEMP_SETTING_Handler();
    TEMP_SETTING_Display();

    /* 更新阈值到全局变量 */
    g_temp_threshold1 = TEMP_SETTING_GetValue1();
    g_temp_threshold2 = TEMP_SETTING_GetValue2();

    /* 从 UART2 读取一帧数据 */
    uart2_rx_len = 0;

    /* 等待第一个字节 (帧起始) */
    if (HAL_UART_Receive(&huart2, &uart2_rx_buf[0], 1, 500) == HAL_OK) {
      uart2_rx_len = 1;

      /* 继续接收剩余字节，使用 3ms 帧间隔超时 */
      while (uart2_rx_len < UART2_BUF_SIZE) {
        if (HAL_UART_Receive(&huart2, &uart2_rx_buf[uart2_rx_len], 1, 3) ==
            HAL_OK) {
          uart2_rx_len++;
        } else {
          /* 超时，认为一帧接收完成 */
          break;
        }
      }

      /* 解析温度数据 */
      if (uart2_rx_len >= 21) {
        if (Parse_Temperature_Data(uart2_rx_buf, uart2_rx_len) == 0) {
          /* 解析成功，打印温度 */
          Print_Temperature();
        }
      }
    }

    /* 短暂延时 */
    HAL_Delay(10);
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
  while (1) {
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
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
