/******************************************************************************
 * Copyright (C) 2026 JesMicro.
 *
 * All Rights Reserved.
 *
 * @file bsp_key.c
 *
 * @par dependencies
 * - bsp_key.h
 *
 * @author Jace | Development Dept. | JesMicro
 *
 * @brief Provides BSP APIs for key scanning and related RTOS operations.
 *
 * Processing flow:
 *
 * Call key_scan() and key_detect_event() from task context,
 * or run key_task_entry() as an RTOS thread.
 *
 * @version V1.1 2026-07-26
 *
 * @note 1 tab == 4 spaces!
 *
 ******************************************************************************/

#include "bsp_key.h"

/************************ Thread Definitions ************************/
osThreadId_t key_task_handle;
const osThreadAttr_t key_task_attributes = {
    .name = "key_task",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

/************************ Queue Handles ************************/
QueueHandle_t g_key_queue;

/**
 * @brief Scans the key input and reports its debounced state.
 *
 * Samples the active-low key GPIO twice with a 10 ms debounce delay
 * between samples. If both samples are GPIO_PIN_RESET, the output state
 * is set to KEY_STATE_PRESSED. Otherwise, it is set to KEY_STATE_RELEASED.
 *
 * @param[out] p_key_state Pointer to the variable that receives the key state.
 *                         This pointer must not be NULL.
 *
 * @return key_scan_result_t Result of the key scan.
 * @retval KEY_SCAN_OK                The key state was read successfully.
 * @retval KEY_SCAN_INVALID_PARAMETER The output pointer is NULL.
 */
key_scan_result_t key_scan(key_state_t *p_key_state)
{
    GPIO_PinState first_pin_state  = GPIO_PIN_SET;
    GPIO_PinState second_pin_state = GPIO_PIN_SET;
    key_state_t key_state          = KEY_STATE_RELEASED;

    if (NULL == p_key_state)
    {
        return KEY_SCAN_INVALID_PARAMETER;
    }

    first_pin_state  = HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin);

    // Debounce the key input before taking the second sample.
    vTaskDelay(10);
    second_pin_state = HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin);

    // 1. Check whether the key is pressed.
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
 * @brief Detects and classifies a key press event.
 *
 * After a debounce delay, scans the key state. If the key is released,
 * the output event is set to KEY_EVENT_NONE. If the key is pressed, the
 * function waits for the specified long-press threshold and scans again.
 * A released key is classified as KEY_EVENT_SHORT_PRESS, while a key that
 * remains pressed is classified as KEY_EVENT_LONG_PRESS.
 *
 * @param[out] p_key_event Pointer to the variable that receives the detected
 *                         key event. This pointer must not be NULL.
 * @param[in] long_press_threshold_ms Long-press threshold in milliseconds.
 *                                    This value must be greater than zero.
 *
 * @return key_detect_result_t Result of the key-event detection.
 * @retval KEY_DETECT_OK                Detection completed successfully.
 * @retval KEY_DETECT_ERROR             The underlying key scan failed.
 * @retval KEY_DETECT_INVALID_PARAMETER An input parameter is invalid.
 */
key_detect_result_t key_detect_event(key_event_t *p_key_event,
                                    uint32_t long_press_threshold_ms)
{
    /* Variables */
    key_scan_result_t key_scan_result = KEY_SCAN_ERROR;
    key_state_t key_state             = KEY_STATE_RELEASED;
    uint32_t start_tick               = 0;
    /* Variables */

    // 0. Check whether the input parameters are valid.
    if ((NULL == p_key_event) || (0 == long_press_threshold_ms))
    {
        return KEY_DETECT_INVALID_PARAMETER;
    }
    *p_key_event = KEY_EVENT_NONE;

    // 1. Check whether the key is pressed.
    key_scan_result = key_scan(&key_state);
    if (KEY_SCAN_OK != key_scan_result)
    {
        return KEY_DETECT_ERROR;
    }

    if (KEY_STATE_PRESSED != key_state)
    {
        return KEY_DETECT_OK;
    }

    // 2. Wait for the long-press threshold to expire.
    // 2.1 If the key is released when the threshold expires,
    //     classify the event as a short press.
    start_tick = HAL_GetTick();
    while ((HAL_GetTick() - start_tick) < long_press_threshold_ms)
    {
        /* Wait for the long-press threshold to expire. */
    }

    key_scan_result = key_scan(&key_state);

    if (KEY_SCAN_OK != key_scan_result)
    {
        return KEY_DETECT_ERROR;
    }

    if (KEY_STATE_PRESSED != key_state)
    {
        *p_key_event = KEY_EVENT_SHORT_PRESS;
        printf("short press\r\n");
        return KEY_DETECT_OK;
    }

    // 2.2 If the key remains pressed when the threshold expires,
    //     classify the event as a long press.
    else
    {
        // 2.2.1 Update the key event.
        *p_key_event = KEY_EVENT_LONG_PRESS;

        // 2.2.2 Wait for the key to be released to prevent
        //       repeated long-press events.
        do
        {
            key_scan_result = key_scan(&key_state);

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
 * @brief Entry function for the key-scanning task.
 *
 * Creates the key-event queue and continuously detects key events using
 * a 1000 ms long-press threshold. Short-press and long-press events are
 * sent to the queue without waiting; KEY_EVENT_NONE is not sent.
 * The task delays for 10 RTOS ticks between detection attempts.
 *
 * @param[in] argument Pointer to the task argument. This parameter is not used.
 *
 * @return None. This task runs indefinitely.
 */
void key_task_entry(void *argument)
{
    key_detect_result_t key_detect_result = KEY_DETECT_ERROR;
    key_event_t key_event                 = KEY_EVENT_NONE;

    g_key_queue = xQueueCreate(10, sizeof(key_event_t));

    // Confirm that the key-event queue was created before use.
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
        // printf("KEY thread test.\r\n");

        // 1. Get a key event.
        key_detect_result = key_detect_event(&key_event, 1000);

        switch (key_detect_result)
        {
            case KEY_DETECT_OK:
            {
                // 1.1 Forward short-press and long-press events.
                if ((KEY_EVENT_SHORT_PRESS == key_event) ||
                    (KEY_EVENT_LONG_PRESS == key_event))
                {
                    // Log only if the queue send operation fails.
                    if (pdPASS != xQueueSend(g_key_queue,
                                            &key_event,
                                            0))
                    {
                        printf("[E][KEY] Failed to send key event.\r\n");
                    }
                }
                break;
            }

            // 2. Report a key-event detection failure.
            case KEY_DETECT_ERROR:
            {
                printf("[E][KEY] Key detection failed, result=%d\r\n",
                       (int)key_detect_result);
                break;
            }

            // 3. Handle an unexpected key-detection result.
            default:
            {
                printf("[E][KEY] Unexpected detection result=%d\r\n",
                       (int)key_detect_result);
                break;
            }
        }
        osDelay(10);
    }
}
