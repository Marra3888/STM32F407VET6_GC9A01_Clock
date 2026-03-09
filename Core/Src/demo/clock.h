//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// ��� ����������� ����������� ���������������� ��������. �� ������ ���������
// ��� �� ����� �����, �� �� �������� ������� ������ �� ��� YouTube-�����
// "����������� � ���������" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// �����: �������� ������ / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#ifndef SRC_DEMO_CLOCK_H_
#define SRC_DEMO_CLOCK_H_


void DrawClock(uint8_t hour, uint8_t min, uint8_t sec, uint8_t light, uint8_t secBubbles);
void Clock_SetEdit(uint8_t editField, uint8_t blink);
void Clock5_Reset(void);
void DrawClock5(uint8_t hour, uint8_t min, uint8_t sec);


#endif /* SRC_DEMO_CLOCK_H_ */
