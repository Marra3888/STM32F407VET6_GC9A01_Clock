/*
 * boat_gauges.c
 *
 *  Created on: 7 янв. 2026 г.
 *      Author: Zver
 */

//void BoatGauges_Draw(void)
//{
//    if (!inited) {
//        inited = 1;
//        HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET);
//
//        // Читаем первые 16 байт картинки
//        uint8_t d[16];
//        Soft_ReadBytes(d, 0x000000, 16);
//
//        dispcolor_FillScreen(BLACK);
//
//        // Вывод HEX
//        char buf[50];
//        snprintf(buf, sizeof(buf), "%02X %02X %02X %02X %02X %02X %02X %02X",
//                 d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);
//        dispcolor_DrawString(10, 50, FONTID_16F, buf, WHITE);
//
//        snprintf(buf, sizeof(buf), "%02X %02X %02X %02X %02X %02X %02X %02X",
//                 d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
//        dispcolor_DrawString(10, 70, FONTID_16F, buf, WHITE);
//
//        // Проверка на BMP заголовок ("BM" = 42 4D)
//        if (d[0] == 0x42 && d[1] == 0x4D) {
//            dispcolor_DrawString(10, 100, FONTID_16F, "IT IS BMP FILE!", RED);
//            dispcolor_DrawString(10, 120, FONTID_16F, "Need offset 54+", RED);
//        } else {
//            dispcolor_DrawString(10, 100, FONTID_16F, "RAW DATA (Maybe)", GREEN);
//        }
//
//        GC9A01A_UpdateRect(0, 0, 240, 240);
//    }
//}







#include "boat_gauges.h"
#include "main.h"
#include "dispcolor.h"
#include "font.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

// --- ПОДКЛЮЧАЕМ КАРТИНКИ (ТОЛЬКО 2 ШТУКИ) ---
#include "img/gauge1.h"
#include "img/gauge2.h"

// Объявляем функции отрисовки (если их нет в хедерах)
extern void GC9A01A_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);
extern void GC9A01A_UpdateRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

#ifndef PI
#define PI 3.14159265358979323846f
#endif

// --- РЕАЛИЗАЦИЯ ОТРИСОВКИ КАРТИНКИ (Если драйвер её не имеет) ---
// Если у вас уже есть эта функция в другом месте - удалите этот блок!
/*
void GC9A01A_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
    // Установка окна
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET);

    uint8_t cmd = 0x2A; HAL_SPI_Transmit(&hspi1, &cmd, 1, 10); // Column
    uint8_t d[4];
    d[0]=x>>8; d[1]=x; d[2]=(x+w-1)>>8; d[3]=(x+w-1);
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET);
    HAL_SPI_Transmit(&hspi1, d, 4, 10);

    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET);
    cmd = 0x2B; HAL_SPI_Transmit(&hspi1, &cmd, 1, 10); // Row
    d[0]=y>>8; d[1]=y; d[2]=(y+h-1)>>8; d[3]=(y+h-1);
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET);
    HAL_SPI_Transmit(&hspi1, d, 4, 10);

    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET);
    cmd = 0x2C; HAL_SPI_Transmit(&hspi1, &cmd, 1, 10); // Write RAM

    // Данные
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET);
    HAL_SPI_Transmit(&hspi1, (uint8_t*)data, w*h*2, 100);

    HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET);
}
*/

// --- ПАРАМЕТРЫ ---
static const int SX = 120;
static const int SY = 120;
static const int R  = 76;

static float lut_x[360];
static float lut_y[360];
static float lut_x2[360];
static float lut_y2[360];
static uint8_t tables_calculated = 0;

static uint8_t  gauge_idx = 0;
static uint8_t  inited = 0;

static float sim_val = 0.0f;
static float sim_dir = 0.5f;

// Массив указателей (ТОЛЬКО 2!)
static const uint16_t* gauge_imgs[] = {
    gauge1,
    gauge2
};

static const int min_val[] = {0,  20};
static const int max_val[] = {40, 100};

static float map_f(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static void CalcTables(void) {
    float rad = 0.01745329f;
    int i = 0; int a = 136;
    while (a != 44) {
        lut_x[i]  = (float)R * cosf(rad * (float)a) + (float)SX;
        lut_y[i]  = (float)R * sinf(rad * (float)a) + (float)SY;
        lut_x2[i] = ((float)R - 20.0f) * cosf(rad * (float)a) + (float)SX;
        lut_y2[i] = ((float)R - 20.0f) * sinf(rad * (float)a) + (float)SY;
        i++; a++;
        if (a >= 360) a = 0;
        if (i >= 360) break;
    }
    tables_calculated = 1;
}

void BoatGauges_Init(void) { if (!tables_calculated) CalcTables(); }
void BoatGauges_ResetView(void) { BoatGauges_Init(); inited = 1; sim_val = (float)min_val[gauge_idx]; }
void BoatGauges_OnK0(void) { gauge_idx++; if(gauge_idx >= 2) gauge_idx = 0; sim_val = (float)min_val[gauge_idx]; }

void BoatGauges_Draw(void)
{
    if (!inited) BoatGauges_ResetView();

    // 1. Эмуляция
    sim_val += sim_dir;
    if (sim_val >= (float)max_val[gauge_idx]) { sim_val = (float)max_val[gauge_idx]; sim_dir = -0.5f; }
    if (sim_val <= (float)min_val[gauge_idx]) { sim_val = (float)min_val[gauge_idx]; sim_dir = 0.5f;  }

    // 2. Угол
    float angle_f = map_f(sim_val, (float)min_val[gauge_idx], (float)max_val[gauge_idx], 0.0f, 267.0f);
    int angle_idx = (int)angle_f;
    if (angle_idx < 0) angle_idx = 0;
    if (angle_idx > 267) angle_idx = 267;

    // 3. ОТРИСОВКА ФОНА (Копируем массив в буфер)
    // Это самый надежный способ: массив -> буфер RAM -> дисплей
    const uint16_t* pImg = gauge_imgs[gauge_idx];
    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 240; x++) {
            dispcolor_DrawPixel(x, y, pImg[y * 240 + x]);
        }
    }

    // 4. Текст
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", (int)sim_val);
    int text_len = strlen(buf);
    int text_x = 120 - (text_len * 10);
    dispcolor_DrawString(text_x, 114, FONTID_32F, buf, BLACK);

    // 5. Стрелка
    int a1 = angle_idx - 4;
    int a2 = angle_idx + 4;
    if (a1 < 0) a1 = 0;
    if (a2 > 267) a2 = 267;

    int16_t x1 = (int16_t)lut_x[angle_idx];
    int16_t y1 = (int16_t)lut_y[angle_idx];
    int16_t x2 = (int16_t)lut_x2[a1];
    int16_t y2 = (int16_t)lut_y2[a1];
    int16_t x3 = (int16_t)lut_x2[a2];
    int16_t y3 = (int16_t)lut_y2[a2];

    dispcolor_FillTriangle(x1, y1, x2, y2, x3, y3, RED);

    // 6. Обновление
    GC9A01A_UpdateRect(0, 0, 240, 240);
}
