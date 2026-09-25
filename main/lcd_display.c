#include "lcd_display.h"
#include "lcd_pins.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "driver/spi_master.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"

#include "esp_lcd_touch.h"
#include "esp_lcd_touch_xpt2046.h"

#define TOUCH_X_MIN 23
#define TOUCH_X_MAX 307
#define TOUCH_Y_MIN 17
#define TOUCH_Y_MAX 227

static SemaphoreHandle_t lcd_transfer_done;

static esp_lcd_panel_handle_t panel_handle;
static esp_lcd_touch_handle_t touch_handle;

static bool IRAM_ATTR lcd_on_transfer_done(
    esp_lcd_panel_io_handle_t panel_io,
    esp_lcd_panel_io_event_data_t *event_data,
    void *user_ctx
) 
{
    SemaphoreHandle_t semaphore = (SemaphoreHandle_t)user_ctx;
    BaseType_t higher_priority_task_woken = pdFALSE;

    xSemaphoreGiveFromISR(
        semaphore,
        &higher_priority_task_woken
    );

    return higher_priority_task_woken == pdTRUE;
}

esp_err_t init_lcd(void)
{
    lcd_transfer_done = xSemaphoreCreateBinary();

    if (lcd_transfer_done == NULL) {
        return ESP_ERR_NO_MEM;
    }

    spi_bus_config_t bus_config = {
        .sclk_io_num = LCD_PIN_SCLK,
        .mosi_io_num = LCD_PIN_MOSI,
        .miso_io_num = LCD_PIN_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
    };

    esp_err_t err = spi_bus_initialize(LCD_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        return err;
    }

    esp_lcd_panel_io_spi_config_t io_config = 
        ILI9341_PANEL_IO_SPI_CONFIG(
            LCD_PIN_CS, 
            LCD_PIN_DC, 
            lcd_on_transfer_done, 
            lcd_transfer_done);

    io_config.flags.psram_dma_direct = true;

    esp_lcd_panel_io_handle_t io_handle = NULL;

    err = esp_lcd_new_panel_io_spi(
        (esp_lcd_spi_bus_handle_t)LCD_SPI_HOST,
        &io_config,
        &io_handle
    );
    if (err != ESP_OK) {
        return err;
    }

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };

    err = esp_lcd_new_panel_ili9341(
        io_handle,
        &panel_config,
        &panel_handle
    );
    if (err != ESP_OK) {
        return err;
    }

    if ((err = esp_lcd_panel_reset(panel_handle)) != ESP_OK) {
        return err;
    }

    if ((err = esp_lcd_panel_init(panel_handle)) != ESP_OK) {
        return err;
    }

    if ((err = esp_lcd_panel_swap_xy(
        panel_handle,
        true
    )) != ESP_OK) {
        return err;
    }

    if ((err = esp_lcd_panel_mirror(
        panel_handle,
        true,
        true
    )) != ESP_OK) {
        return err;
    }

    return esp_lcd_panel_disp_on_off(panel_handle, true);
}

esp_err_t lcd_draw_rgb565(const uint8_t *pixels, uint16_t width, uint16_t height) {
    esp_err_t err = esp_lcd_panel_draw_bitmap(
        panel_handle,
        0,
        0,
        width,
        height,
        pixels
    );

    if (err != ESP_OK) {
        return err;
    }

    if (xSemaphoreTake(
            lcd_transfer_done,
            pdMS_TO_TICKS(1000)
        ) != pdTRUE) {
            return ESP_ERR_TIMEOUT;
        }
    
        return ESP_OK;
}

esp_err_t init_touch(void)
{
    esp_lcd_panel_io_handle_t touch_io_handle = NULL;

    esp_lcd_panel_io_spi_config_t touch_io_config =
        ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(TOUCH_PIN_CS);

    esp_err_t err = esp_lcd_new_panel_io_spi(
        (esp_lcd_spi_bus_handle_t)LCD_SPI_HOST,
        &touch_io_config,
        &touch_io_handle
    );
    if (err != ESP_OK) {
        return err;
    }

    esp_lcd_touch_config_t touch_config = {
        .x_max = LCD_HEIGHT,
        .y_max = LCD_WIDTH,
        .rst_gpio_num = -1,
        .int_gpio_num = TOUCH_PIN_IRQ,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = true,
            .mirror_x = true,
            .mirror_y = true,
        },
    };

    return esp_lcd_touch_new_spi_xpt2046(
        touch_io_handle,
        &touch_config,
        &touch_handle
    );
}

static uint16_t calibrate_axis(
    uint16_t value,
    uint16_t minimum,
    uint16_t maximum,
    uint16_t output_size
) {
    if (value <= minimum) {
        return 0;
    }

    if (value >= maximum) {
        return output_size - 1;
    }

    return (uint16_t)(
        (uint32_t)(value - minimum) *
        (output_size - 1) /
        (maximum - minimum)
    );
}

bool lcd_touch_read(
    uint16_t *x,
    uint16_t *y,
    uint16_t *strength
)
{
    if (touch_handle == NULL) {
        return false;
    }

    if (esp_lcd_touch_read_data(touch_handle) != ESP_OK) {
        return false;
    }

    esp_lcd_touch_point_data_t point = {0};
    uint8_t point_count = 0;

    esp_err_t err = esp_lcd_touch_get_data(
        touch_handle,
        &point,
        &point_count,
        1
    );

    if (err != ESP_OK || point_count == 0) {
        return false;
    }

    *x = calibrate_axis(
        point.x,
        TOUCH_X_MIN,
        TOUCH_X_MAX,
        LCD_WIDTH
    );

    *y = calibrate_axis(
        point.y,
        TOUCH_Y_MIN,
        TOUCH_Y_MAX,
        LCD_HEIGHT
    );
    
    *strength = point.strength;

    return true;
}