/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#ifndef _XCB_IMDKIT_IMDKIT_P_H_
#define _XCB_IMDKIT_IMDKIT_P_H_

#include "common.h"
#include "imdkit.h"
#include "list.h"
#include "message.h"
#include "uthash.h"
#include "ximproto_p.h"

#define LOCALES_BUFSIZE (sizeof(XCB_IM_ALL_LOCALES) + 32)

/*
 * values for the flag of XIM_ERROR
 */
#define XIM_IMID_VALID 0x0001
#define XIM_ICID_VALID 0x0002

/*
 * XIM Error Code
 */
#define XIM_BadAlloc 1
#define XIM_BadStyle 2
#define XIM_BadClientWindow 3
#define XIM_BadFocusWindow 4
#define XIM_BadArea 5
#define XIM_BadSpotLocation 6
#define XIM_BadColormap 7
#define XIM_BadAtom 8
#define XIM_BadPixel 9
#define XIM_BadPixmap 10
#define XIM_BadName 11
#define XIM_BadCursor 12
#define XIM_BadProtocol 13
#define XIM_BadForeground 14
#define XIM_BadBackground 15
#define XIM_LocaleNotSupported 16
#define XIM_BadSomething 999

/*
 * byte order
 */
#define BIGENDIAN (CARD8)0x42    /* MSB first */
#define LITTLEENDIAN (CARD8)0x6c /* LSB first */

/*
 * values for the category of XIM_ENCODING_NEGOTIATON_REPLY
 */
#define XIM_Encoding_NameCategory 0
#define XIM_Encoding_DetailCategory 1

/*
 * value for the index of XIM_ENCODING_NEGOTIATON_REPLY
 */
#define XIM_Default_Encoding_IDX -1

#define _xcb_im_read_frame_with_error(IM, CLIENT, FRAME, DATA, LEN)            \
    do {                                                                       \
        size_t len = (LEN);                                                    \
        frame_read_func(FRAME)(&FRAME, &DATA, &len,                            \
                               (CLIENT)->byte_order != (IM)->byte_order);      \
        if (!DATA) {                                                           \
            frame_free_func(FRAME)(&FRAME);                                    \
            _xcb_im_send_error_message((IM), (CLIENT));                        \
            return;                                                            \
        }                                                                      \
    } while (0)

#define _xcb_im_read_frame(IM, CLIENT, FRAME, DATA, LEN)                       \
    do {                                                                       \
        size_t len = (LEN);                                                    \
        frame_read_func(FRAME)(&FRAME, &DATA, &len,                            \
                               (CLIENT)->byte_order != (IM)->byte_order);      \
        if (!DATA) {                                                           \
            frame_free_func(FRAME)(&FRAME);                                    \
            return;                                                            \
        }                                                                      \
    } while (0)

#define _xcb_im_send_frame(IM, CLIENT, FRAME, SEND_ERROR)                      \
    do {                                                                       \
        bool fail = true;                                                      \
        bool swap = (CLIENT)->byte_order != (IM)->byte_order;                  \
        size_t length = frame_size_func(FRAME);                                \
        uint8_t *reply;                                                        \
        uint8_t *alloc_reply = NULL;                                           \
        uint8_t                                                                \
            static_reply[XCB_IM_HEADER_SIZE + (frame_has_static_size(FRAME)    \
                                                   ? frame_size_func(FRAME)    \
                                                   : 1)];                      \
        if (frame_has_static_size(FRAME)) {                                    \
            reply = static_reply;                                              \
            _xcb_write_xim_message_header(                                     \
                reply, XIM_PROTO_FRAME_OPCODE(FRAME), 0, length, swap);        \
        } else {                                                               \
            reply = _xcb_new_xim_message(XIM_PROTO_FRAME_OPCODE(FRAME), 0,     \
                                         length, swap);                        \
            alloc_reply = reply;                                               \
        }                                                                      \
        do {                                                                   \
            if (!reply) {                                                      \
                break;                                                         \
            }                                                                  \
            frame_write_func(FRAME)(&(FRAME), reply + XCB_IM_HEADER_SIZE,      \
                                    swap);                                     \
            if (!_xcb_im_send_message((IM), (CLIENT), reply, length)) {        \
                break;                                                         \
            }                                                                  \
            fail = false;                                                      \
        } while (0);                                                           \
        free(alloc_reply);                                                     \
        if ((SEND_ERROR) && fail) {                                            \
            _xcb_im_send_error_message((IM), (CLIENT));                        \
        }                                                                      \
    } while (0)

