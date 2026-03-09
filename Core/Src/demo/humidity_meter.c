/*
 * humidity_meter.c
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#include "humidity_meter.h"

#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "main.h"       // HAL_GetTick()
#include "dispcolor.h"
#include "font.h"
#include "gfx_extra.h"  // dispcolor_DrawRoundRect (если нет - см. коммент ниже)

#define DEG2RAD 0.0174532925f

// состояние режима
static uint8_t  hm_inited = 0;
static uint32_t hm_tick = 0;
static const uint32_t hm_period_ms = 1000;

// pivot/геометрия как в скетче
static int16_t x = 120;
static int16_t y = 160;
static const int16_t radius = 40;
static const int16_t tickLength = 15;

// needle geometry
static float n1_x, n1_y, n2_x, n2_y, n3_x, n3_y, n4_x, n4_y;
static float n1_x_old, n1_y_old, n2_x_old, n2_y_old, n3_x_old, n3_y_old, n4_x_old, n4_y_old;

static float humidity = 20.0f;

static float angle_circle;
static float needleAngle;

static float scale_x_out, scale_y_out, scale_x_out_old, scale_y_out_old;
static float scale_x_in,  scale_y_in,  scale_x_in_old,  scale_y_in_old;

static float outArc_x, outArc_y;
//static float outArc_x_old, outArc_y_old;

static void HM_DrawTicks(void)
{
    int k = 216;
    do {
        angle_circle = (float)k * DEG2RAD;

        float edge_x1     = (float)x + (float)(radius + 84) * cosf(angle_circle);
        float edge_y1     = (float)y + (float)(radius + 84) * sinf(angle_circle);
        float edge_x1_out = (float)x + (float)(radius + 80 + tickLength) * cosf(angle_circle);
        float edge_y1_out = (float)y + (float)(radius + 80 + tickLength) * sinf(angle_circle);

        dispcolor_DrawLine((int16_t)edge_x1, (int16_t)edge_y1,
                           (int16_t)edge_x1_out, (int16_t)edge_y1_out,
                           MAGENTA);
        k += 3;
    } while (k < 330);
}

static void HM_SubArc(void)
{
    // В оригинале тут drawLine из точки в ту же точку (по сути рисуется пиксель).
    // Мы делаем FillCircle(r=0) или DrawPixel. Тут поставим пиксель.
    for (int k = 216; k < 330; k++) {
        angle_circle = (float)k * DEG2RAD;
        outArc_x = (float)x + (float)(radius + 69) * cosf(angle_circle);
        outArc_y = (float)y + (float)(radius + 69) * sinf(angle_circle);
        dispcolor_DrawPixel((int16_t)outArc_x, (int16_t)outArc_y, CYAN);
    }
    for (int k = 216; k < 330; k++) {
        angle_circle = (float)k * DEG2RAD;
        outArc_x = (float)x + (float)(radius + 68) * cosf(angle_circle);
        outArc_y = (float)y + (float)(radius + 68) * sinf(angle_circle);
        dispcolor_DrawPixel((int16_t)outArc_x, (int16_t)outArc_y, CYAN);
    }
}

static void HM_IntermediateArc(void)
{
    for (int k = 216; k < 330; k++) {
        angle_circle = (float)k * DEG2RAD;
        outArc_x = (float)x + (float)(radius + 40) * cosf(angle_circle);
        outArc_y = (float)y + (float)(radius + 40) * sinf(angle_circle);
        dispcolor_DrawPixel((int16_t)outArc_x, (int16_t)outArc_y, CYAN);
    }
}

static void HM_MakeMulticolorScale(void)
{
    int k = 216;
    angle_circle = (float)k * DEG2RAD;

    scale_x_out_old = (float)x + (float)(radius + 80) * cosf(angle_circle);
    scale_y_out_old = (float)y + (float)(radius + 80) * sinf(angle_circle);
    scale_x_in_old  = (float)x + (float)(radius + 80) * cosf(angle_circle);
    scale_y_in_old  = (float)y + (float)(radius + 80) * sinf(angle_circle);

    do {
        angle_circle = (float)k * DEG2RAD;

        scale_x_out = (float)x + (float)(radius + 80) * cosf(angle_circle);
        scale_y_out = (float)y + (float)(radius + 80) * sinf(angle_circle);
        scale_x_in  = (float)x + (float)(radius + 72) * cosf(angle_circle);
        scale_y_in  = (float)y + (float)(radius + 72) * sinf(angle_circle);

        uint16_t c;
        if (k > 128 && k < 250)       c = GREEN;
        else if (k > 250 && k < 300)  c = YELLOW;
        else if (k > 299)             c = ORANGE;
        else                          c = GREEN;

        // два треугольника как в Arduino
        dispcolor_FillTriangle((int16_t)scale_x_out_old, (int16_t)scale_y_out_old,
                               (int16_t)scale_x_out,     (int16_t)scale_y_out,
                               (int16_t)scale_x_in_old,  (int16_t)scale_y_in_old,
                               c);

        dispcolor_FillTriangle((int16_t)scale_x_out,     (int16_t)scale_y_out,
                               (int16_t)scale_x_in_old,  (int16_t)scale_y_in_old,
                               (int16_t)scale_x_in,      (int16_t)scale_y_in,
                               c);

        scale_x_out_old = scale_x_out;
        scale_y_out_old = scale_y_out;
        scale_x_in_old  = scale_x_in;
        scale_y_in_old  = scale_y_in;

        k += 4;
    } while (k < 330);
}

static void HM_DrawNeedle(void)
{
    // стереть старую (фон тут BLACK)
    dispcolor_DrawLine(x, y, (int16_t)n1_x_old, (int16_t)n1_y_old, BLACK);
    dispcolor_FillTriangle((int16_t)n1_x_old, (int16_t)n1_y_old,
                           (int16_t)n2_x_old, (int16_t)n2_y_old,
                           (int16_t)n3_x_old, (int16_t)n3_y_old,
                           BLACK);
    dispcolor_DrawLine(x, y, (int16_t)n4_x_old, (int16_t)n4_y_old, BLACK);

    needleAngle = (humidity * DEG2RAD * 1.8f) - 3.14f;

    n1_x = (float)x + (float)(radius + 60) * cosf(needleAngle);
    n1_y = (float)y + (float)(radius + 60) * sinf(needleAngle);

    n2_x = (float)x + (float)(radius + 50) * cosf(needleAngle - 0.05f);
    n2_y = (float)y + (float)(radius + 50) * sinf(needleAngle - 0.05f);

    n3_x = (float)x + (float)(radius + 50) * cosf(needleAngle + 0.05f);
    n3_y = (float)y + (float)(radius + 50) * sinf(needleAngle + 0.05f);

    n4_x = (float)x + (float)(radius - 20) * cosf(needleAngle + 3.14f);
    n4_y = (float)y + (float)(radius - 20) * sinf(needleAngle + 3.14f);

    // рисовать новую
    dispcolor_DrawLine(x, y, (int16_t)n1_x, (int16_t)n1_y, WHITE);
    dispcolor_FillTriangle((int16_t)n1_x, (int16_t)n1_y,
                           (int16_t)n2_x, (int16_t)n2_y,
                           (int16_t)n3_x, (int16_t)n3_y,
                           WHITE);
    dispcolor_DrawLine(x, y, (int16_t)n4_x, (int16_t)n4_y, WHITE);

    // pivot
    dispcolor_FillCircle(x, y, 6, BLACK);
    dispcolor_FillCircle(x, y, 2, WHITE);
    dispcolor_DrawCircle(x, y, 6, WHITE, 0);

    // remember
    n1_x_old = n1_x; n1_y_old = n1_y;
    n2_x_old = n2_x; n2_y_old = n2_y;
    n3_x_old = n3_x; n3_y_old = n3_y;
    n4_x_old = n4_x; n4_y_old = n4_y;
}

static void HM_NumericModule(void)
{
    // очистить область (как textColor(WHITE,BLACK))
    dispcolor_FillRectangle(85, 200, 155, 230, BLACK);

    dispcolor_printf(95, 206, FONTID_16F, WHITE, "%.0f", (double)humidity);
    dispcolor_printf(125, 206, FONTID_16F, WHITE, "%%");
}

void HumidityMeter_ResetView(void)
{
    hm_inited = 1;
    hm_tick = HAL_GetTick();

    dispcolor_FillScreen(BLACK);

    // рамка снизу (roundRect как в Arduino)
    // Если не хочешь gfx_extra, замени на dispcolor_DrawRectangle(80,195, 80+80,195+40, CYAN);
    dispcolor_DrawRoundRect(80, 195, 80, 40, 4, CYAN);

    x = 120; y = 160;

    HM_DrawTicks();
    HM_MakeMulticolorScale();
    HM_SubArc();
    HM_IntermediateArc();

    // init needle old points
    humidity = 20.0f;
    n1_x_old = (float)x; n1_y_old = (float)y;
    n2_x_old = (float)x; n2_y_old = (float)y;
    n3_x_old = (float)x; n3_y_old = (float)y;
    n4_x_old = (float)x; n4_y_old = (float)y;

    HM_DrawNeedle();
    HM_NumericModule();

    dispcolor_Update();
}

void HumidityMeter_Draw(void)
{
    if (!hm_inited) {
        HumidityMeter_ResetView();
    }

    uint32_t now = HAL_GetTick();
    if ((now - hm_tick) < hm_period_ms) return;
    hm_tick = now;

    // random(20,85) в Arduino: 20..84
    humidity = 20.0f + (float)(rand() % 65);

    HM_DrawNeedle();
    HM_IntermediateArc();   // как в оригинале: восстановить дугу под стрелкой
    HM_NumericModule();

    dispcolor_Update();
}

