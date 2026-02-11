#include "lcd.h"
#include <math.h>

#define COLOR(R,G,B) ((R<<16)|(G<<8)|(B))






void draw_sin_wave(lcd_t* lcd)
{
    int w = 640;
    int h = 480;

    float amplitude = h / 3.0f;     // 振幅
    float center_y  = h / 2.0f;     // 中心线
    float freq = 2 * 3.1415926f / w * 4; // 4 个周期

    for (int x = 0; x < w; x++)
    {
        float y = sinf(x * freq);
        int py = (int)(center_y - y * amplitude);

        lcd_draw_pixel(lcd, x, py, 0x000000);
    }
}
 // x 范围
    float x_start = 0;
    float x_end = 20;
    float y_min = -2;
    float y_max = 2;

    // 滚动偏移量
    float scroll = 0;
float complex_wave(float x)
{
    // 正弦 + 余弦 + 指数衰减 + 三角叠加
    return sinf(x * 2)                 // 基本正弦
         + 0.5f * cosf(x * 5)         // 高频余弦
         + 0.2f * sinf(x * 10)        // 更高频
         + expf(-0.1f * x) * sinf(x); // 指数衰减调制
}
float scroll_wave(float x)
{
    scroll += 0.05f;
    return complex_wave(x + scroll);
}
int main()
{
    lcd_t* lcd1 = lcd_create(320 * 2, 240 * 2, "LCD 1");

    uint8_t* fb1 = lcd_fb(lcd1);

    int x1 = 0, x2 = 0;
    lcd_clear(lcd1,0xFFFFFF);
    
    while (1)
    {
        // LCD1: 红点

lcd_draw_function(lcd1,
                          scroll_wave,
                          x_start, x_end,
                          y_min, y_max,
                          COLOR(0,0,0));
        lcd_show(lcd1);
    lcd_clear(lcd1,0xFFFFFF);
        Sleep(100);
    }

    lcd_destroy(lcd1);
}
//gcc main.c lcd.c -o lcd_demo.exe -lgdi32 -mwindows