struct _xcb_im_input_context_t {
    uint16_t id;
    xcb_im_client_t *client;
    uint32_t input_style;
    xcb_window_t client_win;
    xcb_window_t focus_win;

    xcb_im_preedit_attr_t preedit;
    uint32_t preedit_mask;
    xcb_im_status_attr_t status;
    uint32_t status_mask;
    UT_hash_handle hh;
    void *data;
    xcb_im_free_function free_data_function;
    uint16_t forward_event_sequence;
};

struct _xcb_im_client_t {
    xcb_window_t accept_win;
    int connect_id;
    xcb_window_t client_win;
    uint8_t byte_order;
    bool sync;
    uint16_t icid;
    xcb_im_input_context_t *ic_free_list;
    xcb_im_input_context_t *input_contexts;
    list_head queue;

    xcb_im_property_offset_t *offsets;

    UT_hash_handle hh1;
    UT_hash_handle hh2;
};

typedef struct _xcb_im_default_im_attr_t {
    char *name;
    uint16_t type;
    bool (*get_value)(xcb_im_t *im, xcb_im_client_t *client,
                      xcb_im_ximattribute_fr_t *attr);
} xcb_im_default_im_attr_t;

bool _xcb_im_get_input_styles_attr(xcb_im_t *im, xcb_im_client_t *client,
                                   xcb_im_ximattribute_fr_t *attr);

static const xcb_im_default_im_attr_t Default_IMattr[] = {
    {XCB_XIM_XNQueryInputStyle, XimType_XIMStyles,
     _xcb_im_get_input_styles_attr},
    /*    {XCB_XIM_XNQueryIMValuesList, XimType_XIMValuesList}, */
};

typedef struct _xcb_im_default_ic_attr_t {
    char *name;
    uint16_t type;
    bool read;
} xcb_im_default_ic_attr_t;

static const xcb_im_default_ic_attr_t Default_ICattr[] = {
    {XCB_XIM_XNInputStyle, XimType_CARD32, false},
    {XCB_XIM_XNClientWindow, XimType_Window, false},
    {XCB_XIM_XNFocusWindow, XimType_Window, false},
    {XCB_XIM_XNFilterEvents, XimType_CARD32, false},
    {XCB_XIM_XNPreeditAttributes, XimType_NEST, false},
    {XCB_XIM_XNStatusAttributes, XimType_NEST, false},
    {XCB_XIM_XNFontSet, XimType_XFontSet, false},
    {XCB_XIM_XNArea, XimType_XRectangle, true},
    {XCB_XIM_XNAreaNeeded, XimType_XRectangle, true},
    {XCB_XIM_XNColormap, XimType_CARD32, false},
    {XCB_XIM_XNStdColormap, XimType_CARD32, false},
    {XCB_XIM_XNForeground, XimType_CARD32, true},
    {XCB_XIM_XNBackground, XimType_CARD32, true},
    {XCB_XIM_XNBackgroundPixmap, XimType_CARD32, false},
    {XCB_XIM_XNSpotLocation, XimType_XPoint, true},
    {XCB_XIM_XNLineSpace, XimType_CARD32, true},
    {XCB_XIM_XNSeparatorofNestedList, XimType_SeparatorOfNestedList, false},
};

struct _xcb_im_t {
    xcb_connection_t *conn;
    xcb_im_ximattr_fr_t imattr[ARRAY_SIZE(Default_IMattr)];
    xcb_im_xicattr_fr_t icattr[ARRAY_SIZE(Default_ICattr)];
    xcb_im_ext_fr_t extension[ARRAY_SIZE(Default_Extension)];
    uint16_t preeditAttr_id;
    uint16_t statusAttr_id;
    uint16_t separatorAttr_id;
    xcb_im_ximattr_fr_t
        *id2attr[ARRAY_SIZE(Default_IMattr) + ARRAY_SIZE(Default_ICattr)];

