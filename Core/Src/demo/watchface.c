/*
 * photo_clock.c
 *
 *  Created on: 5 янв. 2026 г.
 *      Author: Zver
 */

#include "watchface.h"

#include <math.h>
#include <stdint.h>
#include <stm32f4xx_hal.h>

#include "dispcolor.h"
#include "font.h"
#include "constants.h"
#include "random_move.h"   // GetCurrentPos() демо-компас

#ifndef PI
#define PI 3.14159265358979323846f
#endif
#ifndef xC
#define xC 120
#endif
#ifndef yC
#define yC 120
#endif

extern RTC_HandleTypeDef hrtc;
static void rtc_get_time_date(RTC_DateTypeDef *d);



// ===================== стиль =====================
static const uint16_t BG      = BLACK;
static const uint16_t RING1   = 0x39E7;
static const uint16_t RING2   = 0x7BEF;
static const uint16_t TICK_M  = 0x7BEF;
static const uint16_t TICK_H  = 0xFFFF;

static const uint16_t SUB_BG  = 0x1082;
static const uint16_t SUB_R   = 0x7BEF;

static const uint16_t H_COL   = WHITE;
static const uint16_t M_COL   = WHITE;
static const uint16_t S_COL   = RED;

static const uint16_t DATE_BG = 0x6816;   // фиолетовый

// ===================== расположение кругов =====================
static const int16_t MS_X   = (int16_t)(xC - 60);
static const int16_t MS_Y   = (int16_t)(yC);
static const int16_t MS_R   = 24;

static const int16_t COMP_X = (int16_t)(xC);
static const int16_t COMP_Y = (int16_t)(yC - 60);
static const int16_t COMP_R = 18;

static const int16_t DATE_X = (int16_t)(xC + 60);
static const int16_t DATE_Y = (int16_t)(yC);
static const int16_t DATE_R = 26;

// ===================== состояние =====================
static uint8_t  s_inited = 0;
static int16_t  s_last_ss = -1;

typedef struct {
    int16_t ax, ay, bx, by, cx, cy;
    int16_t dx, dy;
    uint8_t valid;
} NeedlePoly;

static NeedlePoly old_hour = {0};
static NeedlePoly old_min  = {0};

static int16_t old_sx = xC, old_sy = yC;
static int16_t old_tx = xC, old_ty = yC;

static uint8_t s_last_date = 0;
static uint8_t s_last_wd   = 0;
static int16_t s_last_heading = -9999;

// ===================== утилиты =====================
static const char* wd_str(uint8_t wd)
{
    switch (wd) {
        case RTC_WEEKDAY_MONDAY:    return "Mon";
        case RTC_WEEKDAY_TUESDAY:   return "Tue";
        case RTC_WEEKDAY_WEDNESDAY: return "Wed";
        case RTC_WEEKDAY_THURSDAY:  return "Thu";
        case RTC_WEEKDAY_FRIDAY:    return "Fri";
        case RTC_WEEKDAY_SATURDAY:  return "Sat";
        case RTC_WEEKDAY_SUNDAY:    return "Sun";
        default: return "---";
    }
}

// ===================== фон: риски =====================
static void DrawTickRing(void)
{
    dispcolor_FillCircleWu(xC, yC, 120, BG);

    dispcolor_DrawCircle(xC, yC, 118, RING1, 0);
    dispcolor_DrawCircle(xC, yC, 117, RING1, 0);
    dispcolor_DrawCircle(xC, yC, 116, RING1, 0);

    const float r_out = 112.0f;
    for (int i = 0; i < 60; i++) {
        float deg = (float)i * 6.0f - 90.0f;
        float a = deg * PI / 180.0f;
        float ca = cosf(a), sa = sinf(a);

        float r_in = (i % 5 == 0) ? (r_out - 10.0f) : (r_out - 6.0f);

        int x1 = (int)(ca * r_in  + xC);
        int y1 = (int)(sa * r_in  + yC);
        int x2 = (int)(ca * r_out + xC);
        int y2 = (int)(sa * r_out + yC);

        uint16_t col = (i % 5 == 0) ? TICK_H : TICK_M;
        uint8_t thick = (i % 5 == 0) ? 2 : 1;

        dispcolor_DrawLine2((int16_t)x1, (int16_t)y1, (int16_t)x2, (int16_t)y2, col, thick);
    }

    dispcolor_DrawCircle(xC, yC, 104, RING2, 0);
}

