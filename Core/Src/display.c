/*
 * @file     display.c
 * @brief    数码管显示管理模块 - 支持轮询/报警/设置模式
 * @version  v2.0
 */

#include "display.h"
#include "hc595.h"
#include "temperature.h"
#include "temp_setting.h"
#include "iwdg.h"          /* 上电自检期间喂狗, 见 Display_Init */
#include "stm32f1xx_hal.h"

/* ========== 全局变量 ========== */
static DispMode_t g_dispMode = DISP_MODE_ROTATION;   /* 当前显示模式 */
static uint8_t g_rotationChannel = 0;                /* 轮询通道 (0-7) */
static uint32_t g_rotationTimer = 0;                 /* 轮询计时 (ms) */
static uint8_t g_rotationStep = 0;                   /* 轮询步骤 (0=通道号, 1=温度) */

/* 报警显示 */
static uint8_t g_alarmChannels[8] = {0};             /* 各通道报警标志 */
static uint8_t g_alarmDisplayIndex = 0;              /* 多通道报警显示索引 */
static uint32_t g_alarmTimer = 0;                    /* 报警计时 (ms) */
static uint8_t g_alarmStep = 0;                      /* 报警步骤 (0=ALA, 1=通道号, 2=温度) */

/* 外部变量 */
extern float temperature[8];
extern uint8_t temp_valid[8];
extern uint8_t temp_alarm_latched;
extern uint8_t manual_alarm;

/* ========== 辅助函数 ========== */

/**
 * @brief  将温度值转换为段码索引显示数据
 * @param  temp: 温度值
 * @param  d1,d2,d3: 输出的百/十/个位段码索引
 * @param  dotPos: 小数点位置 (0=无, 1=百位后, 2=十位后, 3=个位后)
 * @retval 是否有效 (0=ERR/故障, 1=有效)
 */
static uint8_t TempToDisplay(float temp, uint8_t *d1, uint8_t *d2, uint8_t *d3, uint8_t *dotPos)
{
    int temp_int;

    /* 传感器故障: temp < -400 */
    if (temp < -400.0f)
    {
        *d1 = SEG_idx_E;
        *d2 = SEG_idx_r;
        *d3 = SEG_idx_r;
        *dotPos = 0;
        return 0;
    }

    /* 超高温: >= 1000℃ → HiG */
    if (temp >= 1000.0f)
    {
        *d1 = SEG_idx_H;
        *d2 = SEG_idx_I;
        *d3 = SEG_idx_G;
        *dotPos = 0;
        return 1;
    }

    /* 超低温: <= -100℃ → Lo- */
    if (temp <= -100.0f)
    {
        *d1 = SEG_idx_L;
        *d2 = SEG_idx_o;
        *d3 = SEG_idx_MINUS;
        *dotPos = 0;
        return 1;
    }

    /* 负温度: < 0℃ → -XY */
    if (temp < 0.0f)
    {
        temp = -temp;  /* 取绝对值 */
        temp_int = (int)temp;
        *d1 = SEG_idx_MINUS;
        *d2 = (temp_int / 10) % 10;
        *d3 = temp_int % 10;
        *dotPos = 0;
        return 1;
    }

    /* 正常温度 (0 ~ 999.9) */
    temp_int = (int)temp;

    if (temp_int >= 100)
    {
        /* >=100℃: 只显示整数部分, 无小数点 */
        *d1 = temp_int / 100;
        *d2 = (temp_int / 10) % 10;
        *d3 = temp_int % 10;
        *dotPos = 0;
    }
    else
    {
        /* <100℃: 显示 XY.Z (整数两位 + 小数一位), 小数点在十位后。
         * 注意: 原来这里把 *d1 硬编码成 0, 丢掉了十位 → 28.8 会显示成 08.8。 */
        int temp_frac = (int)((temp - temp_int) * 10);
        *d1 = (temp_int >= 10) ? (uint8_t)((temp_int / 10) % 10) : SEG_idx_OFF;  /* 十位; <10℃ 消隐前导零 */
        *d2 = temp_int % 10;
        *d3 = temp_frac % 10;
        *dotPos = 2;  /* 十位后 */
    }

    return 1;
}

/**
 * @brief  获取报警通道数量
 */
static uint8_t GetAlarmCount(void)
{
    uint8_t count = 0;
    for (int i = 0; i < 8; i++)
    {
        if (g_alarmChannels[i]) count++;
    }
    return count;
}

