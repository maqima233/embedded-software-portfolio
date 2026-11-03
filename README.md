# 嵌入式学习与练习代码

按主题整理的 C 语言练习与 STM32F103 / FreeRTOS 实验源码，保留实现、测试和必要说明，供阅读与复跑。这里是学习代码合集，不是完整产品工程；STM32 文件的构建范围见下文。

## 目录

| 目录 | 内容 | 说明 |
| --- | --- | --- |
| [`c/allocator/`](c/allocator/) | 1 KB 静态内存池、块分裂与释放、相邻空闲块合并、`realloc` | `memory-allocator.c` 和 `my-free-merge.c` 是两个独立练习文件，分别带有测试 |
| [`c/ring-buffer/`](c/ring-buffer/) | 环形缓冲区的拒绝写入、覆盖旧数据两种满缓冲策略 | 两个文件分别演示一种策略 |
| [`c/algorithms/`](c/algorithms/) | 字符串解析与查找、最小栈、位反转、字节转十六进制、链表稳定插入排序、动态数组扩容 | 每个文件可单独编译，包含题面、实现和边界测试 |
| [`stm32/bare-metal/`](stm32/bare-metal/) | STM32F103 寄存器级 GPIO、按键轮询、EXTI 中断和 LED 控制 | 摘录入口源码及相关寄存器定义 |
| [`stm32/freertos/`](stm32/freertos/) | 按键任务向 LED 任务发送事件：二值信号量与结构体消息队列 | 包含实验代码、板级接口和 `.ioc` 配置文件 |

## 编译 C 示例

每个 C 文件都有独立的 `main`，可以分别编译运行。例如：

```sh
gcc -std=c11 -Wall -Wextra -pedantic c/allocator/memory-allocator.c -o allocator-demo
./allocator-demo
```

2026-09-28 本地检查：6 个 C 文件均以以上 C11 警告选项编译并运行退出 0。其中内存池、十六进制解析和子串查找有断言或用例测试；环形缓冲区文件使用打印输出展示运行结果。

2026-11-03 补充近期练习：`min-stack.c`、`reverse-bits.c`、`hex-row-format.c`、`list-insertion-sort.c`、`dyn-array-grow.c`。新增文件以 C11 严格警告选项编译，并运行原文件内的测试，均通过。这些是已完成的单文件练习；尚未完成的预建题目不放入本合集。

## STM32 实验说明

- 裸机实验使用 STM32F103C8，PC13 控制板载 LED，PA0 接按键。轮询实验读取按键状态；EXTI 实验配置 RCC、GPIO、AFIO、EXTI、NVIC 和 SysTick，通过下降沿中断响应按键。
- FreeRTOS 实验在已有 CubeMX 工程上编写任务逻辑：按键任务采样、消抖并分类短按和长按，LED 任务等待通知或队列事件。两个实验曾在 STM32F103 真板验证。
- 本仓库只保留相关源码和 `.ioc`，未包含 HAL、FreeRTOS 及其余生成文件，因此 STM32 目录不能单独构建为完整固件。

## 代码来源

`memory-allocator.c` 的内存池核心从空白文件编写；`my-free-merge.c` 的合并与 `realloc` 在提供的分配器框架上完成。FreeRTOS 任务逻辑由本人编写，板级接口与 CubeMX 工程由学习指导方提供。其他 C 练习的核心函数由本人完成；部分实现经过题意、接口或排错指导，题面和测试包含教师提供的内容，不将整份文件称为未经帮助独立完成。

本仓库按主题整理自个人学习代码，提交历史从整理当天开始。
