
#include "main.h"
#include "local.hpp"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"

#define SPI1_SCK_Pin GPIO_PIN_5
#define SPI1_SCK_GPIO_Port GPIOA
#define SPI1_MISO_Pin GPIO_PIN_6
#define SPI1_MISO_GPIO_Port GPIOA
#define SPI1_MOSI_Pin GPIO_PIN_7
#define SPI1_MOSI_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA

void dump_gpio_info(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {

    GPIO_PinState value = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
    uprintf("	GPIO port: %p pin: %p value: %d ", GPIOx, GPIO_Pin, value);

    uint32_t moder = (GPIOx->MODER >> (GPIO_Pin * 2)) & 0x3;
    uprintf(" mode: %s", moder == 0 ? "Input" : moder == 1 ? "Output" : moder == 2 ? "Alternate Function" : "Analog");

    uint32_t otype = (GPIOx->OTYPER >> GPIO_Pin) & 0x1;
    uprintf(" output type: %s", otype == 0 ? "Push-Pull" : "Open-Drain");

    uint32_t ospeed = (GPIOx->OSPEEDR >> (GPIO_Pin * 2)) & 0x3;
    uprintf(" output speed: %s", ospeed == 0 ? "Low" : ospeed == 1 ? "Medium" : ospeed == 2 ? "Fast" : "High");

    uint32_t pupd = (GPIOx->PUPDR >> (GPIO_Pin * 2)) & 0x3;
    uprintf(" pull-up/pull-down: %s\r\n", pupd == 0 ? "No Pull-Up/Down" : pupd == 1 ? "Pull-Up" : pupd == 2 ? "Pull-Down" : "Reserved");
}

void dump_spi1_gpio_info() {
  uprintf(" SPI1_SCK: ");   dump_gpio_info(SPI1_SCK_GPIO_Port,  SPI1_SCK_Pin);
  uprintf(" SPI1_MISO: ");  dump_gpio_info(SPI1_MISO_GPIO_Port, SPI1_MISO_Pin);
  uprintf(" SPI1_MOSI: ");  dump_gpio_info(SPI1_MOSI_GPIO_Port, SPI1_MOSI_Pin);
  uprintf(" SPI1_CS: ");    dump_gpio_info(SPI1_CS_GPIO_Port,   SPI1_CS_Pin);

  /*
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = SPI1_MISO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(SPI1_MISO_GPIO_Port, &GPIO_InitStruct);

  uprintf(" as ouput SPI1_MISO: ");  dump_gpio_info(SPI1_MISO_GPIO_Port, SPI1_MISO_Pin);
  while (true) {
    HAL_GPIO_TogglePin(LD6_GPIO_Port, LD6_Pin);
    HAL_GPIO_TogglePin(SPI1_MISO_GPIO_Port, SPI1_MISO_Pin);
    HAL_Delay(1000);
  }
  */

  /*
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  HAL_GPIO_Init(SPI1_MISO_GPIO_Port, &GPIO_InitStruct);
  uprintf(" revert SPI1_MISO: ");  dump_gpio_info(SPI1_MISO_GPIO_Port, SPI1_MISO_Pin);
  while (true) {
    HAL_GPIO_TogglePin(LD6_GPIO_Port, LD6_Pin);
    HAL_GPIO_TogglePin(SPI1_MISO_GPIO_Port, SPI1_MISO_Pin);
    HAL_Delay(100);
  }
  */

}

void process_spi_msg(SPI_HandleTypeDef *hspi) {
  HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, &tx, &rx, 1, HAL_MAX_DELAY);
  uint32_t spiFlags = hspi->Instance->SR;
  addTickMsg(spiFlags, status, tx, rx);
  tx = rx;
}

uint8_t buffer[1];
void process_spi_msg2(SPI_HandleTypeDef *hspi) {
  uint32_t spiFlags = hspi->Instance->SR;
  bool is_busy = spiFlags & SPI_FLAG_BSY;
  bool is_overrun = spiFlags & SPI_FLAG_OVR;
  bool is_rx_buffer_empty = ! (spiFlags & SPI_FLAG_RXNE);
  bool is_tx_buffer_empty = spiFlags & SPI_FLAG_TXE;

  if (is_busy) {
    HAL_GPIO_TogglePin(LD5_GPIO_Port, LD5_Pin); // red
    //uprintf("SPI status: Busy : %d\r\n", spiFlags);
    //return;
  }

  if (is_overrun) {
    HAL_GPIO_TogglePin(LD5_GPIO_Port, LD5_Pin); // red
    __HAL_SPI_CLEAR_OVRFLAG(hspi);
    uprintf("SPI status: overrun : %d\r\n", spiFlags);
    //return;
  }

  // ok is overrun, so first wait for entire overrun byte to be sent
  /*
  GPIO_PinState nss = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);
  if (nss == GPIO_PIN_RESET) {
    uprintf("SPI status: nss/pa4 still active : %d\r\n", spiFlags);
    HAL_GPIO_TogglePin(LD5_GPIO_Port, LD5_Pin); // red
    return;
  }
  */

  if (is_rx_buffer_empty) {
    uprintf("SPI status: rx_buffer_is_empty : %d\r\n", spiFlags);
    HAL_GPIO_TogglePin(LD5_GPIO_Port, LD5_Pin); // red
    return;
  }
  
  if (! is_tx_buffer_empty) { 
    uprintf("SPI status: tx_buffer not empty : %d\r\n", spiFlags);
    HAL_GPIO_TogglePin(LD5_GPIO_Port, LD5_Pin); // red
    return;
  }

  // clear overflow and receive data ,. assume RXNE
  // uprintf("tx: 0x%2X 0x%2X 0x%2X 0x%2X\r\n", buffer[0], buffer[1], buffer[2], buffer[3]);
  uprintf("tx: 0x%2X ", buffer[0]);
  HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, buffer, buffer, 1, HAL_MAX_DELAY);
  switch (status) {
     case HAL_OK:       break;
     case HAL_BUSY:     uprintf("SPI R HAL_BUSY\r\n");
     case HAL_ERROR:    uprintf("SPI R HAL_ERROR\r\n");
     case HAL_TIMEOUT:  uprintf("SPI R HAL_TIMEOUT\r\n");
   }
   uprintf("rx: 0x%2X \r\n", buffer[0]);

  return;
}
