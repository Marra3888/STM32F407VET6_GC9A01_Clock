/*
 * mono.c
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#include "mono.h"

#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "main.h"      // HAL_GetTick()
#include "dispcolor.h"

#define DEG2RAD 0.0174532925f

static uint8_t  mono_inited = 0;
static uint32_t mono_tick = 0;
static const uint32_t mono_period_ms = 2000;   // delay(2000)

static const int16_t xc = 120;
static const int16_t yc = 120;
static const int16_t radius = 100;

static float x_new = 125.0f;
static float y_new = 20.0f;
static float x_old = 125.0f;
static float y_old = 20.0f;

static float pieParameter = 0.0f;   // 0..100
static float angle_new = 0.0f;      // 0..360
static float angle_old = 0.0f;

static int iteration = 0;

static void Mono_DrawBase(void)
{
    dispcolor_FillScreen(GREY);
    dispcolor_DrawCircle(xc, yc, (int16_t)(radius + 8), CYAN, 0);

    // initial triangle (как в скетче) чтобы "прогреть"
    x_new = 125.0f; y_new = 20.0f;
    x_old = 125.0f; y_old = 20.0f;
    dispcolor_FillTriangle(xc, yc, (int16_t)x_old, (int16_t)y_old, (int16_t)x_new, (int16_t)y_new, GREEN);
}

static void Mono_DrawSectorGreen(float angle_deg)
{
    for (int j = 0; j < (int)angle_deg; j++) {
        float sliceAngle = ((float)j * DEG2RAD) - (90.0f * DEG2RAD);

        x_old = x_new;  y_old = y_new;
        x_new = (float)xc + (float)radius * cosf(sliceAngle);
        y_new = (float)yc + (float)radius * sinf(sliceAngle);

        dispcolor_FillTriangle(xc, yc,
                               (int16_t)x_new, (int16_t)y_new,
                               (int16_t)x_old, (int16_t)y_old,
                               GREEN);
    }
}

void Mono_ResetView(void)
{
    mono_inited = 1;
    mono_tick = HAL_GetTick();
    iteration = 0;

    Mono_DrawBase();

    pieParameter = (float)(rand() % 101);          // 0..100 (Arduino random(0,100) даёт 0..99, но так даже удобнее)
    angle_new = 360.0f * pieParameter / 100.0f;
    angle_old = angle_new;

    Mono_DrawSectorGreen(angle_new);

    dispcolor_Update();
}

void Mono_Draw(void)
{
    if (!mono_inited) {
        Mono_ResetView();
    }

    uint32_t now = HAL_GetTick();
    if ((now - mono_tick) < mono_period_ms) return;
    mono_tick = now;

    iteration++;

    pieParameter = (float)(rand() % 101);
    angle_new = 360.0f * pieParameter / 100.0f;

    if (angle_new <= angle_old) {
        // уменьшаем сектор: закрашиваем назад серым
        for (int j = (int)angle_old; j > (int)angle_new; j--) {
            float sliceAngle = ((float)j * DEG2RAD) - (90.0f * DEG2RAD);

            x_old = x_new;  y_old = y_new;
            x_new = (float)xc + (float)radius * cosf(sliceAngle);
            y_new = (float)yc + (float)radius * sinf(sliceAngle);

            dispcolor_FillTriangle(xc, yc,
                                   (int16_t)x_new, (int16_t)y_new,
                                   (int16_t)x_old, (int16_t)y_old,
                                   GREY);
        }
    } else {
        // увеличиваем сектор: дорисовываем зелёным
        for (int j = (int)angle_old; j < (int)angle_new; j++) {
            float sliceAngle = ((float)j * DEG2RAD) - (90.0f * DEG2RAD);

            x_old = x_new;  y_old = y_new;
            x_new = (float)xc + (float)radius * cosf(sliceAngle);
            y_new = (float)yc + (float)radius * sinf(sliceAngle);

            dispcolor_FillTriangle(xc, yc,
                                   (int16_t)x_new, (int16_t)y_new,
                                   (int16_t)x_old, (int16_t)y_old,
                                   GREEN);
        }
    }

    angle_old = angle_new;
    dispcolor_Update();
}

