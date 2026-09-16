#include <stdbool.h>

#include "button.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "iot_button.h"
#include "button_gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define BUTTON_PIN GPIO_NUM_2

static button_handle_t shutter_button;
static SemaphoreHandle_t capture_request;

static void shutter_pressed_callback(
    void *button_handle,
    void *user_data
)
{
    (void)button_handle;
    (void)user_data;

    xSemaphoreGive(capture_request);
}

esp_err_t init_shutter_button(void) {
    capture_request = xSemaphoreCreateBinary();

    if (capture_request == NULL) {
        return ESP_ERR_NO_MEM;
    }

    const button_config_t btn_cfg = {0};

    const button_gpio_config_t gpio_cfg = {
        .gpio_num = BUTTON_PIN,
        .active_level = 0,
    };

    esp_err_t err = iot_button_new_gpio_device(
        &btn_cfg,
        &gpio_cfg,
        &shutter_button
    );

    if (err != ESP_OK) {
        return err;
    }

    return iot_button_register_cb(
        shutter_button,
        BUTTON_PRESS_DOWN,
        NULL,
        shutter_pressed_callback,
        NULL
    );
}

bool shutter_button_take_request(void) {
    return xSemaphoreTake(capture_request, 0) == pdTRUE;
}
