/*
 * boat_gauges.h
 *
 *  Created on: 7 янв. 2026 г.
 *      Author: Zver
 */

#ifndef BOAT_GAUGES_H
#define BOAT_GAUGES_H

#include <stdint.h>
#include <stdio.h>

void BoatGauges_Init(void);
void BoatGauges_ResetView(void);
void BoatGauges_Draw(void);
void BoatGauges_OnK0(void); // Переключение типа прибора

#endif // BOAT_GAUGES_H
