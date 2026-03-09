#include "gc9a01a.h"

#include "main.h"
#include "spim.h"
#include "stm32f4xx_hal.h"

#include <stdint.h>
#include <stddef.h>

// ---- extern from CubeMX ----
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim11;

// ---- panel size ----
#ifndef GC9A01A_W
#define GC9A01A_W 240
#endif
#ifndef GC9A01A_H
#define GC9A01A_H 240
#endif

static uint16_t g_w = GC9A01A_W;
static uint16_t g_h = GC9A01A_H;

// Framebuffer: RGB565 stored big-endian in uint16_t
static uint16_t g_fb[GC9A01A_W * GC9A01A_H];

// ---- Pin helpers (from main.h) ----
static inline void CS_L(void)  { HAL_GPIO_WritePin(TFT_CS_GPIO_Port,  TFT_CS_Pin,  GPIO_PIN_RESET); }
static inline void CS_H(void)  { HAL_GPIO_WritePin(TFT_CS_GPIO_Port,  TFT_CS_Pin,  GPIO_PIN_SET); }
static inline void DC_L(void)  { HAL_GPIO_WritePin(TFT_DC_GPIO_Port,  TFT_DC_Pin,  GPIO_PIN_RESET); }
static inline void DC_H(void)  { HAL_GPIO_WritePin(TFT_DC_GPIO_Port,  TFT_DC_Pin,  GPIO_PIN_SET); }
static inline void RST_L(void) { HAL_GPIO_WritePin(TFT_RES_GPIO_Port, TFT_RES_Pin, GPIO_PIN_RESET); }
static inline void RST_H(void) { HAL_GPIO_WritePin(TFT_RES_GPIO_Port, TFT_RES_Pin, GPIO_PIN_SET); }

static inline uint16_t be16(uint16_t rgb565)
{
    return (uint16_t)((rgb565 << 8) | (rgb565 >> 8));
}

static void spi_tx(const uint8_t *p, uint16_t n)
{
    if (!p || !n) return;
    HAL_SPI_Transmit(&hspi1, (uint8_t*)p, n, HAL_MAX_DELAY);
}

static void wr_cmd(uint8_t cmd)
{
    DC_L();
    CS_L();
    spi_tx(&cmd, 1);
    CS_H();
}

static void wr_data(const uint8_t *p, uint16_t n)
{
    DC_H();
    CS_L();
    spi_tx(p, n);
    CS_H();
}

static void wr_u8(uint8_t v) { wr_data(&v, 1); }

static void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    wr_cmd(0x2A);
    uint8_t dx[4] = {(uint8_t)(x0>>8),(uint8_t)x0,(uint8_t)(x1>>8),(uint8_t)x1};
    wr_data(dx, 4);

    wr_cmd(0x2B);
    uint8_t dy[4] = {(uint8_t)(y0>>8),(uint8_t)y0,(uint8_t)(y1>>8),(uint8_t)y1};
    wr_data(dy, 4);

    wr_cmd(0x2C);
}

// ===== Public API (basic drawing) =====

uint16_t GC9A01A_GetWidth(void)  { return g_w; }
uint16_t GC9A01A_GetHeight(void) { return g_h; }

void GC9A01A_SetBL(uint8_t percent)
{
    if (percent > 100) percent = 100;

    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim11);
    uint32_t ccr = (arr * percent) / 100u;

    __HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, ccr);
}

