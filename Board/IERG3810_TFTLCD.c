/* IERG3810_TFTLCD.c */

#include "stm32f10x.h"
#include "IERG3810_TFTLCD.h"
#include "FONT.h"
#include "CFONT.h"
#include "LOGO.H"

void IERG3810_TFTLCD_WrReg(u16 regval)
{
	LCD->LCD_REG=regval;
}

void IERG3810_TFTLCD_WrData(u16 data)
{
	LCD->LCD_RAM=data;
}

void IERG3810_TFTLCD_SetParameter(void)
{
	IERG3810_TFTLCD_WrReg(0X01);	//Software reset
	IERG3810_TFTLCD_WrReg(0X11);	//Exit_sleep_mode
	
	IERG3810_TFTLCD_WrReg(0X3A);	//Set_pixel_format
	IERG3810_TFTLCD_WrData(0X55);	//65536 colors
	
	IERG3810_TFTLCD_WrReg(0X29);	//Display ON
	
	IERG3810_TFTLCD_WrReg(0X36);	//Memory Access Control (section 8.2.29, page 127)
	IERG3810_TFTLCD_WrData(0XCA);
}

/*
void LCD_Set_Parameter(void)
{
	// details in ILI9341 datasheet
	LCD_WR_REG(0X01);		//Software reset
	LCD_WR_REG(0X11);		//Exit_sleep_mode
	
	LCD_WR_REG(0X3A);		//Set_pixel_format
	LCD_WR_DATA(0X55);		//65536 colors
	
	LCD_WR_REG(0X29);		//Display ON
	
	LCD_WR_REG(0X36);		//Memory Access Control
	LCD_WR_DATA(0XCA);
}
*/

void IERG3810_TFTLCD_Init(void)	//set FSMC
{
	RCC->AHBENR|=1<<8;			//FSMC
	RCC->APB2ENR|=1<<3;			//PORTB
	RCC->APB2ENR|=1<<5;			//PORTD
	RCC->APB2ENR|=1<<6;			//PORTE
	RCC->APB2ENR|=1<<8;			//PORTG
	GPIOB->CRL&=0XFFFFFFF0;	//PB0
	GPIOB->CRL|=0X00000003;
	
	//PORTD
	GPIOD->CRH&=0X00FFF000;
	GPIOD->CRH|=0XBB000BBB;
	GPIOD->CRL&=0XFF00FF00;
	GPIOD->CRL|=0X00BB00BB;
	//PORTE
	GPIOE->CRH&=0X00000000;
	GPIOE->CRH|=0XBBBBBBBB;
	GPIOE->CRL&=0X0FFFFFFF;
	GPIOE->CRL|=0XB0000000;
	//PORTG12
	GPIOG->CRH&=0XFFF0FFFF;
	GPIOG->CRH|=0X000B0000;
	GPIOG->CRL&=0XFFFFFFF0;//PG0->RS
	GPIOG->CRL|=0X0000000B;
	
	// LCD uses FSMC Bank 4 memory bank.
	// Use Mode A
	FSMC_Bank1->BTCR[6]=0X00000000;		//FSMC_BCR4 (reset)
	FSMC_Bank1->BTCR[7]=0X00000000;		//FSMC_BTR4 (reset)
	FSMC_Bank1E->BWTR[6]=0X00000000;	//FSMC_BWTR4 (reset)
	FSMC_Bank1->BTCR[6]|=1<<12;			//FSMC_BCR4 -> WREN
	FSMC_Bank1->BTCR[6]|=1<<14;			//FSMC_BCR4 -> EXTMOD
	FSMC_Bank1->BTCR[6]|=1<<4;			//FSMC_BCR4 -> MWID
	FSMC_Bank1->BTCR[7]|=0<<28;			//FSMC_BTR4 -> ACCMOD
	FSMC_Bank1->BTCR[7]|=1<<0;			//FSMC_BTR4 -> ADDSET
	FSMC_Bank1->BTCR[7]|=0XF<<8;		//FSMC_BTR4 -> DATAST
	FSMC_Bank1E->BWTR[6]|=0<<28;		//FSMC_BWTR4 -> ACCMOD
	FSMC_Bank1E->BWTR[6]|=0<<0;			//FSMC_BWTR4 -> ADDSET
	FSMC_Bank1E->BWTR[6]|=3<<8;			//FSMC_BWTR4 -> DATAST
	FSMC_Bank1->BTCR[6]|=1<<0;			//FSMC_BCR4 -> FACCEN
	IERG3810_TFTLCD_SetParameter();		// special setting for LCD module
	LCD_LIGHT_ON;
}

