
#include "main.h"
#include <stdio.h>
#include "stm32f4xx_hal.h"

#define SPI1_SCK_Pin GPIO_PIN_5
#define SPI1_SCK_GPIO_Port GPIOA
#define SPI1_MISO_Pin GPIO_PIN_6
#define SPI1_MISO_GPIO_Port GPIOA
#define SPI1_MOSI_Pin GPIO_PIN_7
#define SPI1_MOSI_GPIO_Port GPIOA

void dump_gpio_info(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    printf("	GPIO port: %p pin: %d\n", GPIOx, GPIO_Pin);

    uint32_t moder = (GPIOx->MODER >> (GPIO_Pin * 2)) & 0x3;
    printf("	GPIO mode: %s\n", moder == 0 ? "Input" : moder == 1 ? "Output" : moder == 2 ? "Alternate Function" : "Analog");

    uint32_t otype = (GPIOx->OTYPER >> GPIO_Pin) & 0x1;
    printf("	GPIO output type: %s\n", otype == 0 ? "Push-Pull" : "Open-Drain");

    uint32_t ospeed = (GPIOx->OSPEEDR >> (GPIO_Pin * 2)) & 0x3;
    printf("	GPIO output speed: %s\n", ospeed == 0 ? "Low" : ospeed == 1 ? "Medium" : ospeed == 2 ? "Fast" : "High");

    uint32_t pupd = (GPIOx->PUPDR >> (GPIO_Pin * 2)) & 0x3;
    printf("	GPIO pull-up/pull-down: %s\n", pupd == 0 ? "No Pull-Up/Down" : pupd == 1 ? "Pull-Up" : pupd == 2 ? "Pull-Down" : "Reserved");
}

void dump_spi1_gpio_info() {
    dump_gpio_info(GPIOA, GPIO_PIN_4);
}
