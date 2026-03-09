//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// ��� ����������� ����������� ���������������� ��������. �� ������ ���������
// ��� �� ����� �����, �� �� �������� ������� ������ �� ��� YouTube-�����
// "����������� � ���������" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// �����: �������� ������ / Nadyrshin Ruslan
//------------------------------------------------------------------------------
//#include <stm32f4xx_hal_rtc.h>
#include <stm32f4xx_hal.h>

#include <rtc.h>

extern RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef sTime = { 0 };
RTC_DateTypeDef sDate = { 0 };

void RTC_Init(void) {
	__HAL_RCC_RTC_ENABLE();

	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	HAL_RTC_Init(&hrtc);

	sTime.Hours = 0x0;
	sTime.Minutes = 0x0;
	sTime.Seconds = 0x0;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD);

	sDate.WeekDay = 3;
	sDate.Month = 10;
	sDate.Date = 21;
	sDate.Year = 20;
	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
}

void RTC_GetTime(RTC_TimeTypeDef *sTime) {
	HAL_RTC_GetTime(&hrtc, sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void RTC_SetTime(RTC_TimeTypeDef *sTime) {
	HAL_RTC_SetTime(&hrtc, sTime, RTC_FORMAT_BIN);
}