static void DrawSubdialBase(int16_t cx, int16_t cy, int16_t r)
{
    dispcolor_FillCircleWu(cx, cy, r, SUB_BG);
    dispcolor_DrawCircle(cx, cy, r, SUB_R, 0);

    for (int i = 0; i < 12; i++) {
        float a = ((float)i * 30.0f - 90.0f) * PI / 180.0f;
        float ca = cosf(a), sa = sinf(a);
        int x1 = (int)(cx + ca * (r - 2));
        int y1 = (int)(cy + sa * (r - 2));
        int x2 = (int)(cx + ca * (r - 1));
        int y2 = (int)(cy + sa * (r - 1));
        dispcolor_DrawLine((int16_t)x1, (int16_t)y1, (int16_t)x2, (int16_t)y2, SUB_R);
    }
}

static void DrawCenterCap(void)
{
    dispcolor_FillCircleWu(xC, yC, 6, BLUE);
    dispcolor_DrawCircle(xC, yC, 6, RED, 0);
}

// ===================== клин-стрелки =====================
static NeedlePoly DrawNeedleTrapezoid(int16_t x0, int16_t y0,
                                      float ang_rad,
                                      int16_t len, int16_t tail,
                                      int16_t halfWidth,
                                      uint16_t color)
{
    NeedlePoly p = {0};

    float ux = cosf(ang_rad);
    float uy = sinf(ang_rad);
    float px = -uy;
    float py =  ux;

    float basex = (float)x0 - ux * (float)tail;
    float basey = (float)y0 - uy * (float)tail;

    float tipx = (float)x0 + ux * (float)len;
    float tipy = (float)y0 + uy * (float)len;

    float lx = basex + px * (float)halfWidth;
    float ly = basey + py * (float)halfWidth;
    float rx = basex - px * (float)halfWidth;
    float ry = basey - py * (float)halfWidth;

    float tipw = (float)halfWidth * 0.35f;
    float tlx = tipx + px * tipw;
    float tly = tipy + py * tipw;
    float trx = tipx - px * tipw;
    float try_ = tipy - py * tipw;

    dispcolor_FillTriangle((int16_t)lx, (int16_t)ly,
                           (int16_t)rx, (int16_t)ry,
                           (int16_t)tlx,(int16_t)tly,
                           color);

    dispcolor_FillTriangle((int16_t)rx, (int16_t)ry,
                           (int16_t)trx,(int16_t)try_,
                           (int16_t)tlx,(int16_t)tly,
                           color);

    p.ax=(int16_t)lx;  p.ay=(int16_t)ly;
    p.bx=(int16_t)rx;  p.by=(int16_t)ry;
    p.cx=(int16_t)tlx; p.cy=(int16_t)tly;
    p.dx=(int16_t)trx; p.dy=(int16_t)try_;
    p.valid = 1;
    return p;
}

static void EraseNeedleTrapezoid(const NeedlePoly *p, uint16_t bg)
{
    if (!p->valid) return;
    dispcolor_FillTriangle(p->ax, p->ay, p->bx, p->by, p->cx, p->cy, bg);
    dispcolor_FillTriangle(p->bx, p->by, p->dx, p->dy, p->cx, p->cy, bg);
}

// ===================== ms слева (статично) =====================
static void DrawMsCircle(void)
{
    DrawSubdialBase(MS_X, MS_Y, MS_R);
    dispcolor_printf((int16_t)(MS_X - 12), (int16_t)(MS_Y - 6), FONTID_6X8M, WHITE, "650");
    dispcolor_printf((int16_t)(MS_X -  8), (int16_t)(MS_Y + 6), FONTID_6X8M, WHITE, "ms");
}

