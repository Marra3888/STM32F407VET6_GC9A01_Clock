//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// ��� ����������� ����������� ���������������� ��������. �� ������ ���������
// ��� �� ����� �����, �� �� �������� ������� ������ �� ��� YouTube-�����
// "����������� � ���������" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// �����: �������� ������ / Nadyrshin Ruslan
//------------------------------------------------------------------------------
//#include <stm32f4xx_hal_gpio.h>
//#include <stm32f4xx_hal_rcc.h>
#include <stm32f4xx_hal.h>

#include "gpio.h"


void gpio_SetGPIOmode_In(GPIO_TypeDef *GPIOx, uint16_t Mask, uint8_t PullMode) {
	gpio_PortClockStart(GPIOx);

	GPIO_InitTypeDef InitStruct;
	InitStruct.Pin = Mask;
	InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	InitStruct.Mode = GPIO_MODE_INPUT;
	InitStruct.Alternate = 0;

	switch (PullMode) {
	case gpio_NoPull:
		InitStruct.Pull = GPIO_NOPULL;
		break;
	case gpio_PullUp:
		InitStruct.Pull = GPIO_PULLUP;
		break;
	case gpio_PullDown:
		InitStruct.Pull = GPIO_PULLDOWN;
		break;
	}

	HAL_GPIO_Init(GPIOx, &InitStruct);
}

void gpio_SetGPIOmode_Out(GPIO_TypeDef *GPIOx, uint16_t Mask) {
	gpio_PortClockStart(GPIOx);

	GPIO_InitTypeDef InitStruct;
	InitStruct.Pin = Mask;
	InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOx, &InitStruct);
}

void gpio_PortClockStart(GPIO_TypeDef *GPIOx) {
	if (GPIOx == GPIOA)
		__HAL_RCC_GPIOA_CLK_ENABLE();
	else if (GPIOx == GPIOB)
		__HAL_RCC_GPIOB_CLK_ENABLE();
	else if (GPIOx == GPIOC)
		__HAL_RCC_GPIOC_CLK_ENABLE();
	else if (GPIOx == GPIOD)
		__HAL_RCC_GPIOD_CLK_ENABLE();
	else if (GPIOx == GPIOE)
		__HAL_RCC_GPIOE_CLK_ENABLE();
}
