/*
 * ampelmann.c
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#include "ampelmann.h"

#include <stdint.h>
#include "main.h"
#include "dispcolor.h"
#include "ampelguys.h"

static const int16_t ampel_y = 50;
static const uint32_t frame_ms = 50;

static const uint16_t AMPEL_COLOR = 0x86B3;
static const uint16_t BG_COLOR    = BLACK;

static uint8_t  inited = 0;
static uint32_t tick = 0;

static int16_t x = -80;
static int16_t prev_x = -80;

static uint8_t phase = 0;   // 0 right (frames 6..11), 1 left (frames 0..5)
static uint8_t frame = 6;

static void DrawBitmap1bpp_MSB_Stride(int16_t x0, int16_t y0,
                                      const uint8_t *bmp,
                                      int16_t w_bits, int16_t h, int16_t bpr,
                                      uint16_t fg, uint16_t bg)
{
    for (int16_t yy = 0; yy < h; yy++) {
        const uint8_t *row = bmp + (yy * bpr);
        for (int16_t xx = 0; xx < w_bits; xx++) {
            uint8_t b = row[xx >> 3];
            uint8_t mask = (uint8_t)(0x80u >> (xx & 7));
            dispcolor_DrawPixel((int16_t)(x0 + xx), (int16_t)(y0 + yy), (b & mask) ? fg : bg);
        }
    }
}

static void ClearPrev(int16_t x0, int16_t y0)
{
    dispcolor_FillRectangle(x0, y0,
                            (int16_t)(x0 + AMPEL_W_BITS - 1),
                            (int16_t)(y0 + AMPEL_H - 1),
                            BG_COLOR);
}

void Ampelmann_ResetView(void)
{
    inited = 1;
    tick = HAL_GetTick();

    dispcolor_FillScreen(BG_COLOR);

    phase = 0;
    frame = 6;
    x = -80;
    prev_x = x;

    dispcolor_Update();
}

static void StepAnim(void)
{
    prev_x = x;

    if (phase == 0) {
        // right walk: 6..11
        frame++;
        if (frame > 11) frame = 6;

        x = (int16_t)(x + 12);
        if (x >= 240) {
            phase = 1;
            frame = 0;
            x = 245;
            prev_x = x;
        }
    } else {
        // left walk: 0..5
        frame++;
        if (frame > 5) frame = 0;

        x = (int16_t)(x - 12);
        if (x <= -100) {
            phase = 0;
            frame = 6;
            x = -80;
            prev_x = x;
        }
    }
}

void Ampelmann_Draw(void)
{
    if (!inited) Ampelmann_ResetView();

    uint32_t now = HAL_GetTick();
    if ((now - tick) < frame_ms) return;
    tick = now;

    ClearPrev(prev_x, ampel_y);
    StepAnim();

    const uint8_t *bmp = &ampelmann[frame][0];
    DrawBitmap1bpp_MSB_Stride(x, ampel_y, bmp, AMPEL_W_BITS, AMPEL_H, AMPEL_BPR, AMPEL_COLOR, BG_COLOR);

    dispcolor_Update();
}