// ===================== дата справа (реальная, фиолетовый круг) =====================
static void DrawDateCircle(uint8_t force)
{
    RTC_DateTypeDef d;
    rtc_get_time_date(&d);  //GetDate (правильный порядок)

    // защита от “нулевых” значений
    if (d.Date == 0 || d.Date > 31) {
        // если RTC не настроен или глюк — покажем заглушку
        dispcolor_FillCircleWu(DATE_X, DATE_Y, DATE_R, DATE_BG);
        dispcolor_DrawCircle(DATE_X, DATE_Y, DATE_R, WHITE, 0);
        dispcolor_printf(DATE_X - 12, DATE_Y - 2, FONTID_6X8M, WHITE, "RTC?");
        return;
    }

    if (!force && d.Date == s_last_date && d.WeekDay == s_last_wd) return;
    s_last_date = d.Date;
    s_last_wd   = d.WeekDay;

    // фиолетовый фон
    dispcolor_FillCircleWu(DATE_X, DATE_Y, DATE_R, DATE_BG);
    dispcolor_DrawCircle(DATE_X, DATE_Y, DATE_R, WHITE, 0);

    // подчистим область текста (чтобы не наслаивалось)
    dispcolor_FillRectangle((int16_t)(DATE_X - 18), (int16_t)(DATE_Y - 12),
                            (int16_t)(DATE_X + 18), (int16_t)(DATE_Y + 14),
                            DATE_BG);

    // текст
    dispcolor_printf((int16_t)(DATE_X - 12), (int16_t)(DATE_Y - 8), FONTID_6X8M, WHITE, "%s", wd_str(d.WeekDay));
    dispcolor_printf((int16_t)(DATE_X -  6), (int16_t)(DATE_Y + 4), FONTID_6X8M, WHITE, "%02u", d.Date);

}

// ===================== компас сверху (демо heading) =====================
static void DrawMiniCompass(int16_t heading_deg)
{
    DrawSubdialBase(COMP_X, COMP_Y, COMP_R);

    dispcolor_DrawString((int16_t)(COMP_X - 3), (int16_t)(COMP_Y - COMP_R + 2), FONTID_6X8M, (char*)"N", WHITE);
    dispcolor_DrawString((int16_t)(COMP_X - 3), (int16_t)(COMP_Y + COMP_R - 10), FONTID_6X8M, (char*)"S", WHITE);
    dispcolor_DrawString((int16_t)(COMP_X - COMP_R + 2), (int16_t)(COMP_Y - 4), FONTID_6X8M, (char*)"W", WHITE);
    dispcolor_DrawString((int16_t)(COMP_X + COMP_R - 8), (int16_t)(COMP_Y - 4), FONTID_6X8M, (char*)"E", WHITE);

    float aN = (float)(heading_deg - 90) * PI / 180.0f;
    float aS = (float)(heading_deg + 180 - 90) * PI / 180.0f;

    int xN = (int)(COMP_X + cosf(aN) * (COMP_R - 4));
    int yN = (int)(COMP_Y + sinf(aN) * (COMP_R - 4));
    int xS = (int)(COMP_X + cosf(aS) * (COMP_R - 6));
    int yS = (int)(COMP_Y + sinf(aS) * (COMP_R - 6));

    dispcolor_DrawLine(COMP_X, COMP_Y, (int16_t)xS, (int16_t)yS, BLUE);
    dispcolor_DrawLine(COMP_X, COMP_Y, (int16_t)xN, (int16_t)yN, RED);

    dispcolor_FillCircleWu(COMP_X, COMP_Y, 2, WHITE);
}

// ===================== создать watchface (один раз) =====================
static void Watchface_Create(void)
{
    dispcolor_FillScreen(BG);

    DrawTickRing();

    DrawMsCircle();

    // базовый круг компаса (стрелку дорисуем в Draw)
    DrawSubdialBase(COMP_X, COMP_Y, COMP_R);

    // дата фиолетовая (форс)
    s_last_date = 0;
    s_last_wd   = 0;
    DrawDateCircle(1);

    // чистый центр
    dispcolor_FillCircleWu(xC, yC, 10, WHITE);
    DrawCenterCap();

    dispcolor_Update();
}

// ===================== API =====================
void Watchface_Reset(void)
{
    s_inited = 0;
    s_last_ss = -1;

    old_hour.valid = 0;
    old_min.valid  = 0;

    old_sx = old_sy = xC;
    old_tx = xC;
    old_ty = yC;

    s_last_date = 0;
    s_last_wd   = 0;
    s_last_heading = -9999;
}

