#ifndef LCD_API_H_INCLUDED
#define LCD_API_H_INCLUDED

#include <stdint.h>

#define MAX_X LCD_WIDTH-1
#define MAX_Y LCD_HEIGHT-1


extern void LCD_write_english(uint16_t x,uint16_t y,uint8_t str,unsigned int color,unsigned int xcolor);//Ð´×Ö·û
extern void LCD_write_english_string(uint16_t x,uint16_t y,char *s,unsigned int color,unsigned int xcolor);//Ó¢ÎÄ×Ö·û´®ÏÔÊ¾


void lcd_clear(unsigned short Color);
void lcd_text_menu(uint16_t x_pos, uint16_t cell, const char * text);
struct menu;
void lcd_menu_update(struct menu * menu);
void lcd_PutString(unsigned int x, unsigned int y, const char * s, unsigned int textColor, unsigned int bkColor);
void lcd_draw_buttons(void);
void lcd_draw_back_button(void);
void lcd_draw_applet_options(const char * text_1, char * text_2, char * text_3, char * text_4);
void lcd_DrawRect(int x1, int y1, int x2, int y2, int col);
void LCD_SetDisplayWindow(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width);
void DrawBMP(uint8_t* ptrBitmap);

extern void lcd_lock();
extern void lcd_background(uint16_t color);
extern void lcd_text(uint8_t col, uint8_t row, const char *text);
extern void lcd_printf(uint8_t col, uint8_t row, uint8_t ww, const char *fmt, ...);
extern void lcd_release();

void lcd_fill(uint16_t xx, uint16_t yy, uint16_t ww, uint16_t hh, uint16_t color);
void lcd_text_xy(uint16_t Xpos, uint16_t Ypos, const char *str,uint16_t Color, uint16_t bkColor);
/**
 * The LCD is written to by more than one task so is controlled by a
 * 'gatekeeper' task.  This is the only task that is actually permitted to
 * access the LCD directly.  Other tasks wanting to display a message send
 * the message to the gatekeeper.
 */
void vLCDTask( void *pvParameters );

//extern xQueueHandle xLCDQueue;

#define mainLCD_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE + 1500 )

#endif // LCD_API_H_INCLUDED
