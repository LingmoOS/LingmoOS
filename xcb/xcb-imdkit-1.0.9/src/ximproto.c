/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#include "ximproto.h"
#include "parser.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void xcb_im_ximattr_fr_read(xcb_im_ximattr_fr_t *frame, uint8_t **data,
                            size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->attribute_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->type_of_the_value, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->length_of_im_attribute, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->im_attribute, frame->length_of_im_attribute,
                            data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_ximattr_fr_write(xcb_im_ximattr_fr_t *frame, uint8_t *data,
                                 bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->attribute_ID, data, swap);
    data = uint16_t_write(&frame->type_of_the_value, data, swap);
    data = uint16_t_write(&frame->length_of_im_attribute, data, swap);
    data = xcb_im_bytearray_t_write(&frame->im_attribute,
                                    frame->length_of_im_attribute, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_ximattr_fr_size(xcb_im_ximattr_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 2;
    size += frame->length_of_im_attribute;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_ximattr_fr_free(xcb_im_ximattr_fr_t *frame) {}

void xcb_im_xicattr_fr_read(xcb_im_xicattr_fr_t *frame, uint8_t **data,
                            size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->attribute_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->type_of_the_value, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->length_of_ic_attribute, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->ic_attribute, frame->length_of_ic_attribute,
                            data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_xicattr_fr_write(xcb_im_xicattr_fr_t *frame, uint8_t *data,
                                 bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->attribute_ID, data, swap);
    data = uint16_t_write(&frame->type_of_the_value, data, swap);
    data = uint16_t_write(&frame->length_of_ic_attribute, data, swap);
    data = xcb_im_bytearray_t_write(&frame->ic_attribute,
                                    frame->length_of_ic_attribute, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_xicattr_fr_size(xcb_im_xicattr_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 2;
    size += frame->length_of_ic_attribute;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_xicattr_fr_free(xcb_im_xicattr_fr_t *frame) {}

void xcb_im_ximattribute_fr_read(xcb_im_ximattribute_fr_t *frame,
                                 uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->attribute_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->value_length, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->value, frame->value_length, data, len,
                            swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_ximattribute_fr_write(xcb_im_ximattribute_fr_t *frame,
                                      uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->attribute_ID, data, swap);
    data = uint16_t_write(&frame->value_length, data, swap);
    data = xcb_im_bytearray_t_write(&frame->value, frame->value_length, data,
                                    swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_ximattribute_fr_size(xcb_im_ximattribute_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += frame->value_length;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_ximattribute_fr_free(xcb_im_ximattribute_fr_t *frame) {}

void xcb_im_xicattribute_fr_read(xcb_im_xicattribute_fr_t *frame,
                                 uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->attribute_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->value_length, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->value, frame->value_length, data, len,
                            swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_xicattribute_fr_write(xcb_im_xicattribute_fr_t *frame,
                                      uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->attribute_ID, data, swap);
    data = uint16_t_write(&frame->value_length, data, swap);
    data = xcb_im_bytearray_t_write(&frame->value, frame->value_length, data,
                                    swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_xicattribute_fr_size(xcb_im_xicattribute_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += frame->value_length;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_xicattribute_fr_free(xcb_im_xicattribute_fr_t *frame) {}

void xcb_im_ximtriggerkey_fr_read(xcb_im_ximtriggerkey_fr_t *frame,
                                  uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint32_t_read(&frame->keysym, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->modifier, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->modifier_mask, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_ximtriggerkey_fr_write(xcb_im_ximtriggerkey_fr_t *frame,
                                       uint8_t *data, bool swap) {
    data = uint32_t_write(&frame->keysym, data, swap);
    data = uint32_t_write(&frame->modifier, data, swap);
    data = uint32_t_write(&frame->modifier_mask, data, swap);
    return data;
}

void xcb_im_ximtriggerkey_fr_free(xcb_im_ximtriggerkey_fr_t *frame) {}

void xcb_im_encodinginfo_fr_read(xcb_im_encodinginfo_fr_t *frame,
                                 uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->length_of_encoding_info, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->encoding_info,
                            frame->length_of_encoding_info, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_encodinginfo_fr_write(xcb_im_encodinginfo_fr_t *frame,
                                      uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->length_of_encoding_info, data, swap);
    data = xcb_im_bytearray_t_write(&frame->encoding_info,
                                    frame->length_of_encoding_info, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_encodinginfo_fr_size(xcb_im_encodinginfo_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += frame->length_of_encoding_info;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_encodinginfo_fr_free(xcb_im_encodinginfo_fr_t *frame) {}

void xcb_im_str_fr_read(xcb_im_str_fr_t *frame, uint8_t **data, size_t *len,
                        bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t_read(&frame->length_of_string, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->string, frame->length_of_string, data, len,
                            swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_str_fr_write(xcb_im_str_fr_t *frame, uint8_t *data, bool swap) {
    data = uint8_t_write(&frame->length_of_string, data, swap);
    data = xcb_im_bytearray_t_write(&frame->string, frame->length_of_string,
                                    data, swap);
    return data;
}

size_t xcb_im_str_fr_size(xcb_im_str_fr_t *frame) {
    size_t size = 0;
    size += 1;
    size += frame->length_of_string;
    return size;
}

void xcb_im_str_fr_free(xcb_im_str_fr_t *frame) {}

void xcb_im_xpcs_fr_read(xcb_im_xpcs_fr_t *frame, uint8_t **data, size_t *len,
                         bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->length_of_string_in_bytes, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->string, frame->length_of_string_in_bytes,
                            data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_xpcs_fr_write(xcb_im_xpcs_fr_t *frame, uint8_t *data,
                              bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->length_of_string_in_bytes, data, swap);
    data = xcb_im_bytearray_t_write(
        &frame->string, frame->length_of_string_in_bytes, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_xpcs_fr_size(xcb_im_xpcs_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += frame->length_of_string_in_bytes;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_xpcs_fr_free(xcb_im_xpcs_fr_t *frame) {}

void xcb_im_ext_fr_read(xcb_im_ext_fr_t *frame, uint8_t **data, size_t *len,
                        bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint8_t_read(&frame->extension_major_opcode, data, len, swap);
    if (!*data) {
        return;
    }
    uint8_t_read(&frame->extension_minor_opcode, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->length_of_extension_name, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->extension_name,
                            frame->length_of_extension_name, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_ext_fr_write(xcb_im_ext_fr_t *frame, uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint8_t_write(&frame->extension_major_opcode, data, swap);
    data = uint8_t_write(&frame->extension_minor_opcode, data, swap);
    data = uint16_t_write(&frame->length_of_extension_name, data, swap);
    data = xcb_im_bytearray_t_write(
        &frame->extension_name, frame->length_of_extension_name, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_ext_fr_size(xcb_im_ext_fr_t *frame) {
    size_t size = 0;
    size += 1;
    size += 1;
    size += 2;
    size += frame->length_of_extension_name;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_ext_fr_free(xcb_im_ext_fr_t *frame) {}

void xcb_im_inputstyle_fr_read(xcb_im_inputstyle_fr_t *frame, uint8_t **data,
                               size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint32_t_read(&frame->inputstyle, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_inputstyle_fr_write(xcb_im_inputstyle_fr_t *frame,
                                    uint8_t *data, bool swap) {
    data = uint32_t_write(&frame->inputstyle, data, swap);
    return data;
}

void xcb_im_inputstyle_fr_free(xcb_im_inputstyle_fr_t *frame) {}

void xcb_im_attr_head_fr_read(xcb_im_attr_head_fr_t *frame, uint8_t **data,
                              size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->attribute_id, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->attribute_length, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_attr_head_fr_write(xcb_im_attr_head_fr_t *frame, uint8_t *data,
                                   bool swap) {
    data = uint16_t_write(&frame->attribute_id, data, swap);
    data = uint16_t_write(&frame->attribute_length, data, swap);
    return data;
}

void xcb_im_attr_head_fr_free(xcb_im_attr_head_fr_t *frame) {}

void xcb_im_short_fr_read(xcb_im_short_fr_t *frame, uint8_t **data, size_t *len,
                          bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->value, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_short_fr_write(xcb_im_short_fr_t *frame, uint8_t *data,
                               bool swap) {
    data = uint16_t_write(&frame->value, data, swap);
    return data;
}

void xcb_im_short_fr_free(xcb_im_short_fr_t *frame) {}

void xcb_im_long_fr_read(xcb_im_long_fr_t *frame, uint8_t **data, size_t *len,
                         bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint32_t_read(&frame->value, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_long_fr_write(xcb_im_long_fr_t *frame, uint8_t *data,
                              bool swap) {
    data = uint32_t_write(&frame->value, data, swap);
    return data;
}

void xcb_im_long_fr_free(xcb_im_long_fr_t *frame) {}

void xcb_im_xrectangle_fr_read(xcb_im_xrectangle_fr_t *frame, uint8_t **data,
                               size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->x, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->y, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->width, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->height, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_xrectangle_fr_write(xcb_im_xrectangle_fr_t *frame,
                                    uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->x, data, swap);
    data = uint16_t_write(&frame->y, data, swap);
    data = uint16_t_write(&frame->width, data, swap);
    data = uint16_t_write(&frame->height, data, swap);
    return data;
}

void xcb_im_xrectangle_fr_free(xcb_im_xrectangle_fr_t *frame) {}

void xcb_im_xpoint_fr_read(xcb_im_xpoint_fr_t *frame, uint8_t **data,
                           size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->x, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->y, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_xpoint_fr_write(xcb_im_xpoint_fr_t *frame, uint8_t *data,
                                bool swap) {
    data = uint16_t_write(&frame->x, data, swap);
    data = uint16_t_write(&frame->y, data, swap);
    return data;
}

void xcb_im_xpoint_fr_free(xcb_im_xpoint_fr_t *frame) {}

void xcb_im_fontset_fr_read(xcb_im_fontset_fr_t *frame, uint8_t **data,
                            size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->length_of_base_font_name, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->base_font_name_list,
                            frame->length_of_base_font_name, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_fontset_fr_write(xcb_im_fontset_fr_t *frame, uint8_t *data,
                                 bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->length_of_base_font_name, data, swap);
    data =
        xcb_im_bytearray_t_write(&frame->base_font_name_list,
                                 frame->length_of_base_font_name, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_fontset_fr_size(xcb_im_fontset_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += frame->length_of_base_font_name;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_fontset_fr_free(xcb_im_fontset_fr_t *frame) {}

// This function is manually modified to match XIM protocol.
void xcb_im_input_styles_fr_read(xcb_im_input_styles_fr_t *frame,
                                 uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    counter *= sizeof(xcb_im_inputstyle_fr_t);
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->XIMStyle_list.items = NULL;
    frame->XIMStyle_list.size = 0;

    while (counter != 0) {
        void *temp = realloc(frame->XIMStyle_list.items,
                             (frame->XIMStyle_list.size + 1) *
                                 sizeof(xcb_im_inputstyle_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->XIMStyle_list.items = temp;
        xcb_im_inputstyle_fr_read(
            &frame->XIMStyle_list.items[frame->XIMStyle_list.size], data,
            &counter, swap);
        if (!*data) {
            return;
        }
        frame->XIMStyle_list.size += 1;
    }

    // If size doesn't match.
    if (frame->XIMStyle_list.size != counter16) {
        free(frame->XIMStyle_list.items);
        frame->XIMStyle_list.size = 0;
        frame->XIMStyle_list.items = NULL;
        *data = NULL;
        return;
    }
}

// This function is manually modified to match XIM protocol.
uint8_t *xcb_im_input_styles_fr_write(xcb_im_input_styles_fr_t *frame,
                                      uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = frame->XIMStyle_list.size;
    data = uint16_t_write(&counter16, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    for (uint32_t i = 0; i < frame->XIMStyle_list.size; i++) {
        data = xcb_im_inputstyle_fr_write(&frame->XIMStyle_list.items[i], data,
                                          swap);
    }
    return data;
}

size_t xcb_im_input_styles_fr_size(xcb_im_input_styles_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size = align_to_4(size, size, NULL);
    for (uint32_t i = 0; i < frame->XIMStyle_list.size; i++) {
        size += xcb_im_inputstyle_fr_size(&frame->XIMStyle_list.items[i]);
    }
    return size;
}

void xcb_im_input_styles_fr_free(xcb_im_input_styles_fr_t *frame) {
    if (frame->XIMStyle_list.items) {
        for (uint32_t i = 0; i < frame->XIMStyle_list.size; i++) {
            xcb_im_inputstyle_fr_free(&frame->XIMStyle_list.items[i]);
        }
    }
    free(frame->XIMStyle_list.items);
}

void xcb_im_packet_header_fr_read(xcb_im_packet_header_fr_t *frame,
                                  uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t_read(&frame->major_opcode, data, len, swap);
    if (!*data) {
        return;
    }
    uint8_t_read(&frame->minor_opcode, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->length, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_packet_header_fr_write(xcb_im_packet_header_fr_t *frame,
                                       uint8_t *data, bool swap) {
    data = uint8_t_write(&frame->major_opcode, data, swap);
    data = uint8_t_write(&frame->minor_opcode, data, swap);
    data = uint16_t_write(&frame->length, data, swap);
    return data;
}

void xcb_im_packet_header_fr_free(xcb_im_packet_header_fr_t *frame) {}

void xcb_im_error_fr_read(xcb_im_error_fr_t *frame, uint8_t **data, size_t *len,
                          bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->flag, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->Error_Code, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->length_of_error_detail, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->type_of_error_detail, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->error_detail, frame->length_of_error_detail,
                            data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_error_fr_write(xcb_im_error_fr_t *frame, uint8_t *data,
                               bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint16_t_write(&frame->flag, data, swap);
    data = uint16_t_write(&frame->Error_Code, data, swap);
    data = uint16_t_write(&frame->length_of_error_detail, data, swap);
    data = uint16_t_write(&frame->type_of_error_detail, data, swap);
    data = xcb_im_bytearray_t_write(&frame->error_detail,
                                    frame->length_of_error_detail, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_error_fr_size(xcb_im_error_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 2;
    size += 2;
    size += 2;
    size += 2;
    size += frame->length_of_error_detail;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_error_fr_free(xcb_im_error_fr_t *frame) {}

void xcb_im_connect_fr_read(xcb_im_connect_fr_t *frame, uint8_t **data,
                            size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint8_t_read(&frame->byte_order, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_2((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->client_major_protocol_version, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->client_minor_protocol_version, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->client_auth_protocol_names.items = NULL;
    frame->client_auth_protocol_names.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->client_auth_protocol_names.items,
                             (frame->client_auth_protocol_names.size + 1) *
                                 sizeof(xcb_im_xpcs_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->client_auth_protocol_names.items = temp;
        xcb_im_xpcs_fr_read(&frame->client_auth_protocol_names
                                 .items[frame->client_auth_protocol_names.size],
                            data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->client_auth_protocol_names.size++;
    }
}

uint8_t *xcb_im_connect_fr_write(xcb_im_connect_fr_t *frame, uint8_t *data,
                                 bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint8_t_write(&frame->byte_order, data, swap);
    data = (uint8_t *)align_to_2((uintptr_t)data, data - start, NULL);
    data = uint16_t_write(&frame->client_major_protocol_version, data, swap);
    data = uint16_t_write(&frame->client_minor_protocol_version, data, swap);
    counter16 = 0;
    for (uint32_t i = 0; i < frame->client_auth_protocol_names.size; i++) {
        counter16 +=
            xcb_im_xpcs_fr_size(&frame->client_auth_protocol_names.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    for (uint32_t i = 0; i < frame->client_auth_protocol_names.size; i++) {
        data = xcb_im_xpcs_fr_write(&frame->client_auth_protocol_names.items[i],
                                    data, swap);
    }
    return data;
}

size_t xcb_im_connect_fr_size(xcb_im_connect_fr_t *frame) {
    size_t size = 0;
    size += 1;
    size = align_to_2(size, size, NULL);
    size += 2;
    size += 2;
    size += 2;
    for (uint32_t i = 0; i < frame->client_auth_protocol_names.size; i++) {
        size +=
            xcb_im_xpcs_fr_size(&frame->client_auth_protocol_names.items[i]);
    }
    return size;
}

void xcb_im_connect_fr_free(xcb_im_connect_fr_t *frame) {
    if (frame->client_auth_protocol_names.items) {
        for (uint32_t i = 0; i < frame->client_auth_protocol_names.size; i++) {
            xcb_im_xpcs_fr_free(&frame->client_auth_protocol_names.items[i]);
        }
    }
    free(frame->client_auth_protocol_names.items);
}

void xcb_im_connect_reply_fr_read(xcb_im_connect_reply_fr_t *frame,
                                  uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->server_major_protocol_version, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->server_minor_protocol_version, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_connect_reply_fr_write(xcb_im_connect_reply_fr_t *frame,
                                       uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->server_major_protocol_version, data, swap);
    data = uint16_t_write(&frame->server_minor_protocol_version, data, swap);
    return data;
}

void xcb_im_connect_reply_fr_free(xcb_im_connect_reply_fr_t *frame) {}

void xcb_im_auth_required_fr_read(xcb_im_auth_required_fr_t *frame,
                                  uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint8_t_read(&frame->auth_protocol_index, data, len, swap);
    if (!*data) {
        return;
    }
    uint8_t_read(&frame->auth_data1, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->auth_data2, frame->auth_data1, data, len,
                            swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_auth_required_fr_write(xcb_im_auth_required_fr_t *frame,
                                       uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint8_t_write(&frame->auth_protocol_index, data, swap);
    data = uint8_t_write(&frame->auth_data1, data, swap);
    data = xcb_im_bytearray_t_write(&frame->auth_data2, frame->auth_data1, data,
                                    swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_auth_required_fr_size(xcb_im_auth_required_fr_t *frame) {
    size_t size = 0;
    size += 1;
    size += 1;
    size += frame->auth_data1;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_auth_required_fr_free(xcb_im_auth_required_fr_t *frame) {}

void xcb_im_auth_reply_fr_read(xcb_im_auth_reply_fr_t *frame, uint8_t **data,
                               size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint8_t_read(&frame->field0, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->field1, frame->field0, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_auth_reply_fr_write(xcb_im_auth_reply_fr_t *frame,
                                    uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint8_t_write(&frame->field0, data, swap);
    data = xcb_im_bytearray_t_write(&frame->field1, frame->field0, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_auth_reply_fr_size(xcb_im_auth_reply_fr_t *frame) {
    size_t size = 0;
    size += 1;
    size += frame->field0;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_auth_reply_fr_free(xcb_im_auth_reply_fr_t *frame) {}

void xcb_im_auth_next_fr_read(xcb_im_auth_next_fr_t *frame, uint8_t **data,
                              size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint8_t_read(&frame->auth_data1, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->auth_data2, frame->auth_data1, data, len,
                            swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_auth_next_fr_write(xcb_im_auth_next_fr_t *frame, uint8_t *data,
                                   bool swap) {
    uint8_t *start = data;
    data = uint8_t_write(&frame->auth_data1, data, swap);
    data = xcb_im_bytearray_t_write(&frame->auth_data2, frame->auth_data1, data,
                                    swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_auth_next_fr_size(xcb_im_auth_next_fr_t *frame) {
    size_t size = 0;
    size += 1;
    size += frame->auth_data1;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_auth_next_fr_free(xcb_im_auth_next_fr_t *frame) {}

void xcb_im_auth_setup_fr_read(xcb_im_auth_setup_fr_t *frame, uint8_t **data,
                               size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->server_auth_protocol_names.items = NULL;
    frame->server_auth_protocol_names.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->server_auth_protocol_names.items,
                             (frame->server_auth_protocol_names.size + 1) *
                                 sizeof(xcb_im_xpcs_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->server_auth_protocol_names.items = temp;
        xcb_im_xpcs_fr_read(&frame->server_auth_protocol_names
                                 .items[frame->server_auth_protocol_names.size],
                            data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->server_auth_protocol_names.size++;
    }
}

uint8_t *xcb_im_auth_setup_fr_write(xcb_im_auth_setup_fr_t *frame,
                                    uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    counter16 = 0;
    for (uint32_t i = 0; i < frame->server_auth_protocol_names.size; i++) {
        counter16 +=
            xcb_im_xpcs_fr_size(&frame->server_auth_protocol_names.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    for (uint32_t i = 0; i < frame->server_auth_protocol_names.size; i++) {
        data = xcb_im_xpcs_fr_write(&frame->server_auth_protocol_names.items[i],
                                    data, swap);
    }
    return data;
}

size_t xcb_im_auth_setup_fr_size(xcb_im_auth_setup_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size = align_to_4(size, size, NULL);
    for (uint32_t i = 0; i < frame->server_auth_protocol_names.size; i++) {
        size +=
            xcb_im_xpcs_fr_size(&frame->server_auth_protocol_names.items[i]);
    }
    return size;
}

void xcb_im_auth_setup_fr_free(xcb_im_auth_setup_fr_t *frame) {
    if (frame->server_auth_protocol_names.items) {
        for (uint32_t i = 0; i < frame->server_auth_protocol_names.size; i++) {
            xcb_im_xpcs_fr_free(&frame->server_auth_protocol_names.items[i]);
        }
    }
    free(frame->server_auth_protocol_names.items);
}

void xcb_im_auth_ng_fr_read(xcb_im_auth_ng_fr_t *frame, uint8_t **data,
                            size_t *len, bool swap) {}

uint8_t *xcb_im_auth_ng_fr_write(xcb_im_auth_ng_fr_t *frame, uint8_t *data,
                                 bool swap) {
    return data;
}

void xcb_im_auth_ng_fr_free(xcb_im_auth_ng_fr_t *frame) {}

void xcb_im_disconnect_fr_read(xcb_im_disconnect_fr_t *frame, uint8_t **data,
                               size_t *len, bool swap) {}

uint8_t *xcb_im_disconnect_fr_write(xcb_im_disconnect_fr_t *frame,
                                    uint8_t *data, bool swap) {
    return data;
}

void xcb_im_disconnect_fr_free(xcb_im_disconnect_fr_t *frame) {}

void xcb_im_disconnect_reply_fr_read(xcb_im_disconnect_reply_fr_t *frame,
                                     uint8_t **data, size_t *len, bool swap) {}

uint8_t *xcb_im_disconnect_reply_fr_write(xcb_im_disconnect_reply_fr_t *frame,
                                          uint8_t *data, bool swap) {
    return data;
}

void xcb_im_disconnect_reply_fr_free(xcb_im_disconnect_reply_fr_t *frame) {}

void xcb_im_open_fr_read(xcb_im_open_fr_t *frame, uint8_t **data, size_t *len,
                         bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    xcb_im_str_fr_read(&frame->field0, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_open_fr_write(xcb_im_open_fr_t *frame, uint8_t *data,
                              bool swap) {
    uint8_t *start = data;
    data = xcb_im_str_fr_write(&frame->field0, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_open_fr_size(xcb_im_open_fr_t *frame) {
    size_t size = 0;
    size += xcb_im_str_fr_size(&frame->field0);
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_open_fr_free(xcb_im_open_fr_t *frame) {
    xcb_im_str_fr_free(&frame->field0);
}

void xcb_im_open_reply_fr_read(xcb_im_open_reply_fr_t *frame, uint8_t **data,
                               size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;
    frame->IM_attribute_supported.items = NULL;
    frame->IM_attribute_supported.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->IM_attribute_supported.items,
                             (frame->IM_attribute_supported.size + 1) *
                                 sizeof(xcb_im_ximattr_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->IM_attribute_supported.items = temp;
        xcb_im_ximattr_fr_read(&frame->IM_attribute_supported
                                    .items[frame->IM_attribute_supported.size],
                               data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->IM_attribute_supported.size++;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;
    frame->IC_attribute_supported.items = NULL;
    frame->IC_attribute_supported.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->IC_attribute_supported.items,
                             (frame->IC_attribute_supported.size + 1) *
                                 sizeof(xcb_im_xicattr_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->IC_attribute_supported.items = temp;
        xcb_im_xicattr_fr_read(&frame->IC_attribute_supported
                                    .items[frame->IC_attribute_supported.size],
                               data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->IC_attribute_supported.size++;
    }
}

uint8_t *xcb_im_open_reply_fr_write(xcb_im_open_reply_fr_t *frame,
                                    uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    counter16 = 0;
    for (uint32_t i = 0; i < frame->IM_attribute_supported.size; i++) {
        counter16 +=
            xcb_im_ximattr_fr_size(&frame->IM_attribute_supported.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    for (uint32_t i = 0; i < frame->IM_attribute_supported.size; i++) {
        data = xcb_im_ximattr_fr_write(&frame->IM_attribute_supported.items[i],
                                       data, swap);
    }
    counter16 = 0;
    for (uint32_t i = 0; i < frame->IC_attribute_supported.size; i++) {
        counter16 +=
            xcb_im_xicattr_fr_size(&frame->IC_attribute_supported.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    for (uint32_t i = 0; i < frame->IC_attribute_supported.size; i++) {
        data = xcb_im_xicattr_fr_write(&frame->IC_attribute_supported.items[i],
                                       data, swap);
    }
    return data;
}

size_t xcb_im_open_reply_fr_size(xcb_im_open_reply_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    for (uint32_t i = 0; i < frame->IM_attribute_supported.size; i++) {
        size += xcb_im_ximattr_fr_size(&frame->IM_attribute_supported.items[i]);
    }
    size += 2;
    size = align_to_4(size, size, NULL);
    for (uint32_t i = 0; i < frame->IC_attribute_supported.size; i++) {
        size += xcb_im_xicattr_fr_size(&frame->IC_attribute_supported.items[i]);
    }
    return size;
}

void xcb_im_open_reply_fr_free(xcb_im_open_reply_fr_t *frame) {
    if (frame->IM_attribute_supported.items) {
        for (uint32_t i = 0; i < frame->IM_attribute_supported.size; i++) {
            xcb_im_ximattr_fr_free(&frame->IM_attribute_supported.items[i]);
        }
    }
    free(frame->IM_attribute_supported.items);
    if (frame->IC_attribute_supported.items) {
        for (uint32_t i = 0; i < frame->IC_attribute_supported.size; i++) {
            xcb_im_xicattr_fr_free(&frame->IC_attribute_supported.items[i]);
        }
    }
    free(frame->IC_attribute_supported.items);
}

void xcb_im_close_fr_read(xcb_im_close_fr_t *frame, uint8_t **data, size_t *len,
                          bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_close_fr_write(xcb_im_close_fr_t *frame, uint8_t *data,
                               bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

void xcb_im_close_fr_free(xcb_im_close_fr_t *frame) {}

void xcb_im_close_reply_fr_read(xcb_im_close_reply_fr_t *frame, uint8_t **data,
                                size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_close_reply_fr_write(xcb_im_close_reply_fr_t *frame,
                                     uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

void xcb_im_close_reply_fr_free(xcb_im_close_reply_fr_t *frame) {}

void xcb_im_register_triggerkeys_fr_read(
    xcb_im_register_triggerkeys_fr_t *frame, uint8_t **data, size_t *len,
    bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint32_t counter32 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    uint32_t_read(&counter32, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter32;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->on_keys_list.items = NULL;
    frame->on_keys_list.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->on_keys_list.items,
                             (frame->on_keys_list.size + 1) *
                                 sizeof(xcb_im_ximtriggerkey_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->on_keys_list.items = temp;
        xcb_im_ximtriggerkey_fr_read(
            &frame->on_keys_list.items[frame->on_keys_list.size], data,
            &counter, swap);
        if (!*data) {
            return;
        }
        frame->on_keys_list.size++;
    }
    uint32_t_read(&counter32, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter32;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->off_keys_list.items = NULL;
    frame->off_keys_list.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->off_keys_list.items,
                             (frame->off_keys_list.size + 1) *
                                 sizeof(xcb_im_ximtriggerkey_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->off_keys_list.items = temp;
        xcb_im_ximtriggerkey_fr_read(
            &frame->off_keys_list.items[frame->off_keys_list.size], data,
            &counter, swap);
        if (!*data) {
            return;
        }
        frame->off_keys_list.size++;
    }
}

uint8_t *
xcb_im_register_triggerkeys_fr_write(xcb_im_register_triggerkeys_fr_t *frame,
                                     uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint32_t counter = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    counter = 0;
    for (uint32_t i = 0; i < frame->on_keys_list.size; i++) {
        counter += xcb_im_ximtriggerkey_fr_size(&frame->on_keys_list.items[i]);
    }
    data = uint32_t_write(&counter, data, swap);
    for (uint32_t i = 0; i < frame->on_keys_list.size; i++) {
        data = xcb_im_ximtriggerkey_fr_write(&frame->on_keys_list.items[i],
                                             data, swap);
    }
    counter = 0;
    for (uint32_t i = 0; i < frame->off_keys_list.size; i++) {
        counter += xcb_im_ximtriggerkey_fr_size(&frame->off_keys_list.items[i]);
    }
    data = uint32_t_write(&counter, data, swap);
    for (uint32_t i = 0; i < frame->off_keys_list.size; i++) {
        data = xcb_im_ximtriggerkey_fr_write(&frame->off_keys_list.items[i],
                                             data, swap);
    }
    return data;
}

size_t
xcb_im_register_triggerkeys_fr_size(xcb_im_register_triggerkeys_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size = align_to_4(size, size, NULL);
    size += 4;
    for (uint32_t i = 0; i < frame->on_keys_list.size; i++) {
        size += xcb_im_ximtriggerkey_fr_size(&frame->on_keys_list.items[i]);
    }
    size += 4;
    for (uint32_t i = 0; i < frame->off_keys_list.size; i++) {
        size += xcb_im_ximtriggerkey_fr_size(&frame->off_keys_list.items[i]);
    }
    return size;
}

void xcb_im_register_triggerkeys_fr_free(
    xcb_im_register_triggerkeys_fr_t *frame) {
    if (frame->on_keys_list.items) {
        for (uint32_t i = 0; i < frame->on_keys_list.size; i++) {
            xcb_im_ximtriggerkey_fr_free(&frame->on_keys_list.items[i]);
        }
    }
    free(frame->on_keys_list.items);
    if (frame->off_keys_list.items) {
        for (uint32_t i = 0; i < frame->off_keys_list.size; i++) {
            xcb_im_ximtriggerkey_fr_free(&frame->off_keys_list.items[i]);
        }
    }
    free(frame->off_keys_list.items);
}

void xcb_im_trigger_notify_fr_read(xcb_im_trigger_notify_fr_t *frame,
                                   uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->flag, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->index_of_keys_list, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->client_select_event_mask, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_trigger_notify_fr_write(xcb_im_trigger_notify_fr_t *frame,
                                        uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint32_t_write(&frame->flag, data, swap);
    data = uint32_t_write(&frame->index_of_keys_list, data, swap);
    data = uint32_t_write(&frame->client_select_event_mask, data, swap);
    return data;
}

void xcb_im_trigger_notify_fr_free(xcb_im_trigger_notify_fr_t *frame) {}

void xcb_im_trigger_notify_reply_fr_read(
    xcb_im_trigger_notify_reply_fr_t *frame, uint8_t **data, size_t *len,
    bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *
xcb_im_trigger_notify_reply_fr_write(xcb_im_trigger_notify_reply_fr_t *frame,
                                     uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_trigger_notify_reply_fr_free(
    xcb_im_trigger_notify_reply_fr_t *frame) {}

void xcb_im_set_event_mask_fr_read(xcb_im_set_event_mask_fr_t *frame,
                                   uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->forward_event_mask, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->synchronous_event_mask, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_set_event_mask_fr_write(xcb_im_set_event_mask_fr_t *frame,
                                        uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint32_t_write(&frame->forward_event_mask, data, swap);
    data = uint32_t_write(&frame->synchronous_event_mask, data, swap);
    return data;
}

void xcb_im_set_event_mask_fr_free(xcb_im_set_event_mask_fr_t *frame) {}

void xcb_im_encoding_negotiation_fr_read(
    xcb_im_encoding_negotiation_fr_t *frame, uint8_t **data, size_t *len,
    bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->supported_list_of_encoding_in_IM_library.items = NULL;
    frame->supported_list_of_encoding_in_IM_library.size = 0;
    while (counter != 0) {
        void *temp =
            realloc(frame->supported_list_of_encoding_in_IM_library.items,
                    (frame->supported_list_of_encoding_in_IM_library.size + 1) *
                        sizeof(xcb_im_str_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->supported_list_of_encoding_in_IM_library.items = temp;
        xcb_im_str_fr_read(
            &frame->supported_list_of_encoding_in_IM_library
                 .items[frame->supported_list_of_encoding_in_IM_library.size],
            data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->supported_list_of_encoding_in_IM_library.size++;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->list_of_encodings_supported_in_th.items = NULL;
    frame->list_of_encodings_supported_in_th.size = 0;
    while (counter != 0) {
        void *temp =
            realloc(frame->list_of_encodings_supported_in_th.items,
                    (frame->list_of_encodings_supported_in_th.size + 1) *
                        sizeof(xcb_im_encodinginfo_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->list_of_encodings_supported_in_th.items = temp;
        xcb_im_encodinginfo_fr_read(
            &frame->list_of_encodings_supported_in_th
                 .items[frame->list_of_encodings_supported_in_th.size],
            data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->list_of_encodings_supported_in_th.size++;
    }
}

uint8_t *
xcb_im_encoding_negotiation_fr_write(xcb_im_encoding_negotiation_fr_t *frame,
                                     uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    counter16 = 0;
    for (uint32_t i = 0;
         i < frame->supported_list_of_encoding_in_IM_library.size; i++) {
        counter16 += xcb_im_str_fr_size(
            &frame->supported_list_of_encoding_in_IM_library.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    for (uint32_t i = 0;
         i < frame->supported_list_of_encoding_in_IM_library.size; i++) {
        data = xcb_im_str_fr_write(
            &frame->supported_list_of_encoding_in_IM_library.items[i], data,
            swap);
    }
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    counter16 = 0;
    for (uint32_t i = 0; i < frame->list_of_encodings_supported_in_th.size;
         i++) {
        counter16 += xcb_im_encodinginfo_fr_size(
            &frame->list_of_encodings_supported_in_th.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    for (uint32_t i = 0; i < frame->list_of_encodings_supported_in_th.size;
         i++) {
        data = xcb_im_encodinginfo_fr_write(
            &frame->list_of_encodings_supported_in_th.items[i], data, swap);
    }
    return data;
}

size_t
xcb_im_encoding_negotiation_fr_size(xcb_im_encoding_negotiation_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    for (uint32_t i = 0;
         i < frame->supported_list_of_encoding_in_IM_library.size; i++) {
        size += xcb_im_str_fr_size(
            &frame->supported_list_of_encoding_in_IM_library.items[i]);
    }
    size = align_to_4(size, size, NULL);
    size += 2;
    size = align_to_4(size, size, NULL);
    for (uint32_t i = 0; i < frame->list_of_encodings_supported_in_th.size;
         i++) {
        size += xcb_im_encodinginfo_fr_size(
            &frame->list_of_encodings_supported_in_th.items[i]);
    }
    return size;
}

void xcb_im_encoding_negotiation_fr_free(
    xcb_im_encoding_negotiation_fr_t *frame) {
    if (frame->supported_list_of_encoding_in_IM_library.items) {
        for (uint32_t i = 0;
             i < frame->supported_list_of_encoding_in_IM_library.size; i++) {
            xcb_im_str_fr_free(
                &frame->supported_list_of_encoding_in_IM_library.items[i]);
        }
    }
    free(frame->supported_list_of_encoding_in_IM_library.items);
    if (frame->list_of_encodings_supported_in_th.items) {
        for (uint32_t i = 0; i < frame->list_of_encodings_supported_in_th.size;
             i++) {
            xcb_im_encodinginfo_fr_free(
                &frame->list_of_encodings_supported_in_th.items[i]);
        }
    }
    free(frame->list_of_encodings_supported_in_th.items);
}

void xcb_im_encoding_negotiation_reply_fr_read(
    xcb_im_encoding_negotiation_reply_fr_t *frame, uint8_t **data, size_t *len,
    bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->category_of_the_encoding_determined, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->index_of_the_encoding_determined, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_encoding_negotiation_reply_fr_write(
    xcb_im_encoding_negotiation_reply_fr_t *frame, uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data =
        uint16_t_write(&frame->category_of_the_encoding_determined, data, swap);
    data = uint16_t_write(&frame->index_of_the_encoding_determined, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

void xcb_im_encoding_negotiation_reply_fr_free(
    xcb_im_encoding_negotiation_reply_fr_t *frame) {}

void xcb_im_query_extension_fr_read(xcb_im_query_extension_fr_t *frame,
                                    uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->extensions_supported_by_the_IM_library.items = NULL;
    frame->extensions_supported_by_the_IM_library.size = 0;
    while (counter != 0) {
        void *temp =
            realloc(frame->extensions_supported_by_the_IM_library.items,
                    (frame->extensions_supported_by_the_IM_library.size + 1) *
                        sizeof(xcb_im_str_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->extensions_supported_by_the_IM_library.items = temp;
        xcb_im_str_fr_read(
            &frame->extensions_supported_by_the_IM_library
                 .items[frame->extensions_supported_by_the_IM_library.size],
            data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->extensions_supported_by_the_IM_library.size++;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_query_extension_fr_write(xcb_im_query_extension_fr_t *frame,
                                         uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    counter16 = 0;
    for (uint32_t i = 0; i < frame->extensions_supported_by_the_IM_library.size;
         i++) {
        counter16 += xcb_im_str_fr_size(
            &frame->extensions_supported_by_the_IM_library.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    for (uint32_t i = 0; i < frame->extensions_supported_by_the_IM_library.size;
         i++) {
        data = xcb_im_str_fr_write(
            &frame->extensions_supported_by_the_IM_library.items[i], data,
            swap);
    }
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_query_extension_fr_size(xcb_im_query_extension_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    for (uint32_t i = 0; i < frame->extensions_supported_by_the_IM_library.size;
         i++) {
        size += xcb_im_str_fr_size(
            &frame->extensions_supported_by_the_IM_library.items[i]);
    }
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_query_extension_fr_free(xcb_im_query_extension_fr_t *frame) {
    if (frame->extensions_supported_by_the_IM_library.items) {
        for (uint32_t i = 0;
             i < frame->extensions_supported_by_the_IM_library.size; i++) {
            xcb_im_str_fr_free(
                &frame->extensions_supported_by_the_IM_library.items[i]);
        }
    }
    free(frame->extensions_supported_by_the_IM_library.items);
}

void xcb_im_query_extension_reply_fr_read(
    xcb_im_query_extension_reply_fr_t *frame, uint8_t **data, size_t *len,
    bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->list_of_extensions_supported_by_th.items = NULL;
    frame->list_of_extensions_supported_by_th.size = 0;
    while (counter != 0) {
        void *temp =
            realloc(frame->list_of_extensions_supported_by_th.items,
                    (frame->list_of_extensions_supported_by_th.size + 1) *
                        sizeof(xcb_im_ext_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->list_of_extensions_supported_by_th.items = temp;
        xcb_im_ext_fr_read(
            &frame->list_of_extensions_supported_by_th
                 .items[frame->list_of_extensions_supported_by_th.size],
            data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->list_of_extensions_supported_by_th.size++;
    }
}

uint8_t *
xcb_im_query_extension_reply_fr_write(xcb_im_query_extension_reply_fr_t *frame,
                                      uint8_t *data, bool swap) {
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    counter16 = 0;
    for (uint32_t i = 0; i < frame->list_of_extensions_supported_by_th.size;
         i++) {
        counter16 += xcb_im_ext_fr_size(
            &frame->list_of_extensions_supported_by_th.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    for (uint32_t i = 0; i < frame->list_of_extensions_supported_by_th.size;
         i++) {
        data = xcb_im_ext_fr_write(
            &frame->list_of_extensions_supported_by_th.items[i], data, swap);
    }
    return data;
}

size_t
xcb_im_query_extension_reply_fr_size(xcb_im_query_extension_reply_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    for (uint32_t i = 0; i < frame->list_of_extensions_supported_by_th.size;
         i++) {
        size += xcb_im_ext_fr_size(
            &frame->list_of_extensions_supported_by_th.items[i]);
    }
    return size;
}

void xcb_im_query_extension_reply_fr_free(
    xcb_im_query_extension_reply_fr_t *frame) {
    if (frame->list_of_extensions_supported_by_th.items) {
        for (uint32_t i = 0; i < frame->list_of_extensions_supported_by_th.size;
             i++) {
            xcb_im_ext_fr_free(
                &frame->list_of_extensions_supported_by_th.items[i]);
        }
    }
    free(frame->list_of_extensions_supported_by_th.items);
}

void xcb_im_get_im_values_fr_read(xcb_im_get_im_values_fr_t *frame,
                                  uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->im_attribute_id.items = NULL;
    frame->im_attribute_id.size = 0;
    while (counter != 0) {
        void *temp =
            realloc(frame->im_attribute_id.items,
                    (frame->im_attribute_id.size + 1) * sizeof(uint16_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->im_attribute_id.items = temp;
        uint16_t_read(
            &frame->im_attribute_id.items[frame->im_attribute_id.size], data,
            &counter, swap);
        if (!*data) {
            return;
        }
        frame->im_attribute_id.size++;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_get_im_values_fr_write(xcb_im_get_im_values_fr_t *frame,
                                       uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    counter16 = frame->im_attribute_id.size * 2;
    data = uint16_t_write(&counter16, data, swap);
    for (uint32_t i = 0; i < frame->im_attribute_id.size; i++) {
        data = uint16_t_write(&frame->im_attribute_id.items[i], data, swap);
    }
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_get_im_values_fr_size(xcb_im_get_im_values_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += frame->im_attribute_id.size * 2;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_get_im_values_fr_free(xcb_im_get_im_values_fr_t *frame) {
    free(frame->im_attribute_id.items);
}

void xcb_im_get_im_values_reply_fr_read(xcb_im_get_im_values_reply_fr_t *frame,
                                        uint8_t **data, size_t *len,
                                        bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->im_attribute_returned.items = NULL;
    frame->im_attribute_returned.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->im_attribute_returned.items,
                             (frame->im_attribute_returned.size + 1) *
                                 sizeof(xcb_im_ximattribute_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->im_attribute_returned.items = temp;
        xcb_im_ximattribute_fr_read(
            &frame->im_attribute_returned
                 .items[frame->im_attribute_returned.size],
            data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->im_attribute_returned.size++;
    }
}

uint8_t *
xcb_im_get_im_values_reply_fr_write(xcb_im_get_im_values_reply_fr_t *frame,
                                    uint8_t *data, bool swap) {
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    counter16 = 0;
    for (uint32_t i = 0; i < frame->im_attribute_returned.size; i++) {
        counter16 +=
            xcb_im_ximattribute_fr_size(&frame->im_attribute_returned.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    for (uint32_t i = 0; i < frame->im_attribute_returned.size; i++) {
        data = xcb_im_ximattribute_fr_write(
            &frame->im_attribute_returned.items[i], data, swap);
    }
    return data;
}

size_t
xcb_im_get_im_values_reply_fr_size(xcb_im_get_im_values_reply_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    for (uint32_t i = 0; i < frame->im_attribute_returned.size; i++) {
        size +=
            xcb_im_ximattribute_fr_size(&frame->im_attribute_returned.items[i]);
    }
    return size;
}

void xcb_im_get_im_values_reply_fr_free(
    xcb_im_get_im_values_reply_fr_t *frame) {
    if (frame->im_attribute_returned.items) {
        for (uint32_t i = 0; i < frame->im_attribute_returned.size; i++) {
            xcb_im_ximattribute_fr_free(&frame->im_attribute_returned.items[i]);
        }
    }
    free(frame->im_attribute_returned.items);
}

void xcb_im_create_ic_fr_read(xcb_im_create_ic_fr_t *frame, uint8_t **data,
                              size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->ic_attributes.items = NULL;
    frame->ic_attributes.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->ic_attributes.items,
                             (frame->ic_attributes.size + 1) *
                                 sizeof(xcb_im_xicattribute_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->ic_attributes.items = temp;
        xcb_im_xicattribute_fr_read(
            &frame->ic_attributes.items[frame->ic_attributes.size], data,
            &counter, swap);
        if (!*data) {
            return;
        }
        frame->ic_attributes.size++;
    }
}

uint8_t *xcb_im_create_ic_fr_write(xcb_im_create_ic_fr_t *frame, uint8_t *data,
                                   bool swap) {
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    counter16 = 0;
    for (uint32_t i = 0; i < frame->ic_attributes.size; i++) {
        counter16 +=
            xcb_im_xicattribute_fr_size(&frame->ic_attributes.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    for (uint32_t i = 0; i < frame->ic_attributes.size; i++) {
        data = xcb_im_xicattribute_fr_write(&frame->ic_attributes.items[i],
                                            data, swap);
    }
    return data;
}

size_t xcb_im_create_ic_fr_size(xcb_im_create_ic_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    for (uint32_t i = 0; i < frame->ic_attributes.size; i++) {
        size += xcb_im_xicattribute_fr_size(&frame->ic_attributes.items[i]);
    }
    return size;
}

void xcb_im_create_ic_fr_free(xcb_im_create_ic_fr_t *frame) {
    if (frame->ic_attributes.items) {
        for (uint32_t i = 0; i < frame->ic_attributes.size; i++) {
            xcb_im_xicattribute_fr_free(&frame->ic_attributes.items[i]);
        }
    }
    free(frame->ic_attributes.items);
}

void xcb_im_create_ic_reply_fr_read(xcb_im_create_ic_reply_fr_t *frame,
                                    uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_create_ic_reply_fr_write(xcb_im_create_ic_reply_fr_t *frame,
                                         uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_create_ic_reply_fr_free(xcb_im_create_ic_reply_fr_t *frame) {}

void xcb_im_destroy_ic_fr_read(xcb_im_destroy_ic_fr_t *frame, uint8_t **data,
                               size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_destroy_ic_fr_write(xcb_im_destroy_ic_fr_t *frame,
                                    uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_destroy_ic_fr_free(xcb_im_destroy_ic_fr_t *frame) {}

void xcb_im_destroy_ic_reply_fr_read(xcb_im_destroy_ic_reply_fr_t *frame,
                                     uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_destroy_ic_reply_fr_write(xcb_im_destroy_ic_reply_fr_t *frame,
                                          uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_destroy_ic_reply_fr_free(xcb_im_destroy_ic_reply_fr_t *frame) {}

void xcb_im_set_ic_values_fr_read(xcb_im_set_ic_values_fr_t *frame,
                                  uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->ic_attribute.items = NULL;
    frame->ic_attribute.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->ic_attribute.items,
                             (frame->ic_attribute.size + 1) *
                                 sizeof(xcb_im_xicattribute_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->ic_attribute.items = temp;
        xcb_im_xicattribute_fr_read(
            &frame->ic_attribute.items[frame->ic_attribute.size], data,
            &counter, swap);
        if (!*data) {
            return;
        }
        frame->ic_attribute.size++;
    }
}

uint8_t *xcb_im_set_ic_values_fr_write(xcb_im_set_ic_values_fr_t *frame,
                                       uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    counter16 = 0;
    for (uint32_t i = 0; i < frame->ic_attribute.size; i++) {
        counter16 += xcb_im_xicattribute_fr_size(&frame->ic_attribute.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    for (uint32_t i = 0; i < frame->ic_attribute.size; i++) {
        data = xcb_im_xicattribute_fr_write(&frame->ic_attribute.items[i], data,
                                            swap);
    }
    return data;
}

size_t xcb_im_set_ic_values_fr_size(xcb_im_set_ic_values_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 2;
    size = align_to_4(size, size, NULL);
    for (uint32_t i = 0; i < frame->ic_attribute.size; i++) {
        size += xcb_im_xicattribute_fr_size(&frame->ic_attribute.items[i]);
    }
    return size;
}

void xcb_im_set_ic_values_fr_free(xcb_im_set_ic_values_fr_t *frame) {
    if (frame->ic_attribute.items) {
        for (uint32_t i = 0; i < frame->ic_attribute.size; i++) {
            xcb_im_xicattribute_fr_free(&frame->ic_attribute.items[i]);
        }
    }
    free(frame->ic_attribute.items);
}

void xcb_im_set_ic_values_reply_fr_read(xcb_im_set_ic_values_reply_fr_t *frame,
                                        uint8_t **data, size_t *len,
                                        bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *
xcb_im_set_ic_values_reply_fr_write(xcb_im_set_ic_values_reply_fr_t *frame,
                                    uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_set_ic_values_reply_fr_free(
    xcb_im_set_ic_values_reply_fr_t *frame) {}

void xcb_im_get_ic_values_fr_read(xcb_im_get_ic_values_fr_t *frame,
                                  uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->ic_attribute.items = NULL;
    frame->ic_attribute.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->ic_attribute.items,
                             (frame->ic_attribute.size + 1) * sizeof(uint16_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->ic_attribute.items = temp;
        uint16_t_read(&frame->ic_attribute.items[frame->ic_attribute.size],
                      data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->ic_attribute.size++;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_get_ic_values_fr_write(xcb_im_get_ic_values_fr_t *frame,
                                       uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    counter16 = frame->ic_attribute.size * 2;
    data = uint16_t_write(&counter16, data, swap);
    for (uint32_t i = 0; i < frame->ic_attribute.size; i++) {
        data = uint16_t_write(&frame->ic_attribute.items[i], data, swap);
    }
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_get_ic_values_fr_size(xcb_im_get_ic_values_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 2;
    size += frame->ic_attribute.size * 2;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_get_ic_values_fr_free(xcb_im_get_ic_values_fr_t *frame) {
    free(frame->ic_attribute.items);
}

void xcb_im_get_ic_values_reply_fr_read(xcb_im_get_ic_values_reply_fr_t *frame,
                                        uint8_t **data, size_t *len,
                                        bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->ic_attribute.items = NULL;
    frame->ic_attribute.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->ic_attribute.items,
                             (frame->ic_attribute.size + 1) *
                                 sizeof(xcb_im_xicattribute_fr_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->ic_attribute.items = temp;
        xcb_im_xicattribute_fr_read(
            &frame->ic_attribute.items[frame->ic_attribute.size], data,
            &counter, swap);
        if (!*data) {
            return;
        }
        frame->ic_attribute.size++;
    }
}

uint8_t *
xcb_im_get_ic_values_reply_fr_write(xcb_im_get_ic_values_reply_fr_t *frame,
                                    uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    counter16 = 0;
    for (uint32_t i = 0; i < frame->ic_attribute.size; i++) {
        counter16 += xcb_im_xicattribute_fr_size(&frame->ic_attribute.items[i]);
    }
    data = uint16_t_write(&counter16, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    for (uint32_t i = 0; i < frame->ic_attribute.size; i++) {
        data = xcb_im_xicattribute_fr_write(&frame->ic_attribute.items[i], data,
                                            swap);
    }
    return data;
}

size_t
xcb_im_get_ic_values_reply_fr_size(xcb_im_get_ic_values_reply_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 2;
    size = align_to_4(size, size, NULL);
    for (uint32_t i = 0; i < frame->ic_attribute.size; i++) {
        size += xcb_im_xicattribute_fr_size(&frame->ic_attribute.items[i]);
    }
    return size;
}

void xcb_im_get_ic_values_reply_fr_free(
    xcb_im_get_ic_values_reply_fr_t *frame) {
    if (frame->ic_attribute.items) {
        for (uint32_t i = 0; i < frame->ic_attribute.size; i++) {
            xcb_im_xicattribute_fr_free(&frame->ic_attribute.items[i]);
        }
    }
    free(frame->ic_attribute.items);
}

void xcb_im_set_ic_focus_fr_read(xcb_im_set_ic_focus_fr_t *frame,
                                 uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_set_ic_focus_fr_write(xcb_im_set_ic_focus_fr_t *frame,
                                      uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_set_ic_focus_fr_free(xcb_im_set_ic_focus_fr_t *frame) {}

void xcb_im_unset_ic_focus_fr_read(xcb_im_unset_ic_focus_fr_t *frame,
                                   uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_unset_ic_focus_fr_write(xcb_im_unset_ic_focus_fr_t *frame,
                                        uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_unset_ic_focus_fr_free(xcb_im_unset_ic_focus_fr_t *frame) {}

void xcb_im_forward_event_fr_read(xcb_im_forward_event_fr_t *frame,
                                  uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->flag, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->sequence_number, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_forward_event_fr_write(xcb_im_forward_event_fr_t *frame,
                                       uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint16_t_write(&frame->flag, data, swap);
    data = uint16_t_write(&frame->sequence_number, data, swap);
    return data;
}

void xcb_im_forward_event_fr_free(xcb_im_forward_event_fr_t *frame) {}

void xcb_im_sync_fr_read(xcb_im_sync_fr_t *frame, uint8_t **data, size_t *len,
                         bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_sync_fr_write(xcb_im_sync_fr_t *frame, uint8_t *data,
                              bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_sync_fr_free(xcb_im_sync_fr_t *frame) {}

void xcb_im_sync_reply_fr_read(xcb_im_sync_reply_fr_t *frame, uint8_t **data,
                               size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_sync_reply_fr_write(xcb_im_sync_reply_fr_t *frame,
                                    uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_sync_reply_fr_free(xcb_im_sync_reply_fr_t *frame) {}

void xcb_im_commit_fr_read(xcb_im_commit_fr_t *frame, uint8_t **data,
                           size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->flag, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->byte_length_of_committed_string, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->committed_string,
                            frame->byte_length_of_committed_string, data, len,
                            swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->keysym.items = NULL;
    frame->keysym.size = 0;
    while (counter != 0) {
        void *temp = realloc(frame->keysym.items,
                             (frame->keysym.size + 1) * sizeof(uint32_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->keysym.items = temp;
        uint32_t_read(&frame->keysym.items[frame->keysym.size], data, &counter,
                      swap);
        if (!*data) {
            return;
        }
        frame->keysym.size++;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_commit_fr_write(xcb_im_commit_fr_t *frame, uint8_t *data,
                                bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint16_t_write(&frame->flag, data, swap);
    data = uint16_t_write(&frame->byte_length_of_committed_string, data, swap);
    data = xcb_im_bytearray_t_write(&frame->committed_string,
                                    frame->byte_length_of_committed_string,
                                    data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    counter16 = frame->keysym.size * 4;
    data = uint16_t_write(&counter16, data, swap);
    for (uint32_t i = 0; i < frame->keysym.size; i++) {
        data = uint32_t_write(&frame->keysym.items[i], data, swap);
    }
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_commit_fr_size(xcb_im_commit_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 2;
    size += 2;
    size += frame->byte_length_of_committed_string;
    size = align_to_4(size, size, NULL);
    size += 2;
    size += frame->keysym.size * 4;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_commit_fr_free(xcb_im_commit_fr_t *frame) {
    free(frame->keysym.items);
}

void xcb_im_commit_chars_fr_read(xcb_im_commit_chars_fr_t *frame,
                                 uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->flag, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->byte_length_of_committed_string, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->committed_string,
                            frame->byte_length_of_committed_string, data, len,
                            swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_commit_chars_fr_write(xcb_im_commit_chars_fr_t *frame,
                                      uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint16_t_write(&frame->flag, data, swap);
    data = uint16_t_write(&frame->byte_length_of_committed_string, data, swap);
    data = xcb_im_bytearray_t_write(&frame->committed_string,
                                    frame->byte_length_of_committed_string,
                                    data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_commit_chars_fr_size(xcb_im_commit_chars_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 2;
    size += 2;
    size += frame->byte_length_of_committed_string;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_commit_chars_fr_free(xcb_im_commit_chars_fr_t *frame) {}

void xcb_im_commit_both_fr_read(xcb_im_commit_both_fr_t *frame, uint8_t **data,
                                size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->flag, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->keysym, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->byte_length_of_committed_string, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->committed_string,
                            frame->byte_length_of_committed_string, data, len,
                            swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_commit_both_fr_write(xcb_im_commit_both_fr_t *frame,
                                     uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint16_t_write(&frame->flag, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    data = uint32_t_write(&frame->keysym, data, swap);
    data = uint16_t_write(&frame->byte_length_of_committed_string, data, swap);
    data = xcb_im_bytearray_t_write(&frame->committed_string,
                                    frame->byte_length_of_committed_string,
                                    data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_commit_both_fr_size(xcb_im_commit_both_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 2;
    size = align_to_4(size, size, NULL);
    size += 4;
    size += 2;
    size += frame->byte_length_of_committed_string;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_commit_both_fr_free(xcb_im_commit_both_fr_t *frame) {}

void xcb_im_reset_ic_fr_read(xcb_im_reset_ic_fr_t *frame, uint8_t **data,
                             size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_reset_ic_fr_write(xcb_im_reset_ic_fr_t *frame, uint8_t *data,
                                  bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_reset_ic_fr_free(xcb_im_reset_ic_fr_t *frame) {}

void xcb_im_reset_ic_reply_fr_read(xcb_im_reset_ic_reply_fr_t *frame,
                                   uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->byte_length_of_committed_string, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->committed_string,
                            frame->byte_length_of_committed_string, data, len,
                            swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_reset_ic_reply_fr_write(xcb_im_reset_ic_reply_fr_t *frame,
                                        uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint16_t_write(&frame->byte_length_of_committed_string, data, swap);
    data = xcb_im_bytearray_t_write(&frame->committed_string,
                                    frame->byte_length_of_committed_string,
                                    data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_reset_ic_reply_fr_size(xcb_im_reset_ic_reply_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 2;
    size += frame->byte_length_of_committed_string;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_reset_ic_reply_fr_free(xcb_im_reset_ic_reply_fr_t *frame) {}

void xcb_im_geometry_fr_read(xcb_im_geometry_fr_t *frame, uint8_t **data,
                             size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_geometry_fr_write(xcb_im_geometry_fr_t *frame, uint8_t *data,
                                  bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_geometry_fr_free(xcb_im_geometry_fr_t *frame) {}

void xcb_im_str_conversion_fr_read(xcb_im_str_conversion_fr_t *frame,
                                   uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->XIMStringConversionPosition, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->XIMStringConversionType, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->XIMStringConversionOperation, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->length_to_multiply_th, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->length_of_the_string_to_b, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->string, frame->length_of_the_string_to_b,
                            data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_str_conversion_fr_write(xcb_im_str_conversion_fr_t *frame,
                                        uint8_t *data, bool swap) {
    uint8_t *start = data;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint32_t_write(&frame->XIMStringConversionPosition, data, swap);
    data = uint32_t_write(&frame->XIMStringConversionType, data, swap);
    data = uint32_t_write(&frame->XIMStringConversionOperation, data, swap);
    data = uint16_t_write(&frame->length_to_multiply_th, data, swap);
    data = uint16_t_write(&frame->length_of_the_string_to_b, data, swap);
    data = xcb_im_bytearray_t_write(
        &frame->string, frame->length_of_the_string_to_b, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    return data;
}

size_t xcb_im_str_conversion_fr_size(xcb_im_str_conversion_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 4;
    size += 4;
    size += 4;
    size += 2;
    size += 2;
    size += frame->length_of_the_string_to_b;
    size = align_to_4(size, size, NULL);
    return size;
}

void xcb_im_str_conversion_fr_free(xcb_im_str_conversion_fr_t *frame) {}

void xcb_im_str_conversion_reply_fr_read(
    xcb_im_str_conversion_reply_fr_t *frame, uint8_t **data, size_t *len,
    bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->XIMStringConversionFeedback, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->length_of_the_retrieved_string, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->retrieved_string,
                            frame->length_of_the_retrieved_string, data, len,
                            swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->feedback_array.items = NULL;
    frame->feedback_array.size = 0;
    while (counter != 0) {
        void *temp =
            realloc(frame->feedback_array.items,
                    (frame->feedback_array.size + 1) * sizeof(uint32_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->feedback_array.items = temp;
        uint32_t_read(&frame->feedback_array.items[frame->feedback_array.size],
                      data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->feedback_array.size++;
    }
}

uint8_t *
xcb_im_str_conversion_reply_fr_write(xcb_im_str_conversion_reply_fr_t *frame,
                                     uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint32_t_write(&frame->XIMStringConversionFeedback, data, swap);
    data = uint16_t_write(&frame->length_of_the_retrieved_string, data, swap);
    data = xcb_im_bytearray_t_write(&frame->retrieved_string,
                                    frame->length_of_the_retrieved_string, data,
                                    swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    counter16 = frame->feedback_array.size * 4;
    data = uint16_t_write(&counter16, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    for (uint32_t i = 0; i < frame->feedback_array.size; i++) {
        data = uint32_t_write(&frame->feedback_array.items[i], data, swap);
    }
    return data;
}

size_t
xcb_im_str_conversion_reply_fr_size(xcb_im_str_conversion_reply_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 4;
    size += 2;
    size += frame->length_of_the_retrieved_string;
    size = align_to_4(size, size, NULL);
    size += 2;
    size = align_to_4(size, size, NULL);
    size += frame->feedback_array.size * 4;
    return size;
}

void xcb_im_str_conversion_reply_fr_free(
    xcb_im_str_conversion_reply_fr_t *frame) {
    free(frame->feedback_array.items);
}

void xcb_im_preedit_start_fr_read(xcb_im_preedit_start_fr_t *frame,
                                  uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_preedit_start_fr_write(xcb_im_preedit_start_fr_t *frame,
                                       uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_preedit_start_fr_free(xcb_im_preedit_start_fr_t *frame) {}

void xcb_im_preedit_start_reply_fr_read(xcb_im_preedit_start_reply_fr_t *frame,
                                        uint8_t **data, size_t *len,
                                        bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->return_value, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *
xcb_im_preedit_start_reply_fr_write(xcb_im_preedit_start_reply_fr_t *frame,
                                    uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint32_t_write(&frame->return_value, data, swap);
    return data;
}

void xcb_im_preedit_start_reply_fr_free(
    xcb_im_preedit_start_reply_fr_t *frame) {}

void xcb_im_preedit_draw_fr_read(xcb_im_preedit_draw_fr_t *frame,
                                 uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->caret, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->chg_first, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->chg_length, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->status, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->length_of_preedit_string, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->preedit_string,
                            frame->length_of_preedit_string, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->feedback_array.items = NULL;
    frame->feedback_array.size = 0;
    while (counter != 0) {
        void *temp =
            realloc(frame->feedback_array.items,
                    (frame->feedback_array.size + 1) * sizeof(uint32_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->feedback_array.items = temp;
        uint32_t_read(&frame->feedback_array.items[frame->feedback_array.size],
                      data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->feedback_array.size++;
    }
}

uint8_t *xcb_im_preedit_draw_fr_write(xcb_im_preedit_draw_fr_t *frame,
                                      uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint32_t_write(&frame->caret, data, swap);
    data = uint32_t_write(&frame->chg_first, data, swap);
    data = uint32_t_write(&frame->chg_length, data, swap);
    data = uint32_t_write(&frame->status, data, swap);
    data = uint16_t_write(&frame->length_of_preedit_string, data, swap);
    data = xcb_im_bytearray_t_write(
        &frame->preedit_string, frame->length_of_preedit_string, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    counter16 = frame->feedback_array.size * 4;
    data = uint16_t_write(&counter16, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    for (uint32_t i = 0; i < frame->feedback_array.size; i++) {
        data = uint32_t_write(&frame->feedback_array.items[i], data, swap);
    }
    return data;
}

size_t xcb_im_preedit_draw_fr_size(xcb_im_preedit_draw_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 4;
    size += 4;
    size += 4;
    size += 4;
    size += 2;
    size += frame->length_of_preedit_string;
    size = align_to_4(size, size, NULL);
    size += 2;
    size = align_to_4(size, size, NULL);
    size += frame->feedback_array.size * 4;
    return size;
}

void xcb_im_preedit_draw_fr_free(xcb_im_preedit_draw_fr_t *frame) {
    free(frame->feedback_array.items);
}

void xcb_im_preedit_caret_fr_read(xcb_im_preedit_caret_fr_t *frame,
                                  uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->position, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->direction, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->style, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_preedit_caret_fr_write(xcb_im_preedit_caret_fr_t *frame,
                                       uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint32_t_write(&frame->position, data, swap);
    data = uint32_t_write(&frame->direction, data, swap);
    data = uint32_t_write(&frame->style, data, swap);
    return data;
}

void xcb_im_preedit_caret_fr_free(xcb_im_preedit_caret_fr_t *frame) {}

void xcb_im_preedit_caret_reply_fr_read(xcb_im_preedit_caret_reply_fr_t *frame,
                                        uint8_t **data, size_t *len,
                                        bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->position, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *
xcb_im_preedit_caret_reply_fr_write(xcb_im_preedit_caret_reply_fr_t *frame,
                                    uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint32_t_write(&frame->position, data, swap);
    return data;
}

void xcb_im_preedit_caret_reply_fr_free(
    xcb_im_preedit_caret_reply_fr_t *frame) {}

void xcb_im_preedit_done_fr_read(xcb_im_preedit_done_fr_t *frame,
                                 uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_preedit_done_fr_write(xcb_im_preedit_done_fr_t *frame,
                                      uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_preedit_done_fr_free(xcb_im_preedit_done_fr_t *frame) {}

void xcb_im_status_start_fr_read(xcb_im_status_start_fr_t *frame,
                                 uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_status_start_fr_write(xcb_im_status_start_fr_t *frame,
                                      uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_status_start_fr_free(xcb_im_status_start_fr_t *frame) {}

void xcb_im_status_draw_text_fr_read(xcb_im_status_draw_text_fr_t *frame,
                                     uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint8_t *start = *data;
    uint16_t counter16 = 0;
    size_t counter = 0;
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->type, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->status, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->length_of_status_string, data, len, swap);
    if (!*data) {
        return;
    }
    xcb_im_bytearray_t_read(&frame->status_string,
                            frame->length_of_status_string, data, len, swap);
    if (!*data) {
        return;
    }
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    uint16_t_read(&counter16, data, len, swap);
    if (!*data) {
        return;
    }
    counter = counter16;
    *data = (uint8_t *)align_to_4((uintptr_t)*data, *data - start, len);
    if (!*data) {
        return;
    }
    if (counter > *len) {
        *data = NULL;
        return;
    }
    *len -= counter;

    frame->feedback_array.items = NULL;
    frame->feedback_array.size = 0;
    while (counter != 0) {
        void *temp =
            realloc(frame->feedback_array.items,
                    (frame->feedback_array.size + 1) * sizeof(uint32_t));
        if (!temp) {
            *data = NULL;
            return;
        }
        frame->feedback_array.items = temp;
        uint32_t_read(&frame->feedback_array.items[frame->feedback_array.size],
                      data, &counter, swap);
        if (!*data) {
            return;
        }
        frame->feedback_array.size++;
    }
}

uint8_t *xcb_im_status_draw_text_fr_write(xcb_im_status_draw_text_fr_t *frame,
                                          uint8_t *data, bool swap) {
    uint8_t *start = data;
    uint16_t counter16 = 0;
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint32_t_write(&frame->type, data, swap);
    data = uint32_t_write(&frame->status, data, swap);
    data = uint16_t_write(&frame->length_of_status_string, data, swap);
    data = xcb_im_bytearray_t_write(&frame->status_string,
                                    frame->length_of_status_string, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    counter16 = frame->feedback_array.size * 4;
    data = uint16_t_write(&counter16, data, swap);
    data = (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
    for (uint32_t i = 0; i < frame->feedback_array.size; i++) {
        data = uint32_t_write(&frame->feedback_array.items[i], data, swap);
    }
    return data;
}

size_t xcb_im_status_draw_text_fr_size(xcb_im_status_draw_text_fr_t *frame) {
    size_t size = 0;
    size += 2;
    size += 2;
    size += 4;
    size += 4;
    size += 2;
    size += frame->length_of_status_string;
    size = align_to_4(size, size, NULL);
    size += 2;
    size = align_to_4(size, size, NULL);
    size += frame->feedback_array.size * 4;
    return size;
}

void xcb_im_status_draw_text_fr_free(xcb_im_status_draw_text_fr_t *frame) {
    free(frame->feedback_array.items);
}

void xcb_im_status_draw_bitmap_fr_read(xcb_im_status_draw_bitmap_fr_t *frame,
                                       uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->type, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->pixmap_data, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *
xcb_im_status_draw_bitmap_fr_write(xcb_im_status_draw_bitmap_fr_t *frame,
                                   uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint32_t_write(&frame->type, data, swap);
    data = uint32_t_write(&frame->pixmap_data, data, swap);
    return data;
}

void xcb_im_status_draw_bitmap_fr_free(xcb_im_status_draw_bitmap_fr_t *frame) {}

void xcb_im_status_done_fr_read(xcb_im_status_done_fr_t *frame, uint8_t **data,
                                size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_status_done_fr_write(xcb_im_status_done_fr_t *frame,
                                     uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    return data;
}

void xcb_im_status_done_fr_free(xcb_im_status_done_fr_t *frame) {}

void xcb_im_ext_set_event_mask_fr_read(xcb_im_ext_set_event_mask_fr_t *frame,
                                       uint8_t **data, size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->filter_event_mask, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->intercept_event_mask, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->select_event_mask, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->forward_event_mask, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->synchronous_event_mask, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *
xcb_im_ext_set_event_mask_fr_write(xcb_im_ext_set_event_mask_fr_t *frame,
                                   uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint32_t_write(&frame->filter_event_mask, data, swap);
    data = uint32_t_write(&frame->intercept_event_mask, data, swap);
    data = uint32_t_write(&frame->select_event_mask, data, swap);
    data = uint32_t_write(&frame->forward_event_mask, data, swap);
    data = uint32_t_write(&frame->synchronous_event_mask, data, swap);
    return data;
}

void xcb_im_ext_set_event_mask_fr_free(xcb_im_ext_set_event_mask_fr_t *frame) {}

void xcb_im_ext_forward_keyevent_fr_read(
    xcb_im_ext_forward_keyevent_fr_t *frame, uint8_t **data, size_t *len,
    bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->flag, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->sequence_number, data, len, swap);
    if (!*data) {
        return;
    }
    uint8_t_read(&frame->xEvent_u_u_type, data, len, swap);
    if (!*data) {
        return;
    }
    uint8_t_read(&frame->keycode, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->state, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->time, data, len, swap);
    if (!*data) {
        return;
    }
    uint32_t_read(&frame->window, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *
xcb_im_ext_forward_keyevent_fr_write(xcb_im_ext_forward_keyevent_fr_t *frame,
                                     uint8_t *data, bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint16_t_write(&frame->flag, data, swap);
    data = uint16_t_write(&frame->sequence_number, data, swap);
    data = uint8_t_write(&frame->xEvent_u_u_type, data, swap);
    data = uint8_t_write(&frame->keycode, data, swap);
    data = uint16_t_write(&frame->state, data, swap);
    data = uint32_t_write(&frame->time, data, swap);
    data = uint32_t_write(&frame->window, data, swap);
    return data;
}

void xcb_im_ext_forward_keyevent_fr_free(
    xcb_im_ext_forward_keyevent_fr_t *frame) {}

void xcb_im_ext_move_fr_read(xcb_im_ext_move_fr_t *frame, uint8_t **data,
                             size_t *len, bool swap) {
    memset(frame, 0, sizeof(*frame));
    uint16_t_read(&frame->input_method_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->input_context_ID, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->X, data, len, swap);
    if (!*data) {
        return;
    }
    uint16_t_read(&frame->Y, data, len, swap);
    if (!*data) {
        return;
    }
}

uint8_t *xcb_im_ext_move_fr_write(xcb_im_ext_move_fr_t *frame, uint8_t *data,
                                  bool swap) {
    data = uint16_t_write(&frame->input_method_ID, data, swap);
    data = uint16_t_write(&frame->input_context_ID, data, swap);
    data = uint16_t_write(&frame->X, data, swap);
    data = uint16_t_write(&frame->Y, data, swap);
    return data;
}

void xcb_im_ext_move_fr_free(xcb_im_ext_move_fr_t *frame) {}