void IERG3810_TFTLCD_DrawDot(u16 x, u16 y, u16 color)
{
	IERG3810_TFTLCD_WrReg(0x2A);		//set x position
	IERG3810_TFTLCD_WrData(x>>8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData(0x01);
	IERG3810_TFTLCD_WrData(0x3F);
	IERG3810_TFTLCD_WrReg(0x2B);		//set y position
	IERG3810_TFTLCD_WrData(y>>8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData(0x01);
	IERG3810_TFTLCD_WrData(0xDF);
	IERG3810_TFTLCD_WrReg(0X2C);		//set point with color
	IERG3810_TFTLCD_WrData(color);
}

void IERG3810_TFTLCD_FillRectangle(u16 color, u16 start_x, u16 length_x, u16 start_y, u16 length_y)
{
	u32 index = 0;
	IERG3810_TFTLCD_WrReg(0x2A);
	IERG3810_TFTLCD_WrData(start_x>>8);
	IERG3810_TFTLCD_WrData(start_x & 0xFF);
	IERG3810_TFTLCD_WrData((length_x + start_x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length_x + start_x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B);
	IERG3810_TFTLCD_WrData(start_y>>8);
	IERG3810_TFTLCD_WrData(start_y & 0xFF);
	IERG3810_TFTLCD_WrData((length_y + start_y - 1) >> 8);
	IERG3810_TFTLCD_WrData((length_y + start_y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0X2C);//LCD_WriteRAM_Prepare();
	for(index=0;index<length_x * length_y;index++)
	{
		IERG3810_TFTLCD_WrData(color);
	}
}

void IERG3810_TFTLCD_SevenSegment(u16 color, u16 start_x, u16 start_y, u8 digit)
{
	if ((digit >> 6) & 0x01)
	{
		IERG3810_TFTLCD_FillRectangle(color, start_x + 0x000A, 0x0037, start_y + 0x0082, 0x000A);
	}
	
	if ((digit >> 5) & 0x01)
	{
		IERG3810_TFTLCD_FillRectangle(color, start_x + 0x0041, 0x000A, start_y + 0x004B, 0x0037);
	}
	
	if ((digit >> 4) & 0x01)
	{
		IERG3810_TFTLCD_FillRectangle(color, start_x + 0x0041, 0x000A, start_y + 0x000A, 0x0037);
	}
	
	if ((digit >> 3) & 0x01)
	{
		IERG3810_TFTLCD_FillRectangle(color, start_x + 0x000A, 0x0037, start_y, 0x000A);
	}
	
	if ((digit >> 2) & 0x01)
	{
		IERG3810_TFTLCD_FillRectangle(color, start_x, 0x000A, start_y + 0x000A, 0x0037);
	}
	
	if ((digit >> 1) & 0x01)
	{
		IERG3810_TFTLCD_FillRectangle(color, start_x, 0x000A, start_y + 0x004B, 0x0037);
	}
	
	if (digit & 0x01)
	{
		IERG3810_TFTLCD_FillRectangle(color, start_x + 0x000A, 0x0037, start_y + 0x0041, 0x000A);
	}
}

void IERG3810_TFTLCD_DrawDigit(u16 color, u16 start_x, u16 start_y, int i)
{
	switch (i) {
		case 0:
			IERG3810_TFTLCD_SevenSegment(color, start_x, start_y, 0x7E);
			return;
		case 1:
			IERG3810_TFTLCD_SevenSegment(color, start_x, start_y, 0x30);
			return;
		case 2:
			IERG3810_TFTLCD_SevenSegment(color, start_x, start_y, 0x6D);
			return;
		case 3:
			IERG3810_TFTLCD_SevenSegment(color, start_x, start_y, 0x79);
			return;
		case 4:
			IERG3810_TFTLCD_SevenSegment(color, start_x, start_y, 0x33);
			return;
		case 5:
			IERG3810_TFTLCD_SevenSegment(color, start_x, start_y, 0x5B);
			return;
		case 6:
			IERG3810_TFTLCD_SevenSegment(color, start_x, start_y, 0x5F);
			return;
		case 7:
			IERG3810_TFTLCD_SevenSegment(color, start_x, start_y, 0x70);
			return;
		case 8:
			IERG3810_TFTLCD_SevenSegment(color, start_x, start_y, 0x7F);
			return;
		case 9:
			IERG3810_TFTLCD_SevenSegment(color, start_x, start_y, 0x7B);
			return;
		default:
			return;
	}
}	

void IERG3810_TFTLCD_ShowChar(u16 x, u16 y, u8 ascii, u16 color, u16 bgcolor)
{
	u8 i, j;
	u8 index;
	u8 height=16, length=8;
	if (ascii<32 || ascii >127) return;
	ascii -=32;
	IERG3810_TFTLCD_WrReg(0x2A);
	IERG3810_TFTLCD_WrData(x>>8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B);
	IERG3810_TFTLCD_WrData(y>>8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C);//LCD_WriteRAM_Prepare();

	for (j=0; j<height/8; j++)
	{
		for(i=0; i<height/2; i++)
		{
			for(index=0;index<length ;index++)
			{
				if ((asc2_1608[ascii][index*2+1-j]>>i) & 0x01) IERG3810_TFTLCD_WrData(color);
				else IERG3810_TFTLCD_WrData(bgcolor);
			}
		}
	}
}

void IERG3810_TFTLCD_ShowChinChar(u16 x, u16 y, int num, u16 color, u16 bgcolor)
{
	u8 i, j;
	u8 index;
	u8 height=16, length=16;
	if (num<0 || num >2) return;

	IERG3810_TFTLCD_WrReg(0x2A);
	IERG3810_TFTLCD_WrData(x>>8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B);
	IERG3810_TFTLCD_WrData(y>>8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C);//LCD_WriteRAM_Prepare();

	for (j=0; j<height/8; j++)
	{
		for(i=0; i<height/2; i++)
		{
			for(index=0;index<length ;index++)
			{
				if ((chi_1616[num][index*2+1-j]>>i) & 0x01) IERG3810_TFTLCD_WrData(color);
				else IERG3810_TFTLCD_WrData(bgcolor);
			}
		}
	}
}

void IERG3810_TFTLCD_ShowCharOverlay(u16 x, u16 y, u8 ascii, u16 color)
{
	u8 i, j;
	u8 index;
	u8 height=16, length=8;
	if (ascii<32 || ascii >127) return;
	ascii -=32;
	IERG3810_TFTLCD_WrReg(0x2A);
	IERG3810_TFTLCD_WrData(x>>8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B);
	IERG3810_TFTLCD_WrData(y>>8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C);//LCD_WriteRAM_Prepare();

	for (j=0; j<height/8; j++)
	{
		for(i=0; i<height/2; i++)
		{
			for(index=0;index<length ;index++)
			{
				if ((asc2_1608[ascii][index*2+1-j]>>i) & 0x01) IERG3810_TFTLCD_DrawDot(x + index, y + j * 8 + i, color);
			}
		}
	}
}

void IERG3810_TFTLCD_ShowChinCharOverlay(u16 x, u16 y, int num, u16 color)
{
	u8 i, j;
	u8 index;
	u8 height=16, length=16;
	if (num<0 || num >2) return;

	IERG3810_TFTLCD_WrReg(0x2A);
	IERG3810_TFTLCD_WrData(x>>8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B);
	IERG3810_TFTLCD_WrData(y>>8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C);//LCD_WriteRAM_Prepare();

	for (j=0; j<height/8; j++)
	{
		for(i=0; i<height/2; i++)
		{
			for(index=0;index<length ;index++)
			{
				if ((chi_1616[num][index*2+1-j]>>i) & 0x01) IERG3810_TFTLCD_DrawDot(x + index, y + j * 8 + i, color);
			}
		}
	}
}

/* Note: This FillRectangleConversion() function assumes that the size of the screen is screen_width x (screen_length - level_and_score_bar_length). 
			This function is intended for the game screen. For other screens, please use the FillRectangle() function. */

void IERG3810_TFTLCD_FillRectangleConversion(u16 color, int start_x, u16 length_x, int start_y, u16 length_y)
{
	if (start_x > screen_width)
		return;

	if (start_y > (screen_length - level_and_score_bar_length))
		return;

	if ((start_x + length_x) < 0)
		return;

	if ((start_y + length_y) < 0)
		return;

	if (start_x < 0)
	{
		if (((start_x + length_x) >= 0) && ((start_x + length_x) <= screen_width))
		{
			length_x += start_x;
			start_x = 0;
		}
		else if ((start_x + length_x) > screen_width)
		{
			length_x = screen_width;
			start_x = 0;
		}
	}

	if (start_y < 0)
	{
		if ((((start_y + length_y) >= 0) && (start_y + length_y)) <= (screen_length - level_and_score_bar_length))
		{
			length_y += start_y;
			start_y = 0;
		}
		else if ((start_y + length_y) > (screen_length - level_and_score_bar_length))
		{
			length_y = (screen_length - level_and_score_bar_length);
			start_y = 0;
		}
	}

	if ((start_x >= 0) && (start_x <= screen_width))
	{
		if ((start_x + length_x) > screen_width)
		{
			length_x = (screen_width - start_x);
		}
	}

	if ((start_y >= 0) && (start_y <= (screen_length - level_and_score_bar_length)))
	{
		if ((start_y + length_y) > (screen_length - level_and_score_bar_length))
		{
			length_y = (screen_length - level_and_score_bar_length - start_y);
		}
	}

	IERG3810_TFTLCD_FillRectangle(color, start_x, length_x, start_y, length_y);
}

void IERG3810_TFTLCD_ShowCar(int x, int y, u16 color)
{	
	/* Row 91 and Row 92*/
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 12, 32, y, 2);

	y += 2;

	/* Row 89 and Row 90 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 36, y, 2);

	y += 2;

	/* Row 87 and Row 88 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 8, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 14, 26, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 40, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 44, 4, y, 2);

	y += 2;

	/* Row 85 and Row 86 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 8, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 14, 28, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 44, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 83 and Row 84 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 14, 28, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 44, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 77 to Row 82 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 6);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 8, 2, y, 6);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 2, y, 6);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 12, 30, y, 6);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 6);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 44, 2, y, 6);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 6);

	y += 6;

	/* Row 75 and Row 76 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 12, 8, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 20, 14, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 34, 8, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 44, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 73 and Row 74*/
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 10, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 20, 14, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 34, 10, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 44, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 69 to Row 72 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 8, 4, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 12, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 14, 26, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 40, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 42, 4, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 4);

	y += 4;

	/* Row 67 and Row 68 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 10, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 12, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 14, 26, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 40, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 44, 4, y, 2);

	y += 2;

	/* Row 65 and Row 66 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 12, 30, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 44, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 63 and Row 64 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(white, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 12, 30, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(white, x + 44, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 61 and Row 62 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(white, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 12, 30, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 44, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 57 to Row 60 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 8, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 12, 30, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 44, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 4);

	y += 4;

	/* Row 55 and Row 56 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 6, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 12, 30, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 6, y, 2);

	y += 2;

	/* Row 51 to Row 54 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 8, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 12, 30, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 44, 2, y, 4);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 4);

	y += 4;

	/* Row 49 and Row 50 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 8, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 12, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 14, 26, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 40, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 42, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 47 and Row 48 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(white, x + 10, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 12, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 14, 26, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 40, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(white, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 44, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 45 and Row 46 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(white, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 10, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 12, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 14, 26, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 40, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(white, x + 44, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 43 and Row 44 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 12, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 14, 6, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 20, 12, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 32, 6, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 38, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 40, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 44, 2, y, 2);

	y += 2;

	/* Row 41 and Row 42 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 2, 6, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 12, 8, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 20, 14, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 34, 8, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 44, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 6, y, 2);

	y += 2;

	/* Row 39 and Row 40 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 2, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 6, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 12, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(white, x + 16, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 18, 24, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 6, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 48, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 52, 2, y, 2);

	y += 2;

	/* Row 37 and Row 38 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 2, 8, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 10, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(white, x + 14, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 16, 28, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 44, 8, y, 2);

	y += 2;

	/* Row 35 and Row 36 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 10, 34, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 44, 4, y, 2);

	y += 2;

	/* Row 33 and Row 34 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 8, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 14, 26, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 40, 8, y, 2);

	y += 2;

	/* Row 31 and Row 32 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 8, 6, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 14, 6, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(cyan, x + 20, 14, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 34, 6, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 40, 6, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 29 and Row 30 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 8, 12, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 20, 14, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 34, 12, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 16 to Row 28 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 12);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 8, 38, y, 12);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 12);

	y += 12;

	/* Row 14 and Row 15 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 10, 34, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 44, 4, y, 2);

	y += 2;

	/* Row 12 and Row 13 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 12, 30, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 44, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 9 and Row 10 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 6, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 8, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 12, 30, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 42, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 46, 2, y, 2);

	y += 2;

	/* Row 7 and Row 8 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 8, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 10, 34, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 44, 2, y, 2);

	y += 2;

	/*Row 5 and Row 6 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 10, 2, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 12, 30, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 42, 2, y, 2);

	y += 2;

	/*Row 3 and Row 4 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 12, 4, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(color, x + 16, 22, y, 2);
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 38, 4, y, 2);

	y += 2;

	/* Row 1 and Row 2 */
	IERG3810_TFTLCD_FillRectangleConversion(black, x + 16, 22, y, 2);

}

void IERG3810_TFTLCD_ShowCarOnLane(int lane, int y, u16 color)
{
	switch (lane)
	{
		case 1:
			IERG3810_TFTLCD_ShowCar(0, y, color);
			return;
		case 2:
			IERG3810_TFTLCD_ShowCar(car_width + laneline_width, y, color);
			return;
		case 3: 
			IERG3810_TFTLCD_ShowCar(2 * (car_width + laneline_width), y, color);
			return;
		case 4:
			IERG3810_TFTLCD_ShowCar(3 * (car_width + laneline_width), y, color);
			return;
	}
}

void IERG3810_TFTLCD_ClearCarOnLane(int lane, int y, u16 bgcolor)
{
	switch (lane)
	{
		case 1:
			IERG3810_TFTLCD_FillRectangleConversion(bgcolor, 0, car_width, y, car_length);
			return;
		case 2:
			IERG3810_TFTLCD_FillRectangleConversion(bgcolor, car_width + laneline_width, car_width, y, car_length);
			return;
		case 3: 
			IERG3810_TFTLCD_FillRectangleConversion(bgcolor, 2 * (car_width + laneline_width), car_width, y, car_length);
			return;
		case 4:
			IERG3810_TFTLCD_FillRectangleConversion(bgcolor, 3 * (car_width + laneline_width), car_width, y, car_length);
			return;
	}
}

void IERG3810_TFTLCD_ShowLaneLine(int laneline, int y, u16 color)
{
	switch (laneline)
	{
		case 1:
			IERG3810_TFTLCD_FillRectangleConversion(color, car_width, laneline_width, y, laneline_length);
			return;
		case 2:
			IERG3810_TFTLCD_FillRectangleConversion(color, 2 * car_width + laneline_width, laneline_width, y, laneline_length);
			return;
		case 3: 
			IERG3810_TFTLCD_FillRectangleConversion(color, 3 * car_width + 2 * laneline_width, laneline_width, y, laneline_length);
			return;
	}
}

void IERG3810_TFTLCD_ClearLaneLine(int laneline, int y, u16 bgcolor)
{
	switch (laneline)
	{
		case 1:
			IERG3810_TFTLCD_FillRectangleConversion(bgcolor, car_width, laneline_width, y, laneline_length);
			return;
		case 2:
			IERG3810_TFTLCD_FillRectangleConversion(bgcolor, 2 * car_width + laneline_width, laneline_width, y, laneline_length);
			return;
		case 3:
			IERG3810_TFTLCD_FillRectangleConversion(bgcolor, 3 * car_width + 2 * laneline_width, laneline_width, y, laneline_length);
			return;
	}
}

void IERG3810_TFTLCD_DrawLogo(u16 x, u16 y) {
	u16 relative_x, relative_y;
	u16 color;
	for( relative_y = y - LOGO_TFT_H + 1; relative_y <= y; relative_y++ ) {
		for( relative_x = x; relative_x < LOGO_TFT_W + x; relative_x++ ) {
			color = logo_hex[((y - relative_y)* LOGO_TFT_W + (relative_x-x)) * 2];
			color <<= 8;
			color = color + logo_hex[((y - relative_y)* LOGO_TFT_W + (relative_x-x)) * 2 + 1];
			IERG3810_TFTLCD_DrawDot(relative_x, relative_y, color);
		}
	}
}

void IERG3810_TFTLCD_ShowStartScreen(void)
{
	u8 SID_1[10] = {0x31, 0x31, 0x35, 0x35, 0x30, 0x36, 0x34, 0x35, 0x39, 0x30};
	u8 name_2[22] = {0x42, 0x45, 0x4B, 0x5A, 0x48, 0x41, 0x4E, 0x20, 0x4D, 0x55, 0x4B, 0x41, 0x4D, 0x42, 0x45, 0x54, 0x4B, 0x41, 0x4C, 0x59, 0x45, 0x56};
	u8 SID_2[10] = {0x31, 0x31, 0x35, 0x35, 0x31, 0x32, 0x33, 0x37, 0x33, 0x39};
	u8 KeyFourMoveLeft[15] = {0x4B, 0x45, 0x59, 0x34, 0x3A, 0x20, 0x4D, 0x4F, 0x56, 0x45, 0x20, 0x4C, 0x45, 0x46, 0x54};
	u8 KeySixMoveRight[16] = {0X4B, 0X45, 0X59, 0x36, 0x3A, 0x20, 0x4D, 0x4F, 0x56, 0x45, 0x20, 0x52, 0x49, 0x47, 0x48, 0x54};
	u8 PressKey5ToStart[20] = {0x50, 0x52, 0x45, 0x53, 0x53, 0x20, 0x4B, 0x45, 0x59, 0x35, 0x20, 0x54, 0x4F, 0x20, 0x53, 0x54, 0x41, 0x52, 0x54};

	int i;

	IERG3810_TFTLCD_FillRectangle(start_screen_bgcolor, 0, screen_width, 0, screen_length);
	IERG3810_TFTLCD_DrawLogo(5, 290);
		
	/* Print Chinese name */
	for (i = 0; i < 3; i++)
		IERG3810_TFTLCD_ShowChinCharOverlay(0x0060 + i * 0x0010, 0x0098, i, start_screen_word_color);
	
	/* Print "1155064590" */
	for (i = 0; i < 10; i++)
		IERG3810_TFTLCD_ShowCharOverlay(0x0050 + i * 0x0008, 0x0088, SID_1[i], start_screen_word_color);

	/* Print "Bekzhan MUKAMBETKALYEV" */
	for (i = 0; i < 22; i++)
		IERG3810_TFTLCD_ShowCharOverlay(0x0020 + i * 0x0008, 0x0068, name_2[i], start_screen_word_color);

	/* Print "1155123739" */
	for (i = 0; i < 10; i++)
		IERG3810_TFTLCD_ShowCharOverlay(0x0050 + i * 0x0008, 0x0058, SID_2[i], start_screen_word_color);

	/* Print "KEY4: MOVE LEFT" */
	for (i = 0; i < 15; i++)
		IERG3810_TFTLCD_ShowCharOverlay(0x003C + i * 0x0008, 0x0038, KeyFourMoveLeft[i], start_screen_word_color);

	/* Print "KEY6: MOVE RIGHT" */
	for (i = 0; i < 16; i++)
		IERG3810_TFTLCD_ShowCharOverlay(0x0038 + i * 0x0008, 0x0028, KeySixMoveRight[i], start_screen_word_color);
	
	/* Print "PRESS KEY5 TO START" */
	for (i = 0; i < 19; i++)
		IERG3810_TFTLCD_ShowCharOverlay(0x002B + i * 0x0008, 0x0018, PressKey5ToStart[i], start_screen_word_color);
}

void IERG3810_TFTLCD_ShowLoudlyCryingFace(u16 x, u16 y, u16 scale)
{
	/* Loudly crying emoji size: (45 x scale) x (45 x scale) */

	/* Column 1*/
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 18 * scale, 9 * scale);

	x += scale;

	/* Column 2 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 14 * scale, 17 * scale);

	x += scale;

	/* Column 3 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 12 * scale, 21 * scale);
	
	x += scale;

	/* Column 4 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 10 * scale, 25 * scale);

	x += scale;

	/* Column 5 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 9 * scale, 27 * scale);

	x += scale;

	/* Column 6 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 8 * scale, 29 * scale);

	x += scale;

	/* Column 7 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 7 * scale, 31 * scale);

	x += scale;

	/* Column 8 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 6 * scale, 16 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 22 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 23 * scale, 16 * scale);

	x += scale;

	/* Column 9 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 5 * scale, 17 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 22 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 24 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 27 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 29 * scale, 11 * scale);

	x += scale;

	/* Column 10 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 4 * scale, 19 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 27 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 30 * scale, 11 * scale);

	x += scale;

	/* Column 11 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 3 * scale, 20 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 28 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 31 * scale, 11 * scale);

	x += scale;

	/* Column 12 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 3 * scale, 20 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 4 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 29 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 32 * scale, 10 * scale);

	x += scale;

	/* Column 13 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 2 * scale, 21 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 5 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 30 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 33 * scale, 10 * scale);

	x += scale;

	/* Column 14 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 2 * scale, 21 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 6 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 31 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 33 * scale, 10 * scale);

	x += scale;

	/* Column 15 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + scale, 22 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 19 * scale);

	x += scale;

	/* Column 16 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + scale, 21 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 22 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 24 * scale, 20 * scale);

	x += scale;

	/* Column 17 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + scale, 21 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 22 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 23 * scale, 21 * scale);

	x += scale;

	/* Column 18 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + scale, 10 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 11 * scale, 5 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 16 * scale, 28 * scale);

	x += scale;

	/* Column 19 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y, 10 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 10 * scale, 7 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 17 * scale, 28 * scale);

	x += scale;

	/* Column 20 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y, 9 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 9 * scale, 7 * scale);
	IERG3810_TFTLCD_FillRectangle(white, x, scale, y + 16 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 17 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 18 * scale, 27 * scale);

	x += scale;

	/* Column 21 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y, 9 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 9 * scale, 6 * scale);
	IERG3810_TFTLCD_FillRectangle(white, x, scale, y + 15 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 17 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 18 * scale, 27 * scale);

	x += scale;

	/* Column 22 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y, 9 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 9 * scale, 6 * scale);
	IERG3810_TFTLCD_FillRectangle(white, x, scale, y + 15 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 18 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 19 * scale, 26 * scale);

	x += scale;

	/* Column 23 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y, 9 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 9 * scale, 6 * scale);
	IERG3810_TFTLCD_FillRectangle(white, x, scale, y + 15 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 18 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 19 * scale, 26 * scale);

	x += scale;

	/* Column 24 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y, 9 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 9 * scale, 6 * scale);
	IERG3810_TFTLCD_FillRectangle(white, x, scale, y + 15 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 18 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 19 * scale, 26 * scale);

	x += scale;

	/* Column 25 */ 
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y, 9 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 9 * scale, 6 * scale);
	IERG3810_TFTLCD_FillRectangle(white, x, scale, y + 15 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 17 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 18 * scale, 27 * scale);

	x += scale;

	/* Column 26 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y, 9 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 9 * scale, 7 * scale);
	IERG3810_TFTLCD_FillRectangle(white, x, scale, y + 16 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 17 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 18 * scale, 27 * scale);

	x += scale;

	/* Column 27 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y, 10 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 10 * scale, 7 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 17 * scale, 28 * scale);

	x += scale;

	/* Column 28 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + scale, 10 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 11 * scale, 5 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 16 * scale, 28 * scale);

	x += scale;

	/* Column 29 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + scale, 21 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 22 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 23 * scale, 21 * scale);

	x += scale;

	/* Column 30 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + scale, 21 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 22 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 24 * scale, 20 * scale);

	x += scale;

	/* Column 31 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + scale, 22 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 19 * scale);

	x += scale;

	/* Column 32 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 2 * scale, 21 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 6 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 31 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 33 * scale, 10 * scale);

	x += scale;

	/* Column 33 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 2 * scale, 21 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 5 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 30 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 33 * scale, 10 * scale);

	x += scale;

	/* Column 34 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 3 * scale, 20 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 4 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 29 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 32 * scale, 10 * scale);

	x += scale;

	/* Column 35 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 3 * scale, 20 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 28 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 31 * scale, 11 * scale);

	x += scale;

	/* Column 36 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 4 * scale, 19 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 23 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 25 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 27 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 30 * scale, 11 * scale);

	x += scale;

	/* Column 37 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 5 * scale, 17 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 22 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 24 * scale, 3 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 27 * scale, 2 * scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 29 * scale, 11 * scale);

	x += scale;

	/* Column 38 */
	IERG3810_TFTLCD_FillRectangle(cyan, x, scale, y + 6 * scale, 16 * scale);
	IERG3810_TFTLCD_FillRectangle(black, x, scale, y + 22 * scale, scale);
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 23 * scale, 16 * scale);

	x += scale;

	/* Column 39 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 7 * scale, 31 * scale);

	x += scale;

	/* Column 40 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 8 * scale, 29 * scale);

	x += scale;

	/* Column 41 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 9 * scale, 27 * scale);

	x += scale;

	/* Column 42 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 10 * scale, 25 * scale);

	x += scale;

	/* Column 43 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 12 * scale, 21 * scale);
	
	x += scale;

	/* Column 44 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 14 * scale, 17 * scale);

	x += scale;

	/* Column 45 */
	IERG3810_TFTLCD_FillRectangle(yellow, x, scale, y + 18 * scale, 9 * scale);
}

