#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>

#include "sd_card.h"

#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"

#define SD_PIN_CMD 38 
#define SD_PIN_CLK 39
#define SD_PIN_D0  40

static uint32_t next_photo_number = 1;
static esp_err_t find_next_photo_number(void);

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

    return find_next_photo_number();
}

esp_err_t save_photo_to_sd(camera_fb_t *pic) {
    if (
        pic == NULL ||
        pic->buf == NULL ||
        pic->len == 0 ||
        pic->format != PIXFORMAT_JPEG
    ) {
        return ESP_ERR_INVALID_ARG;
    }

    char photo_name[50];
    snprintf(
        photo_name,
        sizeof(photo_name), 
        "/sdcard/esp32_cam_%06" PRIu32 ".jpg", 
        next_photo_number
    );

    FILE *file = fopen(photo_name, "wb");
    if (file == NULL) {
        return ESP_FAIL;
    }

    size_t written = fwrite(
        pic->buf,
        1,
        pic->len,
        file
    );

    int close_result = fclose(file);

    if (written != pic->len || close_result != 0) {
        return ESP_FAIL;
    }

    next_photo_number++;
    
    printf("Photo name: %s\n", photo_name);

    return ESP_OK;
}

static bool get_photo_number(const char *filename, uint32_t *photo_number) {
    int end = 0;

    if (sscanf(
        filename,
        "esp32_cam_%" SCNu32 ".jpg%n",
        photo_number,
        &end
    ) != 1) {
        return false;
    }

    return filename[end] == '\0';
}

static esp_err_t find_next_photo_number(void) {
    DIR *dir = opendir("/sdcard");

    if (dir == NULL) {
        return ESP_FAIL;
    }

    uint32_t max_photo_number = 0;

    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        uint32_t photo_number;

        if (!get_photo_number(de->d_name, &photo_number)) {
            continue;
        }

        if (photo_number > max_photo_number) {
            max_photo_number = photo_number;
        }
    }

    next_photo_number = max_photo_number + 1;
    
    closedir(dir);
    return ESP_OK;
}