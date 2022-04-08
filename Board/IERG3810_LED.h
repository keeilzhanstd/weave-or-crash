/* IERG3810_LED.h */

#define DS0_ON	(GPIOB->BRR = 1 << 5)
#define DS0_OFF	(GPIOB->BSRR = 1 << 5)
#define DS1_ON	(GPIOE->BRR = 1 << 5)
#define DS1_OFF	(GPIOE->BSRR = 1 << 5)

void IERG3810_LED_Init(void);
