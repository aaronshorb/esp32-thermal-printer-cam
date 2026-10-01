#ifndef MENU_H
#define MENU_H

#include <stddef.h>
#include <stdint.h>

#include "capture_settings.h"

typedef enum {
    MENU_ITEM_NONE,
    MENU_ITEM_TIMER,
    MENU_ITEM_SAVE,
    MENU_ITEM_PRINT,
    MENU_ITEM_DITHERING
} menu_item_t;

menu_item_t menu_item_from_touch(uint16_t x, uint16_t y);

void menu_apply_selection(
    menu_item_t item,
    capture_settings_t *settings
);

void menu_overlay(
    uint16_t *frame,
    size_t frame_width,
    size_t frame_height,
    const capture_settings_t *settings
);

void menu_draw_countdown(
    uint16_t *frame,
    size_t frame_width,
    size_t frame_height,
    uint8_t seconds_remaining
);

#endif