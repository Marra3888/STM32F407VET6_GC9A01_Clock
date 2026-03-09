/*
 * therm3.c
 *
 *  Created on: 2 янв. 2026 г.
 *      Author: Zver
 *
 * ModeTherm3: circular rainbow ring meter + index marker + setpoint display
 * For GC9A01A 240x240 with dispcolor.* and font.*
 */

/*
 * therm3.c
 *
 * ModeTherm3: circular rainbow ring meter + index marker + setpoint display
 * Optimized:
 *  - ring drawn only when needed (enter mode / change scheme)
 *  - dirty: no redraw when stable
 *  - partial DMA update: UpdateRect(0,0,240,170) for dynamic area
 */

//void Therm3_Draw(uint8_t light)
//{
//    uint16_t bg = light ? WHITE : BLACK;
//    uint16_t fg = light ? BLACK : WHITE;
//
//    // сглаживание до заданной точки
//    float err = s_set - s_display;
//    s_display += err * 0.15f;
//
//    if (s_need_redraw) {
//        s_need_redraw = 0;
//        s_has_old_tag = 0;
//        s_ring_dirty  = 1;
//        s_dirty       = 1;
//    }
//
//    // ничего не меняется -> нет отрисовки, нет обновления
//    if (!s_dirty) {
//        float diff = s_set - s_display;
//        if (diff < 0) diff = -diff;
//        if (diff < 0.02f) return;
//    }
//
//    if (s_ring_dirty) {
//        s_ring_dirty = 0;
//
//        dispcolor_FillScreen(bg);
//
//        RingMeter_Draw((int)(s_display * 10.0f),
//                       (int)(S_MIN * 10.0f),
//                       (int)(S_MAX * 10.0f),
//                       0, 0, R,
//                       s_scheme,
//                       GREY);
//
//        float perc = (s_display - S_MIN) * 100.0f / (S_MAX - S_MIN);
//        perc = clampf(perc, 0.0f, 100.0f);
//        IndexTag_Draw(perc, YELLOW, bg);
//
//        char buf[20];
//        snprintf(buf, sizeof(buf), "%.1f \xB0""C", (double)s_set);
//        dispcolor_DrawString(80, 104, FONTID_32F, buf, fg);
//        dispcolor_DrawString(78, 140, FONTID_16F, (char*)"SETPOINT", fg);
//
//        GC9A01A_Update();
//        s_dirty = 0;
//        return;
//    }
//
//    // очистка центра
//    dispcolor_FillRectangle(60, 90, 180, 169, bg);
//
//    float perc = (s_display - S_MIN) * 100.0f / (S_MAX - S_MIN);
//    perc = clampf(perc, 0.0f, 100.0f);
//    IndexTag_Draw(perc, YELLOW, bg);
//
//    char buf[20];
//    snprintf(buf, sizeof(buf), "%.1f \xB0""C", (double)s_set);
//    dispcolor_DrawString(80, 104, FONTID_32F, buf, fg);
//    dispcolor_DrawString(78, 140, FONTID_16F, (char*)"SETPOINT", fg);
//
//    GC9A01A_UpdateRect(0, 0, 240, 240);
//    s_dirty = 0;
//}

/*
 * therm3.c
 *
 *  Created on: 2 янв. 2026 г.
 *      Author: Zver
 *
 * ModeTherm3: circular rainbow ring meter + index marker + setpoint display
 * For GC9A01A 240x240 with dispcolor.* and font.*
 */

/*
 * therm3.c
 *
 * ModeTherm3: circular rainbow ring meter + index marker + setpoint display
 * Optimized:
 *  - ring drawn only when needed (enter mode / change scheme)
 *  - dirty: no redraw when stable
 *  - partial DMA update: UpdateRect(0,0,240,170) for dynamic area
 */

#include "therm3.h"
#include <dispcolor.h>
#include <font.h>
#include <gc9a01a.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

// ----------------- Constants -----------------
#define DEG2RAD 0.0174532925f

#define CX 120
#define CY 125
#define R  120

#define SWEEP_HALF_DEG 150
#define SEG_DEG        3
#define INC_DEG        6

#define TAG_R_TOP      (R - 45)
#define TAG_R_BASE     (R - 60)
#define TAG_HALF_DEG   6

static const float S_MIN  = 15.0f;
static const float S_MAX  = 35.0f;
static const float S_STEP = 0.5f;

// ----------------- State -----------------
static uint8_t  s_need_redraw = 1;
static uint8_t  s_ring_dirty  = 1;
static uint8_t  s_dirty       = 1;

static int16_t rt_x_old, rt_y_old, rl_x_old, rl_y_old, rr_x_old, rr_y_old;
static uint8_t s_has_old_tag = 0;

static float   s_set = 23.0f;
static float   s_display = 23.0f;
static uint8_t s_scheme = 4;

