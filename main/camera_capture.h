#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H

#include <stdint.h>

#include "esp_err.h"

esp_err_t init_camera(void);

esp_err_t camera_set_preview_mode(void);

esp_err_t camera_set_photo_mode(void);

esp_err_t camera_discard_frames(uint8_t count);

#endif