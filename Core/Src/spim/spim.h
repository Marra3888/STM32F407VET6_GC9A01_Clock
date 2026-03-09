#pragma once
#include <stdint.h>
#include <stm32f4xx_hal.h>

void spim_init(SPI_TypeDef *SPIx, uint8_t fastSpeed);

void SPI_send(SPI_TypeDef *SPIx, uint8_t word16bit, const uint8_t *pBuff, uint16_t Len);
void SPI_send_dma(SPI_TypeDef *SPIx, uint8_t word16bit, const uint8_t *pBuff,
                  uint16_t Len, void (*func)(void));

void SPI_recv(SPI_TypeDef *SPIx, uint8_t word16bit, uint8_t *pBuff, uint16_t Len);
uint16_t SPI_sendrecv(SPI_TypeDef *SPIx, uint8_t word16bit, void *pData);

void SPI_SendRecv_dma(SPI_TypeDef *SPIx, uint8_t word16bit,
                      uint8_t *pTxBuff, uint8_t *pRxBuff,
                      uint16_t Len, void (*func)(void));
