/*
 * rtc_print_mode.c
 *
 *  Created on: 6 янв. 2026 г.
 *      Author: Zver
 */

#include "rtc_print_mode.h"

#include <stdint.h>
#include <stm32f4xx_hal.h>

#include "main.h"      // hrtc, HAL_GetTick
#include "dispcolor.h"
#include "font.h"

extern RTC_HandleTypeDef hrtc;

static uint8_t  inited = 0;
static uint32_t tick = 0;
static const uint32_t period_ms = 200;   // обновление 5 раз/сек

void RTCPrint_ResetView(void)
{
    inited = 1;
    tick = HAL_GetTick();

    dispcolor_FillScreen(BLACK);
    dispcolor_Update();
}

void RTCPrint_Draw(void)
{
    if (!inited) RTCPrint_ResetView();

    uint32_t now = HAL_GetTick();
    if ((now - tick) < period_ms) return;
    tick = now;

    RTC_TimeTypeDef t;
    RTC_DateTypeDef d;

    // Важно: сначала Time, потом Date
    HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);

    dispcolor_FillScreen(BLACK);

    // Если FONTID_16F отсутствует — замени на FONTID_6X8M
    dispcolor_printf(50, 30, FONTID_16F, WHITE, "%02u:%02u:%02u",
                     t.Hours, t.Minutes, t.Seconds);

    dispcolor_printf(50, 70, FONTID_16F, WHITE, "%02u-%02u-%02u",
                     d.Date, d.Month, d.Year);

    dispcolor_printf(50, 105, FONTID_6X8M, GREY, "WeekDay=%u", d.WeekDay);

    dispcolor_Update();
}

