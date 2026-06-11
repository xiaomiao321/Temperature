/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "multi_button.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* MultiButton 5ms 定时器计数器 */
static uint8_t button_tick_divider = 0;

/* UART2 接收变量 */
extern uint8_t uart2_dma_buf[];
extern uint8_t uart2_rx_buf[];
extern volatile uint16_t uart2_rx_len;
extern volatile uint8_t uart2_frame_ready;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;

#define UART2_BUF_SIZE  64
#define UART2_FRAME_SIZE  21  /* 1 帧 Modbus 数据最小长度 */

/* 接收状态 */
static volatile uint8_t dma_rx_active = 0;  /* DMA 是否正在接收 */
static volatile uint16_t rx_count = 0;  /* 成功接收帧计数（调试用） */
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_usart2_rx;
extern UART_HandleTypeDef huart2;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
  /* MultiButton 5ms 定时器 - 每 5ms 调用一次 button_ticks() */
  if (++button_tick_divider >= 5)
  {
    button_tick_divider = 0;
    button_ticks();
  }
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel6 global interrupt.
  */
void DMA1_Channel6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel6_IRQn 0 */

  /* USER CODE END DMA1_Channel6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
  /* USER CODE BEGIN DMA1_Channel6_IRQn 1 */

  /* USER CODE END DMA1_Channel6_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
  uint32_t isr_flag = __HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE);
  
  if (isr_flag)
  {
    /* 清除 IDLE 标志：先读 SR 再读 DR */
    volatile uint32_t tmp = huart2.Instance->SR;
    tmp = huart2.Instance->DR;
    (void)tmp;
    
    /* 关闭 DMA 接收 */
    __HAL_DMA_DISABLE(&hdma_usart2_rx);
    
    /* 计算接收到的数据长度 */
    uint16_t rx_len = UART2_BUF_SIZE - hdma_usart2_rx.Instance->CNDTR;
    
    /* 复制数据到应用缓冲区 */
    if (rx_len >= UART2_FRAME_SIZE)
    {
      uart2_rx_len = rx_len;
      for (uint16_t i = 0; i < rx_len; i++)
      {
        uart2_rx_buf[i] = uart2_dma_buf[i];
      }
      uart2_frame_ready = 1;
      rx_count++;
    }
    
    /* 重启 DMA 接收 - 不清空缓冲区，新数据会覆盖旧数据 */
    hdma_usart2_rx.Instance->CNDTR = UART2_BUF_SIZE;
    __HAL_DMA_ENABLE(&hdma_usart2_rx);
    
    return;
  }
  
  /* 处理 DMA 传输完成中断（收到 64 字节） */
  if (__HAL_DMA_GET_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_usart2_rx)))
  {
    __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_usart2_rx));
    
    /* 复制全部 64 字节 */
    uart2_rx_len = UART2_BUF_SIZE;
    for (uint16_t i = 0; i < UART2_BUF_SIZE; i++)
    {
      uart2_rx_buf[i] = uart2_dma_buf[i];
    }
    uart2_frame_ready = 1;
    rx_count++;
    
    /* 重启 DMA */
    hdma_usart2_rx.Instance->CNDTR = UART2_BUF_SIZE;
    __HAL_DMA_ENABLE(&hdma_usart2_rx);
    
    return;
  }
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
