/*
 * smooth_clock.c
 *
 *  Created on: 6 янв. 2026 г.
 *      Author: Zver
 */

#include "smooth_clock.h"
#include <math.h>
#include <stdint.h>
#include <stm32f4xx_hal.h>
#include "main.h"
#include "dispcolor.h"
#include "font.h"
#include "constants.h"

#ifndef PI
#define PI 3.14159265358979323846f
#endif

extern RTC_HandleTypeDef hrtc;

// Центр экрана
static const int16_t CX = 120;
static const int16_t CY = 120;

// Цвета
static const uint16_t FACE_BG   = BLACK;
static const uint16_t RING_COL  = 0x39E7;   // dark grey
static const uint16_t TICK_COL  = 0x7BEF;   // grey
static const uint16_t DIG_COL   = WHITE;

static const uint16_t HAND_COL  = 0x867D;   // "skyblue-ish"
static const uint16_t SEC_COL   = RED;
static const uint16_t PIVOT_COL = WHITE;

// Геометрия
static const float R = 118.0f;
static const float H_LEN = 62.0f;
static const float M_LEN = 88.0f;
static const float S_LEN = 102.0f;

// Углы
static const float SECOND_ANGLE = 360.0f / 60.0f;
static const float MINUTE_ANGLE = (360.0f / 60.0f) / 60.0f;
static const float HOUR_ANGLE   = MINUTE_ANGLE / 12.0f;

static uint8_t  inited = 0;
static uint32_t tick = 0;
static const uint32_t frame_ms = 100;

// Временные переменные
static float time_secs = 0.0f;
static uint32_t last_ms = 0;
static uint8_t last_rtc_sec = 255;

// Старые координаты (для точечного стирания)
static int16_t ohx = CX, ohy = CY;
static int16_t omx = CX, omy = CY;
static int16_t osx = CX, osy = CY;

// ---------------- helpers ----------------
static void getCoord(int16_t x, int16_t y, float *xp, float *yp, float r, float a_deg)
{
    float a = (a_deg - 90.0f) * PI / 180.0f;
    *xp = cosf(a) * r + (float)x;
    *yp = sinf(a) * r + (float)y;
}

static void RTC_GetSecondsFloat(float *out_secs)
{
    RTC_TimeTypeDef t;
    RTC_DateTypeDef d;

    HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);

    uint32_t now = HAL_GetTick();
    if (last_ms == 0) last_ms = now;

    float base = (float)t.Hours * 3600.0f + (float)t.Minutes * 60.0f + (float)t.Seconds;

    if (t.Seconds != last_rtc_sec) {
        last_rtc_sec = t.Seconds;
        time_secs = base;
        last_ms = now;
    } else {
        float dt = (float)(now - last_ms) / 1000.0f;
        last_ms = now;
        time_secs = base + dt;
    }

    if (time_secs >= 86400.0f) time_secs = 0.0f;
    *out_secs = time_secs;
}

// Эта функция рисует ТОЛЬКО статичные элементы (риски и цифры).
// Она НЕ очищает экран (нет FillScreen).
// Она нужна, чтобы "залечить" дырки, оставленные стиранием стрелок.
static void DrawStaticElements(void)
{
    // 1. Риски
    for (int i = 0; i < 60; i++) {
        // Оптимизация: можно не считать sin/cos каждый раз, а использовать таблицу,
        // но на F4 с FPU это и так быстро.
        float deg = (float)i * 6.0f;
        float a = (deg - 90.0f) * PI / 180.0f;
        float ca = cosf(a), sa = sinf(a);

        float r_out = R;
        float r_in  = (i % 5 == 0) ? (R - 14.0f) : (R - 8.0f);

        int16_t x1 = (int16_t)(CX + ca * r_in);
        int16_t y1 = (int16_t)(CY + sa * r_in);
        int16_t x2 = (int16_t)(CX + ca * r_out);
        int16_t y2 = (int16_t)(CY + sa * r_out);

        uint8_t thick = (i % 5 == 0) ? 2 : 1;
        dispcolor_DrawLine2(x1, y1, x2, y2, TICK_COL, thick);
    }

    // 2. Цифры 1..12
    for (int h = 1; h <= 12; h++) {
        float xp, yp;
        getCoord(CX, CY, &xp, &yp, (R - 28.0f), (float)h * 30.0f);

        int16_t x = (int16_t)xp;
        int16_t y = (int16_t)yp;

        if (h >= 10) x -= 6;
        else         x -= 3;
        y -= 4;

        // Рисуем цифру поверх всего
        dispcolor_printf(x, y, FONTID_6X8M, DIG_COL, "%d", h);
    }
}

