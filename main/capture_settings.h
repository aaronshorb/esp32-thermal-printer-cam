#ifndef CAPTURE_SETTINGS_H
#define CAPTURE_SETTINGS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    DITHERING_FLOYD_STEINBERG,
    DITHERING_BAYER,
    DITHERING_CLUSTERED_DOT,
} dithering_algorithm_t;

typedef struct {
    bool save_photo;
    bool print_photo;
    uint8_t timer_seconds;
    dithering_algorithm_t dithering;
} capture_settings_t;

#endif