/*
 * benchmark_mode.c
 *
 *  Created on: 4 янв. 2026 г.
 *      Author: Zver
 */

#include "benchmark_mode.h"

#include <stdint.h>
#include "main.h"
#include "dispcolor.h"
#include "font.h"
#include "gfx_extra.h"   // dispcolor_DrawRoundRect, dispcolor_FillRoundRect

// -------- helpers ----------
static uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
    // как в Adafruit: 5/6/5
    return (uint16_t)(((r & 0xF8u) << 8) | ((g & 0xFCu) << 3) | (b >> 3));
}

static int imin(int a, int b) { return (a < b) ? a : b; }

// -------- state machine ----------
typedef enum {
    BM_FILL = 0,
    BM_TEXT,
    BM_LINES,
    BM_FASTLINES,
    BM_RECTS,
    BM_FILLED_RECTS,
    BM_FILLED_CIRCLES,
    BM_CIRCLES,
    BM_TRIANGLES,
    BM_FILLED_TRIANGLES,
    BM_ROUNDRECTS,
    BM_FILLED_ROUNDRECTS,
    BM_DONE
} BmStage;

static uint8_t  inited = 0;
static uint32_t tick = 0;
static uint32_t stage_tick = 0;
static BmStage stage = BM_FILL;

static void bm_fill(void)
{
    dispcolor_FillScreen(BLACK); dispcolor_Update();
    dispcolor_FillScreen(RED);   dispcolor_Update();
    dispcolor_FillScreen(GREEN); dispcolor_Update();
    dispcolor_FillScreen(BLUE);  dispcolor_Update();
    dispcolor_FillScreen(BLACK); dispcolor_Update();
}

static void bm_text(void)
{
    dispcolor_FillScreen(BLACK);

    dispcolor_DrawString(0, 0, FONTID_6X8M, (char*)"Hello World!", WHITE);
    dispcolor_printf(0, 12, FONTID_6X8M, YELLOW, "1234.56");
    dispcolor_printf(0, 24, FONTID_6X8M, RED, "DEADBEEF");

    dispcolor_printf(0, 40, FONTID_6X8M, GREEN, "Groop");
    dispcolor_printf(0, 52, FONTID_6X8M, WHITE, "I implore thee,");
    dispcolor_printf(0, 64, FONTID_6X8M, WHITE, "my foonting turlingdromes.");
    dispcolor_printf(0, 76, FONTID_6X8M, WHITE, "And hooptiously drangle me");
    dispcolor_printf(0, 88, FONTID_6X8M, WHITE, "with crinkly bindlewurdles,");

    dispcolor_Update();
}

static void bm_lines(uint16_t color)
{
    int w = 240, h = 240;
    int x1 = 0, y1 = 0;
    int x2, y2;
    dispcolor_FillScreen(BLACK);

    y2 = h - 1;
    for (x2 = 0; x2 < w; x2 += 6) dispcolor_DrawLine(x1, y1, x2, y2, color);
    x2 = w - 1;
    for (y2 = 0; y2 < h; y2 += 6) dispcolor_DrawLine(x1, y1, x2, y2, color);

    dispcolor_Update();

    dispcolor_FillScreen(BLACK);
    x1 = w - 1; y1 = 0; y2 = h - 1;
    for (x2 = 0; x2 < w; x2 += 6) dispcolor_DrawLine(x1, y1, x2, y2, color);
    x2 = 0;
    for (y2 = 0; y2 < h; y2 += 6) dispcolor_DrawLine(x1, y1, x2, y2, color);

    dispcolor_Update();

    dispcolor_FillScreen(BLACK);
    x1 = 0; y1 = h - 1; y2 = 0;
    for (x2 = 0; x2 < w; x2 += 6) dispcolor_DrawLine(x1, y1, x2, y2, color);
    x2 = w - 1;
    for (y2 = 0; y2 < h; y2 += 6) dispcolor_DrawLine(x1, y1, x2, y2, color);

    dispcolor_Update();

    dispcolor_FillScreen(BLACK);
    x1 = w - 1; y1 = h - 1; y2 = 0;
    for (x2 = 0; x2 < w; x2 += 6) dispcolor_DrawLine(x1, y1, x2, y2, color);
    x2 = 0;
    for (y2 = 0; y2 < h; y2 += 6) dispcolor_DrawLine(x1, y1, x2, y2, color);

    dispcolor_Update();
}

static void bm_fastlines(uint16_t c1, uint16_t c2)
{
    dispcolor_FillScreen(BLACK);
    for (int y = 0; y < 240; y += 5) dispcolor_DrawLine(0, y, 239, y, c1);
    for (int x = 0; x < 240; x += 5) dispcolor_DrawLine(x, 0, x, 239, c2);
    dispcolor_Update();
}

static void bm_rects(uint16_t color)
{
    dispcolor_FillScreen(BLACK);
    int cx = 120, cy = 120;
    int n = imin(240, 240);
    for (int i = 2; i < n; i += 6) {
        int i2 = i / 2;
        dispcolor_DrawRectangle((int16_t)(cx - i2), (int16_t)(cy - i2),
                                (int16_t)(cx + i2), (int16_t)(cy + i2),
                                color);
    }
    dispcolor_Update();
}

