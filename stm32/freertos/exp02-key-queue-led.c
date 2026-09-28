/* STM32F103 FreeRTOS 实验：按键事件通过结构体队列按值传递。 */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include "queue.h"
#include "lab-board.h"
#include <stdint.h>

/* 按键事件和队列句柄。 */

typedef struct {
  uint8_t is_long;
  TickType_t tick;
} key_event_t;

static QueueHandle_t g_queue = NULL;

/* 按键采样与事件分类任务。 */
static void key_task(void *argument) {
  (void)argument;
  for(;;) {
    vTaskDelay(pdMS_TO_TICKS(10));
    if (lab_key_is_pressed()) {
      key_event_t evt;
      TickType_t press_tick;
      TickType_t release_tick;
      TickType_t duration;
      vTaskDelay(pdMS_TO_TICKS(20));
      if (lab_key_is_pressed()) {
            press_tick = xTaskGetTickCount();
            for(;;) {
              while (lab_key_is_pressed()) {
              vTaskDelay(pdMS_TO_TICKS(10));
              }
              vTaskDelay(pdMS_TO_TICKS(20));
              if (!lab_key_is_pressed()) {
                release_tick = xTaskGetTickCount();
                break;
              }
              }
            duration = release_tick - press_tick;
            if (duration < pdMS_TO_TICKS(500) || duration >= pdMS_TO_TICKS(1000)) {
              if (duration < pdMS_TO_TICKS(500)) evt.is_long = 0;
              else evt.is_long = 1;
              evt.tick = release_tick;
              if (xQueueSend(g_queue, &evt, pdMS_TO_TICKS(20)) != pdPASS)
              {
                /* 队列满且等待超时：消息未入队，本次事件直接丢弃。 */
              }
            }
      }
    }
  }
}

/* LED 事件处理任务。 */
static void led_task(void *argument) {
  (void)argument;
  key_event_t out;
  for(;;) {
    if (xQueueReceive(g_queue, &out, portMAX_DELAY) == pdTRUE) {
      if (!out.is_long) lab_led_toggle();
      else {
        for (uint32_t i = 0; i < 6; i++) {
          vTaskDelay(pdMS_TO_TICKS(100));
          lab_led_toggle();
        }
      }
    }
  }
}

/* 调度器启动前创建队列和任务。 */
void exp02_key_queue_led_start(void)
{
  g_queue = xQueueCreate(4, sizeof(key_event_t));
  configASSERT(g_queue);
  xTaskCreate(key_task, "key", 128, NULL, 2, NULL);
  xTaskCreate(led_task, "led", 128, NULL, 1, NULL);
}
