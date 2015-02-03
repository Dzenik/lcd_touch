#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "ili_lcd_general.h"
#include "lcd_api.h"
#include "semphr.h"


#define LCD_LOCK char auto_lock = 0;if (lcdUsingTask != xTaskGetCurrentTaskHandle()){ lcd_lock(); auto_lock = 1; }
#define LCD_UNLOCK if (auto_lock) lcd_release()

// we use this sempaphore to ensure multiple threads do not try to use the LCD at the same time
xSemaphoreHandle xLcdSemaphore;

extern const char english[][16];
static uint16_t bg_col;

void LCD_write_english(uint16_t x,uint16_t y,uint8_t str,unsigned int color,unsigned int xcolor)//Ð´×Ö·û
{
    uint16_t xpos = x;
    uint16_t ypos = y;

    unsigned char avl,i,n;

    for(i=0; i<16; i++) //16ÐÐ
    {
        avl= (english[str-32][i]);
        lcd_SetCursor(xpos,ypos);
        ypos++;
        rw_data_prepare();
        for(n=0; n<8; n++) //8ÁÐ
        {
            if(avl&0x80) write_data(color);
            else write_data(xcolor);
            avl<<=1;
        }
    }
}

void LCD_write_english_string(uint16_t x,uint16_t y,char *s,unsigned int color,unsigned int xcolor)//Ó¢ÎÄ×Ö·û´®ÏÔÊ¾
{
    while (*s)
    {
        LCD_write_english(x,y,*s,color,xcolor);
        s++;
        x += 8;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// LOCKING code
//////////////////////////////////////////////////////////////////////////////////////////////////
static volatile xTaskHandle lcdUsingTask = NULL;

void lcd_lock()
{
    while( xSemaphoreTake( xLcdSemaphore, ( portTickType ) 100 ) != pdTRUE )
    {
        printf("Waiting a long time for LCD\r\n");
    }
    lcdUsingTask = xTaskGetCurrentTaskHandle();
}

void lcd_release()
{
    xSemaphoreGive(xLcdSemaphore);
    lcdUsingTask = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// THREADSAFE INTERFACE - uses semaphores to unsure only one thread is drawing at a time
/////////////////////////////////////////////////////////////////////////////////////////////////

void lcd_text_xy(uint16_t Xpos, uint16_t Ypos, const char *str,uint16_t Color, uint16_t bkColor)
{
    LCD_LOCK;
    uint8_t TempChar;

//	printf("lcd text %d,%d %s\r\n", Xpos, Ypos, str);

    while ((TempChar=*str++))
    {
        LCD_write_english(Xpos,Ypos,TempChar,Color, bkColor);
        if (Xpos < MAX_X - 8)
        {
            Xpos+=8;
        }
        else if (Ypos < MAX_Y - 16)
        {
            Xpos=0;
            Ypos+=16;
        }
        else
        {
            Xpos=0;
            Ypos=0;
        }
    }
    LCD_UNLOCK;
}

void lcd_text(uint8_t col, uint8_t row, const char *text)
{
    lcd_text_xy(col * 8, row * 16, text, 0xFFFF, bg_col);
}

void lcd_fill(uint16_t xx, uint16_t yy, uint16_t ww, uint16_t hh, uint16_t color)
{
    int ii, jj;
    LCD_LOCK;
    for (ii = 0; ii < hh; ii++)
    {
        lcd_SetCursor(xx, yy + ii);
        rw_data_prepare();
        for (jj = 0; jj < ww; jj++)
        {
            write_data(color);
        }
    }
    LCD_UNLOCK;
}

void lcd_printf(uint8_t col, uint8_t row, uint8_t ww, const char *fmt, ...)
{
    LCD_LOCK;
    char message[31];
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(message, sizeof(message) - 1, fmt, ap);
    va_end(ap);

    while (len < ww && len < sizeof(message) - 2)
    {
        message[len++] = ' ';
    }
    message[len] = 0;

    lcd_text(col, row, message);

    LCD_UNLOCK;
}

void lcd_clear(uint16_t Color)
{
    LCD_LOCK;
    uint32_t index=0;
    lcd_SetCursor(0,0);
    rw_data_prepare(); /* Prepare to write GRAM */
    for(index = 0; index < LCD_WIDTH * LCD_HEIGHT; index++)
    {
        write_data(Color);
    }
    LCD_UNLOCK;
}

void lcd_background(uint16_t color)
{
    bg_col = color;
}

static void lcd_DrawHLine(int x1, int x2, int col, int y )
{
    lcd_SetCursor(x1, y); //start position
    rw_data_prepare(); /* Prepare to write GRAM */
    while (x1 <= x2)
    {
        write_data(col);
        x1 ++;

    }
}

static void lcd_DrawVLine(int y1, int y2, int col, int x)
{
    lcd_SetCursor(x, y1); //start position
    rw_data_prepare(); /* Prepare to write GRAM */
    while (y1 <= y2)
    {
        write_data(col);
        y1++;
    }
}

void lcd_DrawRect(int x1, int y1, int x2, int y2, int col)
{
    LCD_LOCK;
    lcd_DrawVLine(y1, y2, col, x1);
    lcd_DrawVLine(y1, y2, col, x2);
    lcd_DrawHLine(x1, x2, col, y1);
    lcd_DrawHLine(x1, x2, col, y2);
    LCD_UNLOCK;
}
