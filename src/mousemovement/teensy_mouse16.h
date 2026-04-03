#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int usb_mouse_move16(int16_t x, int16_t y, int8_t wheel, int8_t horiz);

#ifdef __cplusplus
}
#endif
