/*
 * bodmer_spiral.c
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#include "bodmer_spiral.h"

#include <math.h>
#include <stdint.h>

#include "main.h"       // HAL_GetTick()
#include "dispcolor.h"

#define DEG2RAD 0.0174532925f

// ====== state like Arduino sketch ======
static uint8_t  bs_inited = 0;
static uint32_t bs_tick = 0;
static const uint32_t bs_frame_ms = 5;   // было delay(5) (закомментировано), можно 1..10

static int segment = 0;
static uint16_t col = 0;
static int delta = -1;

// rainbow state machine
static uint8_t red_s   = 31;  // 5 bits (0..31)
static uint8_t green_s = 0;   // 6 bits (0..63)
static uint8_t blue_s  = 0;   // 5 bits (0..31)
static uint8_t state_s = 0;

// forward decl
static uint16_t rainbow_next(void);

static void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, uint16_t colour)
{
    // segment size tweak to prevent gaps (как в оригинале)
    const int seg = 7; // degrees covered by one painted segment
    const int inc = 6; // step between segments

    // draw blocks
    for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {
        float a0 = ((float)(i - 90) * DEG2RAD);
        float a1 = ((float)(i + seg - 90) * DEG2RAD);

        float sx  = cosf(a0);
        float sy  = sinf(a0);
        int16_t x0 = (int16_t)(sx * (float)(rx - w) + (float)x);
        int16_t y0 = (int16_t)(sy * (float)(ry - w) + (float)y);
        int16_t x1 = (int16_t)(sx * (float)rx + (float)x);
        int16_t y1 = (int16_t)(sy * (float)ry + (float)y);

        float sx2 = cosf(a1);
        float sy2 = sinf(a1);
        int16_t x2 = (int16_t)(sx2 * (float)(rx - w) + (float)x);
        int16_t y2 = (int16_t)(sy2 * (float)(ry - w) + (float)y);
        int16_t x3 = (int16_t)(sx2 * (float)rx + (float)x);
        int16_t y3 = (int16_t)(sy2 * (float)ry + (float)y);

        dispcolor_FillTriangle(x0, y0, x1, y1, x2, y2, colour);
        dispcolor_FillTriangle(x1, y1, x2, y2, x3, y3, colour);
    }
}

// 16-bit rainbow like Arduino state machine (ignores input "value")
static uint16_t rainbow_next(void)
{
    switch (state_s) {
        case 0:
            green_s++;
            if (green_s == 64) { green_s = 63; state_s = 1; }
            break;

        case 1:
            red_s--;
            if (red_s == 255) { red_s = 0; state_s = 2; }  // underflow check, keeps Arduino behaviour
            break;

        case 2:
            blue_s++;
            if (blue_s == 32) { blue_s = 31; state_s = 3; }
            break;

        case 3:
            green_s--;
            if (green_s == 255) { green_s = 0; state_s = 4; } // underflow
            break;

        case 4:
            red_s++;
            if (red_s == 32) { red_s = 31; state_s = 5; }
            break;

        case 5:
            blue_s--;
            if (blue_s == 255) { blue_s = 0; state_s = 0; } // underflow
            break;

        default:
            state_s = 0;
            red_s = 31; green_s = 0; blue_s = 0;
            break;
    }

    return (uint16_t)((red_s << 11) | (green_s << 5) | blue_s);
}

void BodmerSpiral_ResetView(void)
{
    bs_inited = 1;
    bs_tick = HAL_GetTick();

    // reset animation state
    segment = 0;
    col = 0;
    delta = -1;

    red_s = 31; green_s = 0; blue_s = 0; state_s = 0;

    dispcolor_FillScreen(GREY);
    dispcolor_Update();
}

void BodmerSpiral_Draw(void)
{
    if (!bs_inited) {
        BodmerSpiral_ResetView();
    }

    uint32_t now = HAL_GetTick();
    if ((now - bs_tick) < bs_frame_ms) return;
    bs_tick = now;

    // Arduino: fillArc(120,120, segment*6, 1, 120-segment/4, 120-segment/4, 3, rainbow(col));
    int rx = 120 - segment / 4;
    int ry = 120 - segment / 4;
    if (rx < 1) rx = 1;
    if (ry < 1) ry = 1;

    fillArc(120, 120, segment * 6, 1, rx, ry, 3, rainbow_next());

    segment += delta;
    col += 1;
    if (col > 191) col = 0;

    if (segment < 0) delta = 1;
    if (segment > 298) delta = -1;

    dispcolor_Update();
}

