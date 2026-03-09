//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// ��� ����������� ����������� ���������������� ��������. �� ������ ���������
// ��� �� ����� �����, �� �� �������� ������� ������ �� ��� YouTube-�����
// "����������� � ���������" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// �����: �������� ������ / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#include <stdint.h>
#include <dispcolor.h>
#include <font.h>
#include <math.h>
#include "random_move.h"
#include "constants.h"
#include "thermostat.h"
#include <stm32f4xx_hal.h>


#define MIN_VALUE		100
#define MAX_VALUE		450
#define MIN_ANGLE		-224
#define MAX_ANGLE		44
#define PALETTE_SIZE	(MAX_ANGLE - MIN_ANGLE) / 4

static uint8_t PaletteReady = 0;
static sRGB888 Palette[PALETTE_SIZE];

static void GetBlueRedPalette(uint16_t steps, sRGB888 *pBuff) {
	if (!pBuff)
		return;

	sRGB888 KeyColors[] = { { 0x00, 0x00, 0xFF }, { 0xFF, 0x00, 0x00 } };

	for (uint16_t step = 0; step < steps; step++) {
		float n = (float) step / (float) (steps - 1);

		pBuff->r = ((float) KeyColors[0].r) * (1.0f - n)
				+ ((float) KeyColors[1].r) * n;
		pBuff->g = ((float) KeyColors[0].g) * (1.0f - n)
				+ ((float) KeyColors[1].g) * n;
		pBuff->b = ((float) KeyColors[0].b) * (1.0f - n)
				+ ((float) KeyColors[1].b) * n;

		pBuff++;
	}
}

void Test_Therm(uint8_t light) {
	if (!PaletteReady) {
		GetBlueRedPalette(PALETTE_SIZE, Palette);
		PaletteReady = 1;
	}

	uint16_t bgColor, textColor;
	if (light) {
		bgColor = WHITE;
		textColor = BLACK;
	} else {
		bgColor = BLACK;
		textColor = WHITE;
	}

	dispcolor_FillScreen(bgColor);

	int16_t position = GetCurrentPos(0, MAX_ANGLE - MIN_ANGLE);
	position += MIN_ANGLE;

	int16_t dAngle = (MAX_ANGLE - MIN_ANGLE);
	float temp = position - MIN_ANGLE;
	temp /= dAngle;
	temp = MIN_VALUE + (MAX_VALUE - MIN_VALUE) * temp;
	temp /= 10;
	float currentTemp = 28;

	// �����
	uint8_t mainRadius = 101;
	uint16_t idx = 0;
	for (int16_t angle = MIN_ANGLE; angle < position; idx++, angle += 4) {
		float angleRad = (float) angle * PI / 180;
		int xMain = cos(angleRad) * mainRadius + xC;
		int yMain = sin(angleRad) * mainRadius + yC;
		dispcolor_FillCircle(xMain, yMain, 20,
				RGB565(Palette[idx].r, Palette[idx].g, Palette[idx].b));
	}

	// �������� ������� � ������� �����������
	dispcolor_printf(85, 108, FONTID_32F, textColor, "%.1f \xB0""C", temp);
	dispcolor_printf(90, 205, FONTID_24F, textColor, "%.1f \xB0""C", currentTemp);

	// ����� �������� ��������� �����������
	if (temp < currentTemp - 1)
		dispcolor_DrawString(85, 145, FONTID_16F, "HEAT OFF",
				RGB565(100, 100, 100));
	else {
		dispcolor_FillRectangle(83, 144, 160, 162, GREEN);
		dispcolor_DrawString(85, 145, FONTID_16F, "HEAT ON", BLACK);
	}

	dispcolor_Update();
	HAL_Delay(37);
}
