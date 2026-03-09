/*
 * hvac.c
 *
 *  Created on: 2 янв. 2026 г.
 *      Author: Zver
 */

#include "hvac.h"
#include <dispcolor.h>
#include <font.h>
#include <math.h>
#include <stdio.h>

static HvacState s;
static char* mode_str(HvacMode m);
static char* fan_str(FanMode f);

// Если у тебя нет ORANGE/CYAN — замени на RGB565(...)
#ifndef ORANGE
#define ORANGE RGB565(255,140,0)
#endif
#ifndef CYAN
#define CYAN   RGB565(0,255,255)
#endif

static char* mode_str(HvacMode m)
{
    switch (m) {
    case HVAC_AUTO: return "AUTO";
    case HVAC_COOL: return "COOL";
    case HVAC_HEAT: return "HEAT";
    default:        return "?";
    }
}

static char* fan_str(FanMode f)
{
    switch (f) {
    case FAN_AUTO: return "AUTO";
    case FAN_1:    return "1";
    case FAN_2:    return "2";
    case FAN_3:    return "3";
    default:       return "?";
    }
}

//static uint16_t mode_color(uint8_t light, HvacMode m)
//{
//    if (light) {
//        if (m == HVAC_COOL) return BLUE;
//        if (m == HVAC_HEAT) return RED;
//        return BLACK;
//    } else {
//        if (m == HVAC_COOL) return CYAN;
//        if (m == HVAC_HEAT) return ORANGE;
//        return WHITE;
//    }
//}

void Hvac_Init(void)
{
    s.setpointC = 23.0f;
    s.currentC  = 28.0f;
    s.mode      = HVAC_AUTO;
    s.fan       = FAN_AUTO;
    s.power     = 1;
}

const HvacState* Hvac_Get(void) { return &s; }

void Hvac_OnK0(void)
{
    // +0.5°C, диапазон 16..30
    s.setpointC += 0.5f;
    if (s.setpointC > 30.0f) s.setpointC = 16.0f;
}

void Hvac_OnK1(void)
{
    // смена режима
    if (s.mode == HVAC_AUTO) s.mode = HVAC_COOL;
    else if (s.mode == HVAC_COOL) s.mode = HVAC_HEAT;
    else s.mode = HVAC_AUTO;
}

void Hvac_OnCombo(void)
{
    // смена вентилятора
    if (s.fan == FAN_AUTO) s.fan = FAN_1;
    else if (s.fan == FAN_1) s.fan = FAN_2;
    else if (s.fan == FAN_2) s.fan = FAN_3;
    else s.fan = FAN_AUTO;
}

// простой “псевдо датчик”, чтобы currentC чуть менялась
static void simulate_current_temp(void)
{
    // тянем к уставке очень медленно
    float target = s.setpointC;
    float err = target - s.currentC;
    s.currentC += err * 0.01f; // медленно
}

static void DrawCenteredInBox(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                              uint8_t fontId, char *txt,
                              uint16_t textColor, uint16_t bgColor)
{
    // измеряем ширину: рисуем в bgColor на экране (потом все равно перерисуем этот участок)
    // Если не хочешь "лишнего" рисования — можно сделать отдельную функцию измерения ширины,
    // но у вас её нет, поэтому так.
    int16_t x_end = dispcolor_DrawString(0, 0, fontId, txt, bgColor);
    int16_t w_txt = x_end; // т.к. стартовали с X=0

    int16_t boxW = x1 - x0 + 1;
    int16_t boxH = y1 - y0 + 1;

    // по Y центрируем на глаз под шрифт: обычно baseline сверху, поэтому берем середину минус ~высота/2
    // Если нужно точнее — подберем константу.
    int16_t x = x0 + (boxW - w_txt) / 2;
    int16_t y = y0 + (boxH - 16) / 2; // для FONTID_16F высота ~16

    dispcolor_DrawString(x, y, fontId, txt, textColor);
}

