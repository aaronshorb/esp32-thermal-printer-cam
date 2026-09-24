# ESP32-S3 Camera

An ESP32-S3 camera project that displays a live camera preview on an LCD and captures JPEG photos to microSD when the touchscreen or shutter button is pressed. Captured photos are converted into dithered monochrome bitmaps and printed on a 58mm thermal printer.

## Features

- Live QVGA RGB565 camera preview
- Touch and button-triggered SVGA JPEG capture saved to microSD card
- Conversion of captured photos to dithered monochrome images
- Printing on a CSN-A2 thermal printer over UART
- Automatic return to live preview after capturing and printing a photo

## Hardware

- ESP32-S3-WROOM CAM board
- OV3660 camera
- ILI9341 `320 × 240` SPI LCD
- Momentary push button
- CASHINO CSN-A2 TTL thermal printer
- Two 18650 batteries and a battery holder
- MP1584EN buck converter

## Wiring

### Camera and SD card

The OV3660 connects through the board's built-in ribbon connector, and the microSD card uses the integrated card slot.

The camera's GPIO assignments are defined in `main/camera_pins.h`. The SD card GPIO assignments are defined in `main/sd_card.c`. The shutter button connects between GPIO 45 and GND and uses the ESP32's internal pull-up resistor.

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

### Thermal printer pins

| Printer Pin | ESP32 Connection |
|---|---|
| GND | GND |
| RX | GPIO 3 (ESP32 TX) |
| TX | GPIO 2 (ESP32 RX) |

The printer is powered directly by two 18650 batteries connected in series. A buck converter connected to the battery pack steps the battery voltage down to 5V, and its output is connected to the ESP32's 5V pin.

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
6. Thermal printer

The camera preview operates in QVGA RGB565 mode and sends each frame directly to the LCD.

When the touchscreen or button is pressed:

1. The current preview framebuffer is returned.
2. The camera switches to SVGA JPEG mode.
3. Warm-up frames are discarded.
4. A JPEG photo is captured and saved to SD card.
5. The JPEG is decoded to RGB565.
6. The image is converted to grayscale and then dithered to monochrome.
7. The monochrome image is packed into a 1-bit bitmap.
8. The bitmap is sent to the thermal printer.
9. The camera switches back to QVGA RGB565 preview mode.
10. Additional warm-up frames are discarded before preview resumes.

## Project structure

```text
main/
  main.c                Application initialization and control flow
  camera_capture.c      Camera configuration and mode switching
  lcd_display.c         LCD drawing and touchscreen handling
  button.c              Shutter button handling
  sd_card.c             SD card mounting and JPEG saving
  image_processing.c    Image conversion and dithering
  thermal_printer.c     UART thermal printer communication
```

## Planned features

- Improved capture feedback
- Touchscreen calibration
- Touchscreen menu for configuring the self timer, dithering algorithm, photo saving, and print options
- Printer status monitoring