u16 IERG3810_DetermineNumberOfDigits(u16 x)
{
	if (x < 10)
		return 1;
	else
		return 1 + IERG3810_DetermineNumberOfDigits(x / 10);
}

/* num = 1: rightmost digit of x
	num = 2: second rightmost digit of x
	etc. */

u16 IERG3810_ValueOfDigit(u16 x, u16 num)
{
	int i;
	for (i = 0; i < num - 1; i++)
		x /= 10;
	return x % 10;
}

u16 IERG3810_ValueConversion(u16 x)
{
	return 0x0030 + x;
}

void IERG3810_TFTLCD_ShowFinalScore(u16 score)
{
	u8 YouCrashed[14] = {0x59, 0x4F, 0x55, 0x20, 0x43, 0x52, 0x41, 0x53, 0x48, 0x45, 0x44, 0x21, 0x21, 0x21};
	u8 FinalScore[13] = {0x46, 0x49, 0x4E, 0x41, 0x4C, 0x20, 0x53, 0x43, 0x4F, 0x52, 0x45, 0x3A, 0x20};
	u8 PressKey0ToLobby[20] = {0x50, 0x52, 0x45, 0x53, 0x53, 0x20, 0x4B, 0x45, 0x59, 0x30, 0x20, 0x54, 0x4F, 0x20, 0x4C, 0x4F, 0x42, 0x42, 0x59};

	int i;

	u16 num_of_digits;
	u16 start_x;
	u16 value;
	u16 converted_value;

	IERG3810_TFTLCD_FillRectangle(final_screen_bgcolor, 0, screen_width, 0, screen_length);

	/* Show a loudly crying face */
	IERG3810_TFTLCD_ShowLoudlyCryingFace(0x001E, 0x0076, 4);

	/* Print "YOU CRASHED!!!" */
	for (i = 0; i < 14; i++)
		IERG3810_TFTLCD_ShowCharOverlay(0x0040 + i * 0x0008, 0x0050, YouCrashed[i], final_screen_word_color);

	/* Print "FINAL SCORE: " */
	num_of_digits = IERG3810_DetermineNumberOfDigits(score);
	start_x = 0x0044 - 0x0004 * num_of_digits;

	for (i = 0; i < 13; i++)
		IERG3810_TFTLCD_ShowCharOverlay(start_x + i * 0x0008, 0x0030, FinalScore[i], final_screen_word_color);

	/* Print the score */
	for (i = num_of_digits; i > 0; i--)
	{
		value = IERG3810_ValueOfDigit(score, i);
		converted_value = IERG3810_ValueConversion(value);
		IERG3810_TFTLCD_ShowCharOverlay(start_x + 13 * 0x0008 + (num_of_digits - i) * 0x0008, 0x0030, converted_value, final_screen_word_color);
	}

	/* Print "PRESS KEY0 TO LOBBY" */
	for (i = 0; i < 20; i++)
		IERG3810_TFTLCD_ShowCharOverlay(0x002B + i * 0x0008, 0x0010, PressKey0ToLobby[i], final_screen_word_color);
}

