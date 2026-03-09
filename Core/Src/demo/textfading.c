//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// ��� ����������� ����������� ���������������� ��������. �� ������ ���������
// ��� �� ����� �����, �� �� �������� ������� ������ �� ��� YouTube-�����
// "����������� � ���������" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// �����: �������� ������ / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#include <stm32f4xx_hal.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dispcolor.h>
#include <font.h>
#include "textfading.h"

extern uint8_t brightness;
static char Str[32];

static uint8_t utf8_to_cp1251_next(const char **s)
{
    const uint8_t *p = (const uint8_t *)(*s);
    if (p[0] == 0) return 0;

    if (p[0] < 0x80) { (*s)++; return p[0]; } // ASCII

    if (p[0] == 0xD0) {
        uint8_t b = p[1];
        (*s) += 2;
        if (b == 0x81) return 0xA8;                    // Ё
        if (b >= 0x90 && b <= 0xBF) return (uint8_t)(b + 0x30); // А..п
        return '?';
    }

    if (p[0] == 0xD1) {
        uint8_t b = p[1];
        (*s) += 2;
        if (b == 0x91) return 0xB8;                    // ё
        if (b >= 0x80 && b <= 0x8F) return (uint8_t)(b + 0x70); // р..я
        return '?';
    }

    (*s)++; // неизвестное — пропускаем 1 байт
    return '?';
}

void Test_TextFading(char *pStr, int16_t X, int16_t Y)
{
    const char *p = pStr;
    uint8_t outLen = 0;

    dispcolor_ClearScreen();
    dispcolor_Update();
    HAL_Delay(25);

    dispcolor_SetBrightness(brightness);

    // Печатаем посимвольно, но в CP1251-байтах
    while (*p && outLen < (sizeof(Str) - 1)) {
        uint8_t ch = utf8_to_cp1251_next(&p);
        if (ch == 0) break;

        Str[outLen++] = (char)ch;   // кладём CP1251 байт
        Str[outLen]   = 0;

        dispcolor_ClearScreen();
        dispcolor_printf(X, Y, FONTID_16F, RGB565(255, 255, 255), Str);
        dispcolor_Update();
        HAL_Delay(25);
    }

    HAL_Delay(1000);

    for (uint8_t i = 0; i <= 100; i++) {
        dispcolor_SetBrightness(100 - i);
        HAL_Delay(25);
    }

    dispcolor_ClearScreen();
    dispcolor_Update();
    HAL_Delay(500);

    for (uint8_t i = 0; i <= 100; i++) {
        dispcolor_SetBrightness(i);
        HAL_Delay(25);
    }
}
