#ifndef THERMAL_PRINTER_H
#define THERMAL_PRINTER_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

esp_err_t init_thermal_printer(void);

esp_err_t thermal_printer_write(
    const uint8_t *data,
    size_t length
);

esp_err_t thermal_printer_print_bitmap(
    const uint8_t *bitmap,
    size_t bitmap_length,
    size_t width,
    size_t height
);

#endif