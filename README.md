# ESP32 Thermal Printer Camera

An ESP32-S3 camera project that displays a live camera preview on an LCD and captures JPEG photos when the shutter button is pressed. Photos can be saved to microSD, converted into dithered monochrome bitmaps, and printed on a 58mm thermal printer.

## Features

- Live QVGA RGB565 camera preview
- Button-triggered SVGA JPEG capture with optional microSD storage
- Touchscreen controls for the timer, saving, printing, and dithering
- Floyd–Steinberg, Bayer, and clustered-dot dithering
- Monochrome photo printing on a CSN-A2 thermal printer over UART

## Hardware

- ESP32-S3-WROOM CAM board
- OV3660 camera
- ILI9341 `320 × 240` SPI LCD
- Momentary push button
- On/Off switch
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

After startup, the LCD displays a live QVGA camera preview.

The touchscreen menu controls:

- Timer: off, 3 seconds, or 10 seconds
- Saving to microSD: on or off
- Thermal printing: on or off
- Dithering: Floyd–Steinberg, Bayer, or clustered dot

When the shutter button is pressed:

1. If enabled, the countdown is displayed over the live preview.
2. The camera captures an SVGA JPEG photo.
3. If saving is enabled, the JPEG is saved to microSD.
4. If printing is enabled, the JPEG is converted to a dithered 1-bit bitmap and printed.
5. The live preview resumes.

## Project structure

```text
main/
  main.c                Application initialization and control flow
  camera_capture.c      Camera configuration and mode switching
  lcd_display.c         LCD drawing and touchscreen handling
  menu.c                Touchscreen menu rendering and settings
  button.c              Shutter button handling
  sd_card.c             SD card mounting and JPEG saving
  image_processing.c    Image conversion and dithering
  thermal_printer.c     UART thermal printer communication
```

## Planned features

- Printer status monitoring
