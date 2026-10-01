#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "menu.h"
#include "font8x8_basic.h"
#include "menu_icons.h"

menu_item_t menu_item_from_touch(uint16_t x, uint16_t y) {
    if (x < 70) {
        if (y < 60) {
            return MENU_ITEM_TIMER;
        } else if (y < 120) {
            return MENU_ITEM_SAVE;
        } else if (y < 180) {
            return MENU_ITEM_PRINT;
        }
    }

    if (x < 160 && y >= 180) {
        return MENU_ITEM_DITHERING;
    }

    return MENU_ITEM_NONE;
}

void menu_apply_selection(
    menu_item_t item,
    capture_settings_t *settings
){
    if (settings == NULL) {
        return;
    }

    switch (item) {
        case MENU_ITEM_SAVE:
            settings->save_photo = !settings->save_photo;
            break;

        case MENU_ITEM_PRINT:
            settings->print_photo = !settings->print_photo;
            break;

        case MENU_ITEM_TIMER:
            if (settings->timer_seconds == 0) {
                settings->timer_seconds = 3;
            } else if (settings->timer_seconds == 3) {
                settings->timer_seconds = 10;
            } else {
                settings->timer_seconds = 0;
            }
            break;

        case MENU_ITEM_DITHERING:
            switch (settings->dithering) {
                case DITHERING_FLOYD_STEINBERG:
                    settings->dithering = DITHERING_BAYER;
                    break;

                case DITHERING_BAYER:
                    settings->dithering = DITHERING_CLUSTERED_DOT;
                    break;

                case DITHERING_CLUSTERED_DOT:
                    settings->dithering = DITHERING_FLOYD_STEINBERG;
                    break;
            }
            break;


        case MENU_ITEM_NONE:
            break;
    }
}

static void draw_text(
    uint16_t *frame,
    size_t frame_width,
    size_t frame_height,
    size_t start_x,
    size_t start_y,
    const char *text,
    uint16_t color,
    size_t scale
) {
    if (frame == NULL || text == NULL || scale == 0) {
        return;
    }

    while (*text != '\0') {
        uint8_t character = (uint8_t)*text;

        if (character < 128) {
            for (size_t row = 0; row < 8; row++) {
                uint8_t row_pixels =
                    (uint8_t)font8x8_basic[character][row];

                for (size_t column = 0; column < 8; column++) {
                    bool pixel_is_set =
                        ((row_pixels >> column) & 1U) != 0;

                    if (!pixel_is_set) {
                        continue;
                    }

                    for (size_t scale_y = 0; scale_y < scale; scale_y++) {
                        for (size_t scale_x = 0; scale_x < scale; scale_x++) {
                            size_t x = start_x + column * scale + scale_x;
                            size_t y = start_y + row * scale + scale_y;

                            if (x < frame_width && y < frame_height) {
                                frame[y * frame_width + x] = color;
                            }
                        }
                    }
                }
            }
        }

        start_x += 9 * scale;
        text++;
    }
}

static void draw_icon(
    uint16_t *frame,
    size_t frame_width,
    size_t frame_height,
    size_t start_x,
    size_t start_y,
    const uint8_t *icon,
    size_t icon_width,
    size_t icon_height,
    uint16_t color
) {
    size_t bytes_per_row = (icon_width + 7) / 8;

    for (size_t y = 0; y < icon_height; y++) {
        for (size_t x = 0; x < icon_width; x++) {
            size_t byte_index = y * bytes_per_row + x / 8;

            uint8_t bit_mask = (uint8_t)(0x80 >> (x % 8));

            bool pixel_is_set = (icon[byte_index] & bit_mask) != 0;

            if (pixel_is_set) {
                size_t screen_x = start_x + x;
                size_t screen_y = start_y + y;

                if (screen_x < frame_width && screen_y < frame_height) {
                    frame[screen_y * frame_width + screen_x] = color;
                }
            }
        }
    }
}