/**
 * @brief  更新报警通道标志
 */
static void UpdateAlarmChannels(void)
{
    for (int i = 0; i < 8; i++)
    {
        if (!temp_valid[i])
        {
            g_alarmChannels[i] = 0;
            continue;
        }
        /* 分组集中在 TEMP_ThresholdForChannel() (temperature.c) */
        g_alarmChannels[i] = (temperature[i] >= TEMP_ThresholdForChannel(i)) ? 1 : 0;
    }
}

/* ========== 显示处理函数 ========== */

/**
 * @brief  显示初始化 + 上电自检
 */
void Display_Init(void)
{
    /* 自检序列: 888/888 500ms → 灭/灭 200ms → 123/456 500ms → 灭/灭 200ms → 030/030 300ms
     * 注意: 看门狗在 MX_IWDG_Init() 已启动(超时约 1.6s), 而本自检总长 1.7s,
     *       故每步 HAL_Delay 后必须喂狗, 否则会在自检末尾被复位(上电死循环)。 */
    HC595_Test888();
    HAL_Delay(500);
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    HC595_TestAllOff();
    HAL_Delay(200);
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    HC595_1_DisplayNumber(123);
    HC595_2_DisplayNumber(456);
    HAL_Delay(500);
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    HC595_TestAllOff();
    HAL_Delay(200);
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    HC595_1_DisplayNumber(30);
    HC595_2_DisplayNumber(30);
    HAL_Delay(300);
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    /* 初始化状态 */
    g_dispMode = DISP_MODE_ROTATION;
    g_rotationChannel = 0;
    g_rotationStep = 0;
    g_rotationTimer = HAL_GetTick();
    g_alarmDisplayIndex = 0;
    g_alarmTimer = 0;
    g_alarmStep = 0;

    /* 清空报警标志 */
    for (int i = 0; i < 8; i++)
    {
        g_alarmChannels[i] = 0;
    }
}

/**
 * @brief  把某通道温度显示到指定组; 无有效数据(未收到/传感器故障)则显示 Err
 * @param  group: 1=第一组, 2=第二组
 * @param  ch:    通道 0-7
 */
static void ShowTempOrErr(uint8_t group, uint8_t ch)
{
    uint8_t d1, d2, d3, dotPos;

    if (!temp_valid[ch])
    {
        /* 无有效数据: 显示 Err (与串口一致), 不再显示 0 */
        d1 = SEG_idx_E;
        d2 = SEG_idx_r;
        d3 = SEG_idx_r;
        dotPos = 0;
    }
    else
    {
        TempToDisplay(temperature[ch], &d1, &d2, &d3, &dotPos);
    }

    if (group == 1)
        HC595_1_Display3DigitsWithDot(d1, d2, d3, dotPos);
    else
        HC595_2_Display3DigitsWithDot(d1, d2, d3, dotPos);
}

/**
 * @brief  轮询显示处理 (M0)
 */
static void Display_Rotation_Handler(void)
{
    uint32_t current_time = HAL_GetTick();

    /* 每 1500ms 切换一步 */
    if (current_time - g_rotationTimer >= 1500)
    {
        g_rotationTimer = current_time;

        if (g_rotationStep == 0)
        {
            /* 步骤 0 完成, 切换到步骤 1 (温度显示) */
            g_rotationStep = 1;
        }
        else
        {
            /* 步骤 1 完成, 切换到下一通道步骤 0 (通道号显示) */
            g_rotationStep = 0;
            g_rotationChannel = (g_rotationChannel + 1) % 8;
        }
    }

    if (g_rotationStep == 0)
    {
        /* 显示通道号: 第一组当前通道, 第二组固定 CH8 */
        HC595_1_DisplayChannel(g_rotationChannel + 1);
        HC595_2_DisplayChannel(8);
    }
    else
    {
        /* 显示温度(无效则 Err): 第一组当前通道, 第二组固定 CH8 */
        ShowTempOrErr(1, g_rotationChannel);
        ShowTempOrErr(2, 7);
    }
}

/**
 * @brief  报警显示处理 (M1)
 */
