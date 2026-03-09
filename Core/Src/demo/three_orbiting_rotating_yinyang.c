/*
 * three_orbiting_rotating_yinyang.c
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#include "three_orbiting_rotating_yinyang.h"

#include <math.h>
#include <stdint.h>

#include "main.h"       // HAL_GetTick()
#include "dispcolor.h"

#define DEG2RAD 0.0174532925f

// Цвета как в скетче
static const uint16_t C_BG = AMPEL;
static const uint16_t C_W  = WHITE;
static const uint16_t C_K  = BLACK;

// Параметры
static int angle_00 = 0;
static int orbit_angle = 0;

static const int radius_00 = 40;
static const uint32_t frame_ms = 14;

static const int x_anchor = 77;
static const int y_anchor = 101;
static const int x_offset = 0;
static const int y_offset = -24;

static const int common_radius = 94;
static const int yy_offset = 18;  // как в скетче (смещение внутри "спрайта")

// состояние
static uint8_t inited = 0;
static uint32_t tick = 0;

// чтобы стирать прошлую позицию
static int16_t prev_x_orbit = 0;
static int16_t prev_y_orbit = 0;
static uint8_t has_prev = 0;

static void getCoord_00(int x, int y, int *xp, int *yp, int r, int a)
{
    float sx = cosf(((float)(a - 90)) * DEG2RAD);
    float sy = sinf(((float)(a - 90)) * DEG2RAD);
    *xp = (int)(sx * (float)r + (float)x + (float)yy_offset);
    *yp = (int)(sy * (float)r + (float)y + (float)yy_offset);
}

// рисуем yin-yang "как в спрайте", но в координатах экрана:
// (x0, y0) — левый верх виртуального спрайта
static void yinyang_00_screen(int16_t x0, int16_t y0, int start_angle_00, int r)
{
    // в оригинале x,y передают (radius_00-15), (radius_00-15)
    // тут сделаем то же, но относительно x0,y0
    const int x = (radius_00 - 15);
    const int y = (radius_00 - 15);

    int x1 = 0, y1 = 0;

    // circle #1 (big yin = black)
    getCoord_00(x, y, &x1, &y1, r / 2, start_angle_00);
    dispcolor_FillCircle((int16_t)(x0 + x1), (int16_t)(y0 + y1), (int16_t)(r / 2), C_K);
    dispcolor_FillCircle((int16_t)(x0 + x1), (int16_t)(y0 + y1), (int16_t)(r / 8), C_W);

    // circle #2 (big yang = white)
    getCoord_00(x, y, &x1, &y1, r / 2, start_angle_00 + 180);
    dispcolor_FillCircle((int16_t)(x0 + x1), (int16_t)(y0 + y1), (int16_t)(r / 2), C_W);
    dispcolor_FillCircle((int16_t)(x0 + x1), (int16_t)(y0 + y1), (int16_t)(r / 8), C_K);

    // outer rings
    dispcolor_DrawCircle((int16_t)(x0 + x + yy_offset), (int16_t)(y0 + y + yy_offset), (int16_t)r,     C_W, 0);
    dispcolor_DrawCircle((int16_t)(x0 + x + yy_offset), (int16_t)(y0 + y + yy_offset), (int16_t)(r+1), C_K, 0);
}

static void calculate_orbiting_pos(int16_t *x_orbit, int16_t *y_orbit)
{
    float xo = (float)(x_anchor + x_offset) + (float)common_radius * cosf((float)orbit_angle * DEG2RAD);
    float yo = (float)(y_anchor + y_offset) + (float)common_radius * sinf((float)orbit_angle * DEG2RAD);
    *x_orbit = (int16_t)xo;
    *y_orbit = (int16_t)yo;
}

void ThreeYY_ResetView(void)
{
    inited = 1;
    tick = HAL_GetTick();

    angle_00 = 0;
    orbit_angle = 0;
    has_prev = 0;

    dispcolor_FillScreen(C_BG);
    dispcolor_Update();
}

void ThreeYY_Draw(void)
{
    if (!inited) ThreeYY_ResetView();

    uint32_t now = HAL_GetTick();
    if ((now - tick) < frame_ms) return;
    tick = now;

    // вычислить текущую позицию
    int16_t x_orbit, y_orbit;
    calculate_orbiting_pos(&x_orbit, &y_orbit);

    // стереть прошлую "спрайт-область" кругом (достаточно, т.к. объект круглый)
    // радиус спрайта ~ r+2, берём запас
    if (has_prev) {
        dispcolor_FillCircle(prev_x_orbit + (int16_t)(radius_00), prev_y_orbit + (int16_t)(radius_00),
                             (int16_t)(radius_00 + 10), C_BG);
    }

    // нарисовать первый yin-yang
    int r = radius_00 - 15;
    yinyang_00_screen(x_orbit, y_orbit, angle_00, r);

    // второй раз “сдвинутый орбитальный угол” как в скетче (orbit_angle -=120)
    int saved_orbit = orbit_angle;
    orbit_angle = orbit_angle - 120;

    int16_t x2, y2;
    calculate_orbiting_pos(&x2, &y2);

    // (важно) не стираем тут отдельно: всё равно на следующем кадре всё перерисуем
    yinyang_00_screen(x2, y2, angle_00, r);

    orbit_angle = saved_orbit; // вернуть

    // обновить углы как в Arduino
    angle_00 += 10;
    if (angle_00 > 359) angle_00 = 0;

    orbit_angle += 2;
    if (orbit_angle > 359) orbit_angle = 0;

    // запомнить текущую позицию для стирания на следующем кадре
    prev_x_orbit = x_orbit;
    prev_y_orbit = y_orbit;
    has_prev = 1;

    dispcolor_Update();
}

