/*
 * voltage_meter.h
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#ifndef VOLTAGE_METER_H
#define VOLTAGE_METER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void VoltageMeter_Init(void);        // опционально (если хочешь отдельную init)
void VoltageMeter_ResetView(void);
void VoltageMeter_Draw(void);        // то, что было ModeVoltage_meter_Draw()

#ifdef __cplusplus
}
#endif

#endif // VOLTAGE_METER_H

