/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#ifndef _XCB_IMDKIT_PARSER_H_
#define _XCB_IMDKIT_PARSER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "ximproto.h"

#define READ_FUNC(TYPE)                                                        \
    void TYPE##_read(TYPE *p, uint8_t **data, size_t *len, bool swap);         \
    uint8_t *TYPE##_write(TYPE *p, uint8_t *data, bool swap);

READ_FUNC(uint8_t)
READ_FUNC(uint16_t)
READ_FUNC(uint32_t)
void xcb_im_bytearray_t_read(xcb_im_bytearray_t *p, uint32_t arraylen,
                             uint8_t **data, size_t *len, bool swap);
uint8_t *xcb_im_bytearray_t_write(xcb_im_bytearray_t *p, uint32_t arraylen,
                                  uint8_t *data, bool swap);

uintptr_t align_to_4(uintptr_t ptr, uintptr_t len, size_t *remain);
uintptr_t align_to_2(uintptr_t ptr, uintptr_t len, size_t *remain);

#endif // IMDKIT_PARSE_H
