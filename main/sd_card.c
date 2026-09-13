#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "img_converters.h"

#include "sd_card.h"
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include <esp_camera.h>

#define SD_PIN_CMD 38 
#define SD_PIN_CLK 39
#define SD_PIN_D0  40

esp_err_t init_sd_card(void) {
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.cmd = SD_PIN_CMD;
    slot_config.clk = SD_PIN_CLK;
    slot_config.d0 = SD_PIN_D0;
    slot_config.width = 1;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 3
    };
    sdmmc_card_t *card;
    esp_err_t err = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

// esp_err_t save_photo_to_sd(camera_fb_t *pic) {

//     char photo_name[50];
//     snprintf(photo_name, sizeof(photo_name), "/sdcard/pic_%lld.rgb565", (long long)pic->timestamp.tv_sec);

//     FILE *file = fopen(photo_name, "wb");
//     if (file == NULL) {
//         return ESP_FAIL;
//     }

//     size_t written = fwrite(pic->buf, 1, pic->len, file);
//     int close_result = fclose(file);

//     if (written != pic->len || close_result != 0) {
//         return ESP_FAIL;
//     }

//     return ESP_OK;
// }

esp_err_t save_photo_to_sd(camera_fb_t *pic) {

    char photo_name[50];
    snprintf(photo_name, sizeof(photo_name), "/sdcard/pic_%lld.jpg", (long long)pic->timestamp.tv_sec);

    uint8_t *jpeg_buffer = NULL;
    size_t jpeg_length = 0;

    bool converted = frame2jpg(
        pic,
        80,
        &jpeg_buffer,
        &jpeg_length
    );

    if (!converted) {
        return ESP_FAIL;
    }

    FILE *file = fopen(photo_name, "wb");
    if (file == NULL) {
        free(jpeg_buffer);
        return ESP_FAIL;
    }

    size_t written = fwrite(
        jpeg_buffer,
        1,
        jpeg_length,
        file
    );

    int close_result = fclose(file);

    free(jpeg_buffer);

    if (written != jpeg_length || close_result != 0) {
        return ESP_FAIL;
    }

    return ESP_OK;
}