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
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hc595.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* 按键定义 */
#define BTN1_PIN        Btn1_Pin      /* SET 键 - 确认 */
#define BTN1_Port       GPIOB
#define BTN2_PIN        Btn2_Pin      /* 加键 */
#define BTN2_Port       GPIOB
#define BTN3_PIN        Btn3_Pin      /* 减键 - PB0 */
#define BTN3_Port       GPIOB
#define BTN4_PIN        Btn4_Pin      /* 左移键 - 切换位数 - PB1 */
#define BTN4_Port       GPIOB

/* 闪烁参数 */
#define BLINK_INTERVAL  300           /* 闪烁间隔 (ms) */
#define DEBOUNCE_DELAY  20            /* 按键消抖延时 (ms) */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* 设置状态枚举 */
typedef enum {
    SET_MODE_NORMAL = 0,    /* 正常显示模式 */
    SET_MODE_EDIT           /* 设置编辑模式 */
} SetMode_t;

/* 设置位置枚举 (0-百位，1-十位，2-个位) */
typedef enum {
    SET_POS_HUNDRED = 0,
    SET_POS_TEN = 1,
    SET_POS_UNIT = 2
} SetPos_t;

/* 全局变量 */
static SetMode_t g_setMode = SET_MODE_NORMAL;     /* 当前模式 */
static SetPos_t g_setPos = SET_POS_UNIT;          /* 当前设置位 */
static uint16_t g_setValue = 0;                    /* 设置值 (0-999) */
static uint8_t g_blinkState = 1;                   /* 闪烁状态 (1-亮，0-灭) */
static uint32_t g_lastBlinkTime = 0;               /* 上次闪烁时间 */
static uint8_t g_btn1LastState = 1;                /* 按键状态记录 */
static uint8_t g_btn2LastState = 1;
static uint8_t g_btn3LastState = 1;
static uint8_t g_btn4LastState = 1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* 按键扫描函数 - 返回 0 表示按下，1 表示释放 */
static uint8_t BTN1_Read(void);
static uint8_t BTN2_Read(void);
static uint8_t BTN3_Read(void);
static uint8_t BTN4_Read(void);

/* 按键处理函数 */
static void BTN1_Handler(void);   /* SET 确认键 */
static void BTN2_Handler(void);   /* 加键 */
static void BTN3_Handler(void);   /* 减键 */
static void BTN4_Handler(void);   /* 左移切换键 */

