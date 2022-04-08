/* IERG3810_Interrupt.c */

#include "stm32f10x.h"

void IERG3810_key2_ExtiInit(void) //given
{
	//KEY2 at PE2, EXTI-2, IRQ#8
	RCC->APB2ENR |= 1 <<6;		   //enable port-E clock for KEY2
	GPIOE->CRL   &= 0XFFFFF0FF;	   //modify PE2
	GPIOE->CRL   |= 0X00000800;	   //pull high/low mode '10', input '00'
	GPIOE->ODR   |= 1 << 2;		   //pull high
	RCC->APB2ENR |= 0x01;		   // (RM0008, page146) Enable AFIO clock
	AFIO->EXTICR[0] &= 0xFFFFF0FF; //(RM0008, AFIO_EXTICR1, page-191)
	AFIO->EXTICR[0] |= 0x00000400;
	EXTI->IMR       |= 1<<2;       //(RM0008, page-211) edge trigger
	EXTI->FTSR      |= 1<<2;       //(RM0008, page-212) falling edge
	//EXTI->RTSR |= 1<<2;          //(RM0008, page-212) rising edge

	NVIC->IP[8] = 0x1F;        //set priority of this interrupt
	NVIC->ISER[0] &= ~(1<<8);  //set NVIC 'SET ENABLE REGISTER'
	                           //DDI0337E page-8-3
	NVIC->ISER[0] |= (1<<8);   //IRQ8
}

void IERG3810_NVIC_SetPriorityGroup(u8 prigroup) //given
{
	//Set PRIGROUP AIRCR[10:8]
	u32 temp, temp1;
	temp1 = prigroup & 0x00000007; //only concern 3 bits
	temp1 <<= 8; // 'Why?'
	temp = SCB->AIRCR;	//ARM DDI0337E page 8-22
	temp &= 0x0000F8FF;	//ARM DDI0337E page 8-22
	temp |= 0x05FA0000;	//ARM DDI0337E page 8-22
	temp |= temp1;
	SCB->AIRCR=temp;
}

void IERG3810_keyUP_ExtiInit(void)
{
	//KEYUP at PA0, EXTI-0, IRQ#6
	RCC->APB2ENR |= 1 <<2;		   //enable port-A clock for KEYUP
	GPIOA->CRL   &= 0XFFFFFFF0;	   //modify PA0
	GPIOA->CRL   |= 0X00000008;	   //pull high/low mode '10', input '00'
	GPIOA->ODR   &= ~1;		       //pull low
	RCC->APB2ENR |= 0x01;		   // (RM0008, page146) Enable AFIO clock
	AFIO->EXTICR[0] &= 0xFFFFFFF0; //(RM0008, AFIO_EXTICR1, page-191)
	EXTI->IMR       |= 1;          //(RM0008, page-211) edge trigger
	//EXTI->FTSR      |= 1;        //(RM0008, page-212) falling edge
	EXTI->RTSR |= 1;               //(RM0008, page-212) rising edge

	NVIC->IP[6] = 0x1F;        //set priority of this interrupt
	NVIC->ISER[0] &= ~(1<<6);  //set NVIC 'SET ENABLE REGISTER'
	                           //DDI0337E page-8-3
	NVIC->ISER[0] |= (1<<6);   //IRQ6
}

void IERG3810_PS2key_ExtiInit(void)
{
	//PS2KEY at PC11, PC10, EXTI-11, IRQ#40
	RCC->APB2ENR |= 1 <<4;		   //enable port-C clock for PS2KEY
	GPIOC->CRH   &= 0XFFFF00FF;	   //modify PC11 and PC10
	GPIOC->CRH   |= 0X00008800;	   //pull high/low mode '10', input '00'

	RCC->APB2ENR |= 0x01;		   // (RM0008, page146) Enable AFIO clock
	AFIO->EXTICR[2] &= 0xFFFF0FFF; //(RM0008, AFIO_EXTICR3, page-191)
	AFIO->EXTICR[2] |= 0x00002000;
	EXTI->IMR       |= 1<<11;      //(RM0008, page-211) edge trigger
	EXTI->FTSR      |= 1<<11;      //(RM0008, page-212) falling edge
	//EXTI->RTSR |= 1;             //(RM0008, page-212) rising edge

	NVIC->IP[40] = 0x1F;        //set priority of this interrupt
	NVIC->ISER[1] &= ~(1<<8);  //set NVIC 'SET ENABLE REGISTER'
	                           	//DDI0337E page-8-3
	NVIC->ISER[1] |= (1<<8);   //IRQ40
}
