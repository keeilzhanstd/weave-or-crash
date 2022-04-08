/* IERG3810_KEY.c */

#include "stm32f10x.h"
#include "IERG3810_KEY.h"

void IERG3810_KEY_Init(void) {
	// PA0 KEY_UP
	RCC->APB2ENR |= 1 << 2;
	GPIOA->CRL &= 0xFFFFFFF0;
	GPIOA->CRL |= 0x00000008;
	GPIOA->BRR = 1;
	
	// PE4 KEY0
	RCC->APB2ENR |= 1 << 6;
	GPIOE->CRL &= 0xFFF0FFFF;
	GPIOE->CRL |= 0x00080000;
	GPIOE->BSRR |= 1 << 4;
	
	// PE3 KEY1
	GPIOE->CRL &= 0xFFFF0FFF;
	GPIOE->CRL |= 0x00008000;
	GPIOE->BSRR = 1 << 3;
	
	// PE2 KEY2
	GPIOE->CRL &= 0xFFFFF0FF;
	GPIOE->CRL |= 0x00000800;
	GPIOE->BSRR = 1 << 2;
}
