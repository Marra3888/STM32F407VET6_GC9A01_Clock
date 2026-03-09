//------------------------------------------------------------------------------
// clock.c
//------------------------------------------------------------------------------

#include <stm32f4xx_hal.h>
#include <string.h>
#include <dispcolor.h>
#include <font.h>
#include <math.h>
#include "constants.h"
#include "clock.h"

// 0-none, 1-HH, 2-MM, 3-SS
static uint8_t s_editField = 0;
static uint8_t s_blink = 1;

void Clock_SetEdit(uint8_t editField, uint8_t blink)
{
    s_editField = editField;
    s_blink = blink ? 1 : 0;
}

static void DrawArrow(int16_t angle, uint8_t lineLen, uint8_t thick, uint16_t color)
{
    angle -= 90;
    float angleRad = (float)angle * PI / 180.0f;
    int x = (int)(cosf(angleRad) * lineLen + xC);
    int y = (int)(sinf(angleRad) * lineLen + yC);

    dispcolor_DrawLine2(xC, yC, x, y, color, thick);
}

static void DrawDigitalTime(uint8_t hour, uint8_t min, uint8_t sec,
                            uint16_t digitColor, uint16_t bgColor)
{
    // Цвет подсветки активного поля
    const uint16_t activeColor = YELLOW;

    // "мигание": если поле активно и blink=0 -> рисуем цветом фона (прячем)
    uint16_t hhColor = digitColor;
    uint16_t mmColor = digitColor;
    uint16_t ssColor = digitColor;

    if (s_editField == 1) hhColor = s_blink ? activeColor : bgColor;
    if (s_editField == 2) mmColor = s_blink ? activeColor : bgColor;
    if (s_editField == 3) ssColor = s_blink ? activeColor : bgColor;

    // Рисуем по частям, но X считаем по реальной ширине (ничего не разъезжается)
    int16_t x = 75;
    int16_t y = 80;

    x = dispcolor_printf(x, y, FONTID_16F, hhColor, "%02d", hour);
    x = dispcolor_DrawString(x, y, FONTID_16F, " : ", digitColor);
    x = dispcolor_printf(x, y, FONTID_16F, mmColor, "%02d", min);
    x = dispcolor_DrawString(x, y, FONTID_16F, " : ", digitColor);
    x = dispcolor_printf(x, y, FONTID_16F, ssColor, "%02d", sec);
}

