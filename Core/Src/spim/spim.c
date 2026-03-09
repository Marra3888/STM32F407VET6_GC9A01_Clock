#include <stm32f4xx_hal.h>
#include "spim.h"
#include <stdint.h>

extern SPI_HandleTypeDef hspi1;

static void (*spi_end)(void) = 0;

static SPI_HandleTypeDef* spim_get_handle(SPI_TypeDef *SPIx)
{
    if (SPIx == SPI1) return &hspi1;
    return NULL;
}

void spim_init(SPI_TypeDef *SPIx, uint8_t fastSpeed)
{
    (void)SPIx;
    (void)fastSpeed;
    // CubeMX does init
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi1) {
        void (*cb)(void) = spi_end;
        spi_end = 0;
        if (cb) cb();
    }
}

void SPI_send(SPI_TypeDef *SPIx, uint8_t word16bit, const uint8_t *pBuff, uint16_t Len)
{
    (void)word16bit;
    SPI_HandleTypeDef *hspi = spim_get_handle(SPIx);
    if (!hspi || !pBuff || !Len) return;

    HAL_SPI_Transmit(hspi, (uint8_t*)pBuff, Len, HAL_MAX_DELAY);
}

void SPI_send_dma(SPI_TypeDef *SPIx, uint8_t word16bit, const uint8_t *pBuff,
                  uint16_t Len, void (*func)(void))
{
    (void)word16bit;
    SPI_HandleTypeDef *hspi = spim_get_handle(SPIx);
    if (!hspi || !pBuff || !Len) return;

    spi_end = func;

    if (HAL_SPI_Transmit_DMA(hspi, (uint8_t*)pBuff, Len) != HAL_OK) {
        spi_end = 0;
    }
}

void SPI_recv(SPI_TypeDef *SPIx, uint8_t word16bit, uint8_t *pBuff, uint16_t Len)
{
    (void)word16bit;
    SPI_HandleTypeDef *hspi = spim_get_handle(SPIx);
    if (!hspi || !pBuff || !Len) return;

    HAL_SPI_Receive(hspi, pBuff, Len, HAL_MAX_DELAY);
}

uint16_t SPI_sendrecv(SPI_TypeDef *SPIx, uint8_t word16bit, void *pData)
{
    (void)word16bit;
    SPI_HandleTypeDef *hspi = spim_get_handle(SPIx);
    if (!hspi || !pData) return 0;

    uint8_t rx = 0;
    HAL_SPI_TransmitReceive(hspi, (uint8_t*)pData, &rx, 1, HAL_MAX_DELAY);
    return rx;
}

void SPI_SendRecv_dma(SPI_TypeDef *SPIx, uint8_t word16bit,
                      uint8_t *pTxBuff, uint8_t *pRxBuff,
                      uint16_t Len, void (*func)(void))
{
    (void)word16bit;
    SPI_HandleTypeDef *hspi = spim_get_handle(SPIx);
    if (!hspi || !Len) return;
    if (!pTxBuff && !pRxBuff) return;

    spi_end = func;
    if (HAL_SPI_TransmitReceive_DMA(hspi, pTxBuff, pRxBuff, Len) != HAL_OK) {
        spi_end = 0;
    }
}
