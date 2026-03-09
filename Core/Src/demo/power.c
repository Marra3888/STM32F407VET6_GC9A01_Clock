/*
 * power.c
 *
 *  Created on: 2 янв. 2026 г.
 *      Author: Zver
 */

/*
 * power.c
 *
 * ModePower: circular dial with dots + thin GREY bridges + W and kWh windows + pulse indicator
 * Virtual pulses, 1600 pulses/kWh (no real interrupt input).
 *
 * Dots style:
 *   - ALL dots are "rings" (outer color + inner bg), so grey and red look identical in shape.
 * Bridges:
 *   - thin GREY only (never colored)
 */

#include "power.h"

#include <dispcolor.h>
#include <font.h>
#include <gc9a01a.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"

// use text width helper without changing dispcolor.h
extern int16_t dispcolor_GetTextWidth(uint8_t FontID, char *Str);

#define POWER_PULSES_PER_KWH 1600.0f
#define POWER_USE_STUB 1

#define DEG2RAD 0.0174532925f

#define CX 120
#define CY 120

#define R_SPOT 96
#define DOT_R  5

#define ANG_START 20
#define ANG_END   350
#define ANG_STEP  10

#define BR_THICK  2

#define PULSE_X CX
#define PULSE_Y (CY - 100)
#define PULSE_R 6

#define WBOX_X (CX - 40)
#define WBOX_Y (CY - 49)
#define WBOX_W 80
#define WBOX_H 35

#define KBOX_X (CX - 50)
#define KBOX_Y (CY + 20)
#define KBOX_W 105
#define KBOX_H 35

// mode label between boxes
#define MODE_X (CX - 18)
#define MODE_Y (CY + 2)
#define MODE_W 36
#define MODE_H 16

#define UPD_X 0
#define UPD_Y 0
#define UPD_W 240
#define UPD_H 240

#define SIM_SWITCH_MS 15000u

typedef enum { SIM_SIN=0, SIM_RANDOM=1, SIM_FIXED=2 } SimMode;

static uint8_t s_need_redraw = 1;
static uint8_t s_dirty = 1;

static uint32_t s_pulse_count = 0;
static uint8_t  s_pulse_seen = 0;

static uint32_t s_last_pulse_ms = 0;
static float    s_watt = 0.0f;
static float    s_kwh  = 0.0f;

static uint8_t  s_pulse_color_state = 0;

// virtual generator
static float    s_watt_target = 650.0f;  // smoothed used for pulses
static float    s_watt_goal   = 650.0f;  // mode goal
static uint32_t s_next_pulse_ms = 0;

static SimMode  s_sim_mode = SIM_SIN;
static uint32_t s_sim_next_switch = 0;

// random mode
static uint32_t s_rand_next_ms = 0;
static float    s_rand_hold_w = 0.0f;

// sin mode
static uint32_t s_sin_t0 = 0;

// fixed mode
static float    s_fixed_w = 650.0f;