void GC9A01A_Init(uint16_t width, uint16_t height)
{
    (void)width; (void)height;
    g_w = GC9A01A_W;
    g_h = GC9A01A_H;

    // Reset
    RST_L(); HAL_Delay(100);
    RST_H(); HAL_Delay(100);

    // Backlight ON
    GC9A01A_SetBL(100);

    // Waveshare init sequence (known-good)
    wr_cmd(0xEF);

    wr_cmd(0xEB); wr_u8(0x14);
    wr_cmd(0xFE);
    wr_cmd(0xEF);
    wr_cmd(0xEB); wr_u8(0x14);

    wr_cmd(0x84); wr_u8(0x40);
    wr_cmd(0x85); wr_u8(0xFF);
    wr_cmd(0x86); wr_u8(0xFF);
    wr_cmd(0x87); wr_u8(0xFF);
    wr_cmd(0x88); wr_u8(0x0A);
    wr_cmd(0x89); wr_u8(0x21);
    wr_cmd(0x8A); wr_u8(0x00);
    wr_cmd(0x8B); wr_u8(0x80);
    wr_cmd(0x8C); wr_u8(0x01);
    wr_cmd(0x8D); wr_u8(0x01);
    wr_cmd(0x8E); wr_u8(0xFF);
    wr_cmd(0x8F); wr_u8(0xFF);

    wr_cmd(0xB6); wr_u8(0x00); wr_u8(0x20);

    wr_cmd(0x36); wr_u8(0x08); // BGR
    wr_cmd(0x3A); wr_u8(0x05); // 16-bit

    wr_cmd(0x90); { uint8_t d[] = {0x08,0x08,0x08,0x08}; wr_data(d, sizeof(d)); }

    wr_cmd(0xBD); wr_u8(0x06);
    wr_cmd(0xBC); wr_u8(0x00);

    wr_cmd(0xFF); { uint8_t d[] = {0x60,0x01,0x04}; wr_data(d, sizeof(d)); }

    wr_cmd(0xC3); wr_u8(0x13);
    wr_cmd(0xC4); wr_u8(0x13);
    wr_cmd(0xC9); wr_u8(0x22);
    wr_cmd(0xBE); wr_u8(0x11);

    wr_cmd(0xE1); { uint8_t d[] = {0x10,0x0E}; wr_data(d, 2); }
    wr_cmd(0xDF); { uint8_t d[] = {0x21,0x0C,0x02}; wr_data(d, 3); }

    wr_cmd(0xF0); { uint8_t d[] = {0x45,0x09,0x08,0x08,0x26,0x2A}; wr_data(d, 6); }
    wr_cmd(0xF1); { uint8_t d[] = {0x43,0x70,0x72,0x36,0x37,0x6F}; wr_data(d, 6); }
    wr_cmd(0xF2); { uint8_t d[] = {0x45,0x09,0x08,0x08,0x26,0x2A}; wr_data(d, 6); }
    wr_cmd(0xF3); { uint8_t d[] = {0x43,0x70,0x72,0x36,0x37,0x6F}; wr_data(d, 6); }

    wr_cmd(0xED); { uint8_t d[] = {0x1B,0x0B}; wr_data(d, 2); }
    wr_cmd(0xAE); wr_u8(0x77);
    wr_cmd(0xCD); wr_u8(0x63);

    wr_cmd(0x70); { uint8_t d[] = {0x07,0x07,0x04,0x0E,0x0F,0x09,0x07,0x08,0x03}; wr_data(d, 9); }

    wr_cmd(0xE8); wr_u8(0x34);

    wr_cmd(0x62); { uint8_t d[] = {0x18,0x0D,0x71,0xED,0x70,0x70,0x18,0x0F,0x71,0xEF,0x70,0x70}; wr_data(d, 12); }
    wr_cmd(0x63); { uint8_t d[] = {0x18,0x11,0x71,0xF1,0x70,0x70,0x18,0x13,0x71,0xF3,0x70,0x70}; wr_data(d, 12); }
    wr_cmd(0x64); { uint8_t d[] = {0x28,0x29,0xF1,0x01,0xF1,0x00,0x07}; wr_data(d, 7); }
    wr_cmd(0x66); { uint8_t d[] = {0x3C,0x00,0xCD,0x67,0x45,0x45,0x10,0x00,0x00,0x00}; wr_data(d, 10); }
    wr_cmd(0x67); { uint8_t d[] = {0x00,0x3C,0x00,0x00,0x00,0x01,0x54,0x10,0x32,0x98}; wr_data(d, 10); }
    wr_cmd(0x74); { uint8_t d[] = {0x10,0x85,0x80,0x00,0x00,0x4E,0x00}; wr_data(d, 7); }
    wr_cmd(0x98); { uint8_t d[] = {0x3E,0x07}; wr_data(d, 2); }

    wr_cmd(0x35);
    wr_cmd(0x21);

    wr_cmd(0x11);
    HAL_Delay(120);
    wr_cmd(0x29);
    HAL_Delay(20);

    // clear framebuffer
    for (uint32_t i = 0; i < (uint32_t)GC9A01A_W * (uint32_t)GC9A01A_H; i++) {
        g_fb[i] = be16(0x0000);
    }

    GC9A01A_Update();
}

