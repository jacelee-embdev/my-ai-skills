/******************************************************************************
 * Copyright (C) 2026 JesMicro.
 *
 * All Rights Reserved.
 *
 * @file bsp_key.h
 *
 * @par dependencies
 * - stdint.h
 * - FreeRTOS.h
 * - queue.h
 * - cmsis_os.h
 *
 * @author Jace | Development Dept. | JesMicro
 *
 * @brief Declares key states, events, shared RTOS objects, and BSP APIs.
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

#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

/****************************** Includes ******************************/

#include <stdint.h>    /* C 标准库 */

#include "FreeRTOS.h"  /* 第三方中间件层：FreeRTOS / CMSIS-RTOS */
#include "queue.h"
#include "cmsis_os.h"

/****************************** Includes ******************************/

/****************************** Defines ******************************/

/* 按键扫描结果 */
typedef enum
{
    KEY_SCAN_OK                   = 0,          /* 扫描成功 */
    KEY_SCAN_ERROR                = 1,          /* 一般错误 */
    KEY_SCAN_TIMEOUT              = 2,          /* 扫描超时 */
    KEY_SCAN_RESOURCE_UNAVAILABLE = 3,          /* 资源不可用 */
    KEY_SCAN_INVALID_PARAMETER    = 4,          /* 参数无效 */
    KEY_SCAN_OUT_OF_MEMORY        = 5,          /* 内存不足 */
    KEY_SCAN_ISR_NOT_ALLOWED      = 6,          /* 不允许在中断中调用 */
    KEY_SCAN_RESERVED             = 0x7FFFFFFF  /* 保留值 */
} key_scan_result_t;

/* 按键物理状态 */
typedef enum
{
    KEY_STATE_PRESSED  = 0,  /* 按键按下 */
    KEY_STATE_RELEASED = 1,  /* 按键释放 */
} key_state_t;

/* 按键事件检测结果 */
typedef enum
{
    KEY_DETECT_OK                = 0,  /* 检测成功 */
    KEY_DETECT_ERROR             = 1,  /* 检测错误 */
    KEY_DETECT_TIMEOUT           = 2,  /* 检测超时 */
    KEY_DETECT_INVALID_PARAMETER = 3,  /* 参数无效 */
} key_detect_result_t;

/* 按键事件类型 */
typedef enum
{
    KEY_EVENT_NONE        = 0,  /* 无按键事件 */
    KEY_EVENT_SHORT_PRESS = 1,  /* 短按事件 */
    KEY_EVENT_LONG_PRESS  = 2,  /* 长按事件 */
} key_event_t;

/****************************** Defines ******************************/

/****************************** Declaring ******************************/

extern osThreadId_t g_key_task_handle;             /* 按键任务句柄 */
extern const osThreadAttr_t g_key_task_attributes; /* 按键任务属性 */
extern QueueHandle_t g_key_queue;                  /* 按键事件队列句柄 */

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
key_scan_result_t Key_Scan(key_state_t *p_key_state);

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
                                  uint32_t long_press_threshold_ms);

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
void Key_TaskEntry(void *p_argument);

/****************************** Declaring ******************************/

#endif /* __BSP_KEY_H__ */
