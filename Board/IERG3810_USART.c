/* IERG3810_USART.c */

#include "stm32f10x.h"
#include "IERG3810_USART.h"

void IERG3810_USART1_init(u32 pclk2,u32 bound)
{
	//USART1
	float temp;
	u16 mantissa;
	u16 fraction;
	temp=(float)(pclk2*1000000)/(bound*16);
	mantissa = temp;
	fraction = (temp-mantissa)*16;
	mantissa <<= 4;
	mantissa += fraction;
	RCC->APB2ENR  |= 1<<2;			//(RM0008 page-112)
	RCC->APB2ENR  |= 1<<14;			//(RM0008 page-112)
	GPIOA->CRH    &= 0XFFFFF00F;	//set PA9,PA10 Alternate Function
	GPIOA->CRH    |= 0X000008B0;	//set PA9,PA10 Alternate Function
	RCC->APB2RSTR |= 1<<14;			//(RM0008 page-141)
	RCC->APB2RSTR &= ~(1<<14);		//((~ means inverted)
	USART1->BRR    = mantissa;		//(RM0008 page-820)
	USART1->CR1   |= 0X2008;		//(RM0008 page-821)
}

void IERG3810_USART2_init(u32 pclk1,u32 bound)
{
	//USART2
	float temp;
	u16 mantissa;
	u16 fraction;
	temp=(float)(pclk1*1000000)/(bound*16);
	mantissa = temp;
	fraction = (temp-mantissa)*16;
	mantissa <<= 4;
	mantissa += fraction;
	RCC->APB2ENR  |= 1<<2;			//(RM0008 page-112)
	RCC->APB1ENR  |= 1<<17;			//(RM0008 page-115)
	GPIOA->CRL    &= 0XFFFF00FF;	//set PA2,PA3 Alternate Function
	GPIOA->CRL    |= 0X00008B00;	//set PA2,PA3 Alternate Function
	RCC->APB1RSTR |= 1<<17;			//(RM0008 page-109)
	RCC->APB1RSTR &= ~(1<<17);		//((~ means inverted)
	USART2->BRR    = mantissa;		//(RM0008 page-820)
	USART2->CR1   |= 0X2008;		//(RM0008 page-821)
}