void DrawClock(uint8_t hour, uint8_t min, uint8_t sec, uint8_t light, uint8_t secBubbles)
{
    uint16_t bgColor, riskColor, digitColor, arrowColor, secArcColor;

    if (light) {
        bgColor = WHITE;
        riskColor = digitColor = arrowColor = BLACK;
        secArcColor = MAGENTA;
    } else {
        bgColor = BLACK;
        riskColor = digitColor = arrowColor = WHITE;
        secArcColor = GREEN;
    }

    dispcolor_FillScreen(bgColor);

    // Риски на циферблате
    uint8_t radius1 = 119;
    for (uint16_t angle = 0; angle <= 360; angle += 6) {
        uint8_t riskSize;
        if (!(angle % 90))      riskSize = 13;
        else if (!(angle % 30)) riskSize = 10;
        else                    riskSize = 6;

        uint8_t radius2 = radius1 - riskSize;
        float angleRad = (float)angle * PI / 180.0f;

        int x1 = (int)(cosf(angleRad) * radius1 + xC);
        int y1 = (int)(sinf(angleRad) * radius1 + yC);
        int x2 = (int)(cosf(angleRad) * radius2 + xC);
        int y2 = (int)(sinf(angleRad) * radius2 + yC);

        dispcolor_DrawLine_Wu(x1, y1, x2, y2, riskColor);
    }

    // Цифры на циферблате
    dispcolor_DrawString(165, 30, FONTID_16F, "1", digitColor);
    dispcolor_DrawString(200, 63, FONTID_16F, "2", digitColor);
    dispcolor_DrawString(207, 106, FONTID_32F, "3", digitColor);
    dispcolor_DrawString(200, 160, FONTID_16F, "4", digitColor);
    dispcolor_DrawString(165, 193, FONTID_16F, "5", digitColor);
    dispcolor_DrawString(112, 195, FONTID_32F, "6", digitColor);
    dispcolor_DrawString(65, 193, FONTID_16F, "7", digitColor);
    dispcolor_DrawString(32, 160, FONTID_16F, "8", digitColor);
    dispcolor_DrawString(17, 106, FONTID_32F, "9", digitColor);
    dispcolor_DrawString(32, 63, FONTID_16F, "10", digitColor);
    dispcolor_DrawString(65, 30, FONTID_16F, "11", digitColor);
    dispcolor_DrawString(106, 14, FONTID_32F, "12", digitColor);

    // Электронные часы (с подсветкой активного поля)
    DrawDigitalTime(hour, min, sec, digitColor, bgColor);

    // Минутная стрелка
    DrawArrow((int16_t)(min * 6 + sec / 10), 100, 2, arrowColor);
    // Часовая стрелка
    DrawArrow((int16_t)(hour * 30 + min / 2), 50, 4, arrowColor);

    // Секундная дуга/пузырьки
    if (!sec) sec = 60;

    if (secBubbles) {
        int16_t startAngle = -90;
        int16_t endAngle = (int16_t)(sec * 6 - 90);

        for (int16_t angle = startAngle; angle <= endAngle; angle += 6) {
            float angleRad = (float)angle * PI / 180.0f;
            int x = (int)(cosf(angleRad) * 118 + xC);
            int y = (int)(sinf(angleRad) * 118 + yC);

            if (angle == endAngle) dispcolor_FillCircleWu(x, y, 4, secArcColor);
            else                   dispcolor_FillCircleWu(x, y, 3, secArcColor);
        }
    } else {
        dispcolor_DrawArc(xC, yC, 119, 0, (int16_t)(sec * 6), secArcColor, 2);
    }

    dispcolor_Update();
    HAL_Delay(37);
}

//==================== Clock5 ====================

static uint8_t  s_c5_inited = 0;
static int16_t  s_c5_last_ss = -1;

static int16_t  s_c5_osx = 120, s_c5_osy = 120;
static int16_t  s_c5_omx = 120, s_c5_omy = 120;
static int16_t  s_c5_ohx = 120, s_c5_ohy = 120;

void Clock5_Reset(void)
{
    s_c5_inited = 0;
    s_c5_last_ss = -1;
    s_c5_osx = s_c5_omx = s_c5_ohx = 120;
    s_c5_osy = s_c5_omy = s_c5_ohy = 120;
}

static void Clock5_CreateDial(uint16_t bgColor, uint16_t ringColor, uint16_t markColor, uint16_t outsideColor)
{
    const int xC5 = 120;
    const int yC5 = 120;

    dispcolor_FillScreen(bgColor);

    dispcolor_FillCircleWu(xC5, yC5, 120, outsideColor);
    dispcolor_FillCircleWu(xC5, yC5, 114, ringColor);
    dispcolor_FillCircleWu(xC5, yC5, 103, bgColor);

    // большие метки
    for (int i = 0; i < 360; i += 30) {
        float a = (float)(i - 90) * PI / 180.0f;
        float sx = cosf(a), sy = sinf(a);

        int x0 = (int)(sx * 114.0f + xC5);
        int y0 = (int)(sy * 114.0f + yC5);
        int x1 = (int)(sx * 100.0f + xC5);
        int y1 = (int)(sy * 100.0f + yC5);

        dispcolor_DrawLine_Wu(x0, y0, x1, y1, markColor);
    }

    // мелкие метки
    for (int i = 0; i < 360; i += 6) {
        float a = (float)(i - 90) * PI / 180.0f;
        float sx = cosf(a), sy = sinf(a);

        int x = (int)(sx * 102.0f + xC5);
        int y = (int)(sy * 102.0f + yC5);

        dispcolor_DrawPixel(x, y, markColor);

        if (i == 0 || i == 90 || i == 180 || i == 270) {
            dispcolor_FillCircleWu(x, y, 2, markColor);
        }
    }

    dispcolor_FillCircleWu(xC5, yC5, 3, markColor);
    dispcolor_Update();
}

