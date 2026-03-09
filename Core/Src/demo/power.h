/*
 * power.h
 *
 *  Created on: 2 янв. 2026 г.
 *      Author: Zver
 */

#ifndef SRC_DEMO_POWER_H_
#define SRC_DEMO_POWER_H_

#pragma once
#include <stdint.h>

void Power_Init(void);
void Power_ResetView(void);

// вызвать при импульсе S0 (например из EXTI callback)
void Power_OnPulse(void);

// рисование режима
void Power_Draw(uint8_t light);

#endif /* SRC_DEMO_POWER_H_ */
