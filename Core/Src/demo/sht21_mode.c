/*
 * sht21_mode.c
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#include "sht21_mode.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "main.h"        // HAL_GetTick()
#include "dispcolor.h"
#include "font.h"
#include "gfx_extra.h"   // dispcolor_DrawRoundRect (если нет - см. ниже)

#define DEG2RAD 0.0174532925f

// ======= ВСТАВЬ СЮДА РЕАЛЬНОЕ ЧТЕНИЕ SHT21/HTU21D =======
// Сейчас — демо: температура “дышит” синусом.
static float SHT21_ReadTempC(void)
{
    // медленное "плавание" + небольшой шум
    static float t = 24.0f;

    // дрейф: -0.2..+0.2
    float drift = ((float)(rand() % 401) - 200.0f) / 1000.0f;

    // шум: -0.05..+0.05
    float noise = ((float)(rand() % 101) - 50.0f) / 1000.0f;

    t += drift + noise;

    // ограничим диапазон
    if (t < 18.0f) t = 18.0f;
    if (t > 32.0f) t = 32.0f;

    return t;
}
// =========================================================

static uint8_t  inited = 0;
static uint32_t tick = 0;
static const uint32_t period_ms = 2000;   // iterationTime

// Геометрия как в скетче (только экран у нас 240x240)
static const float center_x = 115.0f;
static const float center_y = 160.0f;
static const int16_t radius = 50;

static float edge_x, edge_y, edge_x_out, edge_y_out;
static float circleAngle;
static float needleAngle;

static float needle_x = 0;
static float needle_y = 0;
static float needle_x_old = 65.0f;
static float needle_y_old = 110.0f;

static float tempC = 20.0f;

static void DrawMarkers(void)
{
    int j = 180;
    do {
        circleAngle = (float)j * DEG2RAD;
        edge_x     = center_x + ((float)radius * cosf(circleAngle));
        edge_y     = center_y + ((float)radius * sinf(circleAngle));
        edge_x_out = center_x + ((float)(radius + 10) * cosf(circleAngle));
        edge_y_out = center_y + ((float)(radius + 10) * sinf(circleAngle));

        dispcolor_DrawLine((int16_t)edge_x, (int16_t)edge_y,
                           (int16_t)edge_x_out, (int16_t)edge_y_out,
                           YELLOW);
        j += 6;
    } while (j < 364);
}

static void Needle_Draw(void)
{
    // стереть старую стрелку
    dispcolor_DrawLine((int16_t)center_x, (int16_t)center_y,
                       (int16_t)needle_x_old, (int16_t)needle_y_old,
                       BLACK);

    // как в Arduino: needleAngle = temp*DEG2RAD*1.8 - 3.14
    needleAngle = (tempC * DEG2RAD * 1.8f) - 3.14f;

    needle_x = center_x + ((float)(radius - 4) * cosf(needleAngle));
    needle_y = center_y + ((float)(radius - 4) * sinf(needleAngle));

    needle_x_old = needle_x;
    needle_y_old = needle_y;

    // новая стрелка
    dispcolor_DrawLine((int16_t)center_x, (int16_t)center_y,
                       (int16_t)needle_x, (int16_t)needle_y,
                       YELLOW);

    // pivot
    dispcolor_FillCircle((int16_t)center_x, (int16_t)center_y, 5, YELLOW);
}

static void Text_Draw(void)
{
    // Заголовок
    dispcolor_DrawString(60, 66, FONTID_6X8M, (char*)"Temp:", WHITE);

    // Стереть старое значение (аналог fillRect(70,10,55,16))
    dispcolor_FillRectangle(125, 66, (int16_t)(150), (int16_t)(73), BLACK);

    // Печать температуры
    dispcolor_printf(125, 66, FONTID_6X8M, WHITE, "%.1f", (double)tempC);

    // Подписи шкалы
    dispcolor_printf(55, 165, FONTID_6X8M, WHITE, "0");
    dispcolor_printf(157, 165, FONTID_6X8M, WHITE, "100");
}

void SHT21Mode_ResetView(void)
{
    inited = 1;
    tick = HAL_GetTick();

    tempC = SHT21_ReadTempC();

    dispcolor_FillScreen(BLACK);

    // рамки как drawRoundRect
    // Если gfx_extra нет: замени на dispcolor_DrawRectangle(x1,y1,x2,y2,color)
    dispcolor_DrawRoundRect(52, 55, 126, 35, 6, YELLOW);
    dispcolor_DrawRoundRect(52, 93, 126, 85, 6, YELLOW);

    DrawMarkers();

    // init old needle pos to pivot to avoid длинной линии при первом стирании
    needle_x_old = center_x;
    needle_y_old = center_y;

    Needle_Draw();
    Text_Draw();

    dispcolor_Update();
}

void SHT21Mode_Draw(void)
{
    if (!inited) {
        SHT21Mode_ResetView();
    }

    uint32_t now = HAL_GetTick();
    if ((now - tick) < period_ms) return;
    tick = now;

    tempC = SHT21_ReadTempC();

    Needle_Draw();
    Text_Draw();

    dispcolor_Update();
}
