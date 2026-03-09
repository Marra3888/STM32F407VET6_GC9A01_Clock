#ifndef GC9A01A_H_
#define GC9A01A_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t GC9A01A_GetWidth(void);
uint16_t GC9A01A_GetHeight(void);

void     GC9A01A_Init(uint16_t width, uint16_t height);
void     GC9A01A_SetBL(uint8_t percent);              // 0..100

void     GC9A01A_Update(void);                        // full frame
void     GC9A01A_UpdateRect(int16_t x, int16_t y, int16_t w, int16_t h); // from framebuffer

uint8_t  GC9A01A_IsBusy(void);                        // 1 = DMA busy

void     GC9A01A_DrawPixel(int16_t x, int16_t y, uint16_t color);
uint16_t GC9A01A_GetPixel(int16_t x, int16_t y);
void     GC9A01A_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

// Legacy compatibility (optional)
void GC9A01A_DrawPartYX(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff);
void GC9A01A_DrawPartXY(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff);

#ifdef __cplusplus
}
#endif

#endif // GC9A01A_H_
