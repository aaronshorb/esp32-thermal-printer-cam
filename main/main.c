#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_camera.h"

#include "camera_capture.h"
#include "lcd_display.h"
#include "button.h"
#include "sd_card.h"
#include "image_processing.h"
#include "thermal_printer.h"

void app_main(void)
{
    ESP_ERROR_CHECK(init_camera());

    ESP_ERROR_CHECK(init_lcd());

    ESP_ERROR_CHECK(init_touch());

    ESP_ERROR_CHECK(init_shutter_button());

    ESP_ERROR_CHECK(init_sd_card());

    ESP_ERROR_CHECK(init_thermal_printer());

    bool was_touched = false;
    ESP_ERROR_CHECK(camera_discard_frames(3));

    while (1) {

        uint16_t x;
        uint16_t y;
        uint16_t strength;

        bool touched = lcd_touch_read(&x, &y, &strength);

        bool touch_requested = touched && !was_touched;
        bool button_requested = shutter_button_take_request();
        bool capture_requested = touch_requested || button_requested;

        if (touch_requested) {
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

        if (capture_requested) {
            esp_camera_fb_return(pic);

            esp_err_t err = camera_set_photo_mode();

            if (err == ESP_OK) {
                err = camera_discard_frames(3);
            }

            if (err == ESP_OK) {
                camera_fb_t *photo = esp_camera_fb_get();

                if (photo == NULL) {
                    err = ESP_FAIL;
                } else {
                    uint8_t *bitmap = NULL;
                    size_t bitmap_length = 0;
                    
                    err = save_photo_to_sd(photo);

                    if (err == ESP_OK) {
                        err = prepare_image_for_printing(
                            photo,
                            &bitmap,
                            &bitmap_length
                        );
                    }
                    
                    esp_camera_fb_return(photo);

                    if (err == ESP_OK) {
                        err = thermal_printer_print_bitmap(
                            bitmap,
                            bitmap_length,
                            384,
                            300
                        );
                    }

                    free(bitmap);
                }
            }

            if (err != ESP_OK) {
                printf("Photo capture failed: %s\n", esp_err_to_name(err));
            }

            ESP_ERROR_CHECK(camera_set_preview_mode());
            ESP_ERROR_CHECK(camera_discard_frames(3));

            (void)shutter_button_take_request();
            
            was_touched = touched;
            continue;
        }
        
        was_touched = touched;
        esp_camera_fb_return(pic);
    }
}