void Hvac_Draw_Round(uint8_t light)
{
	simulate_current_temp();
    // Геометрия круглого дисплея 240x240
    const int xC = 120, yC = 120;
    const int R_outer = 118;
    const int R_inner = 104;

    // Цвета темы
    uint16_t bg = light ? WHITE : BLACK;
    uint16_t fg = light ? BLACK : WHITE;

    uint16_t ringBg = light ? RGB565(230,230,230) : RGB565(30,30,30);
    uint16_t coolC  = light ? BLUE : CYAN;
    uint16_t heatC  = light ? RED  : ORANGE;
    uint16_t autoC  = fg;

    uint16_t modeC = (s.mode == HVAC_COOL) ? coolC :
                     (s.mode == HVAC_HEAT) ? heatC : autoC;

    // Фон (можно и FillScreen, но круг смотрится аккуратнее)
    dispcolor_FillScreen(bg);

    // Кольцо (фон)
    dispcolor_FillCircleWu(xC, yC, R_outer, ringBg);
    dispcolor_FillCircleWu(xC, yC, R_inner, bg);

    // Дуга-статус: зависит от разницы setpoint-current (пример)
    float diff = s.setpointC - s.currentC; // >0 греть, <0 охлаждать
    if (diff > 5.0f) diff = 5.0f;
    if (diff < -5.0f) diff = -5.0f;

    float norm = (diff + 5.0f) / 10.0f;    // 0..1
    int16_t arc = (int16_t)(norm * 180.0f);

    // Рисуем дугу сверху (180..360). Толщина 6.
    dispcolor_DrawArc(xC, yC, R_outer - 1, 180, (int16_t)(180 + arc), modeC, 6);

    // Заголовок
    dispcolor_DrawString(92, 18, FONTID_16F, "HVAC", fg);

    // ===== Капсулы MODE и FAN (сдвинуты ближе друг к другу) =====
    const int gap = 8;
    const int xL0 = 25;
    const int xL1 = 120 - gap/2;
    const int xR0 = 120 + gap/2;
    const int xR1 = 215;

    const int y0 = 48;
    const int y1 = 80;

    // MODE
    dispcolor_FillRectangle(xL0, y0, xL1, y1, ringBg);
    dispcolor_DrawRectangle(xL0, y0, xL1, y1, fg);
//    dispcolor_DrawString(xL0 + 18, y0 + 2, FONTID_16F, "MODE", fg);
    DrawCenteredInBox(xL0, y0 + 3, xL1, 62, FONTID_16F, "MODE", fg, ringBg);
//    dispcolor_printf(xL0 + 8, y0 + 18, FONTID_16F, modeC, "%s", mode_str(s.mode));
    // область для значения внутри MODE рамки
    DrawCenteredInBox(xL0, 64, xL1, 80, FONTID_16F, mode_str(s.mode), modeC, ringBg);

    // FAN
    dispcolor_FillRectangle(xR0, y0, xR1, y1, ringBg);
    dispcolor_DrawRectangle(xR0, y0, xR1, y1, fg);
//    dispcolor_DrawString(xR0 + 8, y0 + 4, FONTID_16F, "FAN", fg);
    DrawCenteredInBox(xR0, y0 + 3, xR1, 62, FONTID_16F, "FAN",  fg, ringBg);
    //    dispcolor_printf(xR0 + 8, y0 + 18, FONTID_16F, fg, "%s", fan_str(s.fan));
    // область для значения внутри FAN рамки
    DrawCenteredInBox(xR0, 64, xR1, 80, FONTID_16F, fan_str(s.fan), fg, ringBg);

//    // ===== Центр: уставка =====
//    // Рамка вокруг центрального блока
//    dispcolor_DrawRectangle(25, 88, 215, 174, fg);
//
//    // Крупно setpoint
//    dispcolor_printf(45, 102, FONTID_32F, modeC, "%.1f \xB0""C", s.setpointC);
//    dispcolor_DrawString(74, 144, FONTID_16F, "SETPOINT", fg);
//
//    // Текущая температура ниже
//    dispcolor_printf(62, 165, FONTID_24F, fg, "NOW %.1f \xB0""C", s.currentC);
    // ===== Центр: уставка =====
    dispcolor_DrawRectangle(25, 88, 215, 164, fg);

    // готовим строки
    char s_set[24];
    char s_now[24];
    snprintf(s_set, sizeof(s_set), "%.1f \xB0""C", s.setpointC);
    snprintf(s_now, sizeof(s_now), "NOW %.1f \xB0""C", s.currentC);

    // зоны внутри рамки (разбили по высоте на 3 строки)
    DrawCenteredInBox(25,  80, 215, 118, FONTID_32F, s_set,      modeC, bg);
    DrawCenteredInBox(25, 119, 215, 135, FONTID_16F, "SETPOINT", fg,    bg);
    DrawCenteredInBox(25, 134, 215, 152, FONTID_24F, s_now,      fg,    bg);

    // ===== Подсказки снизу (сдвинуты ближе к центру) =====
    dispcolor_DrawString(19, 166, FONTID_16F, "K0:+ K1:MODE  K0+K1:FAN", fg);

    dispcolor_Update();
}
