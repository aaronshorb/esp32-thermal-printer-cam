#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"

esp_err_t init_lcd(void);

esp_err_t lcd_draw_rgb565(const uint8_t *pixels, uint16_t width, uint16_t height);

esp_err_t init_touch(void);

bool lcd_touch_read(
    uint16_t *x,
    uint16_t *y,
    uint16_t *strength
);

#endif