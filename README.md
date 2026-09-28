# 嵌入式软件 / C 代码选摘

通信工程本科在读，求职方向是嵌入式软件开发（C 语言）。这里按主题整理了几组学习实验的源码，建议先看内存池，再看 STM32 按键事件处理。

| 入口 | 内容 | 目前的边界 |
| --- | --- | --- |
| [`c/allocator/`](c/allocator/) | 1 KB 静态内存池：best-fit、分裂、释放、相邻块合并与 `realloc` | 两个独立练习文件，各自带测试；不是系统 `malloc` 的替代品 |
| [`stm32/bare-metal/`](stm32/bare-metal/) | STM32F103 寄存器级按键轮询、EXTI 中断与 LED 控制 | 仅摘录本人修改的入口源码和寄存器定义，未包含完整 CubeIDE 工程 |
| [`stm32/freertos/`](stm32/freertos/) | 按键任务到 LED 任务：先用二值信号量，再用结构体队列传短按/长按事件 | 任务代码在已有 CubeMX / FreeRTOS 工程中完成；这里只保留审阅所需源码与 `.ioc` |
| [`c/ring-buffer/`](c/ring-buffer/) | 环形缓冲区的拒绝写入、覆盖旧数据两种满缓冲策略 | 小型练习，`main` 中是演示性检查 |
| [`c/algorithms/`](c/algorithms/) | 十六进制字符串解析、子串查找及自测用例 | C 基础边界练习，不作为完整软件项目 |

## 运行 C 示例

每个 C 文件自带 `main`，可分别编译。以静态内存池为例：

```sh
gcc -std=c11 -Wall -Wextra -pedantic c/allocator/memory-allocator.c -o allocator-demo
./allocator-demo

gcc -std=c11 -Wall -Wextra -pedantic c/allocator/my-free-merge.c -o merge-demo
./merge-demo
```

Windows 上可运行生成的 `.exe`。其他 C 示例使用同样的编译方式；这里没有把演示程序包装成可复用库。

2026-09-28 本地复核：上表涉及的 6 个 C 文件均以 `-std=c11 -Wall -Wextra -pedantic` 零警告编译并退出 0。内存池基本测试、合并与 `realloc` 测试通过；十六进制解析为 15/15，子串查找为 11/11。两份环形缓冲代码只打印运行现象，尚无自动断言，不能把退出 0 当作完整验收。

## STM32 实验说明

- 裸机部分使用 STM32F103C8：PC13 控制板载 LED，PA0 接按键。`key-polling` 轮询输入，`key-exti` 配置 RCC、GPIO、AFIO、EXTI、NVIC 和 SysTick，通过下降沿中断翻转 LED。
- FreeRTOS 实验中，按键任务周期采样并消抖，LED 任务阻塞等待事件。实验一用二值信号量通知；实验二通过按值复制的队列传递事件类型及 tick 时间戳，按短按/长按执行不同 LED 动作。两项实验曾在 STM32F103 真板验证。
- `stm32/freertos/lab-board.*` 是给定的板级接口，`freertos-lab.ioc` 来自 CubeMX 工程；HAL、FreeRTOS 和其他生成文件未复制。因此本仓库的 STM32 部分是**源码选摘，不是开箱即编译的完整固件工程**。

## 代码归属

`memory-allocator.c` 的内存池核心实现由本人从空白文件编写；`my-free-merge.c` 中相邻块合并和 `realloc` 的函数体由本人完成，分配器基建、题面和测试由学习指导方提供。FreeRTOS 两个实验的任务逻辑由本人编写，板级接口与 CubeMX 工程由学习指导方提供。C 小练习的题面及部分测试条件也有指导，核心函数由本人完成。

本仓库是从个人学习仓库整理出的展示副本，提交历史从整理当天开始，并不代表这些代码最初的开发时间。