/* 显示更新函数 */
static void Display_Update(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief  读取按键状态 (低电平有效)
 */
static uint8_t BTN1_Read(void)
{
    return HAL_GPIO_ReadPin(BTN1_Port, BTN1_PIN);
}

static uint8_t BTN2_Read(void)
{
    return HAL_GPIO_ReadPin(BTN2_Port, BTN2_PIN);
}

static uint8_t BTN3_Read(void)
{
    return HAL_GPIO_ReadPin(BTN3_Port, BTN3_PIN);
}

static uint8_t BTN4_Read(void)
{
    return HAL_GPIO_ReadPin(BTN4_Port, BTN4_PIN);
}

/**
 * @brief  BTN1 - SET 确认键处理
 *         正常模式 -> 按下进入设置模式
 *         设置模式 -> 按下确认设置，返回正常模式
 */
static void BTN1_Handler(void)
{
    uint8_t currState = BTN1_Read();
    
    /* 检测按键按下 (下降沿) */
    if(g_btn1LastState == 1 && currState == 0)
    {
        HAL_Delay(DEBOUNCE_DELAY);  /* 消抖 */
        if(BTN1_Read() == 0)
        {
            /* 切换模式 */
            if(g_setMode == SET_MODE_NORMAL)
            {
                /* 进入设置模式，从个位开始 */
                g_setMode = SET_MODE_EDIT;
                g_setPos = SET_POS_UNIT;
                g_blinkState = 1;
                g_lastBlinkTime = HAL_GetTick();
            }
            else
            {
                /* 确认设置，返回正常模式 */
                g_setMode = SET_MODE_NORMAL;
                g_blinkState = 1;  /* 停止闪烁，全亮 */
            }
            
            /* 等待按键释放 */
            while(BTN1_Read() == 0);
            HAL_Delay(DEBOUNCE_DELAY);
        }
    }
    g_btn1LastState = currState;
}

/**
 * @brief  BTN2 - 加键处理
 *         设置模式下有效，当前位数字 +1
 */
static void BTN2_Handler(void)
{
    uint8_t currState = BTN2_Read();
    
    /* 只在设置模式下响应 */
    if(g_setMode != SET_MODE_EDIT)
    {
        g_btn2LastState = currState;
        return;
    }
    
    /* 检测按键按下 (下降沿) */
    if(g_btn2LastState == 1 && currState == 0)
    {
        HAL_Delay(DEBOUNCE_DELAY);  /* 消抖 */
        if(BTN2_Read() == 0)
        {
            /* 根据当前设置位，增加对应位的值 */
            uint8_t digit;
            switch(g_setPos)
            {
                case SET_POS_HUNDRED:
                    digit = (g_setValue / 100) % 10;
                    digit = (digit + 1) % 10;
                    g_setValue = (g_setValue % 100) + digit * 100;
                    break;
                case SET_POS_TEN:
                    digit = (g_setValue / 10) % 10;
                    digit = (digit + 1) % 10;
                    g_setValue = (g_setValue % 10) + digit * 10 + (g_setValue / 100) * 100;
                    break;
                case SET_POS_UNIT:
                    digit = g_setValue % 10;
                    digit = (digit + 1) % 10;
                    g_setValue = (g_setValue / 10) * 10 + digit;
                    break;
            }
            
            /* 等待按键释放 */
            while(BTN2_Read() == 0);
            HAL_Delay(DEBOUNCE_DELAY);
        }
    }
    g_btn2LastState = currState;
}

/**
 * @brief  BTN3 - 减键处理
 *         设置模式下有效，当前位数字 -1
 */
static void BTN3_Handler(void)
{
    uint8_t currState = BTN3_Read();

    /* 只在设置模式下响应 */
    if(g_setMode != SET_MODE_EDIT)
    {
        g_btn3LastState = currState;
        return;
    }

    /* 检测按键按下 (下降沿) */
    if(g_btn3LastState == 1 && currState == 0)
    {
        HAL_Delay(DEBOUNCE_DELAY);  /* 消抖 */
        if(BTN3_Read() == 0)
        {
            /* 翻转 LED 用于调试 - 确认按键被检测到 */
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            
            /* 根据当前设置位，减少对应位的值 */
            uint8_t digit;
            switch(g_setPos)
            {
                case SET_POS_HUNDRED:
                    digit = (g_setValue / 100) % 10;
                    digit = (digit + 9) % 10;  /* -1 等效于 +9 再取模 */
                    g_setValue = (g_setValue % 100) + digit * 100;
                    break;
                case SET_POS_TEN:
                    digit = (g_setValue / 10) % 10;
                    digit = (digit + 9) % 10;
                    g_setValue = (g_setValue % 10) + digit * 10 + (g_setValue / 100) * 100;
                    break;
                case SET_POS_UNIT:
                    digit = g_setValue % 10;
                    digit = (digit + 9) % 10;
                    g_setValue = (g_setValue / 10) * 10 + digit;
                    break;
            }

            /* 等待按键释放 */
            while(BTN3_Read() == 0);
            HAL_Delay(DEBOUNCE_DELAY);
        }
    }
    g_btn3LastState = currState;
}

/**
 * @brief  BTN4 - 左移切换键处理
 *         设置模式下有效，切换设置的位数 (个位->十位->百位->个位 循环)
 */
static void BTN4_Handler(void)
{
    uint8_t currState = BTN4_Read();
    
    /* 只在设置模式下响应 */
    if(g_setMode != SET_MODE_EDIT)
    {
        g_btn4LastState = currState;
        return;
    }
    
    /* 检测按键按下 (下降沿) */
    if(g_btn4LastState == 1 && currState == 0)
    {
        HAL_Delay(DEBOUNCE_DELAY);  /* 消抖 */
        if(BTN4_Read() == 0)
        {
            /* 切换设置位置：个位 -> 十位 -> 百位 -> 个位 循环 */
            g_setPos = (SetPos_t)((g_setPos + 1) % 3);
            
            /* 重置闪烁状态和时间 */
            g_blinkState = 1;
            g_lastBlinkTime = HAL_GetTick();
            
            /* 等待按键释放 */
            while(BTN4_Read() == 0);
            HAL_Delay(DEBOUNCE_DELAY);
        }
    }
    g_btn4LastState = currState;
}

/**
 * @brief  更新显示
 */
static void Display_Update(void)
{
    uint8_t d1, d2, d3;  /* 百位、十位、个位 */
    
    /* 分解数值 */
    d3 = g_setValue % 10;         /* 个位 */
    d2 = (g_setValue / 10) % 10;  /* 十位 */
    d1 = (g_setValue / 100) % 10; /* 百位 */
    
    if(g_setMode == SET_MODE_NORMAL)
    {
        /* 正常模式 - 直接显示 */
        HC595_Display3Digits(d1, d2, d3);
    }
    else
    {
        /* 设置模式 - 带闪烁显示 */
        HC595_Display3DigitsWithBlink(d1, d2, d3, g_setPos, g_blinkState);
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
  /* USER CODE BEGIN 2 */
  HC595_Init();
  
  /* 初始化设置值 */
  g_setValue = 0;
  g_setMode = SET_MODE_NORMAL;
  g_setPos = SET_POS_UNIT;
  g_blinkState = 1;
  g_lastBlinkTime = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* 按键扫描处理 */
    BTN1_Handler();  /* SET 确认键 */
    BTN2_Handler();  /* 加键 */
    BTN3_Handler();  /* 减键 */
    BTN4_Handler();  /* 左移切换键 */
    
    /* 闪烁处理 (仅在设置模式下) */
    if(g_setMode == SET_MODE_EDIT)
    {
        uint32_t currTime = HAL_GetTick();
        if(currTime - g_lastBlinkTime >= BLINK_INTERVAL)
        {
            g_blinkState = !g_blinkState;  /* 切换闪烁状态 */
            g_lastBlinkTime = currTime;
        }
    }
    
    /* 更新显示 */
    Display_Update();
    
    /* 短暂延时 */
    HAL_Delay(10);
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