// ----------------- Helpers -----------------
static inline float clampf(float x, float a, float b)
{
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

static inline uint16_t RGB565_from_5_6_5(uint8_t r5, uint8_t g6, uint8_t b5)
{
    return (uint16_t)((r5 << 11) | (g6 << 5) | (b5));
}

static uint16_t rainbow(uint8_t value)
{
    uint8_t red = 0, green = 0, blue = 0;
    uint8_t quadrant = (uint8_t)(value / 32);

    if (quadrant == 0) {
        blue = 31; green = (uint8_t)(2 * (value % 32)); red = 0;
    } else if (quadrant == 1) {
        blue = (uint8_t)(31 - (value % 32)); green = 63; red = 0;
    } else if (quadrant == 2) {
        blue = 0; green = 63; red = (uint8_t)(value % 32);
    } else {
        blue = 0; green = (uint8_t)(63 - 2 * (value % 32)); red = 31;
    }
    return RGB565_from_5_6_5(red, green, blue);
}

static inline int map_int(int x, int in_min, int in_max, int out_min, int out_max)
{
    return out_min + (int)((int64_t)(x - in_min) * (out_max - out_min) / (in_max - in_min));
}

static void RingMeter_Draw(int value, int vmin, int vmax, int x, int y, int r, uint8_t scheme,
                           uint16_t blankColor)
{
    x += r;
    y += r;

    int w = r / 3;
    int angle = SWEEP_HALF_DEG;
    int v = map_int(value, vmin, vmax, -angle, angle);

    for (int i = -angle + INC_DEG / 2; i < angle - INC_DEG / 2; i += INC_DEG) {

        float sx  = cosf((float)(i - 90) * DEG2RAD);
        float sy  = sinf((float)(i - 90) * DEG2RAD);

        int16_t x0 = (int16_t)(sx * (r - w) + x);
        int16_t y0 = (int16_t)(sy * (r - w) + y);
        int16_t x1 = (int16_t)(sx * (r)     + x);
        int16_t y1 = (int16_t)(sy * (r)     + y);

        float sx2 = cosf((float)(i + SEG_DEG - 90) * DEG2RAD);
        float sy2 = sinf((float)(i + SEG_DEG - 90) * DEG2RAD);

        int16_t x2 = (int16_t)(sx2 * (r - w) + x);
        int16_t y2 = (int16_t)(sy2 * (r - w) + y);
        int16_t x3 = (int16_t)(sx2 * (r)     + x);
        int16_t y3 = (int16_t)(sy2 * (r)     + y);

        uint16_t col = blankColor;

        if (i < v) {
            switch (scheme) {
                case 0: col = RED; break;
                case 1: col = GREEN; break;
                case 2: col = BLUE; break;
                case 3: col = rainbow((uint8_t)map_int(i, -angle, angle, 0, 127)); break;
                case 4: col = rainbow((uint8_t)map_int(i, -angle, angle, 70, 127)); break;
                case 5: col = rainbow((uint8_t)map_int(i, -angle, angle, 127, 63)); break;
                default: col = BLUE; break;
            }
        }

        dispcolor_FillTriangle(x0, y0, x1, y1, x2, y2, col);
        dispcolor_FillTriangle(x1, y1, x2, y2, x3, y3, col);
    }
}

static void IndexTag_Draw(float perc_0_100, uint16_t color, uint16_t bg)
{
    if (s_has_old_tag) {
        dispcolor_FillTriangle(rt_x_old, rt_y_old, rl_x_old, rl_y_old, rr_x_old, rr_y_old, bg);
    }

    float ang_top = (-(240.0f) + (3.0f * perc_0_100)) * DEG2RAD;
    float ang_l   = ang_top - (TAG_HALF_DEG * DEG2RAD);
    float ang_r   = ang_top + (TAG_HALF_DEG * DEG2RAD);

    int16_t rt_x = (int16_t)(CX + (TAG_R_TOP  * cosf(ang_top)));
    int16_t rt_y = (int16_t)(CY + (TAG_R_TOP  * sinf(ang_top)));

    int16_t rl_x = (int16_t)(CX + (TAG_R_BASE * cosf(ang_l)));
    int16_t rl_y = (int16_t)(CY + (TAG_R_BASE * sinf(ang_l)));

    int16_t rr_x = (int16_t)(CX + (TAG_R_BASE * cosf(ang_r)));
    int16_t rr_y = (int16_t)(CY + (TAG_R_BASE * sinf(ang_r)));

    dispcolor_FillTriangle(rt_x, rt_y, rl_x, rl_y, rr_x, rr_y, color);

    rt_x_old = rt_x; rt_y_old = rt_y;
    rl_x_old = rl_x; rl_y_old = rl_y;
    rr_x_old = rr_x; rr_y_old = rr_y;
    s_has_old_tag = 1;
}

// ----------------- API -----------------
void Therm3_Init(void)
{
    s_need_redraw = 1;
    s_ring_dirty  = 1;
    s_dirty       = 1;
    s_has_old_tag = 0;

    s_set = 23.0f;
    s_display = 23.0f;
    s_scheme = 4;
}

void Therm3_ResetView(void)
{
    s_need_redraw = 1;
    s_ring_dirty  = 1;
    s_dirty       = 1;
    s_has_old_tag = 0;
}

void Therm3_OnK0(void)
{
    s_set += S_STEP;
    if (s_set > S_MAX) s_set = S_MIN;
    s_dirty = 1;
//    s_ring_dirty = 1;
}

void Therm3_OnK1(void)
{
    static const uint8_t schemes[] = {4, 3, 5, 0, 2, 1};
    static uint8_t idx = 0;

    idx++;
    if (idx >= (sizeof(schemes) / sizeof(schemes[0]))) idx = 0;
    s_scheme = schemes[idx];

    s_ring_dirty = 1;
    s_dirty = 1;
}

void Therm3_Draw(uint8_t light)
{
    uint16_t bg = light ? WHITE : BLACK;
    uint16_t fg = light ? BLACK : WHITE;

    // Статические переменные для хранения состояния между вызовами
    static float s_prev_set = -999.0f;
    static float s_prev_display = -999.0f;
    static uint8_t prev_light = 255;

    // --- 1) Плавное приближение к setpoint ---
    float err = s_set - s_display;
    s_display += err * 0.15f;

    // --- 2) Логика сброса (первый запуск) ---
    if (s_need_redraw) {
        s_need_redraw = 0;
        s_has_old_tag = 0;
        s_ring_dirty  = 1;
        s_dirty       = 1;
        s_prev_set = -999.0f;
        s_prev_display = -999.0f;
    }

    // --- 3) Смена темы (светлая/темная) ---
    if (prev_light == 255) prev_light = light;
    if (light != prev_light) {
        prev_light = light;
        s_ring_dirty  = 1;
        s_dirty       = 1;
        s_has_old_tag = 0;
        s_prev_set = -999.0f;
    }

    // --- 4) Early return (оптимизация) ---
    // Если ничего не изменилось визуально - выходим
    if (!s_dirty && !s_ring_dirty) {
        float diff = s_set - s_display;
        if (diff < 0) diff = -diff;
        if (diff < 0.02f) return;
    }

    // Рассчитываем позицию (процент)
    float perc = (s_display - S_MIN) * 100.0f / (S_MAX - S_MIN);
    perc = clampf(perc, 0.0f, 100.0f);

    // --- 5) ПОЛНЫЙ REDRAW (Отрисовка всего экрана) ---
    // Выполняется только при старте или смене темы/схемы
    if (s_ring_dirty) {
        s_ring_dirty = 0;

        dispcolor_FillScreen(bg);

        // Рисуем кольцо
        RingMeter_Draw((int)(s_display * 10.0f),
                       (int)(S_MIN * 10.0f),
                       (int)(S_MAX * 10.0f),
                       0, 0, R,
                       s_scheme,
                       GREY);

        // Рисуем тег
        IndexTag_Draw(perc, YELLOW, bg);

        // Рисуем текст
        char buf[20];
        snprintf(buf, sizeof(buf), "%.1f \xB0""C", (double)s_set);
        dispcolor_DrawString(80, 104, FONTID_32F, buf, fg);
        dispcolor_DrawString(78, 140, FONTID_16F, (char*)"SETPOINT", fg);

        s_prev_set = s_set;
        s_prev_display = s_display;

        GC9A01A_UpdateRect(0, 0, 240, 240);
        s_dirty = 0;
        return;
    }

    // --- 6) ЧАСТИЧНОЕ ОБНОВЛЕНИЕ (Динамика) ---

    // А. ОБНОВЛЕНИЕ ТЕКСТА (только если изменилась уставка)
    if (s_set != s_prev_set) {
        // Очищаем узкую область под текстом
        dispcolor_FillRectangle(77, 90, 155, 159, bg);

        char buf[20];
        snprintf(buf, sizeof(buf), "%.1f \xB0""C", (double)s_set);
        dispcolor_DrawString(80, 104, FONTID_32F, buf, fg);
        dispcolor_DrawString(78, 140, FONTID_16F, (char*)"SETPOINT", fg);

        // Обновляем прямоугольник текста
        GC9A01A_UpdateRect(60, 90, 180, 169);
        s_prev_set = s_set;
    }

    // Б. ОБНОВЛЕНИЕ АНИМАЦИИ (Кольцо + Тег)
    // Если температура меняется, перерисовываем кольцо и тег
    if (fabs(s_display - s_prev_display) > 0.05f) {

        // 1. Рисуем Кольцо (оно ложится поверх старого курсора и стирает его)
        // ВАЖНО: Мы убрали IndexTag_Draw(..., bg), чтобы не было мерцания (черной вспышки).
        RingMeter_Draw((int)(s_display * 10.0f),
                       (int)(S_MIN * 10.0f),
                       (int)(S_MAX * 10.0f),
                       0, 0, R,
                       s_scheme,
                       GREY);

        // 2. Рисуем новый тег поверх обновленного кольца
        IndexTag_Draw(perc, YELLOW, bg);

        // 3. Отправляем изменения на дисплей
        GC9A01A_UpdateRect(0, 0, 240, 240);

        s_prev_display = s_display;
    }

    s_dirty = 0;
}
