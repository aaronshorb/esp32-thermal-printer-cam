#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "esp_camera.h"
#include "esp_err.h"
#include "capture_settings.h"

esp_err_t prepare_image_for_printing(
    const camera_fb_t *jpeg,
    uint8_t **out_bitmap,
    size_t *out_length,
    const capture_settings_t *settings
);

#endif
