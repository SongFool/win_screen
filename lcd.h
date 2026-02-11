#pragma once
#include "stdint.h"
/* LCD 句柄类型 */
typedef struct lcd_s lcd_t;
/* 创建一个 LCD 显示器 */
lcd_t* lcd_create(int w, int h, const char* title);

/* 销毁 LCD 显示器 */
void lcd_destroy(lcd_t* lcd);

/* 获取显存指针 */
uint8_t* lcd_fb(lcd_t* lcd);

/* 刷新显示 */
void lcd_show(lcd_t* lcd);

void lcd_clear(lcd_t* lcd,uint32_t color);
void lcd_draw_pixel(lcd_t* lcd,int x, int y, uint32_t color);
typedef float (*func_t)(float x);
void lcd_draw_function(lcd_t* lcd, func_t f,
                       float x_start, float x_end,
                       float y_min, float y_max,
                       uint32_t color);
