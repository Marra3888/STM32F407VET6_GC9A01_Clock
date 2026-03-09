/*
 * ampelguys.h
 *
 *  Created on: 3 янв. 2026 г.
 *      Author: Zver
 */

#ifndef AMPELGUYS_H
#define AMPELGUYS_H

#include <stdint.h>

#define AMPEL_FRAMES   13
#define AMPEL_H        95
#define AMPEL_W_BITS   88
#define AMPEL_BPR      (AMPEL_W_BITS/8)     // 11
#define AMPEL_FRAME_SZ (AMPEL_BPR*AMPEL_H)  // 1045

extern const uint8_t ampelmann[AMPEL_FRAMES][AMPEL_FRAME_SZ];

#endif