void Watchface_Draw(uint8_t hour, uint8_t min, uint8_t sec)
{
	 // init
	    if (!s_inited) {
	        s_inited = 1;
	        s_last_ss = -1;

	        old_hour.valid = 0;
	        old_min.valid  = 0;

	        old_sx = old_sy = xC;
	        old_tx = xC;
	        old_ty = yC;

	        s_last_heading = -9999;
	        s_last_date = 0;
	        s_last_wd   = 0;

	        Watchface_Create();
	    }

	    // обновляем раз в секунду
	    if ((int16_t)sec == s_last_ss) return;
	    s_last_ss = (int16_t)sec;

	    // heading (демо)
	    int16_t heading = (int16_t)GetCurrentPos(0, 360);
	    if (heading < 0) heading = 0;
	    if (heading > 359) heading = (int16_t)(heading % 360);

	    // ===== 1) стереть старые стрелки =====
	    EraseNeedleTrapezoid(&old_hour, BG);
	    EraseNeedleTrapezoid(&old_min,  BG);

	    dispcolor_DrawLine(xC, yC, old_sx, old_sy, BG);
	    dispcolor_DrawLine(xC, yC, old_tx, old_ty, BG);
	    dispcolor_DrawCircle(old_tx, old_ty, 4, BG, 0);
	    dispcolor_FillCircleWu(old_tx, old_ty, 3, BG);

	    // очистить центр
	    dispcolor_FillCircleWu(xC, yC, 10, BG);

	    // ===== 2) восстановить круги (чтобы секундная не "резала") =====
	    DrawMsCircle();

	    // компас (можно всегда, чтобы точно восстановить)
	    DrawMiniCompass(heading);
	    s_last_heading = heading;

	    // дата (форс, чтобы восстановить фиолетовый круг и текст после стираний)
	    DrawDateCircle(1);

	    // ===== 3) посчитать углы =====
	    float sdeg = (float)sec * 6.0f;
	    float mdeg = (float)min * 6.0f + sdeg / 60.0f;
	    float hdeg = (float)(hour % 12) * 30.0f + mdeg / 12.0f;

	    float sa = (sdeg - 90.0f) * PI / 180.0f;
	    float ma = (mdeg - 90.0f) * PI / 180.0f;
	    float ha = (hdeg - 90.0f) * PI / 180.0f;

	    // ===== 4) нарисовать новые час/мин =====
	    old_hour = DrawNeedleTrapezoid(xC, yC, ha, 58, 10, 6, H_COL);
	    old_min  = DrawNeedleTrapezoid(xC, yC, ma, 86, 12, 4, M_COL);

	    // ===== 5) секундная + противовес =====
	    int16_t sx = (int16_t)(cosf(sa) * 92.0f + xC);
	    int16_t sy = (int16_t)(sinf(sa) * 92.0f + yC);
	    dispcolor_DrawLine(xC, yC, sx, sy, S_COL);

	    int16_t tx = (int16_t)(cosf(sa + PI) * 22.0f + xC);
	    int16_t ty = (int16_t)(sinf(sa + PI) * 22.0f + yC);
	    dispcolor_DrawLine(xC, yC, tx, ty, S_COL);

	    dispcolor_DrawCircle(tx, ty, 4, S_COL, 0);
	    dispcolor_FillCircleWu(tx, ty, 2, WHITE);
	    dispcolor_DrawCircle(tx, ty, 2, S_COL, 0);

	    old_sx = sx; old_sy = sy;
	    old_tx = tx; old_ty = ty;

	    // центр поверх
	    DrawCenterCap();

    // ===== электронные часы по центру снизу =====
    {
        const int16_t ytxt = 192;
        // "HH:MM:SS" = 8 символов, FONTID_6X8M ~ 6px/символ
        const int16_t char_w = 6;
        const int16_t text_w = (int16_t)(8 * char_w);
        const int16_t xtxt = (int16_t)(xC - text_w / 2);

        // очистить область под строкой
        dispcolor_FillRectangle((int16_t)(xtxt - 2), (int16_t)(ytxt - 2),
                                (int16_t)(xtxt + text_w + 1), (int16_t)(ytxt + 9),
                                WHITE);

        dispcolor_printf(xtxt, ytxt, FONTID_6X8M, RED, "%02u:%02u:%02u", hour, min, sec);
    }


    dispcolor_Update();
}

static void rtc_get_time_date(RTC_DateTypeDef *d)
{
	RTC_TimeTypeDef t; // локально, наружу не отдаём
    HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, d, RTC_FORMAT_BIN); // обязательно после GetTime
}

void GetTimeRTC(uint8_t *hh, uint8_t *mm, uint8_t *ss)
{
    RTC_TimeTypeDef tnow;
    RTC_DateTypeDef dnow;
    HAL_RTC_GetTime(&hrtc, &tnow, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &dnow, RTC_FORMAT_BIN);
    if (tnow.Hours >= 24) tnow.Hours = 0;
    *hh = tnow.Hours;
    *mm = tnow.Minutes;
    *ss = tnow.Seconds;
}
