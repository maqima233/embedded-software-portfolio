/*
 * regs.h
 *
 *  Created on: 2026年8月7日
 *      Author: tuodu
 */


#ifndef REGS_H_
#define REGS_H_

#include <stdint.h>

#define RCC_APB2ENR (*(volatile uint32_t*)0x40021018)
#define GPIOC_CRH (*(volatile uint32_t*)0x40011004)
#define GPIOC_ODR (*(volatile uint32_t*)0x4001100C)
#define GPIOA_CRL (*(volatile uint32_t*)0x40010800)
#define GPIOA_IDR (*(volatile uint32_t*)0x40010808)
#define GPIOA_ODR (*(volatile uint32_t*)0x4001080C)
#define AFIO_EXTICR1 (*(volatile uint32_t*)0x40010008)
#define EXTI_IMR (*(volatile uint32_t*)0x40010400)
#define EXTI_RTSR (*(volatile uint32_t*)0x40010408)
#define EXTI_FTSR (*(volatile uint32_t*)0x4001040C)
#define EXTI_PR (*(volatile uint32_t*)0x40010414)
#define NVIC_ISER0 (*(volatile uint32_t*)0xE000E100)
#define SYST_CSR    (*(volatile uint32_t*)0xE000E010)
#define SYST_RVR    (*(volatile uint32_t*)0xE000E014)


#endif /* REGS_H_ */
