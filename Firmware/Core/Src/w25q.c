#include <w25q.h>
#include "main.h"


#define SPI_ON   SPI1_NSS_GPIO_Port->BRR = SPI1_NSS_Pin
#define SPI_OFF  SPI1_NSS_GPIO_Port->BSRR = SPI1_NSS_Pin
#define HSPI     &hspi1
#define HSPI_DMA &hdma_spi1_tx

static uint8_t bufTx[4];
static uint8_t bufRx[4];

volatile uint8_t dmaFlagSPI = 0;

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  dmaFlagSPI = 1;
}


void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  dmaFlagSPI = 1;
}


static void DMA_Wait()
{
  while (!dmaFlagSPI) {};
  dmaFlagSPI = 0;
}


static void W25_Delay(void)
{
  for (volatile int i = 0; i < 100; i++) {
    asm("nop");
  }
}


void W25_ResetClock(void)
{
  bufTx[0] = 0;
  HAL_SPI_Transmit(HSPI, bufTx, 1, HAL_MAX_DELAY);
  W25_Delay();
}


uint32_t W25_ReadID(void)
{
  uint32_t res;
  bufTx[0] = W25_JEDEC_ID;
  SPI_ON;
  HAL_SPI_TransmitReceive(HSPI, bufTx, bufRx, 4, HAL_MAX_DELAY);
  SPI_OFF;
  res = (bufRx[1] << 16) | (bufRx[2] << 8) | (bufRx[3] << 0);
  return res;
}



void W25_ReadPage(int n, uint8_t *buf)
{
  uint32_t address;
  address = n * W25_PAGE_SIZE;
  bufTx[0] = W25_DATA_READ;
  bufTx[1] = (address >> 16) & 0xFF;
  bufTx[2] = (address >> 8) & 0xFF;
  bufTx[3] = (address >> 0) & 0xFF;
  SPI_ON;
  HAL_SPI_Transmit(HSPI, bufTx, 4, HAL_MAX_DELAY);
  HAL_SPI_Receive_DMA(HSPI, buf, W25_PAGE_SIZE);
  DMA_Wait();
  SPI_OFF;
}


void W25_WriteEnable(void)
{
  bufTx[0] = W25_WRITE_ENABLE;
  SPI_ON;
  HAL_SPI_Transmit(HSPI, bufTx, 1, HAL_MAX_DELAY);
  SPI_OFF;
  W25_Delay();
}


uint8_t W25_GetStatus(void)
{
  bufTx[0] = W25_STATUS;
  SPI_ON;
  HAL_SPI_TransmitReceive(HSPI, bufTx, bufRx, 2, HAL_MAX_DELAY);
  SPI_OFF;
  W25_Delay();
  return (bufRx[1] & 0x01);
}


void W25_WritePage(int pageNum, uint8_t* data, uint16_t size)
{
  uint32_t address = pageNum * W25_PAGE_SIZE;
  bufTx[0] = W25_PAGE_PROGRAM;
  bufTx[1] = (address >> 16) & 0xFF;
  bufTx[2] = (address >> 8) & 0xFF;
  bufTx[3] = 0;
  SPI_ON;
  HAL_SPI_Transmit(HSPI, bufTx, 4, HAL_MAX_DELAY);
  HAL_SPI_Transmit_DMA(HSPI, data, size);
  DMA_Wait();
  SPI_OFF;
  W25_Delay();
}


void W25_ChipErase(void)
{
  bufTx[0] = W25_CHIP_ERASE;
  SPI_ON;
  HAL_SPI_Transmit(HSPI, bufTx, 1, HAL_MAX_DELAY);
  SPI_OFF;
  W25_Delay();
}