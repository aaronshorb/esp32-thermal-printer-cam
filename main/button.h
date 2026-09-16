#ifndef BUTTON_H
#define BUTTON_H

#include<stdbool.h>

#include "esp_err.h"

esp_err_t init_shutter_button(void);

bool shutter_button_take_request(void);

#endif