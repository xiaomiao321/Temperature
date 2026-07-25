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
#include "dma.h"
#include "iwdg.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "board_config.h"
#include "hc595.h"
#include "key.h"
#include "display.h"
#include "temp_setting.h"
#include "temperature.h"
#include "report.h"

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
/* UART2 DMA 接收缓冲区 */
#define UART2_BUF_SIZE 64
uint8_t uart2_dma_buf[UART2_BUF_SIZE];  /* DMA 接收缓冲区 */
uint8_t uart2_rx_buf[UART2_BUF_SIZE];   /* 应用接收缓冲区 */
volatile uint16_t uart2_rx_len = 0;     /* 接收到的数据长度 */
volatile uint8_t uart2_frame_ready = 0; /* 帧接收完成标志 */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* UART2 收帧缓冲 uart2_* 定义在上方 PV 段;
 * DMA 句柄定义在 dma.c (dma.h 未导出, 故此处 extern)。 */
extern DMA_HandleTypeDef hdma_usart2_rx;

/* 温度/报警/阈值/串口输出均由各模块提供, 不再在本文件重复 extern 或定义:
 *   temperature[] / temp_valid[] / manual_alarm / temp_alarm_latched -> temperature.h
 *   阈值 -> temp_setting.h 的 TEMP_SETTING_GetValue1/2()
 *   串口 -> report.h 的 Report_Banner() / Report_Temperature() */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  uint32_t lastPrintTick = 0;   /* 上次串口日志打印时刻, 用于 1s 定时打印 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* 检查复位原因 - 看门狗复位标志 */
  if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) {
    /* 是看门狗复位，可以点亮 LED 或发送日志指示 */
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_RESET);

    /* 清除复位标志 */
    __HAL_RCC_CLEAR_RESET_FLAGS();
  }
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */

  /* 初始化按键和数码管 */
  KEY_Init();
  HC595_1_Init(); /* 初始化第一组数码管 */
  HC595_2_Init(); /* 初始化第二组数码管 */
  TEMP_SETTING_Init();  /* 从 Flash 加载阈值 */
  Display_Init();       /* 上电自检 */

  /* 上电初始化继电器状态：正常态 = 全部继电器释放
   * (K1~K3 断开; K4 防火阀释放 = 走开阀线, 阀保持开) */
  HAL_GPIO_WritePin(K1_ALARM_Port, K1_ALARM_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(K2_FAN_Port, K2_FAN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(K3_GAS_Port, K3_GAS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(K4_DAMPER_Port, K4_DAMPER_Pin, GPIO_PIN_RESET);
#if (DAMPER_HAS_LIMIT_SWITCH == 0)
  HAL_GPIO_WritePin(K5_DAMPER2_Port, K5_DAMPER2_Pin, GPIO_PIN_RESET);
#endif

  /* 打印开机 banner (UART1 已初始化, 现场插串口即可确认程序在跑) */
  Report_Banner();

  /* 启动 UART2 DMA 空闲中断接收 */
  huart2.Instance->CR1 |= USART_CR1_IDLEIE; /* 使能 IDLE 中断 */

  /* 启动 DMA 接收 */
  hdma_usart2_rx.Instance->CNDTR = UART2_BUF_SIZE;
  __HAL_DMA_ENABLE(&hdma_usart2_rx);
  HAL_UART_Receive_DMA(&huart2, uart2_dma_buf, UART2_BUF_SIZE);

  /* 启动看门狗 */
  __HAL_IWDG_START(&hiwdg);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* 显示管理: 根据设置模式分流 */
    if (TEMP_SETTING_GetMode() == SET_MODE_EDIT) {
        Display_SetMode(DISP_MODE_SETTING);
        TEMP_SETTING_Handler();
        TEMP_SETTING_Display();
    } else {
        /* 仅在刚从设置模式退出时切回轮询; 轮询↔报警由 Display_CheckAlarm 管理,
         * 不要每轮强制 ROTATION, 否则会反复清零计时器把画面冻结在 CH1/CH8。 */
        if (Display_GetMode() == DISP_MODE_SETTING) {
            Display_SetMode(DISP_MODE_ROTATION);
        }
        TEMP_SETTING_Handler();   /* 仍检测 SET 键进入设置模式 */
        Display_CheckAlarm();     /* 检查 + 可能的模式切换为报警 */
        Display_Handler();        /* 执行 M0/M1 显示 */
    }

    /* 处理警报逻辑 (手动按钮/复位/LED) */
    TEMP_Alarm_Handler();

    /* 处理 UART2 接收到的数据: 只解析更新温度, 打印与收帧解耦 */
    if (uart2_frame_ready) {
      uart2_frame_ready = 0; /* 清除标志 */

      if (uart2_rx_len >= 21) {
        Parse_Temperature_Data(uart2_rx_buf, uart2_rx_len);
      }
    }

    /* 每 1s 定时打印一次调试日志到 UART1
     * (不依赖温度帧: 没接温度模块也照打, 无效通道显示 ERR,
     *  现场插上串口即可确认 MCU 在跑) */
    if (HAL_GetTick() - lastPrintTick >= 1000) {
      lastPrintTick = HAL_GetTick();
      Report_Temperature();
    }

    /* 喂狗 */
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

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
