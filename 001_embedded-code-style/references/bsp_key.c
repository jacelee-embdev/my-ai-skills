/******************************************************************************
 * Copyright (C) 2026 JesMicro.
 *
 * All Rights Reserved.
 *
 * @file bsp_key.c
 *
 * @par dependencies
 * - bsp_key.h
 * - stdio.h
 * - gpio.h
 * - task.h
 *
 * @author Jace | Development Dept. | JesMicro
 *
 * @brief Implements key debouncing, press detection, and RTOS event delivery.
 *
 * Processing flow:
 *
 * Call Key_Scan() and Key_DetectEvent() from task context,
 * or create a thread with Key_TaskEntry() and g_key_task_attributes.
 * Receive key events from g_key_queue after the queue has been created.
 *
 * @version V1.0 2026-10-10
 *
 * @note 1 tab == 4 spaces!
 *
 ******************************************************************************/

/****************************** Includes ******************************/

#include "bsp_key.h"  /* 用户 BSP 层：按键模块接口 */

#include <stdio.h>    /* C 标准库 */

#include "gpio.h"     /* 用户 Core 层：GPIO 配置接口（基于 HAL） */

#include "task.h"     /* 中间件层：FreeRTOS 任务接口 */

/****************************** Includes ******************************/

/************************ Variable Definitions ************************/

osThreadId_t g_key_task_handle = NULL;  /* 按键任务句柄 */

/* 按键任务属性 */
const osThreadAttr_t g_key_task_attributes =
{
    .name       = "key_task",                    /* 任务名称 */
    .stack_size = 128 * 4,                       /* 栈大小，单位为字节 */
    .priority   = (osPriority_t)osPriorityNormal, /* 普通任务优先级 */
};

QueueHandle_t g_key_queue = NULL;  /* 按键事件队列句柄 */

/************************ Variable Definitions ************************/

/************************ Function Definitions ************************/

/**
 * @brief 扫描按键输入并输出消抖后的物理状态。
 *
 * 处理步骤：
 * 1. 检查输出指针是否有效。
 * 2. 读取按键引脚，延时 10 个系统节拍后再次读取。
 * 3. 两次均为低电平时输出按下状态，否则输出释放状态。
 *
 * @param[out] p_key_state : 接收按键状态的指针，不得为空。
 *
 * @return key_scan_result_t : 按键扫描结果。
 * @retval KEY_SCAN_OK                扫描成功。
 * @retval KEY_SCAN_INVALID_PARAMETER 输出指针为空。
 *
 * @note 该函数包含任务延时，应在任务上下文中调用。
 */
key_scan_result_t Key_Scan(key_state_t *p_key_state)
{
    GPIO_PinState first_pin_state  = GPIO_PIN_SET;       /* 首次采样电平 */
    GPIO_PinState second_pin_state = GPIO_PIN_SET;       /* 再次采样电平 */
    key_state_t key_state          = KEY_STATE_RELEASED; /* 按键物理状态 */

    /* 1. 检查输出指针。 */
    if (NULL == p_key_state)
    {
        return KEY_SCAN_INVALID_PARAMETER;
    }

    /* 2. 读取按键引脚，延时消抖后再次采样。 */
    first_pin_state = HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin);
    vTaskDelay(10);
    second_pin_state = HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin);

    /* 3. 两次采样均为低电平时判定为按下。 */
    if ((GPIO_PIN_RESET == first_pin_state) &&
        (GPIO_PIN_RESET == second_pin_state))
    {
        key_state = KEY_STATE_PRESSED;
        *p_key_state = key_state;
    }

    *p_key_state = key_state;
    return KEY_SCAN_OK;
}

/**
 * @brief 检测按键并区分短按和长按事件。
 *
 * 处理步骤：
 * 1. 检查参数并将输出事件初始化为无事件。
 * 2. 扫描按键；未按下时直接返回。
 * 3. 忙等至长按阈值后再次扫描，已释放时判定为短按。
 * 4. 仍按下时判定为长按，并等待释放以避免重复上报。
 *
 * @param[out] p_key_event            : 接收按键事件的指针，不得为空。
 * @param[in] long_press_threshold_ms : 长按阈值，单位为毫秒，须大于零。
 *
 * @return key_detect_result_t : 按键事件检测结果。
 * @retval KEY_DETECT_OK                检测成功。
 * @retval KEY_DETECT_ERROR             底层按键扫描失败。
 * @retval KEY_DETECT_INVALID_PARAMETER 输入参数无效。
 *
 * @note 该函数包含任务延时，应在任务上下文中调用。
 */
