/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#ifndef COMMON_H
#define COMMON_H

#include "ximproto.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

// some atom name
#define XIM_SERVERS "XIM_SERVERS"
#define XIM_LOCALES "LOCALES"
#define XIM_TRANSPORT "TRANSPORT"
#define _XIM_PROTOCOL "_XIM_PROTOCOL"
#define _XIM_XCONNECT "_XIM_XCONNECT"

#define ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))

#define XIM_MESSAGE_BYTES(hdr) ((hdr)->length * 4u)

/*
 * categories in XIM_SERVERS
 */
#define XIM_SERVER_CATEGORY "@server="
#define XIM_LOCAL_CATEGORY "@locale="
#define XIM_TRANSPORT_CATEGORY "@transport="

#define DebugLog(S...)                                                         \
    do {                                                                       \
        if (im->logger) {                                                      \
            im->logger(S);                                                     \
        }                                                                      \
    } while (0)

/*
 * values for the type of XIMATTR & XICATTR
 */
#define XimType_SeparatorOfNestedList 0
#define XimType_CARD8 1
#define XimType_CARD16 2
#define XimType_CARD32 3
#define XimType_STRING8 4
#define XimType_Window 5
#define XimType_XIMStyles 10
#define XimType_XRectangle 11
#define XimType_XPoint 12
#define XimType_XFontSet 13
#define XimType_XIMOptions 14
#define XimType_XIMHotKeyTriggers 15
#define XimType_XIMHotKeyState 16
#define XimType_XIMStringConversion 17
#define XimType_XIMValuesList 18
#define XimType_NEST 0x7FFF

enum {
    XIM_ATOM_SERVER_NAME,
    XIM_ATOM_XIM_SERVERS,
    XIM_ATOM_LOCALES,
    XIM_ATOM_TRANSPORT,
    XIM_ATOM_XIM_PROTOCOL,
    XIM_ATOM_XIM_CONNECT,
    XIM_ATOM_LAST
};

static const xcb_im_ext_list Default_Extension[] = {
    // client -> server, by set ic in xlib
    {"XIM_EXT_MOVE", XCB_XIM_EXTENSION, XCB_XIM_EXT_MOVE},
#if 0
    // not in any imdkit
    // server -> client
    {"XIM_EXT_SET_EVENT_MASK", XIM_EXTENSION, XIM_EXT_SET_EVENT_MASK},
    // server <-> client
    // not enabled by xlib
    {"XIM_EXT_FORWARD_KEYEVENT", XIM_EXTENSION, XIM_EXT_FORWARD_KEYEVENT},
#endif
};

typedef void (*xcb_xim_callback)();

bool _xcb_im_init_atoms(xcb_connection_t *conn, size_t n,
                        const char **atom_names, xcb_atom_t *atoms);

uint32_t _xcb_get_event_mask(xcb_connection_t *conn, xcb_window_t window);
bool _xcb_change_event_mask(xcb_connection_t *conn, xcb_window_t window,
                            uint32_t mask, bool remove);

size_t _xcb_im_ic_attr_size(uint32_t type);

uint8_t *_xcb_im_get_ic_value(void *p, uint32_t type, uint8_t *data, bool swap);

#endif // COMMON_H