static inline float clampf(float x, float a, float b)
{
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

void Power_Init(void)
{
    s_need_redraw = 1;
    s_dirty = 1;

    s_pulse_count = 0;
    s_pulse_seen = 0;

    s_last_pulse_ms = 0;
    s_watt = 0.0f;
    s_kwh  = 0.0f;

    s_pulse_color_state = 0;

    s_watt_target = 650.0f;
    s_watt_goal   = 650.0f;
    s_next_pulse_ms = 0;

    s_sim_mode = SIM_SIN;
    s_sim_next_switch = 0;

    s_rand_next_ms = 0;
    s_rand_hold_w = 0.0f;

    s_sin_t0 = 0;
    s_fixed_w = 650.0f;
}

void Power_ResetView(void)
{
    s_need_redraw = 1;
    s_dirty = 1;
}

void Power_OnPulse(void)
{
    s_pulse_count++;
    s_pulse_seen = 1;
}

static void BuildDotList(int16_t *px, int16_t *py, int n)
{
    int idx = 0;
    for (int a = ANG_START; a < ANG_END && idx < n; a += ANG_STEP, idx++) {
        float rad = (float)(a - 90) * DEG2RAD;
        px[idx] = (int16_t)(cosf(rad) * R_SPOT + CX);
        py[idx] = (int16_t)(sinf(rad) * R_SPOT + CY);
    }
}

static void DrawStaticDial(uint16_t bg)
{
    dispcolor_FillScreen(bg);

    // windows
    dispcolor_DrawRectangle(WBOX_X, WBOX_Y, WBOX_X + WBOX_W, WBOX_Y + WBOX_H, GREEN);
    dispcolor_DrawRectangle(KBOX_X, KBOX_Y, KBOX_X + KBOX_W, KBOX_Y + KBOX_H, GREEN);

    // labels
    dispcolor_DrawString(CX - 18, CY + 57, FONTID_16F, (char*)"Watt", GREEN);
    dispcolor_DrawString(CX - 12, CY - 72, FONTID_16F, (char*)"kWh",  GREEN);

    const int N = (ANG_END - ANG_START) / ANG_STEP; // 33
    int16_t px[N], py[N];
    BuildDotList(px, py, N);

    // grey bridges
    for (int i = 0; i < N - 1; i++) {
        dispcolor_DrawLine2(px[i], py[i], px[i + 1], py[i + 1], GREY, BR_THICK);
    }

    // grey filled dots
    for (int i = 0; i < N; i++) {
        dispcolor_FillCircle(px[i], py[i], DOT_R, GREY);
    }

    // pulse indicator (grey ring)
    dispcolor_FillCircle(PULSE_X, PULSE_Y, PULSE_R, GREY);
    dispcolor_FillCircle(PULSE_X, PULSE_Y, PULSE_R - 1, bg);
}

static void DrawPowerScale(float watt, uint16_t bg)
{
    const int N = (ANG_END - ANG_START) / ANG_STEP;
    int16_t px[N], py[N];
    BuildDotList(px, py, N);

    // base: grey bridges
    for (int i = 0; i < N - 1; i++) {
        dispcolor_DrawLine2(px[i], py[i], px[i + 1], py[i + 1], GREY, BR_THICK);
    }

    // base: all dots grey
    for (int i = 0; i < N; i++) {
        dispcolor_FillCircle(px[i], py[i], DOT_R, GREY);
    }

    // active length
    const float W_MAX = 1475.0f;
    float norm = watt / W_MAX;
    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;

    int redDots = (int)(norm * (float)N + 0.5f);
    if (redDots < 0) redDots = 0;
    if (redDots > N) redDots = N;

    // active red dots
    for (int i = 0; i < redDots; i++) {
        dispcolor_FillCircle(px[i], py[i], DOT_R, RED);
    }
}

static void DrawPulseIndicator(uint16_t bg)
{
    s_pulse_color_state++;
    if (s_pulse_color_state >= 3) s_pulse_color_state = 0;

    uint16_t c = bg;
    if (s_pulse_color_state == 1) c = GREEN;
    else if (s_pulse_color_state == 2) c = MAGENTA;

    dispcolor_FillCircle(PULSE_X, PULSE_Y, PULSE_R - 1, c);
}

static void DrawModeLabel(uint16_t bg)
{
    const char *m = (s_sim_mode == SIM_SIN) ? "SIN" : (s_sim_mode == SIM_RANDOM) ? "RND" : "FIX";

    // clear area
    dispcolor_FillRectangle(MODE_X, MODE_Y, MODE_X + MODE_W, MODE_Y + MODE_H, bg);

    // center by text width
    int16_t mw = dispcolor_GetTextWidth(FONTID_16F, (char*)m);
    int16_t mx = MODE_X + (MODE_W - mw) / 2;

    dispcolor_DrawString(mx, MODE_Y, FONTID_16F, (char*)m, GREEN);
}

static void DrawNumbers(uint16_t bg)
{
    // clear inside windows
    dispcolor_FillRectangle(WBOX_X + 2, WBOX_Y + 2, WBOX_X + WBOX_W - 2, WBOX_Y + WBOX_H - 2, bg);
    dispcolor_FillRectangle(KBOX_X + 2, KBOX_Y + 2, KBOX_X + KBOX_W - 2, KBOX_Y + KBOX_H - 2, bg);

    char wbuf[16];
    snprintf(wbuf, sizeof(wbuf), "%.0f", (double)s_watt);

    char kbuf[24];
    snprintf(kbuf, sizeof(kbuf), "%.3f", (double)s_kwh);

    // center X inside frames
    int16_t ww = dispcolor_GetTextWidth(FONTID_16F, wbuf);
    int16_t kw = dispcolor_GetTextWidth(FONTID_16F, kbuf);

    int16_t wx = WBOX_X + (WBOX_W - ww) / 2;
    int16_t wy = WBOX_Y + (WBOX_H - 16) / 2;

    int16_t kx = KBOX_X + (KBOX_W - kw) / 2;
    int16_t ky = KBOX_Y + (KBOX_H - 16) / 2;

    dispcolor_DrawString(wx, wy, FONTID_16F, wbuf, YELLOW);
    dispcolor_DrawString(kx, ky, FONTID_16F, kbuf, YELLOW);

    DrawModeLabel(bg);
}

static void Sim_UpdateTarget(uint32_t now)
{
    // exact 15s tick (no drift)
    if (s_sim_next_switch == 0) s_sim_next_switch = now + SIM_SWITCH_MS;

    if ((int32_t)(now - s_sim_next_switch) >= 0) {
        s_sim_mode = (SimMode)(((int)s_sim_mode + 1) % 3);  // SIN->RND->FIX->SIN
        s_sim_next_switch += SIM_SWITCH_MS;

        // reset per-mode timers
        s_sin_t0 = now;
        s_rand_next_ms = 0;
        s_next_pulse_ms = 0;

        s_dirty = 1;
    }

    // GOAL
    if (s_sim_mode == SIM_SIN) {
        if (s_sin_t0 == 0) s_sin_t0 = now;
        float phase = (float)(now - s_sin_t0) * 0.0002f;
        s_watt_goal = 750.0f + 650.0f * sinf(phase);
        s_watt_goal = clampf(s_watt_goal, 0.0f, 2000.0f);
    } else if (s_sim_mode == SIM_RANDOM) {
        if (s_rand_next_ms == 0) {
            s_rand_next_ms = now + 1200u;
            s_rand_hold_w = 200.0f + (float)(rand() % 1401);
            s_dirty = 1;
        }
        if ((int32_t)(now - s_rand_next_ms) >= 0) {
            s_rand_next_ms = now + 1200u;
            s_rand_hold_w = 200.0f + (float)(rand() % 1401);
            s_dirty = 1;
        }
        s_watt_goal = s_rand_hold_w;
    } else {
        s_watt_goal = s_fixed_w;
    }

    // asymmetric smoothing
    float diff = s_watt_goal - s_watt_target;
    const float alpha_up   = 0.18f;
    const float alpha_down = 0.05f;
    float alpha = (diff >= 0.0f) ? alpha_up : alpha_down;

    s_watt_target += diff * alpha;
    s_watt_target = clampf(s_watt_target, 0.0f, 2000.0f);
}

static void Sim_GeneratePulse(uint32_t now)
{
    if (s_watt_target < 1.0f) {
        s_next_pulse_ms = now + 1000u;
        return;
    }

    uint32_t period_ms = (uint32_t)(3600000000.0f / (s_watt_target * POWER_PULSES_PER_KWH));
    if (period_ms < 30u) period_ms = 30u;

    if (s_next_pulse_ms == 0) s_next_pulse_ms = now + period_ms;

    if ((int32_t)(now - s_next_pulse_ms) >= 0) {
        Power_OnPulse();
        s_next_pulse_ms = now + period_ms;
    }
}

void Power_Draw(uint8_t light)
{
    uint16_t bg = light ? WHITE : BLACK;
    uint32_t now = HAL_GetTick();

#if POWER_USE_STUB
    Sim_UpdateTarget(now);
    Sim_GeneratePulse(now);
#endif

    if (s_pulse_seen) {
        s_pulse_seen = 0;

        uint32_t dt = (s_last_pulse_ms == 0) ? 0u : (now - s_last_pulse_ms);
        s_last_pulse_ms = now;

        if (dt > 0u) {
            s_watt = 5760000.0f / (float)dt;
        }

        s_kwh = (float)s_pulse_count / POWER_PULSES_PER_KWH;
        s_dirty = 1;
    }

    if (s_last_pulse_ms != 0u && (now - s_last_pulse_ms) > 5000u) {
        if (s_watt > 1.0f) {
            s_watt *= 0.95f;
            s_dirty = 1;
        } else {
            s_watt = 0.0f;
        }
    }

    if (s_need_redraw) {
        s_need_redraw = 0;
        DrawStaticDial(bg);
        s_dirty = 1;
        GC9A01A_Update();
    }

    if (!s_dirty) return;
    s_dirty = 0;

    DrawPowerScale(s_watt, bg);
    DrawNumbers(bg);

    if (s_last_pulse_ms != 0u && (now - s_last_pulse_ms) < 300u) {
        DrawPulseIndicator(bg);
    } else {
        dispcolor_FillCircle(PULSE_X, PULSE_Y, PULSE_R - 1, bg);
    }

    GC9A01A_UpdateRect(UPD_X, UPD_Y, UPD_W, UPD_H);
}
