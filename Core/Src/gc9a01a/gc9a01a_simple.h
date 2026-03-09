/*
 * gc9a01a_simple.h
 *
 *  Created on: 1 янв. 2026 г.
 *      Author: Zver
 */

#ifndef GC9A01A_SIMPLE_H_
#define GC9A01A_SIMPLE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t GC9A01A_Simple_GetWidth(void);
uint16_t GC9A01A_Simple_GetHeight(void);

void GC9A01A_Simple_Init(uint16_t width, uint16_t height);
void GC9A01A_Simple_SetBL(uint8_t percent); // 0..100 (GPIO on/off)

void GC9A01A_Simple_Update(void);

void GC9A01A_Simple_DrawPixel(int16_t x, int16_t y, uint16_t color);
uint16_t GC9A01A_Simple_GetPixel(int16_t x, int16_t y);
void GC9A01A_Simple_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void GC9A01A_Simple_FillScreen(uint16_t color);

#ifdef __cplusplus
}
#endif

#endif // GC9A01A_SIMPLE_H_
