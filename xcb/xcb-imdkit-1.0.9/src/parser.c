/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#include "parser.h"
#include <stdlib.h>
#include <string.h>
/* For byte swapping */
#define Swap8(n) (n)

#define Swap16(n) (swap ? (((n) << 8 & 0xFF00) | ((n) >> 8 & 0xFF)) : n)
#define Swap32(n)                                                              \
    (swap ? (((n) << 24 & 0xFF000000) | ((n) << 8 & 0xFF0000) |                \
             ((n) >> 8 & 0xFF00) | ((n) >> 24 & 0xFF))                         \
          : n)
#define Swap64(n)                                                              \
    (swap ? (((n) << 56 & 0xFF00000000000000) |                                \
             ((n) << 40 & 0xFF000000000000) | ((n) << 24 & 0xFF0000000000) |   \
             ((n) << 8 & 0xFF00000000) | ((n) >> 8 & 0xFF000000) |             \
             ((n) >> 24 & 0xFF0000) | ((n) >> 40 & 0xFF00) |                   \
             ((n) >> 56 & 0xFF))                                               \
          : n)

void uint8_t_read(uint8_t *p, uint8_t **data, size_t *len, bool swap) {
    if (*len < 1) {
        *data = NULL;
        return;
    }

    *p = **data;

    *data += 1;
    *len -= 1;
}

uint8_t *uint8_t_write(uint8_t *p, uint8_t *data, bool swap) {
    *data = *p;
    data += 1;
    return data;
}

void uint16_t_read(uint16_t *p, uint8_t **data, size_t *len, bool swap) {
    if (*len < 2) {
        *data = NULL;
        return;
    }

    *p = Swap16(*((uint16_t *)*data));
    *data += 2;
    *len -= 2;
}

uint8_t *uint16_t_write(uint16_t *p, uint8_t *data, bool swap) {
    *((uint16_t *)data) = Swap16(*p);
    data += 2;
    return data;
}

void uint32_t_read(uint32_t *p, uint8_t **data, size_t *len, bool swap) {
    if (*len < 4) {
        *data = NULL;
        return;
    }

    *p = Swap32(*((uint32_t *)*data));
    *data += 4;
    *len -= 4;
}

uint8_t *uint32_t_write(uint32_t *p, uint8_t *data, bool swap) {
    *((uint32_t *)data) = Swap32(*p);
    data += 4;
    return data;
}

void xcb_im_bytearray_t_read(xcb_im_bytearray_t *p, uint32_t arraylen,
                             uint8_t **data, size_t *len, bool swap) {
    if (*len < arraylen) {
        *data = NULL;
        return;
    }

    *p = *data;
    *data += arraylen;
    *len -= arraylen;
}

uint8_t *xcb_im_bytearray_t_write(xcb_im_bytearray_t *p, uint32_t arraylen,
                                  uint8_t *data, bool swap) {
    memcpy(data, *p, arraylen);
    return data + arraylen;
}

uintptr_t align_to_4(uintptr_t ptr, uintptr_t len, size_t *remain) {
    uintptr_t diff = (len % 4) ? 4 - (len % 4) : 0;
    if (remain) {
        if (*remain < diff) {
            return 0;
        }
        *remain -= diff;
    }
    return ptr + diff;
}

uintptr_t align_to_2(uintptr_t ptr, uintptr_t len, size_t *remain) {
    uintptr_t diff = (len % 2) ? 2 - (len % 2) : 0;
    if (remain) {
        if (*remain < diff) {
            return 0;
        }
        *remain -= diff;
    }
    return ptr + diff;
}
