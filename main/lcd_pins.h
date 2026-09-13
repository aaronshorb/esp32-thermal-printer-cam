#ifndef LCD_PINS_H
#define LCD_PINS_H

#include "driver/gpio.h"
#include "driver/spi_master.h"

#define LCD_SPI_HOST      SPI2_HOST
#define LCD_PIN_SCLK      GPIO_NUM_21
#define LCD_PIN_MOSI      GPIO_NUM_47
#define LCD_PIN_MISO      GPIO_NUM_14
#define LCD_PIN_CS        GPIO_NUM_42
#define LCD_PIN_DC        GPIO_NUM_41
#define LCD_PIN_RST       (-1)
#define LCD_PIN_BL        (-1)
#define TOUCH_PIN_CS      GPIO_NUM_1
#define TOUCH_PIN_IRQ     (-1)
#define LCD_WIDTH         320
#define LCD_HEIGHT        240

#endif