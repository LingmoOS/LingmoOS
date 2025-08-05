/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#include "message.h"
#include "common.h"
#include "parser.h"
#include "ximproto.h"
#include <stdlib.h>
#include <string.h>

static uint32_t lookup_offset(xcb_im_property_offset_t **offsets,
                              xcb_atom_t atom) {
    if (!offsets) {
        return 0;
    }
    xcb_im_property_offset_t *result = NULL;
    HASH_FIND(hh, *offsets, &atom, sizeof(xcb_atom_t), result);
    if (!result) {
        return 0;
    }
    return result->offset;
}

static void set_offset(xcb_im_property_offset_t **offsets, xcb_atom_t atom,
                       uint32_t offset) {
    if (!offsets) {
        return;
    }
    xcb_im_property_offset_t *result = NULL;
    HASH_FIND(hh, *offsets, &atom, sizeof(xcb_atom_t), result);
    if (!result) {
        if (offset != 0) {
            result = calloc(1, sizeof(xcb_im_property_offset_t));
            result->atom = atom;
            result->offset = offset;
            HASH_ADD(hh, *offsets, atom, sizeof(xcb_atom_t), result);
        }
    } else {
        if (offset == 0) {
            HASH_DEL(*offsets, result);
            free(result);
        } else {
            result->offset = offset;
        }
    }
}

uint8_t *_xcb_new_xim_message(uint8_t major_opcode, uint8_t minor_opcode,
                              size_t length, bool swap) {
    uint8_t *message = calloc(length + XCB_IM_HEADER_SIZE, 1);
    if (message) {
        _xcb_write_xim_message_header(message, major_opcode, minor_opcode,
                                      length, swap);
    }

    return message;
}

uint8_t *_xcb_write_xim_message_header(uint8_t *message, uint8_t major_opcode,
                                       uint8_t minor_opcode, size_t length,
                                       bool swap) {
    uint16_t p_len = length / 4;
    message = uint8_t_write(&major_opcode, message, swap);
    message = uint8_t_write(&minor_opcode, message, swap);
    message = uint16_t_write(&p_len, message, swap);
    return message;
}

// length is the body without header size in byte
bool _xcb_send_xim_message(xcb_connection_t *conn, xcb_atom_t protocol_atom,
                           xcb_window_t window, uint8_t *data, size_t length,
                           const char *name, size_t len) {
    if (!data) {
        return false;
    }

    // add header size
    length += XCB_IM_HEADER_SIZE;

    xcb_client_message_event_t event;
    memset(&event, 0, sizeof(event));

    event.response_type = XCB_CLIENT_MESSAGE;
    event.sequence = 0;
    event.window = window;
    event.type = protocol_atom;

    if (length > XCB_XIM_CM_DATA_SIZE) {
        xcb_atom_t atom;

        xcb_intern_atom_cookie_t atom_cookie =
            xcb_intern_atom(conn, false, len, name);
        xcb_intern_atom_reply_t *atom_reply =
            xcb_intern_atom_reply(conn, atom_cookie, NULL);
        if (!atom_reply) {
            return false;
        }
        atom = atom_reply->atom;
        free(atom_reply);
        xcb_get_property_cookie_t get_property_cookie = xcb_get_property(
            conn, false, window, atom, XCB_ATOM_STRING, 0L, 10000L);

        xcb_get_property_reply_t *get_property_reply =
            xcb_get_property_reply(conn, get_property_cookie, NULL);
        if (!get_property_reply) {
            return false;
        }
        free(get_property_reply);
        xcb_void_cookie_t cookie =
            xcb_change_property_checked(conn, XCB_PROP_MODE_APPEND, window,
                                        atom, XCB_ATOM_STRING, 8, length, data);
        xcb_generic_error_t *error = NULL;
        if ((error = xcb_request_check(conn, cookie)) != NULL) {
            free(error);
        }
        event.format = 32;
        event.data.data32[0] = length;
        event.data.data32[1] = atom;
        for (size_t i = 2; i < ARRAY_SIZE(event.data.data32); i++) {
            event.data.data32[i] = 0;
        }
    } else {
        event.format = 8;

        memcpy(event.data.data8, data, length);
        /* Clear unused field with NULL */
        for (size_t i = length; i < XCB_XIM_CM_DATA_SIZE; i++) {
            event.data.data8[i] = 0;
        }
    }
    xcb_void_cookie_t send_event_cookie = xcb_send_event_checked(
        conn, false, window, XCB_EVENT_MASK_NO_EVENT, (const char *)&event);
    xcb_generic_error_t *error = NULL;
    if ((error = xcb_request_check(conn, send_event_cookie)) != NULL) {
        free(error);
    }
    return true;
}

void _xcb_send_xim_error_message(xcb_connection_t *conn,
                                 xcb_atom_t protocol_atom, xcb_window_t window,
                                 bool swap) {
    // use stack to avoid alloc fails
    uint8_t message[XCB_IM_HEADER_SIZE];
    _xcb_write_xim_message_header(message, XCB_XIM_ERROR, 0, 0, swap);
    _xcb_send_xim_message(conn, protocol_atom, window, message, 0, NULL, 0);
}

uint8_t *_xcb_read_xim_message(xcb_connection_t *conn, xcb_window_t window,
                               xcb_im_property_offset_t **offsets,
                               xcb_client_message_event_t *ev,
                               xcb_im_packet_header_fr_t *hdr, bool swap) {
    uint8_t *p = NULL;

    if (ev->format == 8) {
        /* ClientMessage only */
        uint8_t *rec = ev->data.data8;
        size_t len = sizeof(ev->data.data8);
        uint8_t_read(&hdr->major_opcode, &rec, &len, false);
        uint8_t_read(&hdr->minor_opcode, &rec, &len, false);
        uint16_t_read(&hdr->length, &rec, &len, false);

        // check message is well formed
        if (len >= hdr->length * 4) {
            p = malloc(hdr->length * 4);
        }
        if (p) {
            memcpy(p, rec, hdr->length * 4);
        }
    } else if (ev->format == 32) {
        /* ClientMessage and WindowProperty */
        size_t length = ev->data.data32[0];
        xcb_atom_t atom = ev->data.data32[1];

        uint32_t offset = lookup_offset(offsets, atom);
        uint32_t end = offset + length;
        uint32_t dword_begin = offset / 4;
        uint32_t dword_end = (end + 3) / 4;
        xcb_get_property_cookie_t cookie =
            xcb_get_property(conn, true, window, atom, XCB_ATOM_ANY,
                             dword_begin, dword_end - dword_begin);

        xcb_get_property_reply_t *reply =
            xcb_get_property_reply(conn, cookie, NULL);
        uint8_t *rec;

        if (!reply || reply->format == 0 || reply->length == 0) {
            free(reply);
            return (unsigned char *)NULL;
        }

        rec = xcb_get_property_value(reply) + (offset % 4);

        if (reply->bytes_after) {
            set_offset(offsets, atom, offset + length);
        } else {
            set_offset(offsets, atom, 0);
        }

        uint8_t_read(&hdr->major_opcode, &rec, &length, swap);
        uint8_t_read(&hdr->minor_opcode, &rec, &length, swap);
        uint16_t_read(&hdr->length, &rec, &length, swap);

        size_t buffer_size = ((size_t)hdr->length) * 4;
        // check message is well formed
        if (buffer_size <= length) {
            /* if hit, it might be an error */
            p = malloc(buffer_size);
        }

        if (p) {
            memcpy(p, rec, buffer_size);
        }
        free(reply);
    }
    return p;
}
