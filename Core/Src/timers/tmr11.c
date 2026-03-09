//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// ��� ����������� ����������� ���������������� ��������. �� ������ ���������
// ��� �� ����� �����, �� �� �������� ������� ������ �� ��� YouTube-�����
// "����������� � ���������" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// �����: �������� ������ / Nadyrshin Ruslan
//------------------------------------------------------------------------------
//#include <stm32f4xx_hal_rcc.h>
//#include <stm32f4xx_hal_tim.h>
//#include <stm32f4xx_hal_gpio.h>
//#include "timers/tmr11.h"
//#include "timers.h"
#include <stm32f4xx_hal.h>
#include <gpio.h>

extern TIM_HandleTypeDef htim11;
TIM_OC_InitTypeDef sConfigOC = { 0 };

//==============================================================================
// ��������� ��������� ������ 1-4 ������� 2 � ������ ���
//==============================================================================
void tmr11_PWM_init(uint16_t MaxValue, uint16_t Value) {
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF3_TIM11; //GPIO_AF5_SPI1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	htim11.Instance = TIM11;
	htim11.Init.Prescaler = 1000;
	htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim11.Init.Period = 100 - 1;
	htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; // TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&htim11);
	HAL_TIM_PWM_Init(&htim11);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 50;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

	HAL_TIM_PWM_ConfigChannel(&htim11, &sConfigOC, TIM_CHANNEL_1);
	HAL_TIM_PWM_MspInit(&htim11);
}
//==============================================================================

//==============================================================================
// ��������� ��������� �������� ���
//==============================================================================
void tmr11_PWM_set(uint16_t Value) {
	htim11.Instance->CCR1 = Value;
}
//==============================================================================

//==============================================================================
// ��������� �������� ������
//==============================================================================
void tmr11_start(void) {
	HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);
}
//==============================================================================

//==============================================================================
// ��������� ������������� ������
//==============================================================================
void tmr11_stop(void) {
	HAL_TIM_PWM_Stop(&htim11, TIM_CHANNEL_1);
}
//==============================================================================
