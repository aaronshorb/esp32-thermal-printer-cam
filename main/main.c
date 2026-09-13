#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_camera.h"

#include "camera_capture.h"
#include "lcd_display.h"
#include "sd_card.h"

void app_main(void)
{
    ESP_ERROR_CHECK(init_camera());

    ESP_ERROR_CHECK(init_lcd());

    ESP_ERROR_CHECK(init_touch());

    ESP_ERROR_CHECK(init_sd_card());

    bool was_touched = false;

    while (1) {

        uint16_t x;
        uint16_t y;
        uint16_t strength;

        bool touched = lcd_touch_read(&x, &y, &strength);

        if (touched && !was_touched) {
            printf(
                "Touch: x=%u, y=%u, pressure=%u\n",
                x,
                y,
                strength
            );
        }

        camera_fb_t *pic = esp_camera_fb_get();

        if (pic == NULL) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        ESP_ERROR_CHECK(            
            lcd_draw_rgb565(
                pic->buf,
                pic->width,
                pic->height
            )
        );

        if (touched && !was_touched) {
            esp_err_t save_error = save_photo_to_sd(pic);
            if (save_error == ESP_OK) {
                vTaskDelay(pdMS_TO_TICKS(1750));
            } else {
                printf("Failed to save photo: %s\n", esp_err_to_name(save_error));
            }
        }
        
        was_touched = touched;
        esp_camera_fb_return(pic);
    }
}
