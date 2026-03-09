//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// ��� ����������� ����������� ���������������� ��������. �� ������ ���������
// ��� �� ����� �����, �� �� �������� ������� ������ �� ��� YouTube-�����
// "����������� � ���������" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// �����: �������� ������ / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#ifndef TMR11_H
#define TMR11_H

#include <stdint.h>

void tmr11_start(void);
void tmr11_stop(void);

/**
 * Инициализация PWM на TIM11.
 * MaxValue — период (ARR), Value — начальная скважность (CCR).
 */
//void tmr11_PWM_init(uint16_t MaxValue, uint16_t Value);
//
///** Установить значение PWM (CCR) */
//void tmr11_PWM_set(uint16_t Value);

#define tmr11_PWM_init(a,b)  ((void)0)
#define tmr11_PWM_set(v)     ((void)0)

#endif // TMR11_H
