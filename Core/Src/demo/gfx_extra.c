/*
 * gfx_extra.c
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#include "gfx_extra.h"
#include "dispcolor.h"

static void GFX_DrawCircleQuadrants(int16_t x0, int16_t y0, int16_t r,
                                   uint8_t corners, uint16_t color)
{
    // corners bitmask:
    // 0x1 = top-left, 0x2 = top-right, 0x4 = bottom-right, 0x8 = bottom-left
    int16_t f = (int16_t)(1 - r);
    int16_t ddF_x = 1;
    int16_t ddF_y = (int16_t)(-2 * r);
    int16_t x = 0;
    int16_t y = r;

    while (x <= y) {
        if (corners & 0x1) { // TL
            dispcolor_DrawPixel((int16_t)(x0 - y), (int16_t)(y0 - x), color);
            dispcolor_DrawPixel((int16_t)(x0 - x), (int16_t)(y0 - y), color);
        }
        if (corners & 0x2) { // TR
            dispcolor_DrawPixel((int16_t)(x0 + x), (int16_t)(y0 - y), color);
            dispcolor_DrawPixel((int16_t)(x0 + y), (int16_t)(y0 - x), color);
        }
        if (corners & 0x4) { // BR
            dispcolor_DrawPixel((int16_t)(x0 + x), (int16_t)(y0 + y), color);
            dispcolor_DrawPixel((int16_t)(x0 + y), (int16_t)(y0 + x), color);
        }
        if (corners & 0x8) { // BL
            dispcolor_DrawPixel((int16_t)(x0 - y), (int16_t)(y0 + x), color);
            dispcolor_DrawPixel((int16_t)(x0 - x), (int16_t)(y0 + y), color);
        }

        if (f >= 0) {
            y--;
            ddF_y = (int16_t)(ddF_y + 2);
            f = (int16_t)(f + ddF_y);
        }
        x++;
        ddF_x = (int16_t)(ddF_x + 2);
        f = (int16_t)(f + ddF_x);
    }
}

void dispcolor_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                            int16_t r, uint16_t color)
{
    if (w <= 0 || h <= 0) return;
    if (r < 0) r = 0;

    int16_t maxr = (int16_t)((w < h ? w : h) / 2);
    if (r > maxr) r = maxr;

    // edges
    dispcolor_DrawLine((int16_t)(x + r), (int16_t)y,
                       (int16_t)(x + w - r - 1), (int16_t)y, color);

    dispcolor_DrawLine((int16_t)(x + r), (int16_t)(y + h - 1),
                       (int16_t)(x + w - r - 1), (int16_t)(y + h - 1), color);

    dispcolor_DrawLine((int16_t)x, (int16_t)(y + r),
                       (int16_t)x, (int16_t)(y + h - r - 1), color);

    dispcolor_DrawLine((int16_t)(x + w - 1), (int16_t)(y + r),
                       (int16_t)(x + w - 1), (int16_t)(y + h - r - 1), color);

    // corners (quadrants)
    GFX_DrawCircleQuadrants((int16_t)(x + r),         (int16_t)(y + r),         r, 0x1, color); // TL
    GFX_DrawCircleQuadrants((int16_t)(x + w - r - 1), (int16_t)(y + r),         r, 0x2, color); // TR
    GFX_DrawCircleQuadrants((int16_t)(x + w - r - 1), (int16_t)(y + h - r - 1), r, 0x4, color); // BR
    GFX_DrawCircleQuadrants((int16_t)(x + r),         (int16_t)(y + h - r - 1), r, 0x8, color); // BL
}

static void GFX_FillCircleQuadrants(int16_t x0, int16_t y0, int16_t r,
                                   uint8_t corners, int16_t delta, uint16_t color)
{
    // Рисуем четверти окружности вертикальными линиями.
    // corners: 0x1 = left side, 0x2 = right side
    int16_t f = (int16_t)(1 - r);
    int16_t ddF_x = 1;
    int16_t ddF_y = (int16_t)(-2 * r);
    int16_t x = 0;
    int16_t y = r;

    while (x <= y) {
        if (corners & 0x1) {
            // левый верх/низ
            dispcolor_DrawLine((int16_t)(x0 - y), (int16_t)(y0 - x),
                               (int16_t)(x0 - y), (int16_t)(y0 + x + delta), color);
            dispcolor_DrawLine((int16_t)(x0 - x), (int16_t)(y0 - y),
                               (int16_t)(x0 - x), (int16_t)(y0 + y + delta), color);
        }
        if (corners & 0x2) {
            // правый верх/низ
            dispcolor_DrawLine((int16_t)(x0 + x), (int16_t)(y0 - y),
                               (int16_t)(x0 + x), (int16_t)(y0 + y + delta), color);
            dispcolor_DrawLine((int16_t)(x0 + y), (int16_t)(y0 - x),
                               (int16_t)(x0 + y), (int16_t)(y0 + x + delta), color);
        }

        if (f >= 0) {
            y--;
            ddF_y = (int16_t)(ddF_y + 2);
            f = (int16_t)(f + ddF_y);
        }
        x++;
        ddF_x = (int16_t)(ddF_x + 2);
        f = (int16_t)(f + ddF_x);
    }
}

void dispcolor_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                            int16_t r, uint16_t color)
{
    if (w <= 0 || h <= 0) return;
    if (r < 0) r = 0;

    int16_t maxr = (int16_t)((w < h ? w : h) / 2);
    if (r > maxr) r = maxr;

    if (r == 0) {
        dispcolor_FillRectangle(x, y, (int16_t)(x + w - 1), (int16_t)(y + h - 1), color);
        return;
    }

    // Центральная часть (без углов)
    dispcolor_FillRectangle((int16_t)(x + r), (int16_t)y,
                            (int16_t)(x + w - r - 1), (int16_t)(y + h - 1),
                            color);

    // Боковые прямоугольники (между скруглёнными углами)
    dispcolor_FillRectangle((int16_t)x, (int16_t)(y + r),
                            (int16_t)(x + r - 1), (int16_t)(y + h - r - 1),
                            color);

    dispcolor_FillRectangle((int16_t)(x + w - r), (int16_t)(y + r),
                            (int16_t)(x + w - 1), (int16_t)(y + h - r - 1),
                            color);

    // Скруглённые части (две стороны: левая и правая)
    int16_t delta = (int16_t)(h - 2 * r - 1);
    if (delta < 0) delta = 0;

    GFX_FillCircleQuadrants((int16_t)(x + r),         (int16_t)(y + r), r, 0x1, delta, color);
    GFX_FillCircleQuadrants((int16_t)(x + w - r - 1), (int16_t)(y + r), r, 0x2, delta, color);
}
