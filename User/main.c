#include "stm32f10x.h"
#include "stdlib.h"
#include "IERG3810_LED.h"
#include "IERG3810_Buzzer.h"
#include "IERG3810_KEY.h"
#include "IERG3810_Clock.h"
#include "IERG3810_TFTLCD.h"
#include "IERG3810_Interrupt.h"

#define car_array_size 30
#define laneline_array_size 30

u16 CountdownHeartBeat = 0;
u16 lanelineHeartBeat = 0;
u16 carHeartBeat = 0;
u16 barHeartBeat = 0;
u16 LEDAndBuzzerHeartBeat = 0;
u16 TFTLCDHeartBeat = 0;

u16 difficulty = 1;
u16 laneline_refresh_period = 1;
u16 car_refresh_period = 1;
u16 bar_refresh_period = 1;

u16 score = 0;
void AddScore(void);

u32 timeout = 10000;
u32 ps2key_previous = 0;
u32 ps2key = 0;
u32 ps2count = 0;
u32 input = 0;

u16 countdown_flag = 0;
u16 collision_flag = 0;
u16 game_flag = 0;
u16 LED_and_buzzer_flag = 0;
u16 finish_flag = 0;
u16 back_flag = 0;
u16 create_car_flag = 0;
u16 change_difficulty_flag = 0;
u16 TFTLCD_init_flag = 0;
u16 start_game_flag = 0;

//EXTI-11 handler for PS2keyboard CLK
void EXTI15_10_IRQHandler(void)
{
  // students design program here.
	if (ps2count >= 1 && ps2count <= 8)
	{
		input = GPIOC->IDR;
		input &= 0x00000400;
		input >>= 10;
		if (input)
			ps2key |= input << (ps2count - 1);
	}
	
	ps2count++;
	
  //Delay(10); //We found that the processor is too fast and get error.
  // A short delay can eliminate the error.
  EXTI->PR = 1<<11; //Clear this exception pending bit
	EXTI->PR = 1<<11; //Clear this exception pending bit
}

void IERG3810_SYSTICK_Init10ms(void)
{
	//SYSTICK
	SysTick->CTRL = 0; // clear
	SysTick->LOAD = 89999; // What should be filled? Refer to DDI-0337E
											   // Refer to 0337E page 8-10.
			// CLKSOURCE=0: STCLK (FCLK/8)
			// CLKSOURCE=1: FCLK/8
			// set CLKSOURCE = 0 is synchornized and better than CLKSOURCE=1
			// Refer to clock tree on RM0008 pag-93.
	//SCB->SHP[11] = 0x00;
	//SCB->SHP[11] = 0x3F;
	SysTick->CTRL |= 0x3;		// What should be filled?
													// set internal clock, use interrupt, start count
}

void IERG3810_TIM2_Init(u16 arr, u16 psc)
{
	//TIM2, IRQ#28
	RCC->APB1ENR|=1<<0;			 //RM0008 page-115, refer to lab-1
	TIM2->ARR=arr;					 //RM0008 page-419
	TIM2->PSC=psc;					 //RM0008 page-418
	TIM2->DIER|=1<<0;				 //RM0008 page-409
	TIM2->CR1|=0x01;				 //RM0008 page-404
	//NVIC->IP[28] = 0x5F;				 //refer to lab-4, DDI0337E
	NVIC->ISER[0] |= (1<<28);		 //refer to lab-4, DDI0337E
}

void TIM2_IRQHandler(void)
{
	if (game_flag) {
    lanelineHeartBeat++;
    carHeartBeat++;
    barHeartBeat++;
  }
	TIM2->SR &= ~(1<<0);
	TIM2->SR &= ~(1<<0);
}

void IERG3810_TIM3_Init(u16 arr, u16 psc)
{
	//TIM3, IRQ#29
	RCC->APB1ENR|=1<<1;			 //RM0008 page-115, refer to lab-1
	TIM3->ARR=arr;					 //RM0008 page-419
	TIM3->PSC=psc;					 //RM0008 page-418
	TIM3->DIER|=1<<0;				 //RM0008 page-409
	TIM3->CR1|=0x01;				 //RM0008 page-404
	//NVIC->IP[29] = 0x7F;				 //refer to lab-4, DDI0337E
	NVIC->ISER[0] |= (1<<29);		 //refer to lab-4, DDI0337E
}

