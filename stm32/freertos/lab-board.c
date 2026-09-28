/* FreeRTOS 实验所用板级接口，由学习指导方提供。 */
#include "main.h"
#include "lab-board.h"

void lab_board_init(void)
{
  GPIO_InitTypeDef gpio = {0};

  /* 开时钟：LED 在 GPIOC，按键在 GPIOA */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* PC13：推挽输出。先写高电平再配模式，避免上电瞬间闪一下 */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  gpio.Pin   = GPIO_PIN_13;
  gpio.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &gpio);

  /* PA0：上拉输入（按键另一端接 GND，按下 = 低电平） */
  gpio.Pin  = GPIO_PIN_0;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &gpio);
}

void lab_led_toggle(void)
{
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void lab_led_write(uint8_t on)
{
  /* 低电平点亮，所以 on=1 时写 RESET */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

uint8_t lab_key_is_pressed(void)
{
  return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) ? 1u : 0u;
}
