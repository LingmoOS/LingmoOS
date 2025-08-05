/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#ifndef _XCB_IMDKIT_IMCLIENT_P_H_
#define _XCB_IMDKIT_IMCLIENT_P_H_

#include "common.h"
#include "imclient.h"
#include "list.h"
#include "message.h"
#include "uthash.h"
#include "ximproto.h"
#include "ximproto_p.h"

// this phase is basically a directly mapping from _XimSOMETHING function in
// Xlib state machine is more suitable for xcb asynchronous nature.
typedef enum _xcb_xim_connect_phase_t {
    XIM_CONNECT_DONE,
    XIM_CONNECT_FAIL,
    XIM_CONNECT_CHECK_SERVER,
    XIM_CONNECT_CONNECT,
} xcb_xim_connect_phase_t;

typedef enum _xcb_xim_connect_check_server_phase_t {
    XIM_CONNECT_CHECK_SERVER_PREPARE,
    XIM_CONNECT_CHECK_SERVER_LOCALE,
    XIM_CONNECT_CHECK_SERVER_LOCALE_WAIT,
    XIM_CONNECT_CHECK_SERVER_TRANSPORT,
    XIM_CONNECT_CHECK_SERVER_TRANSPORT_WAIT,
} xcb_xim_connect_check_server_phase_t;

typedef enum _xcb_xim_connect_connect_phase_t {
    XIM_CONNECT_CONNECT_PREPARE,
    XIM_CONNECT_CONNECT_WAIT,
    XIM_CONNECT_CONNECT_WAIT_REPLY,
} xcb_xim_connect_connect_phase_t;

typedef enum _xcb_xim_open_phase_t {
    XIM_OPEN_INVALID,
    XIM_OPEN_WAIT_OPEN_REPLY,
    XIM_OPEN_WAIT_EXTENSION_REPLY,
    XIM_OPEN_WAIT_ENCODING_REPLY,
    XIM_OPEN_DONE
} xcb_xim_open_phase_t;

typedef struct _xcb_xim_connect_state_t {
    xcb_xim_connect_phase_t phase;
    xcb_xim_open_callback callback;
    void *user_data;
    union {
        struct {
            int index;
            xcb_xim_connect_check_server_phase_t subphase;
            xcb_window_t window;
            xcb_window_t requestor_window;
        } check_server;

        struct {
            xcb_xim_connect_connect_phase_t subphase;
        } connect;
    };
} xcb_xim_connect_state_t;

typedef struct _xcb_xim_imattr_table_t {
    xcb_im_ximattr_fr_t attr;
    UT_hash_handle hh;
} xcb_xim_imattr_table_t;

typedef struct _xcb_xim_icattr_table_t {
    xcb_im_xicattr_fr_t attr;
    UT_hash_handle hh;
} xcb_xim_icattr_table_t;

typedef struct _xcb_xim_extensionx_t {
    uint16_t major_code;
    uint16_t minor_code;
} xcb_xim_extension_t;

typedef struct _xcb_xim_request_queue_t {
    uint8_t major_code;
    uint8_t minor_code;
    void *user_data;
    union {
        xcb_im_create_ic_fr_t create_ic;
        xcb_im_destroy_ic_fr_t destroy_ic;
        xcb_im_get_im_values_fr_t get_im_values;
        xcb_im_get_ic_values_fr_t get_ic_values;
        xcb_im_set_ic_values_fr_t set_ic_values;
        uint8_t forward_event[8 + sizeof(xcb_key_press_event_t)];
        xcb_im_reset_ic_fr_t reset_ic;
    } frame;

    union {
        void (*generic)();
        xcb_xim_create_ic_callback create_ic;
        xcb_xim_destroy_ic_callback destroy_ic;
        xcb_xim_get_im_values_callback get_im_values;
        xcb_xim_get_ic_values_callback get_ic_values;
        xcb_xim_set_ic_values_callback set_ic_values;
        xcb_xim_reset_ic_callback reset_ic;
    } callback;
    list_head list;
} xcb_xim_request_queue_t;

struct _xcb_xim_t {
    // basic data which should always be valid
    xcb_connection_t *conn;
    char *server_name;
    int screen_id;
    xcb_xim_im_callback im_callback;
    void *user_data;