void TIM3_IRQHandler(void)
{
	if (game_flag)
		create_car_flag++;
	TIM3->SR &= ~(1<<0);
	TIM3->SR &= ~(1<<0);
}

void IERG3810_TIM4_Init(u16 arr, u16 psc)
{
	//TIM4, IRQ#30
	RCC->APB1ENR|=1<<2;			 //RM0008 page-115, refer to lab-1
	TIM4->ARR=arr;					 //RM0008 page-419
	TIM4->PSC=psc;					 //RM0008 page-418
	TIM4->DIER|=1<<0;				 //RM0008 page-409
	TIM4->CR1|=0x01;				 //RM0008 page-404
	//NVIC->IP[30] = 0x9F;				 //refer to lab-4, DDI0337E
	NVIC->ISER[0] |= (1<<30);		 //refer to lab-4, DDI0337E
}

void TIM4_IRQHandler(void)
{
	if (game_flag)
		change_difficulty_flag++;
	TIM4->SR &= ~(1<<0);			//RM0008 page-410
	TIM4->SR &= ~(1<<0);			//do one more times
}

void Countdown3s(void)
{
	int i;
	for (i = 3; i >= 1; i--) {
		IERG3810_TFTLCD_FillRectangle(black, 0, screen_width, 0, screen_length);
		IERG3810_TFTLCD_DrawDigit(red, 0x0050, 0x0050, i);
		
		CountdownHeartBeat = 0;
		countdown_flag++;
		while (CountdownHeartBeat < 100);
		countdown_flag--;
		CountdownHeartBeat = 0;
	}
}
/* Car */

typedef struct car {
	int lane;
	int y;
	u16 color;
} Car;

Car player_car;
Car car_array[car_array_size];
u16 other_cars_color[4] = {green, blue, yellow, magenta};

void PlayerCar_init(void)
{
	player_car.lane = 2;
	player_car.y = bottom_boundary;
	player_car.color = red;
}

void Car_init(void)
{
	int i;
	for (i = 0; i < car_array_size; i++)
	{
			car_array[i].lane = 0;
			car_array[i].y = 0;
			car_array[i].color = 0;
	}
}

void CreateCar(void)
{
	int i;
	for (i = 0; i < car_array_size; i++)
	{
		if (car_array[i].lane == 0)
			break;
	}
	car_array[i].lane = rand() % 4 + 1;
	car_array[i].y = (screen_length - level_and_score_bar_length);
	car_array[i].color = other_cars_color[rand() % 4];
}

void DestroyCar(void)
{
	int i;
	for (i = 0; i < car_array_size; i++)
	{
		if (car_array[i].y < -car_length)
		{
			car_array[i].lane = 0;
			car_array[i].y = 0;
			car_array[i].color = 0;
			AddScore();
		}
	}
}

void ShowAllCars(void)
{
	int i;
	//IERG3810_TFTLCD_ShowCarOnLane(player_car.lane, player_car.y, player_car.color);
	for (i = 0; i < car_array_size; i++)
	{
		if (car_array[i].lane)
			IERG3810_TFTLCD_ShowCarOnLane(car_array[i].lane, car_array[i].y, car_array[i].color);
	}
}

void ClearAllCars(void)
{
	int i;
	//IERG3810_TFTLCD_ClearCarOnLane(player_car.lane, player_car.y, player_car.color);
	for (i = 0; i < car_array_size; i++)
	{
		if (car_array[i].lane)
			IERG3810_TFTLCD_ClearCarOnLane(car_array[i].lane, car_array[i].y, road_bgcolor);
	}
}

void CarsMove(void)
{
	int i;
	for (i = 0; i < car_array_size; i++)
		if (car_array[i].lane)
			car_array[i].y -= difficulty;
}

/* Lane line */

typedef struct laneline {
	int exist;
	int y;
	u16 color;
} LaneLine;

LaneLine laneline_array[laneline_array_size];

void LaneLine_init(void)
{
	int i;
	int x;
	
	for (i = 0; i < laneline_array_size; i++)
	{
		laneline_array[i].exist = 0;
		laneline_array[i].y = -laneline_length;
		laneline_array[i].color = 0;
	}
	
	x = (screen_length - level_and_score_bar_length) / (laneline_length * 2) + 1;

	for (i = 0; i <= x; i++) {
		laneline_array[i].exist = 1;
		laneline_array[i].y = i * (laneline_length * 2);
		laneline_array[i].color = laneline_color;
	}
}	

