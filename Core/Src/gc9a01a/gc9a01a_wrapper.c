/*
 * gc9a01a_wrapper.c
 *
 *  Created on: 1 янв. 2026 г.
 *      Author: Zver
 */

#include "gc9a01a_simple.h"

uint16_t GC9A01A_GetWidth(void)  { return GC9A01A_Simple_GetWidth(); }
uint16_t GC9A01A_GetHeight(void) { return GC9A01A_Simple_GetHeight(); }

void GC9A01A_Init(uint16_t w, uint16_t h) { GC9A01A_Simple_Init(w, h); }
void GC9A01A_Update(void)                 { GC9A01A_Simple_Update(); }
void GC9A01A_SetBL(uint8_t v)             { GC9A01A_Simple_SetBL(v); }

void GC9A01A_DrawPixel(int16_t x, int16_t y, uint16_t c) { GC9A01A_Simple_DrawPixel(x, y, c); }
uint16_t GC9A01A_GetPixel(int16_t x, int16_t y)          { return GC9A01A_Simple_GetPixel(x, y); }
void GC9A01A_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c)
{
    GC9A01A_Simple_FillRect(x, y, w, h, c);
}

// Optional stubs
void GC9A01A_DrawPartYX(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff)
{
    (void)x; (void)y; (void)w; (void)h; (void)pBuff;
}

void GC9A01A_DrawPartXY(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff)
{
    (void)x; (void)y; (void)w; (void)h; (void)pBuff;
}