void IERG3810_TFTLCD_ShowLevel(u16 difficulty)
{
	u8 Level[6] = {0x4C, 0x45, 0x56, 0x45, 0X4C, 0x20};

	int i;

	u16 num_of_digits;
	u16 value;
	u16 converted_value;

	/* Print "LEVEL " */
	for (i = 0; i < 6; i++)
		IERG3810_TFTLCD_ShowCharOverlay(0x0000 + i * 0x0008, screen_length - level_and_score_bar_length, Level[i], level_and_score_bar_word_color);

	/* Print difficulty value */
	num_of_digits = IERG3810_DetermineNumberOfDigits(difficulty);

	for (i = num_of_digits; i > 0; i--)
	{
		value = IERG3810_ValueOfDigit(difficulty, i);
		converted_value = IERG3810_ValueConversion(value);
		IERG3810_TFTLCD_ShowCharOverlay(0x0030 + (num_of_digits - i) * 0x0008, screen_length - level_and_score_bar_length, converted_value, level_and_score_bar_word_color);
	}
}

void IERG3810_TFTLCD_ShowScore(u16 score)
{
	u8 Score[7] = {0x53, 0x43, 0x4F, 0x52, 0x45, 0x3A, 0x20};

	int i;

	u16 num_of_digits;
	u16 start_x;
	u16 value;
	u16 converted_value;

	num_of_digits = IERG3810_DetermineNumberOfDigits(score);
	start_x = screen_width - 0x0038 - 0x0008 * num_of_digits;

	/* Print "SCORE: " */
	for (i = 0; i < 7; i++)
		IERG3810_TFTLCD_ShowCharOverlay(start_x + i * 0x0008, screen_length - level_and_score_bar_length, Score[i], level_and_score_bar_word_color);

	/* Print score value */
	for (i = num_of_digits; i > 0; i--)
	{
		value = IERG3810_ValueOfDigit(score, i);
		converted_value = IERG3810_ValueConversion(value);
		IERG3810_TFTLCD_ShowCharOverlay(start_x + 0x0038 + (num_of_digits - i) * 0x0008, screen_length - level_and_score_bar_length, converted_value, level_and_score_bar_word_color);
	}
}