void GC9A01A_DrawPixel(int16_t x, int16_t y, uint16_t color)
{
    if (x < 0 || y < 0 || x >= (int16_t)g_w || y >= (int16_t)g_h) return;
    g_fb[(uint32_t)y * g_w + (uint32_t)x] = be16(color);
}

uint16_t GC9A01A_GetPixel(int16_t x, int16_t y)
{
    if (x < 0 || y < 0 || x >= (int16_t)g_w || y >= (int16_t)g_h) return 0;
    uint16_t be = g_fb[(uint32_t)y * g_w + (uint32_t)x];
    return (uint16_t)((be << 8) | (be >> 8));
}

void GC9A01A_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if (w <= 0 || h <= 0) return;
    if (x >= (int16_t)g_w || y >= (int16_t)g_h) return;

    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (w <= 0 || h <= 0) return;

    if (x + w > (int16_t)g_w) w = (int16_t)g_w - x;
    if (y + h > (int16_t)g_h) h = (int16_t)g_h - y;

    uint16_t v = be16(color);
    for (int16_t row = 0; row < h; row++) {
        uint32_t idx = (uint32_t)(y + row) * g_w + (uint32_t)x;
        for (int16_t col = 0; col < w; col++) {
            g_fb[idx + (uint32_t)col] = v;
        }
    }
}

// ===== DMA Update (full + rect, callback chain) =====

#ifndef GC9A01A_DMA_CHUNK
#define GC9A01A_DMA_CHUNK  30000u
#endif

typedef enum {
    UPD_NONE = 0,
    UPD_FULL,
    UPD_RECT
} upd_kind_t;

static volatile uint32_t s_off = 0;
static volatile uint32_t s_total = 0;
static volatile uint8_t  s_busy = 0;

// pending request (last-wins)
static volatile upd_kind_t s_pending_kind = UPD_NONE;
static volatile int16_t s_pend_x = 0, s_pend_y = 0, s_pend_w = 0, s_pend_h = 0;

// rect streaming state
static volatile int16_t s_rx = 0, s_ry = 0, s_rw = 0, s_rh = 0;
static volatile int16_t s_row = 0;
static volatile uint16_t s_row_off_bytes = 0;    // 0..rw*2
static volatile uint16_t s_row_bytes_total = 0;  // rw*2

static void tx_next_full(void);
static void tx_next_rect(void);

static inline void queue_pending_full(void)
{
    s_pending_kind = UPD_FULL;
}

static inline void queue_pending_rect(int16_t x, int16_t y, int16_t w, int16_t h)
{
    s_pending_kind = UPD_RECT;
    s_pend_x = x; s_pend_y = y; s_pend_w = w; s_pend_h = h;
}

static uint8_t clip_rect(int16_t *x, int16_t *y, int16_t *w, int16_t *h)
{
    if (*w <= 0 || *h <= 0) return 0;
    if (*x >= (int16_t)g_w || *y >= (int16_t)g_h) return 0;

    if (*x < 0) { *w += *x; *x = 0; }
    if (*y < 0) { *h += *y; *y = 0; }
    if (*w <= 0 || *h <= 0) return 0;

    if (*x + *w > (int16_t)g_w) *w = (int16_t)g_w - *x;
    if (*y + *h > (int16_t)g_h) *h = (int16_t)g_h - *y;

    return (*w > 0 && *h > 0);
}

