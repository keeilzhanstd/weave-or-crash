/* IERG3810_LED.c */

#include "stm32f10x.h"
#include "IERG3810_LED.h"

void IERG3810_LED_Init(void) {
	// PB5 LED0
	RCC->APB2ENR |= 1 << 3;
	GPIOB->CRL &= 0xFF0FFFFF;
	GPIOB->CRL |= 0x00300000;
	GPIOB->BSRR = 1 << 5;

	// PE5 LED1
	RCC->APB2ENR |= 1 << 6;
	GPIOE->CRL &= 0xFF0FFFFF;
	GPIOE->CRL |= 0x00300000;
	GPIOE->BSRR = 1 << 5;
}

// Note: The information about DS1 (LED1) in the table is WRONG.
// 			It should be "Low = Lit" instead of "High = Lit".
