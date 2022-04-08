/* IERG3810_TFTLCD.h */

typedef struct
{
	u16 LCD_REG;
	u16 LCD_RAM;
} LCD_TypeDef;

#define LCD_BASE	((u32)(0x6C000000 | 0x000007FE))
#define LCD		((LCD_TypeDef *) LCD_BASE)
#define LCD_LIGHT_ON	(GPIOB->BSRR = 1)

#define black	0x0000
#define white	0xFFFF
#define green	0x07E0
#define red	0xF800
#define blue 	0x001F
#define yellow	0xFFE0
#define cyan	0x07FF
#define magenta	0xF81F
#define grey	0x8410

/* Car size: 54 x 92 */
#define car_length 92
#define car_width 54

/* Lane line size: 8 x 40 */
#define laneline_length 40
#define laneline_width 8

#define road_bgcolor grey
#define laneline_color yellow
#define start_screen_bgcolor 0xFF47
#define start_screen_word_color 0x00B8
#define final_screen_bgcolor red
#define final_screen_word_color yellow
#define level_and_score_bar_color red
#define level_and_score_bar_word_color cyan

#define screen_length 320
#define screen_width 240
#define bottom_boundary 10
#define level_and_score_bar_length 16

void IERG3810_TFTLCD_WrReg(u16);
void IERG3810_TFTLCD_WrData(u16);
void IERG3810_TFTLCD_SetParameter(void);
void LCD_Set_Parameter(void);
void IERG3810_TFTLCD_Init(void);
void IERG3810_TFTLCD_DrawDot(u16, u16, u16);
void IERG3810_TFTLCD_FillRectangle(u16, u16, u16, u16, u16);
void IERG3810_TFTLCD_SevenSegment(u16, u16, u16, u8);
void IERG3810_TFTLCD_DrawDigit(u16, u16, u16, int);
void IERG3810_TFTLCD_ShowChar(u16, u16, u8, u16, u16);
void IERG3810_TFTLCD_ShowChinChar(u16, u16, int, u16, u16);
void IERG3810_TFTLCD_ShowCharOverlay(u16, u16, u8, u16);
void IERG3810_TFTLCD_ShowChinCharOverlay(u16, u16, int, u16);
void IERG3810_TFTLCD_FillRectangleConversion(u16, int, u16, int, u16);
void IERG3810_TFTLCD_ShowCar(int, int, u16);
void IERG3810_TFTLCD_ShowCarOnLane(int, int, u16);
void IERG3810_TFTLCD_ClearCarOnLane(int, int, u16);
void IERG3810_TFTLCD_ShowLaneLine(int, int, u16);
void IERG3810_TFTLCD_ClearLaneLine(int, int, u16);
void IERG3810_TFTLCD_ShowStartScreen(void);
u16 IERG3810_DetermineNumberOfDigits(u16);
u16 IERG3810_ValueOfDigit(u16, u16);
u16 IERG3810_ValueConversion(u16);
void IERG3810_TFTLCD_ShowFinalScore(u16);
void IERG3810_TFTLCD_ShowLevel(u16);
void IERG3810_TFTLCD_ShowScore(u16);
