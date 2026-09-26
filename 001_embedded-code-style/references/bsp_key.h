/******************************************************************************
 * Copyright (C) 2026 JesMicro.
 *
 * All Rights Reserved.
 *
 * @file bsp_key.h
 *
 * @par dependencies
 * - stdio.h
 * - stdint.h
 * - main.h
 * - cmsis_os.h
 * - FreeRTOS.h
 * - task.h
 * - queue.h
 * - gpio.h
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

#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

/****************************** Includes ******************************/
#include <stdint.h>     // The Compiler Library
#include <stdio.h>

#include "main.h"       // Core / OS layer
#include "cmsis_os.h"

#include "FreeRTOS.h"   // Specific file for operation
#include "task.h"
#include "queue.h"
#include "gpio.h"
/****************************** Includes ******************************/

/****************************** Defines ******************************/

/************************ Thread Definitions ************************/
extern osThreadId_t key_task_handle;
extern const osThreadAttr_t key_task_attributes;

/************************ Queue Handles ************************/
extern QueueHandle_t g_key_queue;

typedef enum
{
    KEY_SCAN_OK                   = 0,          /* Success. */
    KEY_SCAN_ERROR                = 1,          /* General error. */
    KEY_SCAN_TIMEOUT              = 2,          /* Polling timed out. */
    KEY_SCAN_RESOURCE_UNAVAILABLE = 3,          /* Resource unavailable. */
    KEY_SCAN_INVALID_PARAMETER    = 4,          /* Invalid parameter. */
    KEY_SCAN_OUT_OF_MEMORY        = 5,          /* Out of memory. */
    KEY_SCAN_ISR_NOT_ALLOWED      = 6,          /* Invalid in ISR context. */
    KEY_SCAN_RESERVED             = 0x7FFFFFFF  /* Reserved. */
} key_scan_result_t; /* Return value of key_scan(). */

typedef enum
{
    KEY_STATE_PRESSED  = 0,  /* Key is pressed. */
    KEY_STATE_RELEASED = 1,  /* Key is released. */
} key_state_t; /* Physical state of the key. */

typedef enum
{
    KEY_DETECT_OK                = 0,  /* Detection completed successfully. */
    KEY_DETECT_ERROR             = 1,  /* General detection error. */
    KEY_DETECT_TIMEOUT           = 2,  /* Detection timed out. */
    KEY_DETECT_INVALID_PARAMETER = 3,  /* Invalid input parameter. */
} key_detect_result_t; /* Return value of key_detect_event(). */

typedef enum
{
    KEY_EVENT_NONE        = 0,  /* No key press event was detected. */
    KEY_EVENT_SHORT_PRESS = 1,  /* A short key press was detected. */
    KEY_EVENT_LONG_PRESS  = 2,  /* A long key press was detected. */
} key_event_t; /* Detected key press event. */

/****************************** Defines ******************************/

/************************ Function Declarations ************************/

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
key_scan_result_t key_scan(key_state_t *p_key_state);

/**
 * @brief Entry function for the key-event detection task.
 *
 * Creates the key-event queue and continuously detects key events using a
 * 1000 ms long-press threshold. Short-press and long-press events are sent
 * to the queue without waiting; KEY_EVENT_NONE is not sent. The task delays
 * for 10 RTOS ticks between detection attempts.
 *
 * @param[in] argument Pointer to the task argument. This parameter is not used.
 *
 * @return None. This task never returns. If queue creation fails,
 *         the task logs the error and remains blocked indefinitely.
 */
void key_task_entry(void *argument);

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
                                     uint32_t long_press_threshold_ms);
/************************ Function Declarations ************************/

#endif /* End of __BSP_KEY_H__ */
