#include <windows.h>
#include "stdint.h"
#include "lcd.h"
struct lcd_s
{
    HWND hwnd;
    uint16_t w;
    uint16_t h;
    uint8_t* fb;
    char title[64];
};

/* 窗口回调，用于所有 LCD 窗口 */
static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    lcd_t* lcd = (lcd_t*)GetWindowLongPtr(h, GWLP_USERDATA);
    if (!lcd)
        return DefWindowProc(h, m, w, l);

    switch (m)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(h, &ps);

            BITMAPINFO bmi = {0};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = lcd->w;
            bmi.bmiHeader.biHeight = -lcd->h; // 从上到下
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 24;
            bmi.bmiHeader.biCompression = BI_RGB;

            StretchDIBits(
                hdc,
                0, 0, lcd->w, lcd->h,
                0, 0, lcd->w, lcd->h,
                lcd->fb,
                &bmi,
                DIB_RGB_COLORS,
                SRCCOPY
            );

            EndPaint(h, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            exit(0);
            return 0;
    }

    return DefWindowProc(h, m, w, l);
}
/* 创建 LCD */
lcd_t* lcd_create(int w, int h, const char* title)
{
    static int class_registered = 0;
    if (!class_registered)
    {
        WNDCLASS wc = {0};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "LCD_MULTI";
        RegisterClass(&wc);
        class_registered = 1;
    }

    lcd_t* lcd = (lcd_t*)malloc(sizeof(lcd_t));
    lcd->w = w;
    lcd->h = h;
    lcd->fb = (uint8_t*)malloc(w * h * 3);
    memset(lcd->fb, 0, w*h*3);

    strncpy(lcd->title, title, sizeof(lcd->title)-1);

    lcd->hwnd = CreateWindow(
        "LCD_MULTI",
        lcd->title,
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
        100, 100,
        w + 16, h + 39,
        NULL, NULL,
        GetModuleHandle(NULL),
        NULL
    );

    /* 让窗口回调里知道 lcd 指针 */
    SetWindowLongPtr(lcd->hwnd, GWLP_USERDATA, (LONG_PTR)lcd);

    ShowWindow(lcd->hwnd, SW_SHOW);
    return lcd;
}
/* 销毁 LCD */
void lcd_destroy(lcd_t* lcd)
{
    if (!lcd) return;
    DestroyWindow(lcd->hwnd);
    free(lcd->fb);
    free(lcd);
}

/* 获取 framebuffer */
uint8_t* lcd_fb(lcd_t* lcd)
{
    return lcd->fb;
}

/* 刷新显示 */
void lcd_show(lcd_t* lcd)
{
    if (!lcd) return;

    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    InvalidateRect(lcd->hwnd, NULL, FALSE);
}
void lcd_clear(lcd_t* lcd,uint32_t color)
{
    for (int y = 0; y < lcd->h; y++)
    {
        for (int x = 0; x < lcd->w; x++)
        {
            lcd_draw_pixel(lcd,x, y, color);
        }
    }
}

void lcd_draw_pixel(lcd_t* lcd,int x, int y, uint32_t color)
{
    if (x < 0 || y < 0 || x >= lcd->w || y >= lcd->h)
        return;

    uint8_t* p = lcd->fb + (y * lcd->w + x) * 3;
    p[0] = color & 0xFF;         // B
    p[1] = (color >> 8) & 0xFF;  // G
    p[2] = (color >> 16) & 0xFF; // R
}
void lcd_draw_line(lcd_t* lcd,
                   int x0, int y0,
                   int x1, int y1,
                   uint32_t color)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (1)
    {
        lcd_draw_pixel(lcd, x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void lcd_draw_function(lcd_t* lcd, func_t f,
                       float x_start, float x_end,
                       float y_min, float y_max,
                       uint32_t color)
{
    int w = lcd->w;
    int h = lcd->h;

    int last_x = 0;
    int last_y = h - 1 - (int)((f(x_start) - y_min) / (y_max - y_min) * h);

    for (int i = 1; i < w; i++)
    {
        float x = x_start + (x_end - x_start) * i / (w - 1);
        float y_val = f(x);

        int py = h - 1 - (int)((y_val - y_min) / (y_max - y_min) * h);

        lcd_draw_line(lcd, last_x, last_y, i, py, color);

        last_x = i;
        last_y = py;
    }
}
