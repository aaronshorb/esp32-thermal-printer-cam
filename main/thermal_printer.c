#include "thermal_printer.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define UART_PORT_NUM       UART_NUM_1
#define UART_RX_PIN         GPIO_NUM_2
#define UART_TX_PIN         GPIO_NUM_3
#define UART_BAUD_RATE      115200
#define UART_BUFFER_SIZE    1024

esp_err_t thermal_printer_write(const uint8_t *data, size_t length) {
    if (data == NULL || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    int bytes_written = uart_write_bytes(UART_PORT_NUM, data, length);

    if (bytes_written < 0 || (size_t)bytes_written != length) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t thermal_printer_print_bitmap(
    const uint8_t *bitmap,
    size_t bitmap_length,
    size_t width,
    size_t height
) {
    if (
        bitmap == NULL ||
        width != 384 ||
        height == 0 ||
        height > 300
    ) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t command[] = {
        0x12,
        0x56,
        (uint8_t)(height & 0xFF),
        (uint8_t)(height >> 8)
    };
    
    esp_err_t err = thermal_printer_write(
        command,
        sizeof(command)
    );

    if (err != ESP_OK) {
        return err;
    }

    err = thermal_printer_write(bitmap, bitmap_length);

    if (err != ESP_OK) {
        return err;
    }

    static const uint8_t feed_two_lines[] = {
        0x1B, 0x64, 0x02
    };

    return thermal_printer_write(
        feed_two_lines,
        sizeof(feed_two_lines)
    );
}

esp_err_t init_thermal_printer(void) {
    uart_config_t uart_config = {
        .baud_rate  = UART_BAUD_RATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    esp_err_t err = uart_driver_install(
        UART_PORT_NUM, 
        UART_BUFFER_SIZE * 2, 
        0, 
        0, 
        NULL, 
        0
    );

    if (err != ESP_OK) {
        return err;
    }

    err = uart_param_config(UART_PORT_NUM,&uart_config);

    if (err != ESP_OK) {
        uart_driver_delete(UART_PORT_NUM);
        return err;
    }

    err = uart_set_pin(
        UART_PORT_NUM,
        UART_TX_PIN,
        UART_RX_PIN,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );

    if (err != ESP_OK) {
        uart_driver_delete(UART_PORT_NUM);
        return err;
    }

    static const uint8_t initialize_command[] = {0x1B, 0x40};

    err = thermal_printer_write(initialize_command, sizeof(initialize_command));

    if (err != ESP_OK) {
        uart_driver_delete(UART_PORT_NUM);
        return err;
    }

    return ESP_OK;
}
