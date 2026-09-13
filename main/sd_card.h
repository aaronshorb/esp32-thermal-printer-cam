#ifndef SD_CARD_H
#define SD_CARD_H

#include "esp_err.h"
#include "esp_camera.h"

esp_err_t init_sd_card(void);

esp_err_t save_photo_to_sd(camera_fb_t *pic);

#endif