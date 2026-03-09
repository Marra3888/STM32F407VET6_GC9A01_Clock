/*
 * voltage_meter.c
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#include "voltage_meter.h"

#include <stdlib.h>
#include <math.h>

#include "dispcolor.h"
#include "font.h"
#include "main.h"     // для HAL_GetTick (если надо) или можно включить stm32xxxx_hal.h

#define DEG2RAD 0.0174532925f

static uint8_t  vm_inited = 0;
static uint32_t vm_tick = 0;
static const uint32_t vm_frame_ms = 200;

// dial geometry
static const int16_t vm_center_x = 120;
static const int16_t vm_center_y = 120;
static const int16_t vm_radius   = 120;

static float vm_pivot_x, vm_pivot_y;
static float vm_angleOffset = 3.14f;

// needle points (current and old)
static float vm_p1x, vm_p1y, vm_p2x, vm_p2y, vm_p3x, vm_p3y, vm_p4x, vm_p4y, vm_p5x, vm_p5y;
static float vm_p1x_old, vm_p1y_old, vm_p2x_old, vm_p2y_old, vm_p3x_old, vm_p3y_old;
static float vm_p4x_old, vm_p4y_old, vm_p5x_old, vm_p5y_old;

static float vm_voltage = 240.0f;
static float vm_avgVoltage = 235.0f;

// rolling average
static const uint8_t vm_nvalues = 10;
static uint8_t vm_current = 0;
static uint8_t vm_cvalues = 0;
static float vm_sum = 0;
static float vm_values[10];

// colors (RGB565)
static const uint16_t VM_BLACK   = 0x0000;
static const uint16_t VM_WHITE   = 0xFFFF;
static const uint16_t VM_RED     = 0xF800;
static const uint16_t VM_GREY    = 0x84B5;
static const uint16_t VM_AFRICA  = 0xAB21;

static float VoltageMeter_MovingAverage(float value)
{
    vm_sum += value;
    if (vm_cvalues == vm_nvalues) {
        vm_sum -= vm_values[vm_current];
    }

    vm_values[vm_current] = value;

    vm_current++;
    if (vm_current >= vm_nvalues) vm_current = 0;

    if (vm_cvalues < vm_nvalues) vm_cvalues++;

    return vm_sum / (float)vm_cvalues;
}

static void VoltageMeter_DrawPivot(void)
{
    dispcolor_FillCircle((int16_t)vm_pivot_x, (int16_t)vm_pivot_y, 8, VM_RED);
    dispcolor_DrawCircle((int16_t)vm_pivot_x, (int16_t)vm_pivot_y, 8, VM_BLACK, 0);
    dispcolor_DrawCircle((int16_t)vm_pivot_x, (int16_t)vm_pivot_y, 3, VM_BLACK, 0);
}

static void VoltageMeter_DisplayNumerical(float v)
{
    dispcolor_FillRectangle((int16_t)(vm_center_x - 82), (int16_t)(vm_center_y + 40), (int16_t)(vm_center_x - 82 + 120 - 1), (int16_t)(vm_center_y + 40 + 16  - 1),
        VM_AFRICA);

    dispcolor_printf((int16_t)(vm_center_x - 80), (int16_t)(vm_center_y + 40), FONTID_16F, VM_BLACK, "%.1f", (double)v);
}

static void VoltageMeter_CreateDial(void)
{
    dispcolor_FillCircle(vm_center_x, vm_center_y, 120, VM_AFRICA);
    dispcolor_DrawCircle(vm_center_x, vm_center_y, 118, VM_GREY, 0);
    dispcolor_DrawCircle(vm_center_x, vm_center_y, 117, VM_BLACK, 0);
    dispcolor_DrawCircle(vm_center_x, vm_center_y, 116, VM_BLACK, 0);
    dispcolor_DrawCircle(vm_center_x, vm_center_y, 115, VM_GREY, 0);

    for (int j = 30; j < 75; j += 5) {
        float a = (j * DEG2RAD * 1.8f) - 3.14f;
        float arc_x = vm_pivot_x + ((vm_radius + 15) * cosf(a));
        float arc_y = vm_pivot_y + ((vm_radius + 15) * sinf(a));
        dispcolor_FillCircle((int16_t)arc_x, (int16_t)arc_y, 2, VM_BLACK);
    }

    // Если dispcolor.h не принимает const char*, может быть warning.
    // Вариант без правки библиотеки:
    dispcolor_DrawString((int16_t)(vm_center_x + 15), (int16_t)(vm_center_y + 40), FONTID_16F, (char*)"   V - AC", VM_BLACK);

    dispcolor_DrawLine((int16_t)(vm_center_x - 80), (int16_t)(vm_center_y + 70),
                       (int16_t)(vm_center_x + 80), (int16_t)(vm_center_y + 70), VM_WHITE);
}

static void VoltageMeter_Needle(float value)
{
    // erase old
    dispcolor_DrawLine((int16_t)vm_pivot_x, (int16_t)vm_pivot_y,
                       (int16_t)vm_p1x_old, (int16_t)vm_p1y_old, VM_AFRICA);

    dispcolor_FillTriangle((int16_t)vm_p1x_old, (int16_t)vm_p1y_old,
                           (int16_t)vm_p2x_old, (int16_t)vm_p2y_old,
                           (int16_t)vm_p3x_old, (int16_t)vm_p3y_old, VM_AFRICA);

    dispcolor_FillTriangle((int16_t)vm_pivot_x, (int16_t)vm_pivot_y,
                           (int16_t)vm_p4x_old, (int16_t)vm_p4y_old,
                           (int16_t)vm_p5x_old, (int16_t)vm_p5y_old, VM_AFRICA);

    // new geometry
    float needleAngle = (value * DEG2RAD * 1.8f) - 3.14f;

    vm_p1x = vm_pivot_x + (vm_radius * cosf(needleAngle));
    vm_p1y = vm_pivot_y + (vm_radius * sinf(needleAngle));

    vm_p2x = vm_pivot_x + ((vm_radius - 15) * cosf(needleAngle - 0.05f));
    vm_p2y = vm_pivot_y + ((vm_radius - 15) * sinf(needleAngle - 0.05f));

    vm_p3x = vm_pivot_x + ((vm_radius - 15) * cosf(needleAngle + 0.05f));
    vm_p3y = vm_pivot_y + ((vm_radius - 15) * sinf(needleAngle + 0.05f));

    vm_p4x = vm_pivot_x + ((vm_radius - 90) * cosf(vm_angleOffset + (needleAngle - 0.2f)));
    vm_p4y = vm_pivot_y + ((vm_radius - 90) * sinf(vm_angleOffset + (needleAngle - 0.2f)));

    vm_p5x = vm_pivot_x + ((vm_radius - 90) * cosf(vm_angleOffset + (needleAngle + 0.2f)));
    vm_p5y = vm_pivot_y + ((vm_radius - 90) * sinf(vm_angleOffset + (needleAngle + 0.2f)));

    // save old
    vm_p1x_old = vm_p1x; vm_p1y_old = vm_p1y;
    vm_p2x_old = vm_p2x; vm_p2y_old = vm_p2y;
    vm_p3x_old = vm_p3x; vm_p3y_old = vm_p3y;
    vm_p4x_old = vm_p4x; vm_p4y_old = vm_p4y;
    vm_p5x_old = vm_p5x; vm_p5y_old = vm_p5y;

    // draw new
    dispcolor_DrawLine((int16_t)vm_pivot_x, (int16_t)vm_pivot_y,
                       (int16_t)vm_p1x, (int16_t)vm_p1y, VM_BLACK);

    dispcolor_FillTriangle((int16_t)vm_p1x, (int16_t)vm_p1y,
                           (int16_t)vm_p2x, (int16_t)vm_p2y,
                           (int16_t)vm_p3x, (int16_t)vm_p3y, VM_BLACK);

    dispcolor_DrawLine((int16_t)(vm_center_x - 80), (int16_t)(vm_center_y + 70),
                       (int16_t)(vm_center_x + 80), (int16_t)(vm_center_y + 70), VM_WHITE);

    dispcolor_FillTriangle((int16_t)vm_pivot_x, (int16_t)vm_pivot_y,
                           (int16_t)vm_p4x, (int16_t)vm_p4y,
                           (int16_t)vm_p5x, (int16_t)vm_p5y, VM_BLACK);
}

void VoltageMeter_Init(void)
{
    vm_inited = 0;
}

void VoltageMeter_ResetView(void)
{
    vm_inited = 1;
    vm_tick = HAL_GetTick();

    vm_pivot_x = (float)vm_center_x;
    vm_pivot_y = (float)(vm_center_y + 50);

    vm_p1x_old = vm_pivot_x; vm_p1y_old = vm_pivot_y;
    vm_p2x_old = vm_pivot_x; vm_p2y_old = vm_pivot_y;
    vm_p3x_old = vm_pivot_x; vm_p3y_old = vm_pivot_y;
    vm_p4x_old = vm_pivot_x; vm_p4y_old = vm_pivot_y;
    vm_p5x_old = vm_pivot_x; vm_p5y_old = vm_pivot_y;

    vm_sum = 0;
    vm_current = 0;
    vm_cvalues = 0;
    for (int i = 0; i < 10; i++) vm_values[i] = 0;

    dispcolor_FillScreen(VM_BLACK);
    VoltageMeter_CreateDial();

    vm_voltage = 240.0f;
    vm_avgVoltage = vm_voltage;

    VoltageMeter_DisplayNumerical(vm_avgVoltage);
    VoltageMeter_Needle(vm_avgVoltage);
    VoltageMeter_DrawPivot();

    dispcolor_Update();
}

void VoltageMeter_Draw(void)
{
    if (!vm_inited) {
        VoltageMeter_ResetView();
    }

    uint32_t now = HAL_GetTick();
    if ((now - vm_tick) < vm_frame_ms) return;
    vm_tick = now;

    vm_voltage = 230.0f + (float)(rand() % 21); // 230..250
    vm_avgVoltage = VoltageMeter_MovingAverage(vm_voltage);

    VoltageMeter_DisplayNumerical(vm_avgVoltage);
    VoltageMeter_Needle(vm_avgVoltage);
    VoltageMeter_DrawPivot();

    dispcolor_Update();
}