void ShowAllLaneLines(void)
{
	int i;
	int j;
	for (i = 0; i < laneline_array_size; i++)
	{
		if (laneline_array[i].exist)
		{
			for (j = 1; j < 4; j++)
			{
				IERG3810_TFTLCD_ShowLaneLine(j, laneline_array[i].y, laneline_array[i].color);
			}
		}
	}
}

void ClearAllLaneLines(void)
{
	IERG3810_TFTLCD_FillRectangleConversion(road_bgcolor, car_width, laneline_width, 0, screen_length - level_and_score_bar_length);
	IERG3810_TFTLCD_FillRectangleConversion(road_bgcolor, 2 * car_width + laneline_width, laneline_width, 0, screen_length - level_and_score_bar_length);
	IERG3810_TFTLCD_FillRectangleConversion(road_bgcolor, 3 * car_width + 2 * laneline_width, laneline_width, 0, screen_length - level_and_score_bar_length);
}

void LaneLinesMove(void)
{
	int i;
	int count = 0;
	
	for (i = 0; i < laneline_array_size; i++)
	{
		if (laneline_array[i].exist)
		{
			count++;
		}
	}
	
	for (i = 0; i < laneline_array_size; i++)
	{
		if (laneline_array[i].exist)
		{
			laneline_array[i].y -= (2 * difficulty);
			if (laneline_array[i].y <= -laneline_length)
			{
				laneline_array[i].y += (count * (laneline_length * 2));
			}
		}
	}
}

void DrawRoad(void)
{
	IERG3810_TFTLCD_FillRectangle(road_bgcolor, 0, screen_width, 0, screen_length - level_and_score_bar_length);
	IERG3810_TFTLCD_ShowCarOnLane(player_car.lane, player_car.y, player_car.color);
	ShowAllCars();
	ShowAllLaneLines();
}

void AddScore(void)
{
	score += difficulty;
}

void DrawBar(void)
{
	IERG3810_TFTLCD_FillRectangle(level_and_score_bar_color, 0, screen_width, screen_length - level_and_score_bar_length, level_and_score_bar_length);
	IERG3810_TFTLCD_ShowLevel(difficulty);
	IERG3810_TFTLCD_ShowScore(score);
}

void UpdateBar(void)
{
	IERG3810_TFTLCD_FillRectangle(level_and_score_bar_color, 0, screen_width, screen_length - level_and_score_bar_length, level_and_score_bar_length);
	IERG3810_TFTLCD_ShowLevel(difficulty);
	IERG3810_TFTLCD_ShowScore(score);
}

void DetectCollision(void)
{
	int i;
	for (i = 0; i < car_array_size; i++)
	{
		if ((player_car.lane == car_array[i].lane) && (car_array[i].y <= (bottom_boundary + car_length)))
		{
			car_array[i].lane = 0;
			car_array[i].y = 0;
			car_array[i].color = 0;
			collision_flag++;
			break;
		}
	}
}

void UpdateSpawnFrequency(void)
{
	int car_spawn_duration;
	int ARR;
	
	/* Frequency of spawning new cars doubles. */
	//if (((difficulty % 4) == 0) && (((TIM3->ARR + 1) / 2 - 1) > 0))
	//	TIM3->ARR = ((TIM3->ARR + 1) / 2 - 1);
	
	/* Update the frequency of spawning new cars. */
	car_spawn_duration = ((((screen_length - level_and_score_bar_length + car_length) / difficulty) * 20) / 1000);
	ARR = ((car_spawn_duration * 72000000) / 21600);
	if (ARR > 0) {
		TIM3->ARR = ARR;
		TIM3->EGR = 0x0001;
	}
}

void IncreaseDifficulty(void)
{
	difficulty += 1;
	UpdateSpawnFrequency();
}

