//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// ��� ����������� ����������� ���������������� ��������. �� ������ ���������
// ��� �� ����� �����, �� �� �������� ������� ������ �� ��� YouTube-�����
// "����������� � ���������" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// �����: �������� ������ / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#include <stdint.h>
#include <string.h>
#include <dispcolor.h>
#include <font.h>
#include <math.h>
#include "random_move.h"
#include "constants.h"
#include "compas.h"
#include <stm32f4xx_hal.h>


void DrawCompas(uint8_t light) {
	uint16_t bgColor, riskColor, textColor;
	if (light) {
		bgColor = WHITE;
		riskColor = textColor = BLACK;
	} else {
		bgColor = BLACK;
		riskColor = textColor = WHITE;
	}

	dispcolor_FillScreen(bgColor);

	// ����� �� ����������
	uint8_t radius1 = 119;
	for (uint16_t angle = 90; angle < 450; angle += 4) {
		uint8_t riskSize;
		if (!(angle % 20))
			riskSize = 12;
		else if (!(angle % 10))
			riskSize = 8;
		else
			riskSize = 4;

		uint8_t radius2 = radius1 - riskSize;
		float angleRad = (float) angle * PI / 180;
		int x1 = cos(angleRad) * radius1 + xC;
		int y1 = sin(angleRad) * radius1 + yC;
		int x2 = cos(angleRad) * radius2 + xC;
		int y2 = sin(angleRad) * radius2 + yC;

		dispcolor_DrawLine_Wu(x1, y1, x2, y2, riskColor);
	}

	// ������ ������
	if (!light) {
		dispcolor_DrawCircle_Wu(xC, yC, 119, riskColor);
		dispcolor_DrawCircle_Wu(xC, yC, 120, riskColor);
	}

	// ������� ��������
	dispcolor_DrawString(118, 13, FONTID_6X8M, "0", textColor);
	dispcolor_DrawString(150, 18, FONTID_6X8M, "20", textColor);
	dispcolor_DrawString(183, 38, FONTID_6X8M, "40", textColor);
	dispcolor_DrawString(204, 66, FONTID_6X8M, "60", textColor);
	dispcolor_DrawString(216, 98, FONTID_6X8M, "80", textColor);
	dispcolor_DrawString(210, 135, FONTID_6X8M, "100", textColor);
	dispcolor_DrawString(197, 170, FONTID_6X8M, "120", textColor);
	dispcolor_DrawString(177, 197, FONTID_6X8M, "140", textColor);
	dispcolor_DrawString(143, 216, FONTID_6X8M, "160", textColor);
	dispcolor_DrawString(111, 221, FONTID_6X8M, "180", textColor);
	dispcolor_DrawString(80, 216, FONTID_6X8M, "200", textColor);
	dispcolor_DrawString(47, 197, FONTID_6X8M, "220", textColor);
	dispcolor_DrawString(25, 170, FONTID_6X8M, "240", textColor);
	dispcolor_DrawString(13, 135, FONTID_6X8M, "260", textColor);
	dispcolor_DrawString(13, 98, FONTID_6X8M, "280", textColor);
	dispcolor_DrawString(25, 66, FONTID_6X8M, "300", textColor);
	dispcolor_DrawString(46, 38, FONTID_6X8M, "320", textColor);
	dispcolor_DrawString(80, 18, FONTID_6X8M, "340", textColor);

	// ������� NESW
	dispcolor_DrawString(116, 36, FONTID_16F, "N", textColor);
	dispcolor_DrawString(116, 189, FONTID_16F, "S", textColor);
	dispcolor_DrawString(37, 113, FONTID_16F, "W", textColor);
	dispcolor_DrawString(193, 113, FONTID_16F, "E", textColor);

	dispcolor_DrawCircle_Wu(xC, yC, 89, riskColor);

	// �������
	int16_t position = GetCurrentPos(0, 360);

	float angleRed = (float) position * PI / 180;
	int xRed = cos(angleRed) * 100 + xC;
	int yRed = sin(angleRed) * 100 + yC;
	float angleBlue = (float) (position + 180) * PI / 180;
	int xBlue = cos(angleBlue) * 100 + xC;
	int yBlue = sin(angleBlue) * 100 + yC;

	float angle1 = (float) (position + 90) * PI / 180;
	int x1 = cos(angle1) * 20 + xC;
	int y1 = sin(angle1) * 20 + yC;
	float angle2 = (float) (position + 270) * PI / 180;
	int x2 = cos(angle2) * 20 + xC;
	int y2 = sin(angle2) * 20 + yC;

	dispcolor_FillTriangle(xRed, yRed, x1, y1, x2, y2, RED);
	dispcolor_FillTriangle(xBlue, yBlue, x1, y1, x2, y2, BLUE);

	dispcolor_Update();
    HAL_Delay(37);
}