key_detect_result_t Key_DetectEvent(key_event_t *p_key_event,
                                  uint32_t long_press_threshold_ms)
{
    key_scan_result_t key_scan_result = KEY_SCAN_ERROR;    /* 扫描结果 */
    key_state_t key_state            = KEY_STATE_RELEASED; /* 按键状态 */
    uint32_t start_tick              = 0;                 /* 起始毫秒计数 */

    /* 1. 检查输入参数并初始化输出事件。 */
    if ((NULL == p_key_event) || (0 == long_press_threshold_ms))
    {
        return KEY_DETECT_INVALID_PARAMETER;
    }
    *p_key_event = KEY_EVENT_NONE;

    /* 2. 扫描按键，未按下时直接返回。 */
    key_scan_result = Key_Scan(&key_state);
    if (KEY_SCAN_OK != key_scan_result)
    {
        return KEY_DETECT_ERROR;
    }

    if (KEY_STATE_PRESSED != key_state)
    {
        return KEY_DETECT_OK;
    }

    /* 3. 等待长按阈值到达后，再次扫描按键。 */
    start_tick = HAL_GetTick();
    while ((HAL_GetTick() - start_tick) < long_press_threshold_ms)
    {
        /* 忙等至长按阈值到达。 */
    }

    key_scan_result = Key_Scan(&key_state);
    if (KEY_SCAN_OK != key_scan_result)
    {
        return KEY_DETECT_ERROR;
    }

    if (KEY_STATE_PRESSED != key_state)
    {
        /* 3.1 阈值到达时已释放，判定为短按。 */
        *p_key_event = KEY_EVENT_SHORT_PRESS;
        printf("short press\r\n");
        return KEY_DETECT_OK;
    }
    else
    {
        /* 4. 阈值到达时仍按下，判定为长按。 */
        *p_key_event = KEY_EVENT_LONG_PRESS;

        /* 4.1 等待按键释放，避免重复上报长按事件。 */
        do
        {
            key_scan_result = Key_Scan(&key_state);
            if (KEY_SCAN_OK != key_scan_result)
            {
                return KEY_DETECT_ERROR;
            }
        }
        while (KEY_STATE_PRESSED == key_state);

        printf("long press\r\n");
        return KEY_DETECT_OK;
    }
}

/**
 * @brief 按键事件检测任务的入口函数。
 *
 * 处理步骤：
 * 1. 创建可容纳 10 个按键事件的队列。
 * 2. 使用 1000 毫秒长按阈值检测按键事件。
 * 3. 将短按和长按事件以零等待方式发送到队列，不发送无事件。
 * 4. 每轮检测结束后延时 10 个系统节拍。
 *
 * @param[in] p_argument : 任务参数指针，本任务不使用该参数，允许为空。
 *
 * @return 无返回值；任务持续运行，不返回调用方。
 *
 * @note 队列创建失败时，输出错误日志并持续循环延时。
 */
void Key_TaskEntry(void *p_argument)
{
    key_detect_result_t key_detect_result = KEY_DETECT_ERROR; /* 检测结果 */
    key_event_t key_event                = KEY_EVENT_NONE;   /* 按键事件 */

    /* 1. 创建按键事件队列并检查句柄。 */
    g_key_queue = xQueueCreate(10, sizeof(key_event_t));
    if (NULL == g_key_queue)
    {
        printf("[E][KEY] g_key_queue creation failed.\r\n");

        for (;;)
        {
            vTaskDelay(1000);
        }
    }

    for (;;)
    {
        /* 2. 检测按键事件并检查检测结果。 */
        key_detect_result = Key_DetectEvent(&key_event, 1000);

        switch (key_detect_result)
        {
            case KEY_DETECT_OK:
            {
                /* 3. 仅转发短按和长按事件。 */
                if ((KEY_EVENT_SHORT_PRESS == key_event) ||
                    (KEY_EVENT_LONG_PRESS == key_event))
                {
                    /* 3.1 以零等待方式发送事件，失败时输出日志。 */
                    if (pdPASS != xQueueSend(g_key_queue, &key_event, 0))
                    {
                        printf("[E][KEY] Failed to send key event.\r\n");
                    }
                }
                break;
            }

            case KEY_DETECT_ERROR:
            {
                /* 3.2 输出按键检测失败日志。 */
                printf("[E][KEY] Key detection failed, result=%d\r\n",
                       (int)key_detect_result);
                break;
            }

            default:
            {
                /* 3.3 输出未预期的检测结果。 */
                printf("[E][KEY] Unexpected detection result=%d\r\n",
                       (int)key_detect_result);
                break;
            }
        }

        /* 4. 延时 10 个系统节拍后进行下一轮检测。 */
        osDelay(10);
    }
}

/************************ Function Definitions ************************/