    ssize_t id2preeditoffset[ARRAY_SIZE(Default_IMattr) +
                             ARRAY_SIZE(Default_ICattr)];
    uint32_t
        id2preeditmask[ARRAY_SIZE(Default_IMattr) + ARRAY_SIZE(Default_ICattr)];

    ssize_t id2statusoffset[ARRAY_SIZE(Default_IMattr) +
                            ARRAY_SIZE(Default_ICattr)];
    uint32_t
        id2statusmask[ARRAY_SIZE(Default_IMattr) + ARRAY_SIZE(Default_ICattr)];

    ssize_t
        id2icoffset[ARRAY_SIZE(Default_IMattr) + ARRAY_SIZE(Default_ICattr)];
    uint32_t event_mask;
    xcb_im_trigger_keys_t onKeys;
    xcb_im_trigger_keys_t offKeys;
    xcb_im_styles_t inputStyles;
    xcb_im_encodings_t encodings;
    char *locale;
    char *serverName;
    xcb_window_t serverWindow;
    int screen_id;
    xcb_atom_t atoms[XIM_ATOM_LAST];
    xcb_im_client_t *free_list;
    xcb_im_client_t *clients_by_id;
    xcb_im_client_t *clients_by_win;
    uint16_t connect_id;
    xcb_screen_t *screen;
    xcb_screen_t *default_screen;
    uint32_t sequence;
    xcb_im_callback callback;
    void *user_data;
    void (*logger)(const char *, ...);

    uint8_t byte_order;
    bool init;
    bool sync;
    bool use_sync_mode;
    bool use_sync_event;
};

typedef union _xcb_im_ic_attr_value_t {
    xcb_rectangle_t rect;
    xcb_point_t point;
    uint32_t byte4;
    struct {
        uint8_t *data;
        uint8_t len;
    } raw;
} xcb_im_ic_attr_value_t;

typedef struct _xcb_im_queue_t {
    uint16_t icid;
    xcb_im_packet_header_fr_t hdr;
    xcb_im_forward_event_fr_t frame;
    xcb_key_press_event_t event;
    list_head list;
} xcb_im_queue_t;

xcb_im_input_context_t *_xcb_im_new_input_context(xcb_im_t *im,
                                                  xcb_im_client_t *client);
const xcb_im_default_ic_attr_t *_xcb_im_default_ic_attr_entry(xcb_im_t *im,
                                                              uint32_t id);

bool _xcb_im_send_message(xcb_im_t *im, xcb_im_client_t *client, uint8_t *data,
                          size_t length);

void _xcb_im_send_error_message(xcb_im_t *im, xcb_im_client_t *client);

void _xcb_im_destroy_client(xcb_im_t *im, xcb_im_client_t *client);
void _xcb_im_destroy_ic(xcb_im_t *im, xcb_im_input_context_t *ic);
void _xcb_im_send_set_event_mask(xcb_im_t *im, xcb_im_client_t *client,
                                 uint32_t icid, uint32_t forward_event_mask,
                                 uint32_t sync_mask);
void _xcb_im_set_ic_event_mask(xcb_im_t *im, xcb_im_input_context_t *client);
void _xcb_im_set_im_event_mask(xcb_im_t *im, xcb_im_client_t *ic);
void _xcb_im_add_queue(xcb_im_t *im, xcb_im_client_t *client, uint16_t icid,
                       const xcb_im_packet_header_fr_t *hdr,
                       xcb_im_forward_event_fr_t *frame, uint8_t *data);
void _xcb_im_process_queue(xcb_im_t *im, xcb_im_client_t *client);
static inline bool _xcb_im_has_trigger_key(xcb_im_t *im) {
    return im->onKeys.nKeys || im->offKeys.nKeys;
}
#endif // _XCB_IMDKIT_IMDKIT_P_H_