    // some global data
    uint32_t sequence;
    uint8_t byte_order;

    // set by _xcb_xim_init
    xcb_screen_t *screen;
    xcb_screen_t *default_screen;
    xcb_atom_t atoms[XIM_ATOM_LAST]; // add one for SERVER_NAME
    bool init;

    // set by _xcb_xim_get_servers
    xcb_atom_t *server_atoms;
    int n_server_atoms;

    // used by _xcb_xim_check_server / _xcb_xim_connect
    xcb_xim_connect_state_t connect_state;

    // _xcb_xim_check_server
    char *trans_addr;
    xcb_window_t im_window;
    // and also server_atom

    xcb_window_t im_client_window;

    xcb_im_property_offset_t *offsets;

    // _xcb_xim_connect_wait
    int major_code;
    int minor_code;
    uint32_t accept_win;

    // xim open
    xcb_xim_open_phase_t open_state;
    uint16_t connect_id;
    xcb_xim_imattr_table_t *imattr;
    xcb_xim_icattr_table_t *icattr;
    xcb_xim_extension_t *extensions;
    xcb_im_trigger_keys_t onKeys;
    xcb_im_trigger_keys_t offKeys;

    // request
    xcb_xim_request_queue_t *current;
    list_head queue;
    size_t nExtensions;
    bool auto_connect;
    // Indicate whether we need a recheck on the new server.
    bool recheck;
    bool yield_recheck;

    // some ic values
    xcb_window_t client_window;
    xcb_window_t focus_window;

    void (*logger)(const char *, ...);

    bool use_compound_text;
    bool use_utf8_string;
    xcb_xim_encoding_t encoding;

    uint32_t event_sequence;
};

#define _xcb_xim_read_frame(FRAME, DATA, LEN, FAIL)                            \
    do {                                                                       \
        FAIL = false;                                                          \
        uint8_t *_data = DATA;                                                 \
        size_t len = (size_t)(LEN);                                            \
        frame_read_func(FRAME)(&FRAME, &_data, &len, false);                   \
        if (!_data) {                                                          \
            frame_free_func(FRAME)(&FRAME);                                    \
            FAIL = true;                                                       \
        }                                                                      \
    } while (0)

#define _xcb_xim_send_frame(IM, FRAME, FAIL)                                   \
    do {                                                                       \
        FAIL = true;                                                           \
        size_t length = frame_size_func(FRAME);                                \
        uint8_t *reply = NULL;                                                 \
        uint8_t *alloc_reply = NULL;                                           \
        uint8_t                                                                \
            static_reply[XCB_IM_HEADER_SIZE + (frame_has_static_size(FRAME)    \
                                                   ? frame_size_func(FRAME)    \
                                                   : 1)];                      \
        if (frame_has_static_size(FRAME)) {                                    \
            reply = static_reply;                                              \
            _xcb_write_xim_message_header(                                     \
                reply, XIM_PROTO_FRAME_OPCODE(FRAME), 0, length, false);       \
        } else {                                                               \
            reply = _xcb_new_xim_message(XIM_PROTO_FRAME_OPCODE(FRAME), 0,     \
                                         length, false);                       \
            alloc_reply = reply;                                               \
        }                                                                      \
        do {                                                                   \
            if (!reply) {                                                      \
                break;                                                         \
            }                                                                  \
            frame_write_func(FRAME)(&(FRAME), reply + XCB_IM_HEADER_SIZE,      \
                                    false);                                    \
            if (!_xcb_xim_send_message((IM), reply, length)) {                 \
                break;                                                         \
            }                                                                  \
            FAIL = false;                                                      \
        } while (0);                                                           \
        free(alloc_reply);                                                     \
    } while (0)

bool _xcb_xim_send_message(xcb_xim_t *im, uint8_t *data, size_t length);
void _xcb_xim_request_free(xcb_xim_request_queue_t *request);
bool _xcb_xim_sync(xcb_xim_t *im, xcb_xic_t ic);
void _xcb_xim_process_fail_callback(xcb_xim_t *im,
                                    xcb_xim_request_queue_t *request);

#endif // _XCB_IMDKIT_IMCLIENT_P_H_