void menu_overlay(
    uint16_t *frame,
    size_t frame_width,
    size_t frame_height,
    const capture_settings_t *settings
) {
    if (frame == NULL || settings == NULL) {
        return;
    }

    const uint8_t *current_timer_icon = timer_off_icon;

    switch (settings->timer_seconds) {
        case 0:
            current_timer_icon = timer_off_icon;
            break;

        case 3:
            current_timer_icon = timer_3_icon;
            break;

        case 10:
            current_timer_icon = timer_10_icon;
            break;

        default:
            current_timer_icon = timer_off_icon;
            break;
    }

    const char *algorithm_name = "Unknown";

    switch (settings->dithering) {
        case DITHERING_FLOYD_STEINBERG:
            algorithm_name = "Floyd-Steinberg";
            break;

        case DITHERING_BAYER:
            algorithm_name = "Bayer";
            break;

        case DITHERING_CLUSTERED_DOT:
            algorithm_name = "Clustered Dot";
            break;
    }

    draw_icon(
        frame,
        frame_width,
        frame_height,
        20 + 1,
        20 + 1,
        current_timer_icon,
        ICON_WIDTH,
        ICON_HEIGHT,
        0x0000
    );

    draw_icon(
        frame,
        frame_width,
        frame_height,
        20,
        20,
        current_timer_icon,
        ICON_WIDTH,
        ICON_HEIGHT,
        0xFFFF
    );

    draw_icon(
        frame,
        frame_width,
        frame_height,
        20 + 1,
        85 + 1,
        save_icon,
        ICON_WIDTH,
        ICON_HEIGHT,
        0x0000
    );

    draw_icon(
        frame,
        frame_width,
        frame_height,
        20,
        85,
        save_icon,
        ICON_WIDTH,
        ICON_HEIGHT,
        0xFFFF
    );

    draw_icon(
        frame,
        frame_width,
        frame_height,
        20 + 1,
        150 + 1,
        print_icon,
        ICON_WIDTH,
        ICON_HEIGHT,
        0x0000
    );

    draw_icon(
        frame,
        frame_width,
        frame_height,
        20,
        150,
        print_icon,
        ICON_WIDTH,
        ICON_HEIGHT,
        0xFFFF
    );

    draw_text(
        frame,
        frame_width,
        frame_height,
        20 + 1,
        215 + 1,
        algorithm_name,
        0x0000,
        1
    );

    draw_text(
        frame,
        frame_width,
        frame_height,
        20,
        215,
        algorithm_name,
        0xFFFF,
        1
    );

    if (!settings->save_photo) {
        draw_icon(
            frame,
            frame_width,
            frame_height,
            20 + 1,
            85 + 1,
            off_icon,
            ICON_WIDTH,
            ICON_HEIGHT,
            0x0000
        );

        draw_icon(
            frame,
            frame_width,
            frame_height,
            20,
            85,
            off_icon,
            ICON_WIDTH,
            ICON_HEIGHT,
            0x00F8
        );
    };

    if (!settings->print_photo) {
        draw_icon(
            frame,
            frame_width,
            frame_height,
            20 + 1,
            150 + 1,
            off_icon,
            ICON_WIDTH,
            ICON_HEIGHT,
            0x0000
        );

        draw_icon(
            frame,
            frame_width,
            frame_height,
            20,
            150,
            off_icon,
            ICON_WIDTH,
            ICON_HEIGHT,
            0x00F8
        );
    };

}

void menu_draw_countdown(
    uint16_t *frame,
    size_t frame_width,
    size_t frame_height,
    uint8_t seconds_remaining
) {
    char countdown_text[4];

    snprintf(
        countdown_text,
        sizeof(countdown_text),
        "%u",
        (unsigned int)seconds_remaining
    );

    size_t scale = 6;
    size_t character_count = strlen(countdown_text);

    size_t text_width = character_count * 9 * scale - scale;
    size_t text_height = 8 * scale;

    size_t x = (frame_width - text_width) / 2;
    size_t y = (frame_height - text_height) / 2;

    draw_text(
        frame,
        frame_width,
        frame_height,
        x + 2,
        y + 2,
        countdown_text,
        0x0000,
        scale
    );

    draw_text(
        frame,
        frame_width,
        frame_height,
        x,
        y,
        countdown_text,
        0xFFFF,
        scale
    );
}