static void Display_Alarm_Handler(void)
{
    uint32_t current_time = HAL_GetTick();
    uint8_t alarm_count;
    uint8_t current_alarm_ch;

    /* 更新报警通道标志 */
    UpdateAlarmChannels();

    /* 检查是否还有报警 */
    alarm_count = GetAlarmCount();
    if (alarm_count == 0 && !manual_alarm)
    {
        /* 无报警, 返回轮询模式 */
        g_dispMode = DISP_MODE_ROTATION;
        return;
    }

    /* 每 1000ms 切换一步 (0=ALA, 1=通道号, 2=温度) */
    if (current_time - g_alarmTimer >= 1000)
    {
        g_alarmTimer = current_time;
        g_alarmStep = (g_alarmStep + 1) % 3;

        /* 步骤 2 完成后切换到下一个报警通道 */
        if (g_alarmStep == 0)
        {
            /* 找到下一个报警通道 */
            uint8_t found = 0;
            for (int i = 0; i < 8; i++)
            {
                g_alarmDisplayIndex = (g_alarmDisplayIndex + 1) % 8;
                if (g_alarmChannels[g_alarmDisplayIndex])
                {
                    found = 1;
                    break;
                }
            }

            /* 无温度报警通道时 (仅手动报警), 保持 CH8 */
            if (!found)
            {
                g_alarmDisplayIndex = 7;  /* 默认 CH8 */
            }
        }
    }

    /* 获取当前显示的报警通道 */
    current_alarm_ch = g_alarmDisplayIndex;

    switch (g_alarmStep)
    {
    case 0:
        /* 报警标识 ALA */
        HC595_1_Display3Digits(SEG_idx_A, SEG_idx_L, SEG_idx_A);
        HC595_2_DisplayChannel(8);
        break;

    case 1:
        /* 通道号 (手动报警时显示 MAN) */
        if (manual_alarm && alarm_count == 0)
        {
            HC595_1_Display3Digits(SEG_idx_M, SEG_idx_A, SEG_idx_N);
        }
        else
        {
            HC595_1_DisplayChannel(current_alarm_ch + 1);
        }
        HC595_2_DisplayChannel(8);
        break;

    case 2:
        /* 报警通道温度(无效则 Err); 第二组固定 CH8 */
        ShowTempOrErr(1, current_alarm_ch);
        ShowTempOrErr(2, 7);
        break;
    }
}

/**
 * @brief  显示主处理函数 (主循环调用)
 */
void Display_Handler(void)
{
    if (g_dispMode == DISP_MODE_ROTATION)
    {
        Display_Rotation_Handler();
    }
    else if (g_dispMode == DISP_MODE_ALARM)
    {
        Display_Alarm_Handler();
    }
    /* DISP_MODE_SETTING 由 temp_setting 模块处理 */
}

/**
 * @brief  设置显示模式
 */
void Display_SetMode(DispMode_t mode)
{
    /* 幂等: 主循环每轮都会调用本函数; 若模式未变仍复位状态,
     * 会把轮询/报警计时器反复清零, 导致画面冻结(卡在 CH1/CH8 或 ALA)。 */
    if (g_dispMode == mode)
    {
        return;
    }

    g_dispMode = mode;

    if (mode == DISP_MODE_ROTATION)
    {
        g_rotationChannel = 0;
        g_rotationStep = 0;
        g_rotationTimer = HAL_GetTick();
    }
    else if (mode == DISP_MODE_ALARM)
    {
        g_alarmTimer = HAL_GetTick();
        g_alarmStep = 0;
    }
}

/**
 * @brief  获取当前显示模式
 */
DispMode_t Display_GetMode(void)
{
    return g_dispMode;
}

/**
 * @brief  通知按键按下 (用于退出设置模式计时)
 */
void Display_NotifyKeyPress(void)
{
    /* 预留: 可扩展自动退出设置模式计时 */
}

/**
 * @brief  检查是否需要进入报警模式
 */
void Display_CheckAlarm(void)
{
    UpdateAlarmChannels();

    if (GetAlarmCount() > 0 || manual_alarm)
    {
        /* 非设置模式才可进入报警模式 */
        if (g_dispMode != DISP_MODE_SETTING && g_dispMode != DISP_MODE_ALARM)
        {
            g_dispMode = DISP_MODE_ALARM;
            g_alarmTimer = HAL_GetTick();
            g_alarmStep = 0;
        }
    }
    else
    {
        /* 报警解除, 返回轮询模式 */
        if (g_dispMode == DISP_MODE_ALARM)
        {
            g_dispMode = DISP_MODE_ROTATION;
            g_rotationChannel = 0;
            g_rotationStep = 0;
            g_rotationTimer = HAL_GetTick();
        }
    }
}
