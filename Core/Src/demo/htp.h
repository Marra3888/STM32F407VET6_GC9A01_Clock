/*
 * htp.h
 *
 *  Created on: 2 янв. 2026 г.
 *      Author: Zver
 */

#ifndef SRC_DEMO_HTP_H_
#define SRC_DEMO_HTP_H_

#pragma once
#include <stdint.h>

typedef struct {
    float t_c;
    float rh;
    float p_hpa;
    uint8_t valid;
} HTP_Data;

void HTP_Init(void);
void HTP_ResetView(void);
HTP_Data HTP_Read(void);     // заглушка/датчик
void HTP_Draw_Round(uint8_t light); // 0=dark, 1=light

#endif /* SRC_DEMO_HTP_H_ */
