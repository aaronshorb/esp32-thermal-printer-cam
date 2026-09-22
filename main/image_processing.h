#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "esp_camera.h"
#include "esp_err.h"

esp_err_t prepare_image_for_printing(const camera_fb_t *jpeg);

#endif
