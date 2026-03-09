/*
 * gif_mode.c
 *
 *  Created on: 13 янв. 2026 г.
 *      Author: Zver
 */

#include "gif_mode.h"
#include "main.h"
#include "dispcolor.h"
#include "AnimatedGIF.h"
#include "gc9a01a.h"
#include <string.h>
#include <stdint.h>

// Подключите ваш GIF-массив (убедитесь, что он есть в img/x_wing.h)
#include "img/MANresize.h"

// ====== config ======
#ifndef GIF_MAX_W
#define GIF_MAX_W 240
#endif
#ifndef GIF_MAX_H
#define GIF_MAX_H 240
#endif

//static GIFIMAGE gif;
// Кладём структуру в CCM RAM (64KB), чтобы не занимать основную память
__attribute__((section(".ccmram"))) static GIFIMAGE gif;
static uint8_t inited = 0;
static uint8_t opened = 0;

static int16_t gif_x0 = 0;   // offset for centering
static int16_t gif_y0 = 0;
static int16_t gif_w = 0,  gif_h = 0;

static uint32_t next_tick = 0;

void GifMode_Reset(void)
{
    inited = 0;
    opened = 0;
}

// draw callback: one decoded line -> framebuffer
void GIFDraw(GIFDRAW *pDraw)
{
    int y = gif_y0 + pDraw->iY + pDraw->y;
    if (y < 0 || y >= 240) return;

    int x0 = gif_x0 + pDraw->iX;

    for (int i = 0; i < pDraw->iWidth; i++) {
        int x = x0 + i;
        if (x < 0) continue;
        if (x >= 240) break;

        uint8_t index = pDraw->pPixels[i];

        // transparency: keep previous pixel (as GIF expects)
        if (pDraw->ucHasTransparency && index == pDraw->ucTransparent) {
            continue;
        }

        uint16_t color = pDraw->pPalette[index];
        dispcolor_DrawPixel((int16_t)x, (int16_t)y, color);
    }
}

static void GifMode_Open(void)
{
    if (opened) return;

    opened = (uint8_t)GIF_openRAM(&gif, (uint8_t*)MANresize, (int)sizeof(MANresize), GIFDraw);
    if (!opened) return;

    int cw = GIF_getCanvasWidth(&gif);   // 200
    int ch = GIF_getCanvasHeight(&gif);  // 200
    int sw = (int)GC9A01A_GetWidth();    // 240
    int sh = (int)GC9A01A_GetHeight();   // 240

    gif_x0 = (int16_t)((sw - cw) / 2);   // 20
    gif_y0 = (int16_t)((sh - ch) / 2);   // 20

    // clear once on enter (so the borders are clean)
    dispcolor_ClearScreen();
    GC9A01A_Update();
    while (GC9A01A_IsBusy()) {}

    next_tick = HAL_GetTick();
}

void GifMode_Init(void)
{
    memset(&gif, 0, sizeof(gif));
    GIF_begin(&gif, GIF_PALETTE_RGB565_LE);

    opened = 0;
    GifMode_Open();

    inited = 1;
}

void GifMode_Draw(void)
{
    if (!inited) GifMode_Init();

    if (!GIF_openRAM(&gif, (uint8_t *)MANresize, sizeof(MANresize), GIFDraw)) {
        return;
    }

    // вычислить 1 раз (можно вынести в Init)
    gif_w = (int16_t)GIF_getCanvasWidth(&gif);   // 200
    gif_h = (int16_t)GIF_getCanvasHeight(&gif);  // 200
    gif_x0 = (int16_t)((240 - gif_w) / 2);       // 20
    gif_y0 = (int16_t)((240 - gif_h) / 2);       // 20

    // очистить фон один раз
    dispcolor_ClearScreen();
    GC9A01A_Update();
    while (GC9A01A_IsBusy()) {}

    int delay_ms = 0;

    while (1)
    {
        // не трогаем framebuffer пока DMA шлет
        while (GC9A01A_IsBusy()) {}

        uint32_t now = HAL_GetTick();
        if (now < next_tick) continue;

        // если видите "смаз/непонятные кадры" из-за disposal/прозрачности — попробуйте раскомментировать:
        // dispcolor_FillRect(gif_x0, gif_y0, gif_w, gif_h, BLACK);

        GIF_playFrame(&gif, &delay_ms, NULL);

        // обновляем только область 200x200
        GC9A01A_UpdateRect(gif_x0, gif_y0, gif_w, gif_h);

        if (delay_ms < 1) delay_ms = 1;
        next_tick = now + (uint32_t)delay_ms;
    }
}
