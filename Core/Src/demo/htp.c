/*
 * htp.c
 *
 *  Created on: 2 янв. 2026 г.
 *      Author: Zver
 */


//------------------------------------------------------------------------------
// HTP mode: Temperature / Humidity / Pressure screen for dispcolor
// Uses FontID=24 for big цифры (f24f), FontID=16 for text (f16f)
//------------------------------------------------------------------------------

/*
 * htp.c
 *
 * HTP round UI with static layer + dirty + partial update rect.
 */

#include "htp.h"
#include <dispcolor.h>
#include <font.h>
#include <gc9a01a.h>
#include <math.h>
#include <stdio.h>

static uint8_t s_need_redraw = 1;
static HTP_Data s_data;

static HTP_Data s_prev = {0};
static uint8_t  s_dirty = 1;

#ifndef HTP_USE_STUB
#define HTP_USE_STUB 1
#endif

static inline float clampf(float x, float a, float b)
{
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

static int16_t TextWidth(uint8_t fontId, const char *s)
{
    int16_t w = 0;
    while (*s) {
        if (*s == '\n' || *s == '\r') { s++; continue; }
        w += font_GetCharWidth(font_GetFontStruct(fontId, *s));
        s++;
    }
    return w;
}

static uint8_t FontHeight(uint8_t fontId, const char *s, uint8_t fallback)
{
    while (*s) {
        if (*s != '\n' && *s != '\r') {
            return font_GetCharHeight(font_GetFontStruct(fontId, *s));
        }
        s++;
    }
    return fallback;
}

static void DrawCenteredInBox(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                              uint8_t fontId, char *txt,
                              uint16_t textColor, uint8_t approxHeight)
{
    int16_t w_txt = TextWidth(fontId, txt);
    uint8_t h_txt = FontHeight(fontId, txt, approxHeight);

    int16_t boxW = x1 - x0 + 1;
    int16_t boxH = y1 - y0 + 1;

    int16_t x = x0 + (boxW - w_txt) / 2;
    int16_t y = y0 + (boxH - (int16_t)h_txt) / 2;

    dispcolor_DrawString(x, y, fontId, txt, textColor);
}

// --------- API ----------
void HTP_Init(void)
{
    s_need_redraw = 1;
    s_data.valid = 0;

    s_prev.valid = 0;
    s_dirty = 1;
}

void HTP_ResetView(void)
{
    s_need_redraw = 1;
    s_prev.valid = 0;
    s_dirty = 1;
}

HTP_Data HTP_Read(void)
{
#if HTP_USE_STUB
    static float t = 23.5f;
    static float h = 45.0f;
    static float p = 1008.0f;

    t += 0.05f; if (t > 26.0f)   t = 23.0f;
    h += 0.10f; if (h > 70.0f)   h = 35.0f;
    p += 0.20f; if (p > 1018.0f) p = 1002.0f;

    HTP_Data d = { t, h, p, 1 };
    return d;
#else
    HTP_Data d = {0};
    return d;
#endif
}

void HTP_Draw_Round(uint8_t light)
{
    HTP_Data newd = HTP_Read();

// thresholds  --------------------------------------------------------
    const float DT = 0.05f;
    const float DH = 0.2f;
    const float DP = 0.3f;

// dirty check (skip if stable)
    if (s_need_redraw) {
        uint8_t changed = 0;
//        s_need_redraw = 1;


        if (newd.valid != s_prev.valid) {
            changed = 1;
        } else if (newd.valid) {
            float dt = newd.t_c   - s_prev.t_c;   if (dt < 0) dt = -dt;
            float dh = newd.rh    - s_prev.rh;    if (dh < 0) dh = -dh;
            float dp = newd.p_hpa - s_prev.p_hpa; if (dp < 0) dp = -dp;
            if (dt > DT || dh > DH || dp > DP) changed = 1;
        }

        if (!changed && !s_dirty) return;
    }

    s_data = newd;
    s_prev = newd;
    s_dirty = 0;

    const int xC = 120, yC = 120;
    const int R_outer = 118;
    const int R_inner = 104;

    uint16_t bg     = light ? WHITE : BLACK;
    uint16_t fg     = light ? BLACK : WHITE;
    uint16_t ringBg = light ? RGB565(230,230,230) : RGB565(30,30,30);

    uint16_t tC = light ? RED   : ORANGE;
    uint16_t hC = light ? BLUE  : CYAN;
    uint16_t pC = light ? GREEN : RGB565(0,200,80);

// --- STATIC layer ------------------------------------------------------------------------------------------------------
    if (s_need_redraw) {
//        s_need_redraw = 0;



//        GC9A01A_Update();
        // дальше рисуем динамику и отправляем rect
    }

// --- DYNAMIC layer -------------------------------------------------------------------------------------------------
    if (!s_data.valid) {
        dispcolor_FillRectangle(40, 108, 200, 132, bg);
        dispcolor_DrawString(60, 112, FONTID_16F, (char*)"NO SENSOR", RED);
        GC9A01A_UpdateRect(0, 0, 240, 195);
        return;
    }


		dispcolor_FillScreen(bg);
        dispcolor_FillCircleWu(xC, yC, R_outer, ringBg);
        dispcolor_FillCircleWu(xC, yC, R_inner, bg);


    float t_norm = (clampf(s_data.t_c, -10.0f, 40.0f) + 10.0f) / 50.0f;
    float h_norm = clampf(s_data.rh, 0.0f, 100.0f) / 100.0f;
    float p_norm = (clampf(s_data.p_hpa, 980.0f, 1050.0f) - 980.0f) / 70.0f;

    int16_t t_arc = (int16_t)(t_norm * 180.0f);
    int16_t h_arc = (int16_t)(h_norm * 180.0f);
    int16_t p_arc = (int16_t)(p_norm * 180.0f);

    // erase old arcs to avoid tails
    dispcolor_DrawArc(xC, yC, R_outer -  2, 180, 360, ringBg, 6);
    dispcolor_DrawArc(xC, yC, R_outer - 12, 180, 360, ringBg, 6);
    dispcolor_DrawArc(xC, yC, R_outer - 22, 180, 360, ringBg, 6);

    // draw new arcs
    dispcolor_DrawArc(xC, yC, R_outer -  2, 180, (int16_t)(180 + t_arc), tC, 6);
    dispcolor_DrawArc(xC, yC, R_outer - 12, 180, (int16_t)(180 + h_arc), hC, 6);
    dispcolor_DrawArc(xC, yC, R_outer - 22, 180, (int16_t)(180 + p_arc), pC, 6);




            dispcolor_DrawString(105, 18, FONTID_16F, (char*)"HTP", fg);

            const int gap = 8;
            const int xL0 = 25;
            const int xL1 = 120 - gap/2;
            const int xR0 = 120 + gap/2;
            const int xR1 = 215;
            const int y0 = 48;
            const int y1 = 86;

            dispcolor_FillRectangle(xL0, y0, xL1, y1, ringBg);
            dispcolor_DrawRectangle(xL0, y0, xL1, y1, fg);
            DrawCenteredInBox(xL0, y0 + 2, xL1, y0 + 18, FONTID_16F, (char*)"TEMP", fg, 16);

            dispcolor_FillRectangle(xR0, y0, xR1, y1, ringBg);
            dispcolor_DrawRectangle(xR0, y0, xR1, y1, fg);
            DrawCenteredInBox(xR0, y0 + 2, xR1, y0 + 18, FONTID_16F, (char*)"HUM", fg, 16);

            dispcolor_DrawRectangle(25, 92, 215, 166, fg);

    dispcolor_FillRectangle(xL0 + 1, y0 + 19, xL1 - 1, y1 - 1, ringBg);
    char t_small[24];
    snprintf(t_small, sizeof(t_small), "%.1f \xB0""C", (double)s_data.t_c);
    DrawCenteredInBox(xL0, y0 + 18, xL1, y1, FONTID_16F, t_small, tC, 16);

    dispcolor_FillRectangle(xR0 + 1, y0 + 19, xR1 - 1, y1 - 1, ringBg);
    char h_small[24];
    snprintf(h_small, sizeof(h_small), "%.1f %%", (double)s_data.rh);
    DrawCenteredInBox(xR0, y0 + 18, xR1, y1, FONTID_16F, h_small, hC, 16);

    dispcolor_FillRectangle(26, 93, 214, 165, ringBg);
    char t_big[24];
    snprintf(t_big, sizeof(t_big), "%.1f \xB0""C", (double)s_data.t_c);
    DrawCenteredInBox(25,  96, 215, 134, FONTID_32F, t_big, tC, 32);
    DrawCenteredInBox(25, 136, 215, 152, FONTID_16F, (char*)"TEMPERATURE", fg, 16);

    dispcolor_FillRectangle(68, 168, 170, 190, bg);
    char p_str[32];
    snprintf(p_str, sizeof(p_str), "P %.1f hPa", (double)s_data.p_hpa);
    DrawCenteredInBox(68, 168, 170, 190, FONTID_16F, p_str, pC, 16);

    GC9A01A_UpdateRect(0, 0, 240, 240);
}
