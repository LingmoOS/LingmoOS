/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#ifndef MESSAGE_H
#define MESSAGE_H
#include "ximproto.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <uthash.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

typedef struct _xcb_im_property_offset_t {
    xcb_atom_t atom;
    uint32_t offset;
    UT_hash_handle hh;
} xcb_im_property_offset_t;

// Return a new alloc xim message. length is the content size without header
// size.
uint8_t *_xcb_new_xim_message(uint8_t major_opcode, uint8_t minor_opcode,
                              size_t length, bool swap);

// Write xim message header and return the pointer to content.
uint8_t *_xcb_write_xim_message_header(uint8_t *message, uint8_t major_opcode,
                                       uint8_t minor_opcode, size_t length,
                                       bool swap);

// length is the body without header size in byte
bool _xcb_send_xim_message(xcb_connection_t *conn, xcb_atom_t protocol_atom,
                           xcb_window_t window, uint8_t *data, size_t length,
                           const char *name, size_t len);

void _xcb_send_xim_error_message(xcb_connection_t *conn,
                                 xcb_atom_t protocol_atom, xcb_window_t window,
                                 bool swap);

uint8_t *_xcb_read_xim_message(xcb_connection_t *conn, xcb_window_t window,
                               xcb_im_property_offset_t **offsets,
                               xcb_client_message_event_t *ev,
                               xcb_im_packet_header_fr_t *hdr, bool swap);

#endif // MESSAGE_H