static void bm_filled_rects(uint16_t c1, uint16_t c2)
{
    dispcolor_FillScreen(BLACK);
    int cx = 119, cy = 119;
    int n = imin(240, 240);
    for (int i = n; i > 0; i -= 6) {
        int i2 = i / 2;
        dispcolor_FillRectangle((int16_t)(cx - i2), (int16_t)(cy - i2),
                                (int16_t)(cx + i2), (int16_t)(cy + i2),
                                c1);
        dispcolor_DrawRectangle((int16_t)(cx - i2), (int16_t)(cy - i2),
                                (int16_t)(cx + i2), (int16_t)(cy + i2),
                                c2);
    }
    dispcolor_Update();
}

static void bm_filled_circles(int r, uint16_t color)
{
    dispcolor_FillScreen(BLACK);
    int r2 = r * 2;
    for (int x = r; x < 240; x += r2) {
        for (int y = r; y < 240; y += r2) {
            dispcolor_FillCircle((int16_t)x, (int16_t)y, (int16_t)r, color);
        }
    }
    dispcolor_Update();
}

static void bm_circles(int r, uint16_t color)
{
    // как в оригинале: экран не очищаем специально
    int r2 = r * 2;
    for (int x = 0; x < 240 + r; x += r2) {
        for (int y = 0; y < 240 + r; y += r2) {
            dispcolor_DrawCircle((int16_t)x, (int16_t)y, (int16_t)r, color, 0);
        }
    }
    dispcolor_Update();
}

static void bm_triangles(void)
{
    dispcolor_FillScreen(BLACK);
    int cx = 119, cy = 119;
    int n = imin(cx, cy);
    for (int i = 0; i < n; i += 5) {
        dispcolor_DrawTriangle((int16_t)cx,     (int16_t)(cy - i),
                               (int16_t)(cx - i), (int16_t)(cy + i),
                               (int16_t)(cx + i), (int16_t)(cy + i),
                               color565((uint8_t)i, (uint8_t)i, (uint8_t)i));
    }
    dispcolor_Update();
}

static void bm_filled_triangles(void)
{
    dispcolor_FillScreen(BLACK);
    int cx = 119, cy = 119;
    int n = imin(cx, cy);
    for (int i = n; i > 10; i -= 5) {
        dispcolor_FillTriangle((int16_t)cx, (int16_t)(cy - i),
                               (int16_t)(cx - i), (int16_t)(cy + i),
                               (int16_t)(cx + i), (int16_t)(cy + i),
                               color565(0, (uint8_t)(i*10), (uint8_t)(i*10)));
        dispcolor_DrawTriangle((int16_t)cx, (int16_t)(cy - i),
                               (int16_t)(cx - i), (int16_t)(cy + i),
                               (int16_t)(cx + i), (int16_t)(cy + i),
                               color565((uint8_t)(i*10), (uint8_t)(i*10), 0));
    }
    dispcolor_Update();
}

static void bm_roundrects(void)
{
    dispcolor_FillScreen(BLACK);
    int cx = 119, cy = 119;
    int w = imin(240, 240);
    for (int i = 0; i < w; i += 6) {
        int i2 = i / 2;
        dispcolor_DrawRoundRect((int16_t)(cx - i2), (int16_t)(cy - i2),
                                (int16_t)i, (int16_t)i,
                                (int16_t)(i / 8),
                                color565((uint8_t)i, 0, 0));
    }
    dispcolor_Update();
}

static void bm_filled_roundrects(void)
{
    dispcolor_FillScreen(BLACK);
    int cx = 119, cy = 119;
    int w = imin(240, 240);
    for (int i = w; i > 20; i -= 6) {
        int i2 = i / 2;
        dispcolor_FillRoundRect((int16_t)(cx - i2), (int16_t)(cy - i2),
                                (int16_t)i, (int16_t)i,
                                (int16_t)(i / 8),
                                color565(0, (uint8_t)i, 0));
    }
    dispcolor_Update();
}

void Benchmark_ResetView(void)
{
    inited = 1;
    tick = HAL_GetTick();
    stage_tick = tick;
    stage = BM_FILL;

    dispcolor_FillScreen(BLACK);
    dispcolor_Update();
}

uint8_t Benchmark_Draw(void)
{
    if (!inited) Benchmark_ResetView();

    uint32_t now = HAL_GetTick();
    if ((now - stage_tick) < 1000u) return 0;
    stage_tick = now;

    switch (stage) {
        case BM_FILL:              bm_fill();                         stage = BM_TEXT; break;
        case BM_TEXT:              bm_text();                         stage = BM_LINES; break;
        case BM_LINES:             bm_lines(CYAN);                    stage = BM_FASTLINES; break;
        case BM_FASTLINES:         bm_fastlines(RED, BLUE);           stage = BM_RECTS; break;
        case BM_RECTS:             bm_rects(GREEN);                   stage = BM_FILLED_RECTS; break;
        case BM_FILLED_RECTS:      bm_filled_rects(YELLOW, MAGENTA);  stage = BM_FILLED_CIRCLES; break;
        case BM_FILLED_CIRCLES:    bm_filled_circles(10, MAGENTA);    stage = BM_CIRCLES; break;
        case BM_CIRCLES:           bm_circles(10, WHITE);             stage = BM_TRIANGLES; break;
        case BM_TRIANGLES:         bm_triangles();                    stage = BM_FILLED_TRIANGLES; break;
        case BM_FILLED_TRIANGLES:  bm_filled_triangles();             stage = BM_ROUNDRECTS; break;
        case BM_ROUNDRECTS:        bm_roundrects();                   stage = BM_FILLED_ROUNDRECTS; break;
        case BM_FILLED_ROUNDRECTS: bm_filled_roundrects();            stage = BM_FILL; break; // сразу по кругу
        default:
            stage = BM_FILL;
            break;
    }

    return 0;
}

