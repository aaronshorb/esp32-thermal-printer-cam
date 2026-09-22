# ESP32-S3 Camera

An ESP32 camera project that displays a live camera preview on an LCD and captures a JPEG to microSD card when the touchscreen or button is pressed. Captured photos are converted into dithered monochrome images.

The project is intended to later support printing photos on a thermal printer.

## Features

- Live QVGA RGB565 camera preview
- Touch and button-triggered SVGA JPEG capture saved to microSD card
- Automatic return to live preview after capturing a photo
- Conversion of captured photos to dithered monochrome images

## Hardware

- ESP32-S3-WROOM CAM board
- OV3660 camera
- ILI9341 `320 × 240` SPI LCD
- Momentary push button

## Wiring

### Camera and SD card

The OV3660 connects through the board's built-in ribbon connector, and the microSD card uses the integrated card slot.

The camera's GPIO assignments are defined in `main/camera_pins.h`. The SD card GPIO assignments are defined in `main/sd_card.c`. The shutter button connects between GPIO 2 and GND and uses the ESP32's internal pull-up resistor.

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
- `espressif/button`
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
4. Push button
5. SD card

The camera preview operates in QVGA RGB565 mode and sends each frame directly to the LCD.

When the touchscreen or button is pressed:

1. The current preview framebuffer is returned.
2. The camera switches to SVGA JPEG mode.
3. Warm-up frames are discarded.
4. A JPEG photo is captured and saved to SD card.
5. The JPEG is decoded to RGB565.
6. The image is converted to grayscale and then dithered to monochrome.
7. The camera switches back to QVGA RGB565 preview mode.
8. Additional warm-up frames are discarded before preview resumes.

## Project structure

```text
main/
  main.c                Application initialization and control flow
  camera_capture.c      Camera configuration and mode switching
  lcd_display.c         LCD drawing and touchscreen handling
  button.c              Shutter button handling
  sd_card.c             SD card mounting and JPEG saving
  image_processing.c    Image conversion and dithering
```

## Planned features

- Improved capture feedback
- Touchscreen calibration
- Support for thermal printer
