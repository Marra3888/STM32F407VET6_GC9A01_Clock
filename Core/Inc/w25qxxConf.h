#ifndef _W25QXXCONFIG_H
#define _W25QXXCONFIG_H

#include "main.h"

// Этот SPI не используется (так как у нас Soft SPI), но нужен для компиляции
extern SPI_HandleTypeDef hspi1;
#define _W25QXX_SPI                   hspi1

// Настройки пина Chip Select (PB0)
#define _W25QXX_CS_GPIO               GPIOB
#define _W25QXX_CS_PIN                GPIO_PIN_0

#define _W25QXX_USE_FREERTOS          0
#define _W25QXX_DEBUG                 0

#endif
