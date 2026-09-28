/* FreeRTOS 实验所用板级接口，由学习指导方提供。 */
#ifndef LAB_BOARD_H
#define LAB_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 初始化 PC13 为推挽输出(灭)、PA0 为上拉输入。可重复调用。 */
void lab_board_init(void);

/* 翻转 LED */
void lab_led_toggle(void);

/* 1 = 亮, 0 = 灭（内部已处理低电平有效） */
void lab_led_write(uint8_t on);

/* 1 = 按键当前被按下, 0 = 松开（原始电平，未消抖） */
uint8_t lab_key_is_pressed(void);

#ifdef __cplusplus
}
#endif

#endif /* LAB_BOARD_H */
