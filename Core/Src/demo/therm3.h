/*
 * therm3.h
 *
 *  Created on: 2 янв. 2026 г.
 *      Author: Zver
 */

#ifndef SRC_DEMO_THERM3_H_
#define SRC_DEMO_THERM3_H_

#pragma once
#include <stdint.h>

void Therm3_Init(void);
void Therm3_ResetView(void);
void Therm3_Draw(uint8_t light);// рисует кадр + Update()

void Therm3_OnK0(void);// +0.5 (или шаг)
void Therm3_OnK1(void);// next color scheme

#endif /* SRC_DEMO_THERM3_H_ */
