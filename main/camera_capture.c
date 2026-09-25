#include "camera_capture.h"

#include "camera_pins.h"

#include "esp_err.h"
#include "esp_camera.h"

static const camera_config_t preview_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,

    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_RGB565,
    .frame_size = FRAMESIZE_QVGA,   

    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

static esp_err_t rotate_camera(void) {
    sensor_t *sensor = esp_camera_sensor_get();

    if (sensor == NULL) {
        return ESP_FAIL;
    }

    if (
        sensor->set_hmirror(sensor, 1) != 0 ||
        sensor->set_vflip(sensor, 1) != 0
    ) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t init_camera(void) {
    esp_err_t err = esp_camera_init(&preview_config);

    if (err != ESP_OK) {
        return err;
    }

    return rotate_camera();
}

esp_err_t camera_set_preview_mode(void) {
    esp_err_t err = esp_camera_reconfigure(&preview_config);

    if (err != ESP_OK) {
        return err;
    }

    return rotate_camera();
}

esp_err_t camera_set_photo_mode(void) {
    camera_config_t photo_config = preview_config;
    photo_config.pixel_format = PIXFORMAT_JPEG;
    photo_config.frame_size = FRAMESIZE_SVGA;
    photo_config.jpeg_quality = 8;

    esp_err_t err = esp_camera_reconfigure(&photo_config);

    if (err != ESP_OK) {
        return err;
    }

    return rotate_camera();
}

esp_err_t camera_discard_frames(uint8_t count) {
    for (int i = 0; i < count; i++) {
        camera_fb_t *frame = esp_camera_fb_get();
        
        if (frame == NULL) {
            return ESP_FAIL;
        }

        esp_camera_fb_return(frame);
    }
    return ESP_OK;
}