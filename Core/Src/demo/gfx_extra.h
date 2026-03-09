/*
 * gfx_extra.h
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#ifndef GFX_EXTRA_H
#define GFX_EXTRA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void dispcolor_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void dispcolor_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif // GFX_EXTRA_H