void DrawClock5(uint8_t hour, uint8_t min, uint8_t sec)
{
    const int xC5 = 120;
    const int yC5 = 120;
    uint8_t forceRedraw = (s_editField != 0);

    // Тема (можно поменять)
    const uint16_t bgColor   = BLACK;
    const uint16_t outsideColor   = GREEN;
//    const uint16_t ringColor = RGB565(120, 0, 30); // BORDEAUX
    const uint16_t ringColor = BLUE;
    const uint16_t markColor = WHITE;

    const uint16_t hourColor = ORANGE;
    const uint16_t minColor  = ORANGE;
    const uint16_t secColor  = CYAN;

    if (!s_c5_inited) {
        s_c5_inited = 1;
        s_c5_last_ss = -1;
        s_c5_osx = s_c5_omx = s_c5_ohx = xC5;
        s_c5_osy = s_c5_omy = s_c5_ohy = yC5;

        Clock5_CreateDial(bgColor, ringColor, markColor, outsideColor);
    }

    // обновляем раз в секунду
    if (!forceRedraw && (int16_t)sec == s_c5_last_ss) {
        // но цифровое время уже нарисовали, можно обновить экран
        dispcolor_Update();
        HAL_Delay(40);
        return;
    }
    s_c5_last_ss = (int16_t)sec;

    float sdeg = (float)sec * 6.0f;
    float mdeg = (float)min * 6.0f + sdeg * 0.01666667f;
    float hdeg = (float)hour * 30.0f + mdeg * 0.0833333f;

    float ha = (hdeg - 90.0f) * PI / 180.0f;
    float ma = (mdeg - 90.0f) * PI / 180.0f;
    float sa = (sdeg - 90.0f) * PI / 180.0f;

    float hx = cosf(ha), hy = sinf(ha);
    float mx = cosf(ma), my = sinf(ma);
    float sx = cosf(sa), sy = sinf(sa);

    // стереть старые стрелки цветом фона (работает из-за однотонного центра)
    dispcolor_DrawLine2(xC5, yC5, s_c5_ohx, s_c5_ohy, bgColor, 4);
    dispcolor_DrawLine2(xC5, yC5, s_c5_omx, s_c5_omy, bgColor, 2);
    dispcolor_DrawLine2(xC5, yC5, s_c5_osx, s_c5_osy, bgColor, 1);

    dispcolor_FillCircleWu(xC5, yC5, 47, BORDEAUX);
    dispcolor_FillCircleWu(xC5, yC5, 44, CYAN);
    dispcolor_FillCircleWu(xC5, yC5, 41, GREY);
    dispcolor_FillCircleWu(xC5, yC5, 38, ORANGE);
    dispcolor_FillCircleWu(xC5, yC5, 35, WHITE);
    dispcolor_FillCircleWu(xC5, yC5, 32, MAGENTA);
    dispcolor_FillCircleWu(xC5, yC5, 29, BLUE);
    dispcolor_FillCircleWu(xC5, yC5, 26, GREEN);
    dispcolor_FillCircleWu(xC5, yC5, 23, RED);
    dispcolor_FillCircleWu(xC5, yC5, 20, BLACK);


    // новые точки
    s_c5_ohx = (int16_t)(hx * 62.0f + xC5);
    s_c5_ohy = (int16_t)(hy * 62.0f + yC5);
    s_c5_omx = (int16_t)(mx * 84.0f + xC5);
    s_c5_omy = (int16_t)(my * 84.0f + yC5);
    s_c5_osx = (int16_t)(sx * 90.0f + xC5);
    s_c5_osy = (int16_t)(sy * 90.0f + yC5);

    // нарисовать новые
    dispcolor_DrawLine2(xC5, yC5, s_c5_ohx, s_c5_ohy, hourColor, 4);
    dispcolor_DrawLine2(xC5, yC5, s_c5_omx, s_c5_omy, minColor,  2);
    dispcolor_DrawLine2(xC5, yC5, s_c5_osx, s_c5_osy, secColor,  1);

    dispcolor_FillCircleWu(xC5, yC5, 6, BORDEAUX);


    dispcolor_Update();
    HAL_Delay(40);
}
