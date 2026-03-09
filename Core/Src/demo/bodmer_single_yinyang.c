/*
 * bodmer_single_yinyang.c
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#include "bodmer_single_yinyang.h"

#include <math.h>
#include <stdint.h>

#include "main.h"       // HAL_GetTick()
#include "dispcolor.h"

#define DEG2RAD 0.0174532925f

static const uint16_t C_BG = GREY;
static const uint16_t C_W  = WHITE;
static const uint16_t C_K  = GREEN;

static uint8_t  inited = 0;
static uint32_t tick = 0;

static int angle = 0;
static const uint32_t frame_ms = 50;   // wait=10 из Arduino

// "sprite" placement like pushSprite(40,40)
static const int16_t sprite_x = 40;
static const int16_t sprite_y = 40;

// geometry like Arduino
static const int16_t radius = 80;
// sprite size: radius*2+1
static const int16_t spr_w = (int16_t)(radius * 2 + 1);
static const int16_t spr_h = (int16_t)(radius * 2 + 1);

static void getCoord(int x, int y, int *xp, int *yp, int r, int a)
{
    float sx = cosf(((float)(a - 90)) * DEG2RAD);
    float sy = sinf(((float)(a - 90)) * DEG2RAD);
    *xp = (int)(sx * (float)r + (float)x);
    *yp = (int)(sy * (float)r + (float)y);
}

// Рисуем yin-yang как будто в спрайте, но прямо на экран, с базовым оффсетом (x0,y0)
static void yinyang_screen(int16_t x0, int16_t y0, int start_angle, int r)
{
    int x = r; // центр внутри спрайта = (radius, radius)
    int y = r;

    int x1 = 0, y1 = 0;

    // circle 1 (yin)
    getCoord(x, y, &x1, &y1, r / 2, start_angle);
    dispcolor_FillCircle((int16_t)(x0 + x1), (int16_t)(y0 + y1), (int16_t)(r / 2), C_K);
    dispcolor_FillCircle((int16_t)(x0 + x1), (int16_t)(y0 + y1), (int16_t)(r / 8), C_W);

    // circle 2 (yang)
    getCoord(x, y, &x1, &y1, r / 2, start_angle + 180);
    dispcolor_FillCircle((int16_t)(x0 + x1), (int16_t)(y0 + y1), (int16_t)(r / 2), C_W);
    dispcolor_FillCircle((int16_t)(x0 + x1), (int16_t)(y0 + y1), (int16_t)(r / 8), C_K);

    // outline
    dispcolor_DrawCircle((int16_t)(x0 + x), (int16_t)(y0 + y), (int16_t)r,     C_W, 0);
    dispcolor_DrawCircle((int16_t)(x0 + x), (int16_t)(y0 + y), (int16_t)(r-1), C_K, 0);
}

void BodmerSingleYY_ResetView(void)
{
    inited = 1;
    tick = HAL_GetTick();
    angle = 0;

    dispcolor_FillScreen(C_BG);
    dispcolor_Update();
}

void BodmerSingleYY_Draw(void)
{
    if (!inited) BodmerSingleYY_ResetView();

    uint32_t now = HAL_GetTick();
    if ((now - tick) < frame_ms) return;
    tick = now;

    // стереть область "спрайта"
    dispcolor_FillRectangle(sprite_x, sprite_y, (int16_t)(sprite_x + spr_w - 1), (int16_t)(sprite_y + spr_h - 1), C_BG);

    // нарисовать yin-yang
    yinyang_screen(sprite_x, sprite_y, angle, radius);

    // update angle
    angle += 3;
    if (angle > 359) angle = 0;

    dispcolor_Update();
}

