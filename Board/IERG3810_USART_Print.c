/* IERG3810_USART_Print.c */

#include "stm32f10x.h"
#include "IERG3810_USART_Print.h"

void USART_print(u8 USARTport, char *st)
{
	u8 i=0;
	while (st[i] !=0x00)
	{
		if (USARTport == 1) USART1->DR = st[i];
		if (USARTport == 2) USART2->DR = st[i];
		if (USARTport == 1) while(!((USART1->SR >> 7) & 0x1));	//(RM0008 page-818)
		if (USARTport == 2) while(!((USART2->SR >> 7) & 0x1));	//(RM0008 page-818)
		if (i == 255) break;
		i++;
	}
}

void USART_printu8(u8 USARTport, u8 content)
{
	if (USARTport == 1) USART1->DR = content;
	if (USARTport == 2) USART2->DR = content;
	if (USARTport == 1) while(!((USART1->SR >> 7) & 0x1));	//(RM0008 page-818)
	if (USARTport == 2) while(!((USART2->SR >> 7) & 0x1));	//(RM0008 page-818)
}