void Game(void)
{
	PlayerCar_init();
	Car_init();
	LaneLine_init();
	DrawRoad();
	DrawBar();
	
	lanelineHeartBeat = 0;
	carHeartBeat = 0;
	barHeartBeat = 0;
	
	// Re-initialize the timers
	TIM2->EGR = 0x0001;
	UpdateSpawnFrequency();
	TIM3->EGR = 0x0001;
	TIM4->EGR = 0x0001;
	
	game_flag++;
	
	while (1)
	{
		if (ps2count >= 11)
		{
			if (ps2key_previous != 0xF0)
			{
				if (ps2key == 0x6B)
				{
					if (player_car.lane > 1)
					{
						IERG3810_TFTLCD_ClearCarOnLane(player_car.lane, player_car.y, road_bgcolor);
						player_car.lane--;
						IERG3810_TFTLCD_ShowCarOnLane(player_car.lane, player_car.y, player_car.color);
					}
					
					DS1_ON;
					LEDAndBuzzerHeartBeat = 0;
					LED_and_buzzer_flag++;
					while (LEDAndBuzzerHeartBeat < 5);
					LED_and_buzzer_flag--;
					LEDAndBuzzerHeartBeat = 0;
					DS1_OFF;
				}
				
				if (ps2key == 0x74)
				{
					if (player_car.lane < 4)
					{
						IERG3810_TFTLCD_ClearCarOnLane(player_car.lane, player_car.y, road_bgcolor);
						player_car.lane++;
						IERG3810_TFTLCD_ShowCarOnLane(player_car.lane, player_car.y, player_car.color);
					}
					
					DS1_ON;
					LEDAndBuzzerHeartBeat = 0;
					LED_and_buzzer_flag++;
					while (LEDAndBuzzerHeartBeat < 5);
					LED_and_buzzer_flag--;
					LEDAndBuzzerHeartBeat = 0;
					DS1_OFF;
				}
			}
			
			ps2key_previous = ps2key;
			ps2key = 0;
			ps2count = 0;
			
			EXTI->PR = 1<<11; //Clear this exception pending bit
			// EXTI->IMR |= (1<<11); optional, resuem interrupt
		} //end of "if PS2 keybaord received data correctly"
		timeout--;
	
		if (timeout == 0) //clear PS2 keyboard data when timeout
		{
			timeout = 5000;
			ps2key_previous = 0;
			ps2key = 0;
			ps2count = 0;
		} // end of "clear PS2 keyboard data when timeout"
		
		if (lanelineHeartBeat >= laneline_refresh_period)
		{
			lanelineHeartBeat = 0;
			ClearAllLaneLines();
			LaneLinesMove();
			ShowAllLaneLines();
			//GPIOE->ODR ^= 1 << 5;
			// this one is working
		}
		
		if (carHeartBeat >= car_refresh_period)
		{
			carHeartBeat = 0;
			//GPIOB->ODR ^= 1 << 5;
			// this one is working
			ClearAllCars();
			CarsMove();
			DestroyCar();
			if (create_car_flag)
			{
				create_car_flag--;
				CreateCar();
				//GPIOB->ODR ^= 1 << 5;
			}
			ShowAllCars();
		}

		if (barHeartBeat >= bar_refresh_period)
		{
			barHeartBeat = 0;
			UpdateBar();
		}

		/* The difficulty increases every 40 seconds. */
		if (change_difficulty_flag)
		{
			change_difficulty_flag--;
			IncreaseDifficulty();
			//GPIOB->ODR ^= 1 << 5;
		}
		
		DetectCollision();
		
		if (collision_flag) {
			
			collision_flag--;
			
			DS0_ON;
			BUZZER_ON;
			
			LEDAndBuzzerHeartBeat = 0;
			LED_and_buzzer_flag++;
			while (LEDAndBuzzerHeartBeat < 10);
			LED_and_buzzer_flag--;
			LEDAndBuzzerHeartBeat = 0;
			
			DS0_OFF;
			BUZZER_OFF;
			
			ps2key_previous = 0;
			ps2key = 0;
			ps2count = 0;
			
			game_flag--;
			
			lanelineHeartBeat = 0;
			carHeartBeat = 0;
			barHeartBeat = 0;
			
			return;
		}
	}
}

