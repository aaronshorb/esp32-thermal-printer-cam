# ESP32-S3 Camera

An ESP32 camera project that displays a live camera preview on an LCD and captures a JPEG to microSD card when the touchscreen is pressed.

The project is intended to later support printing photos on a thermal printer.

## Features

- Live QVGA RGB565 camera preview
- Touch-triggered SVGA JPEG capture saved to microSD card
- Automatic return to live preview after capturing a photo

## Hardware

- ESP32-S3-WROOM CAM board
- OV3660 camera
- ILI9341 `320 × 240` SPI LCD

## Wiring

### Camera and SD card

The OV3660 connects through the board's built-in ribbon connector, and the microSD card uses the integrated card slot.

The camera's GPIO assignments are defined in `main/camera_pins.h`. The SD card GPIO assignments are defined in `main/sd_card.c`.

### LCD and touchscreen pins

| LCD Pin | ESP32 Connection |
|---|---|
| VCC | 3.3V |
| GND | GND |
| CS | GPIO 42 |
| RESET | Not connected |
| DC | GPIO 41 |
| MOSI | GPIO 47 |
| SCK | GPIO 21 |
| LED | 3.3V |
| MISO | GPIO 14 |
| T_CLK | GPIO 21 |
| T_CS | GPIO 1 |
| T_DIN | GPIO 47 |
| T_DO | GPIO 14 |
| T_IRQ | Not connected |

## Dependencies

- `espressif/esp32-camera`
- `espressif/esp_lcd_ili9341`
- `atanisoft/esp_lcd_touch_xpt2046`

## Build and flash

Activate the ESP-IDF environment, then run:

```bash
idf.py set-target esp32s3
idf.py build
idf.py flash monitor
```

## Operation

At startup, the application initializes:

1. Camera
2. LCD
3. Touchscreen
4. SD card

The camera preview operates in QVGA RGB565 mode and sends each frame directly to the LCD.

When the touchscreen is pressed:

1. The current preview framebuffer is returned.
2. The camera switches to SVGA JPEG mode.
3. Warm-up frames are discarded.
4. A JPEG photo is captured.
5. The JPEG data is saved directly to the SD card.
6. The camera switches back to QVGA RGB565 preview mode.
7. Additional warm-up frames are discarded before preview resumes.

## Project structure

```text
main/
  main.c              Application initialization and main capture loop
  camera_capture.c    Camera configuration and mode switching
  lcd_display.c       LCD drawing and touchscreen handling
  sd_card.c           SD card mounting and JPEG saving
```

## Current limitations

- Timestamp filenames may repeat after restarting the ESP32 and could overwrite an existing photo.

## Planned features

- Sequential photo filenames
- Improved capture feedback
- Touchscreen calibration
- Monochrome photo conversion for thermal printing and support for thermal printer