// Полная перерисовка циферблата (при старте)
static void DrawDial(void)
{
    dispcolor_FillScreen(FACE_BG);

    // Внешние кольца
    dispcolor_DrawCircle(CX, CY, (int16_t)R, RING_COL, 0);
    dispcolor_DrawCircle(CX, CY, (int16_t)(R - 1), RING_COL, 0);

    // Рисуем содержимое
    DrawStaticElements();

    dispcolor_Update();
}

// Умное стирание стрелок
static void EraseHands(void)
{
    // 1. Стираем стрелки по их СТАРЫМ координатам цветом ФОНА.
    // Толщину берем такую же или чуть больше (например, 7 вместо 6), чтобы убрать весь antialiasing мусор.

    // Минутная (была толщиной 6)
    dispcolor_DrawLine2(CX, CY, omx, omy, FACE_BG, 7);
    // Часовая (была толщиной 6)
    dispcolor_DrawLine2(CX, CY, ohx, ohy, FACE_BG, 7);
    // Секундная (была 1) - стираем толщиной 3 для надежности
    dispcolor_DrawLine2(CX, CY, osx, osy, FACE_BG, 3);

    // В этот момент у нас на экране черные линии, которые могли "разрезать" цифры и риски.

    // 2. ВОССТАНАВЛИВАЕМ статику поверх черных линий
    // Это закроет разрезы на цифрах и рисках.
    DrawStaticElements();

    // 3. Чистим центр (пивот) кружком, чтобы там не было каши из линий
    dispcolor_FillCircleWu(CX, CY, 6, FACE_BG);
}

static void DrawHands(float t)
{
    float h_angle = t * HOUR_ANGLE;
    float m_angle = t * MINUTE_ANGLE;
    float s_angle = t * SECOND_ANGLE;

    float xp, yp;

    // --- Новые координаты ---

    // Минутная
    getCoord(CX, CY, &xp, &yp, M_LEN, m_angle);
    omx = (int16_t)xp; omy = (int16_t)yp; // Сохраняем для следующего стирания
    dispcolor_DrawLine2(CX, CY, omx, omy, HAND_COL, 6);
    dispcolor_DrawLine2(CX, CY, omx, omy, FACE_BG, 2); // Прорезь

    // Часовая
    getCoord(CX, CY, &xp, &yp, H_LEN, h_angle);
    ohx = (int16_t)xp; ohy = (int16_t)yp;
    dispcolor_DrawLine2(CX, CY, ohx, ohy, HAND_COL, 6);
    dispcolor_DrawLine2(CX, CY, ohx, ohy, FACE_BG, 2); // Прорезь

    // Секундная
    getCoord(CX, CY, &xp, &yp, S_LEN, s_angle);
    osx = (int16_t)xp; osy = (int16_t)yp;
    dispcolor_DrawLine(CX, CY, osx, osy, SEC_COL);

    // Пивот
    dispcolor_FillCircleWu(CX, CY, 4, PIVOT_COL);
}

void SmoothClock_ResetView(void)
{
    inited = 1;
    tick = HAL_GetTick();

    last_ms = 0;
    last_rtc_sec = 255;
    time_secs = 0.0f;

    // Сброс координат в центр, чтобы первое стирание было корректным
    ohx = omx = osx = CX;
    ohy = omy = osy = CY;

    DrawDial();
}

void SmoothClock_Draw(void)
{
    if (!inited) SmoothClock_ResetView();

    uint32_t now = HAL_GetTick();
    if ((now - tick) < frame_ms) return;
    tick = now;

    float t;
    RTC_GetSecondsFloat(&t);

    // Алгоритм:
    // 1. Стереть старые стрелки (линиями)
    // 2. Восстановить цифры/риски, которые могли быть задеты
    // 3. Нарисовать новые стрелки
    EraseHands();
    DrawHands(t);

    dispcolor_Update();
}
