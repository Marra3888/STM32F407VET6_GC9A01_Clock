/*
 * tachometer.c
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#include "tachometer.h"

#include <math.h>
#include <stdint.h>

#include "main.h"       // HAL_GetTick()
#include "dispcolor.h"
#include "font.h"
#include "gfx_extra.h"

#define TACHO_DEG2RAD 0.0174532925f

static uint8_t  tacho_inited = 0;
static uint32_t tacho_tick = 0;
static const uint32_t tacho_frame_ms = 200;   // как cycleTime в Arduino

static const float   t_center_x = 120.0f;
static const float   t_center_y = 120.0f;
static const int16_t t_radius   = 87;

static float t_edge_x = 0, t_edge_y = 0;
static float t_edge_x_out = 0, t_edge_y_out = 0;

static float t_needletail_x = 0, t_needletail_y = 0;
static float t_needletail_x_old = 0, t_needletail_y_old = 0;

static float t_needle_x = 0, t_needle_y = 0;
static float t_needle_x_old = 120.0f, t_needle_y_old = 120.0f;

static float t_angle_circle = 0;
static float t_angle_needle = 0;

static int t_iteration = 0;
static int t_stopper = 90;     // 90*2 = 180
static int t_v = 1;
static int t_speed = 0;

// фон у нас серый из твоей палитры
static const uint16_t T_BG = GREY;

static void Tachometer_DrawBigDial(void)
{
    dispcolor_FillScreen(T_BG);

    // outer rings
    dispcolor_DrawCircle((int16_t)t_center_x, (int16_t)t_center_y, (int16_t)(t_radius + 10), MAGENTA, 0);
    dispcolor_DrawCircle((int16_t)t_center_x, (int16_t)t_center_y, (int16_t)(t_radius + 8),  BLUE, 0);
    dispcolor_DrawCircle((int16_t)t_center_x, (int16_t)t_center_y, (int16_t)(t_radius + 7),  CYAN, 0);

    // scale markers each 6 degrees
    for (int j = 0; j < 360; j += 6) {
        t_angle_circle = (float)j * TACHO_DEG2RAD;

        t_edge_x     = t_center_x + ((float)t_radius * cosf(t_angle_circle));
        t_edge_y     = t_center_y + ((float)t_radius * sinf(t_angle_circle));
        t_edge_x_out = t_center_x + ((float)(t_radius + 8) * cosf(t_angle_circle));
        t_edge_y_out = t_center_y + ((float)(t_radius + 8) * sinf(t_angle_circle));

        dispcolor_DrawLine((int16_t)t_edge_x, (int16_t)t_edge_y, (int16_t)t_edge_x_out, (int16_t)t_edge_y_out, WHITE);
    }

    // major markers (0, 90, 180, 270)
    for (int j = 0; j < 271; j += 90) {
        t_angle_circle = (float)j * TACHO_DEG2RAD;

        t_edge_x     = t_center_x + ((float)(t_radius - 5) * cosf(t_angle_circle));
        t_edge_y     = t_center_y + ((float)(t_radius - 5) * sinf(t_angle_circle));
        t_edge_x_out = t_center_x + ((float)(t_radius - 1) * cosf(t_angle_circle));
        t_edge_y_out = t_center_y + ((float)(t_radius - 1) * sinf(t_angle_circle));

        dispcolor_FillCircle((int16_t)t_edge_x, (int16_t)t_edge_y, 3, GREEN);
        dispcolor_DrawLine((int16_t)t_edge_x, (int16_t)t_edge_y, (int16_t)t_edge_x_out, (int16_t)t_edge_y_out, GREEN);
    }

    // labels (6x8)
    dispcolor_printf((int16_t)(t_center_x - 110), (int16_t)(t_center_y - 5),   FONTID_6X8M, WHITE, "0");
    dispcolor_printf((int16_t)(t_center_x - 5),   (int16_t)(t_center_y - 111), FONTID_6X8M, WHITE, "50");
    dispcolor_printf((int16_t)(t_center_x + 100), (int16_t)(t_center_y - 5),   FONTID_6X8M, WHITE, "100");
    dispcolor_printf((int16_t)(t_center_x - 10),  (int16_t)(t_center_y + 104), FONTID_6X8M, WHITE, "150");
    dispcolor_printf((int16_t)(t_center_x - 100), (int16_t)(t_center_y + 50),  FONTID_6X8M, WHITE, "180");

    dispcolor_FillCircle((int16_t)(t_center_x - 70), (int16_t)(t_center_y + 50), 3, GREEN);

    // окно спидометра (у тебя нет roundRect — рисуем прямоугольник)
//    dispcolor_DrawRectangle((int16_t)(t_center_x - 100), (int16_t)(t_center_y + 5), (int16_t)(t_center_x - 100 + 50), (int16_t)(t_center_y + 5 + 22), CYAN);
    dispcolor_DrawRoundRect((int16_t)(t_center_x - 100), (int16_t)(t_center_y + 5), 50, 22, 3, CYAN);
}

static void Tachometer_Needle(void)
{
    if (t_speed > 180) t_speed = 180;
    if (t_speed < 0)   t_speed = 0;

    // erase old needle
    dispcolor_DrawLine((int16_t)t_needle_x_old, (int16_t)t_needle_y_old,
                       (int16_t)t_needletail_x_old, (int16_t)t_needletail_y_old, T_BG);

    // angle in radians (как в Arduino)
    t_angle_needle = ((float)t_speed * TACHO_DEG2RAD * 1.8f) - 3.14f;

    // head
    t_needle_x = t_center_x + ((float)(t_radius - 10) * cosf(t_angle_needle));
    t_needle_y = t_center_y + ((float)(t_radius - 10) * sinf(t_angle_needle));

    // tail (исправлено: в Arduino-скетче была опечатка center_x вместо center_y)
    t_needletail_x = t_center_x - ((float)(t_radius - 60) * cosf(t_angle_needle - 6.28f));
    t_needletail_y = t_center_y - ((float)(t_radius - 60) * sinf(t_angle_needle - 6.28f));

    // remember
    t_needle_x_old = t_needle_x;
    t_needle_y_old = t_needle_y;
    t_needletail_x_old = t_needletail_x;
    t_needletail_y_old = t_needletail_y;

    // draw new
    dispcolor_DrawLine((int16_t)t_needle_x, (int16_t)t_needle_y,
                       (int16_t)t_needletail_x, (int16_t)t_needletail_y, RED);

    // pivot
    dispcolor_FillCircle((int16_t)t_center_x, (int16_t)t_center_y, 6, T_BG);
    dispcolor_DrawCircle((int16_t)t_center_x, (int16_t)t_center_y, 6, RED, 0);
}

static void Tachometer_NumericDial(void)
{
    // clear area (x,y,w,h) -> FillRectangle(x1,y1,x2,y2)
    dispcolor_FillRectangle(
        (int16_t)(t_center_x - 93), (int16_t)(t_center_y + 9),
        (int16_t)(t_center_x - 93 + 40 - 1), (int16_t)(t_center_y + 9 + 16 - 1),
        T_BG
    );

    int16_t x = (t_speed > 99) ? (int16_t)(t_center_x - 93) : (int16_t)(t_center_x - 83);
    dispcolor_printf(x, (int16_t)(t_center_y + 9), FONTID_16F, YELLOW, "%d", t_speed);
}

void Tachometer_ResetView(void)
{
    tacho_inited = 1;
    tacho_tick = HAL_GetTick();

    t_iteration = 0;
    t_v = 1;
    t_speed = 0;

    // safe old coords
    t_needle_x_old = t_center_x;
    t_needle_y_old = t_center_y;
    t_needletail_x_old = t_center_x;
    t_needletail_y_old = t_center_y;

    Tachometer_DrawBigDial();
    Tachometer_Needle();
    Tachometer_NumericDial();
    dispcolor_Update();
}

void Tachometer_Draw(void)
{
    if (!tacho_inited) {
        Tachometer_ResetView();
    }

    uint32_t now = HAL_GetTick();
    if ((now - tacho_tick) < tacho_frame_ms) return;
    tacho_tick = now;

    // Arduino loop() logic
    if (t_iteration == 0) {
        t_v = +1;
    }

    t_iteration += t_v;
    if (t_iteration < 0) t_iteration = 0;

    t_speed = t_iteration * 2;

    if (t_iteration >= t_stopper) {
        t_v = -1;
    }

    Tachometer_Needle();
    Tachometer_NumericDial();
    dispcolor_Update();
}

