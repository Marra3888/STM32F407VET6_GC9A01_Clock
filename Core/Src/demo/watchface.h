/*
 * photo_clock.h
 *
 *  Created on: 5 янв. 2026 г.
 *      Author: Zver
 */

#ifndef WATCHFACE_H
#define WATCHFACE_H

#include <stdint.h>
#include <stdio.h>

void Watchface_Reset(void);
void Watchface_Draw(uint8_t hour, uint8_t min, uint8_t sec);
void GetTimeRTC(uint8_t *hh, uint8_t *mm, uint8_t *ss);

#endif

