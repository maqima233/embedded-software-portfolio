/* STM32F103 FreeRTOS 实验：按键任务通过二值信号量通知 LED 任务。 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lab-board.h"

/* 二值信号量句柄。 */
static SemaphoreHandle_t g_sem = NULL;

/* 按键采样与消抖任务。 */
static void key_task(void *argument) {
	(void)argument;
	static uint8_t last_raw = 0;   /* 上一轮的原始读数 */
	static uint8_t confirm = 0;    /* 已被"连续两轮一致"确认的稳定电平 */
	for (;;){
	        vTaskDelay(pdMS_TO_TICKS(20));        /* 采样周期 20ms，同时就是稳定窗口 */
	        uint8_t raw = lab_key_is_pressed();   /* 只读一次，存起来用 */
	        if (raw == last_raw){                 /* 连续两轮读数一致 → 该电平可信（消抖） */
                if(raw && !confirm) xSemaphoreGive(g_sem);  /* 只有 松开→按下 这一跳变给一次 */
                confirm = raw;
	        }
	        last_raw = raw;
	    }
}

/* LED 事件处理任务。 */
static void led_task(void *argument) {
	(void)argument;
	for(;;){
		 if (xSemaphoreTake(g_sem, portMAX_DELAY) == pdTRUE)
		        {
		            lab_led_toggle();
		        }
	}
}

/* 调度器启动前创建信号量和任务。 */
void exp01_key_sem_led_start(void)
{
    g_sem = xSemaphoreCreateBinary();
    configASSERT(g_sem != NULL);
    xTaskCreate(key_task,   "key", 128, NULL, 2, NULL);
    xTaskCreate(led_task, "led",   128, NULL, 1, NULL);
}