int main(void)
{
	IERG3810_clock_tree_init();
	IERG3810_LED_Init();
	IERG3810_Buzzer_Init();
	IERG3810_NVIC_SetPriorityGroup(4);
	IERG3810_KEY_Init();
	IERG3810_PS2key_ExtiInit();
	
	/* Timer for counting down, LED flashing, buzzer beeping, and TFTLCD initialization, frequency: 100 Hz */
	IERG3810_SYSTICK_Init10ms();
	
	/* Timer for updating the game screen, frequency: 50 Hz */
	IERG3810_TIM2_Init(999, 1439);
	
	/* Timer for spawning new cars, initial frequency: 1/3 Hz */
	IERG3810_TIM3_Init(9999, 21599);
	UpdateSpawnFrequency();
	
	/* Timer for changing the difficulty, frequency: 1/40 Hz */
	IERG3810_TIM4_Init(49999, 57599);
	
	IERG3810_TFTLCD_Init();
	
	TFTLCDHeartBeat = 0;
	TFTLCD_init_flag++;
	while (TFTLCDHeartBeat < 10);
	TFTLCD_init_flag--;
	TFTLCDHeartBeat = 0;
	
	IERG3810_TFTLCD_ShowStartScreen();
	
	while (1)
	{
		if (ps2count >= 11)
		{
			if (ps2key_previous != 0xF0)
			{
				if (ps2key == 0x73)
				{
					start_game_flag++;
					
					DS1_ON;
					LEDAndBuzzerHeartBeat = 0;
					LED_and_buzzer_flag++;
					while (LEDAndBuzzerHeartBeat < 5);
					LED_and_buzzer_flag--;
					LEDAndBuzzerHeartBeat = 0;
					DS1_OFF;
				}
			}
			
			ps2key_previous = ps2key;
			ps2key = 0;
			ps2count = 0;
			
			EXTI->PR = 1<<11; //Clear this exception pending bit
			// EXTI->IMR |= (1<<11); optional, resuem interrupt
		} //end of "if PS2 keybaord received data correctly"
		timeout--;
		
		if (timeout == 0) //clear PS2 keyboard data when timeout
		{
			timeout = 5000;
			ps2key_previous = 0;
			ps2key = 0;
			ps2count = 0;
		} // end of "clear PS2 keyboard data when timeout"
		
		if (start_game_flag)
		{
			start_game_flag--;
			ps2key_previous = 0;
			ps2key = 0;
			ps2count = 0;
			Countdown3s();
			Game();
			finish_flag++;
		}
		
		if (finish_flag)
		{
			finish_flag--;
			
			IERG3810_TFTLCD_ShowFinalScore(score);
			
			while (1)
			{
				if (ps2count >= 11)
				{
					if (ps2key_previous != 0xF0)
					{
						if (ps2key == 0x70)
						{
							back_flag++;
							
							DS1_ON;
							LEDAndBuzzerHeartBeat = 0;
							LED_and_buzzer_flag++;
							while (LEDAndBuzzerHeartBeat < 5);
							LED_and_buzzer_flag--;
							LEDAndBuzzerHeartBeat = 0;
							DS1_OFF;
						}
					}
					
					ps2key_previous = ps2key;
					ps2key = 0;
					ps2count = 0;
					
					EXTI->PR = 1<<11; //Clear this exception pending bit
					// EXTI->IMR |= (1<<11); optional, resuem interrupt
				} //end of "if PS2 keybaord received data correctly"
				timeout--;
				
				if (timeout == 0) //clear PS2 keyboard data when timeout
				{
					timeout = 20000;
					ps2key_previous = 0;
					ps2key = 0;
					ps2count = 0;
				} // end of "clear PS2 keyboard data when timeout"
				
				if (back_flag)
				{
					back_flag--;
					
					CountdownHeartBeat = 0;
					lanelineHeartBeat = 0;
					carHeartBeat = 0;
					barHeartBeat = 0;
					LEDAndBuzzerHeartBeat = 0;
					TFTLCDHeartBeat = 0;
					
					difficulty = 1;
					UpdateSpawnFrequency();
					
					laneline_refresh_period = 1;
					car_refresh_period = 1;
					bar_refresh_period = 1;
					
					score = 0;
					
					timeout = 10000;
					ps2key_previous = 0;
					ps2key = 0;
					ps2count = 0;
					input = 0;
					
					countdown_flag = 0;
					collision_flag = 0;
					game_flag = 0;
					LED_and_buzzer_flag = 0;
					finish_flag = 0;
					back_flag = 0;
					create_car_flag = 0;
					change_difficulty_flag = 0;
					TFTLCD_init_flag = 0;
					start_game_flag = 0;
					
					Car_init();
					LaneLine_init();
					PlayerCar_init();
					
					IERG3810_TFTLCD_ShowStartScreen();
					break;
				}
			}
		}
	}
}
