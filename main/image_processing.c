#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "image_processing.h"

#include "esp_camera.h"
#include "esp_err.h"
#include "img_converters.h"

#define PRINT_IMAGE_WIDTH  384

static esp_err_t jpeg_to_rgb565(
    const camera_fb_t *jpeg,
    uint16_t **out_rgb565
) {
    if (jpeg == NULL || out_rgb565 == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    *out_rgb565 = NULL;

    size_t pixel_count = 
        (size_t)((jpeg->width / 2) * (jpeg->height / 2));

    uint16_t *rgb565 = malloc(
        pixel_count * sizeof(*rgb565)
    );

    if (rgb565 == NULL) {
        return ESP_ERR_NO_MEM;
    }

    bool converted = jpg2rgb565(
        jpeg->buf,
        jpeg->len,
        (uint8_t *)rgb565,
        JPEG_IMAGE_SCALE_1_2
    );

    if(!converted) {
        free(rgb565);
        return ESP_FAIL;
    }

    *out_rgb565 = rgb565;
    return ESP_OK;
}

static esp_err_t rgb565_to_cropped_grayscale(
    const uint16_t *rgb565,
    size_t source_width,
    size_t source_height,
    uint8_t **out_grayscale
) {
    if (rgb565 == NULL || out_grayscale == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    size_t pixel_count = PRINT_IMAGE_WIDTH * source_height;

    uint8_t *grayscale = malloc(pixel_count * sizeof(*grayscale));
    
    if (grayscale == NULL) {
        return ESP_ERR_NO_MEM;
    }

    size_t crop_left =
        (source_width - PRINT_IMAGE_WIDTH) / 2;

    for (size_t i = 0; i < source_height; i++) {
        for (size_t j = 0; j < PRINT_IMAGE_WIDTH; j++) {
            size_t source_index =
                i * source_width + j + crop_left;

            size_t output_index =
                i * PRINT_IMAGE_WIDTH + j;

            uint16_t pixel = rgb565[source_index];

            uint8_t red =
                ((pixel >> 11) & 0x1F) * 255 / 31;

            uint8_t green =
                ((pixel >> 5) & 0x3F) * 255 / 63;

            uint8_t blue =
                (pixel & 0x1F) * 255 / 31;

            grayscale[output_index] =
                (77 * red + 150 * green + 29 * blue) >> 8;
        }
    }

    *out_grayscale = grayscale;

    return ESP_OK;
}

static esp_err_t save_grayscale_test(
    const uint8_t *grayscale,
    size_t width,
    size_t height
)
{
    if (grayscale == NULL || width == 0 || height == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    FILE *file = fopen(
        "/sdcard/test_image.pgm",
        "wb"
    );

    if (file == NULL) {
        return ESP_FAIL;
    }

    if (fprintf(file, "P5\n%zu %zu\n255\n", width, height) < 0) {
        fclose(file);
        return ESP_FAIL;
    }

    size_t pixel_count = width * height;

    size_t written = fwrite(
        grayscale,
        sizeof(*grayscale),
        pixel_count,
        file
    );

    if (written != pixel_count) {
        fclose(file);
        return ESP_FAIL;
    }

    if (fclose(file) != 0) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

// static esp_err_t bayer_dither(
//     uint8_t *grayscale,
//     size_t width,
//     size_t height
// )
// {
//     if (grayscale == NULL || width == 0 || height == 0) {
//         return ESP_ERR_INVALID_ARG;
//     }

//     static const uint8_t bayer[4][4] = {
//         {  0,  8,  2, 10 },
//         { 12,  4, 14,  6 },
//         {  3, 11,  1,  9 },
//         { 15,  7, 13,  5 }
//     };

//     for (size_t y = 0; y < height; y++) {
//         for (size_t x = 0; x < width; x++) {
//             size_t index = y * width + x;

//             uint8_t threshold =
//                 bayer[y % 4][x % 4] * 16 + 8;

//             grayscale[index] =
//                 grayscale[index] > threshold ? 255 : 0;
//         }
//     }

//     return ESP_OK;
// }

// static esp_err_t clustered_dot_dither(
//     uint8_t *grayscale,
//     size_t width,
//     size_t height
// ) {
//     if (grayscale == NULL || width == 0 || height == 0) {
//         return ESP_ERR_INVALID_ARG;
//     }

//     static const uint8_t clustered[4][4] = {
//         { 12,  5,  6, 13 },
//         {  4,  0,  1,  7 },
//         { 11,  3,  2,  8 },
//         { 15, 10,  9, 14 }
//     };

//     for (size_t y = 0; y < height; y++) {
//         for (size_t x = 0; x < width; x++) {
//             size_t index = y * width + x;

//             uint8_t threshold =
//                 clustered[y % 4][x % 4] * 16 + 8;
            
//                 grayscale[index] = 
//                     grayscale[index] > threshold ? 255 : 0;
//         }
//     }

//     return ESP_OK;
// }

static uint8_t clamp_pixel(int value) {
    return value < 0 ? 0 : (value > 255 ? 255 : value);
}

static esp_err_t floyd_steinberg_dither(
    uint8_t *grayscale,
    size_t width,
    size_t height
)
{
    if (grayscale == NULL || width == 0 || height == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t index = y * width + x;

            int old_pixel = grayscale[index];
            int new_pixel = old_pixel < 128 ? 0 : 255;
            int error = old_pixel - new_pixel;

            grayscale[index] = new_pixel;

            if (x + 1 < width) {
                size_t neighbor_index = index + 1;

                grayscale[neighbor_index] = clamp_pixel(
                    grayscale[neighbor_index] + error * 7 / 16
                );
            }

            if (y + 1 < height && x > 0) {
                size_t neighbor_index = index + width - 1;

                grayscale[neighbor_index] = clamp_pixel(
                    grayscale[neighbor_index] + error * 3 / 16
                );
            }

            if (y + 1 < height) {
                size_t neighbor_index = index + width;

                grayscale[neighbor_index] = clamp_pixel(
                    grayscale[neighbor_index] + error * 5 / 16
                );
            }

            if (y + 1 < height && x + 1 < width) {
                size_t neighbor_index = index + width + 1;

                grayscale[neighbor_index] = clamp_pixel(
                    grayscale[neighbor_index] + error / 16
                );
            }
        }
    }

    return ESP_OK;
}

static esp_err_t convert_monochrome_to_1bit_bitmap(
    const uint8_t *monochrome,
    size_t width,
    size_t height,
    uint8_t **out_bitmap,
    size_t *out_length
) {
    if (
        monochrome == NULL ||
        out_bitmap == NULL ||
        out_length == NULL ||
        width == 0 ||
        height == 0
    ) {
        return ESP_ERR_INVALID_ARG;
    }

    *out_bitmap = NULL;
    *out_length = 0;

    size_t bytes_per_row = (width + 7) / 8;
    size_t bitmap_length = bytes_per_row * height;

    uint8_t *bitmap = malloc(bitmap_length);

    if (bitmap == NULL) {
        return ESP_ERR_NO_MEM;
    }

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < bytes_per_row; x++) {
            uint8_t byte_val = 0;

            for (size_t bit = 0; bit < 8; bit++) {
                size_t pixel_x = x * 8 + bit;
                
                if (pixel_x >= width) {
                    break;
                }

                size_t source_index = y * width + pixel_x;

                bool pixel_is_black = monochrome[source_index] < 128;

                if (pixel_is_black) {
                    byte_val |= (uint8_t)(0x80 >> bit);
                }
            }

            bitmap[y * bytes_per_row + x] = byte_val;
        }
    }

    *out_bitmap = bitmap;
    *out_length = bitmap_length;

    return ESP_OK;
}

esp_err_t prepare_image_for_printing(
    const camera_fb_t *jpeg,
    uint8_t **out_bitmap,
    size_t *out_length
) {
    uint16_t *rgb565 = NULL;
    uint8_t *grayscale = NULL;

    esp_err_t err = jpeg_to_rgb565(
        jpeg,
        &rgb565
    );

    if (err != ESP_OK) {
        return err;
    }

    err = rgb565_to_cropped_grayscale(
        rgb565,
        jpeg->width / 2,
        jpeg->height / 2,
        &grayscale
    );

    free(rgb565);

    if (err != ESP_OK) {
        return err;
    }

    // err = bayer_dither(
    //     grayscale,
    //     PRINT_IMAGE_WIDTH,
    //     jpeg->height / 2
    // );

    // err = clustered_dot_dither(
    //     grayscale,
    //     PRINT_IMAGE_WIDTH,
    //     jpeg->height / 2
    // );

    err = floyd_steinberg_dither(
        grayscale,
        PRINT_IMAGE_WIDTH,
        jpeg->height / 2
    );

    if (err != ESP_OK) {
        free(grayscale);
        return err;
    }

    err = convert_monochrome_to_1bit_bitmap(
        grayscale,
        PRINT_IMAGE_WIDTH,
        jpeg->height / 2,
        out_bitmap,
        out_length
    );



    if (err != ESP_OK) {
        free(grayscale);
        return err;
    }
    err = save_grayscale_test(
        grayscale,
        PRINT_IMAGE_WIDTH,
        jpeg->height / 2
    );



    free(grayscale);

    return err;
}
