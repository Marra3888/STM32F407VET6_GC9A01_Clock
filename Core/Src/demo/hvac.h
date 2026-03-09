/*
 * hvac.h
 *
 *  Created on: 2 янв. 2026 г.
 *      Author: Zver
 */

#ifndef SRC_DEMO_HVAC_H_
#define SRC_DEMO_HVAC_H_

#ifndef HVAC_H
#define HVAC_H

#include <stdint.h>

typedef enum {
    HVAC_AUTO = 0,
    HVAC_COOL,
    HVAC_HEAT
} HvacMode;

typedef enum {
    FAN_AUTO = 0,
    FAN_1,
    FAN_2,
    FAN_3
} FanMode;

typedef struct {
    float setpointC;
    float currentC;
    HvacMode mode;
    FanMode fan;
    uint8_t power;   // 0/1
} HvacState;

void Hvac_Init(void);
void Hvac_Draw_Round(uint8_t light);
void Hvac_OnK0(void);          // K0 short
void Hvac_OnK1(void);          // K1 short
void Hvac_OnCombo(void);       // K0+K1 hold
const HvacState* Hvac_Get(void);

#endif

#endif /* SRC_DEMO_HVAC_H_ */
