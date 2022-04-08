/* IERG3810_Buzzer.h */

#define BUZZER_ON	(GPIOB->BSRR = 1 << 8)
#define BUZZER_OFF 	(GPIOB->BRR = 1 << 8)

void IERG3810_Buzzer_Init(void);
