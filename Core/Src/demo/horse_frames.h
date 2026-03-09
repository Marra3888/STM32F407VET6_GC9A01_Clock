/*
 * horse_frames.h
 *
 *  Created on: 2 янв. 2026 г.
 *      Author: Zver
 */

#ifndef SRC_DEMO_HORSE_FRAMES_H_
#define SRC_DEMO_HORSE_FRAMES_H_

#pragma once
#include <stdint.h>

#define HORSE_W 144
#define HORSE_H 96
#define HORSE_BPR (HORSE_W/8)              // 18
#define HORSE_BYTES (HORSE_BPR*HORSE_H)    // 1728

extern const uint8_t horse_01[HORSE_BYTES];
extern const uint8_t horse_02[HORSE_BYTES];
extern const uint8_t horse_03[HORSE_BYTES];
extern const uint8_t horse_04[HORSE_BYTES];
extern const uint8_t horse_05[HORSE_BYTES];
extern const uint8_t horse_06[HORSE_BYTES];
extern const uint8_t horse_07[HORSE_BYTES];
extern const uint8_t horse_08[HORSE_BYTES];
extern const uint8_t horse_09[HORSE_BYTES];
extern const uint8_t horse_10[HORSE_BYTES];

#endif /* SRC_DEMO_HORSE_FRAMES_H_ */