static void finish_and_maybe_restart(void)
{
    CS_H();
    s_busy = 0;

    upd_kind_t k = s_pending_kind;
    if (k == UPD_NONE) return;

    s_pending_kind = UPD_NONE;

    if (k == UPD_FULL) {
        GC9A01A_Update();
    } else {
        GC9A01A_UpdateRect(s_pend_x, s_pend_y, s_pend_w, s_pend_h);
    }
}

static void tx_next_full(void)
{
    if (s_off >= s_total) {
        finish_and_maybe_restart();
        return;
    }

    uint32_t remain = s_total - s_off;
    uint16_t chunk = (remain > GC9A01A_DMA_CHUNK) ? (uint16_t)GC9A01A_DMA_CHUNK : (uint16_t)remain;

    const uint8_t *p = ((const uint8_t*)g_fb) + s_off;
    s_off += chunk;

    SPI_send_dma(SPI1, 0, p, chunk, tx_next_full);
}

static void tx_next_rect(void)
{
    if (s_row >= s_rh) {
        finish_and_maybe_restart();
        return;
    }

    uint16_t remain = (uint16_t)(s_row_bytes_total - s_row_off_bytes);
    uint16_t chunk = (remain > GC9A01A_DMA_CHUNK) ? (uint16_t)GC9A01A_DMA_CHUNK : remain;

    uint32_t fb_index = (uint32_t)(s_ry + s_row) * g_w + (uint32_t)s_rx;
    const uint8_t *row_ptr = (const uint8_t*)&g_fb[fb_index];

    const uint8_t *p = row_ptr + s_row_off_bytes;
    s_row_off_bytes = (uint16_t)(s_row_off_bytes + chunk);

    if (s_row_off_bytes >= s_row_bytes_total) {
        s_row++;
        s_row_off_bytes = 0;
    }

    SPI_send_dma(SPI1, 0, p, chunk, tx_next_rect);
}

void GC9A01A_Update(void)
{
    if (s_busy) { queue_pending_full(); return; }
    s_busy = 1;

    set_window(0, 0, (uint16_t)(g_w - 1), (uint16_t)(g_h - 1));

    DC_H();
    CS_L();

    s_off = 0;
    s_total = (uint32_t)g_w * (uint32_t)g_h * 2u;

    tx_next_full();
}

void GC9A01A_UpdateRect(int16_t x, int16_t y, int16_t w, int16_t h)
{
    if (!clip_rect(&x, &y, &w, &h)) return;

    if (s_busy) { queue_pending_rect(x, y, w, h); return; }
    s_busy = 1;

    set_window((uint16_t)x, (uint16_t)y, (uint16_t)(x + w - 1), (uint16_t)(y + h - 1));

    DC_H();
    CS_L();

    s_rx = x; s_ry = y; s_rw = w; s_rh = h;
    s_row = 0;
    s_row_off_bytes = 0;
    s_row_bytes_total = (uint16_t)(w * 2);

    tx_next_rect();
}

uint8_t GC9A01A_IsBusy(void)
{
    return s_busy ? 1u : 0u;
}

// Legacy compatibility: copy buffer to framebuffer and update rect
void GC9A01A_DrawPartXY(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff)
{
    if (!pBuff) return;
    if (!clip_rect(&x, &y, &w, &h)) return;

    for (int16_t row = 0; row < h; row++) {
        uint32_t fb_idx = (uint32_t)(y + row) * g_w + (uint32_t)x;
        uint32_t src_idx = (uint32_t)row * (uint32_t)w;
        for (int16_t col = 0; col < w; col++) {
            uint16_t c = pBuff[src_idx + (uint32_t)col];
            g_fb[fb_idx + (uint32_t)col] = be16(c);
        }
    }

    GC9A01A_UpdateRect(x, y, w, h);
}

void GC9A01A_DrawPartYX(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff)
{
    (void)x; (void)y; (void)w; (void)h; (void)pBuff;
}
