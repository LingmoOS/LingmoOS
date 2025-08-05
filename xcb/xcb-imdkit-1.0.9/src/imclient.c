/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#include "imclient.h"
#include "clientprotocolhandler.h"
#include "common.h"
#include "imclient_p.h"
#include "list.h"
#include "message.h"
#include "parser.h"
#include "uthash.h"
#include "ximproto.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

#define CHECK_NEXT_SERVER(IM)                                                  \
    do {                                                                       \
        (IM)->connect_state.check_server.subphase =                            \
            XIM_CONNECT_CHECK_SERVER_PREPARE;                                  \
        (IM)->connect_state.check_server.index++;                              \
        if ((IM)->connect_state.check_server.requestor_window) {               \
            xcb_destroy_window(                                                \
                (IM)->conn,                                                    \
                (IM)->connect_state.check_server.requestor_window);            \
        }                                                                      \
    } while (0);

void _xcb_xim_close(xcb_xim_t *im);

bool _xcb_xim_send_message(xcb_xim_t *im, uint8_t *data, size_t length) {
    char atomName[64];
    int len = sprintf(atomName, "_client%u", im->sequence++);
    im->sequence = (im->sequence + 1) % XCB_XIM_ATOM_ROTATION_SIZE;
    return _xcb_send_xim_message(im->conn, im->atoms[XIM_ATOM_XIM_PROTOCOL],
                                 im->accept_win, data, length, atomName, len);
}

bool _xcb_xim_check_server_name(xcb_xim_t *im, char *name, int namelen) {
    size_t category_len = strlen(XIM_SERVER_CATEGORY);
    if (strncmp(name, XIM_SERVER_CATEGORY, category_len) != 0) {
        return false;
    }

    if (!im->server_name) {
        return true;
    }

    return (strncmp(name + category_len, im->server_name,
                    namelen - category_len) == 0);
}

bool _xcb_xim_check_transport(xcb_xim_t *im, char *address, char **trans_addr) {
    size_t category_len = strlen(XIM_TRANSPORT_CATEGORY);
    if (strncmp(address, XIM_TRANSPORT_CATEGORY,
                strlen(XIM_TRANSPORT_CATEGORY)) != 0) {
        return false;
    }

    char *p, *pp = address + category_len;

    for (;;) {
        *trans_addr = pp;

        // find first non / , character
        for (p = pp; (*p != '/') && (*p != ',') && (*p); p++) {
            ;
        }
        if (*p == ',') {
            pp = p + 1;
            continue;
        }
        if (!(*p)) {
            return false;
        }

        if ((p - pp) == strlen("X") && (0 == strncmp(pp, "X", strlen("X")))) {
            break;
        }
        pp = p + 1;
    }
    pp = p + 1;
    for (p = pp; (*p != ',') && (*p); p++) {
        ;
    }
    if (*p) {
        *p = '\0';
    }
    return true;
}

bool _xcb_xim_check_server_prepare(xcb_xim_t *im) {
    xcb_atom_t server_atom =
        im->server_atoms[im->connect_state.check_server.index];
    xcb_get_selection_owner_reply_t *owner_reply =
        xcb_get_selection_owner_reply(
            im->conn, xcb_get_selection_owner(im->conn, server_atom), NULL);
    if (!owner_reply) {
        return false;
    }
    im->connect_state.check_server.window = owner_reply->owner;
    free(owner_reply);

    xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(
        im->conn, xcb_get_atom_name(im->conn, server_atom), NULL);
    if (!reply) {
        return false;
    }

    bool result =
        _xcb_xim_check_server_name(im, xcb_get_atom_name_name(reply),
                                   xcb_get_atom_name_name_length(reply));
    free(reply);

    if (!result) {
        return false;
    }

    xcb_window_t w = xcb_generate_id(im->conn);

    xcb_create_window(im->conn, XCB_COPY_FROM_PARENT, w,
                      im->default_screen->root, 0, 0, 1, 1, 1,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      im->default_screen->root_visual, 0, NULL);
    im->connect_state.check_server.requestor_window = w;
    return true;
}

void _xcb_xim_check_server_transport(xcb_xim_t *im) {
    xcb_atom_t server_atom =
        im->server_atoms[im->connect_state.check_server.index];
    xcb_convert_selection(im->conn,
                          im->connect_state.check_server.requestor_window,
                          server_atom, im->atoms[XIM_ATOM_TRANSPORT],
                          im->atoms[XIM_ATOM_TRANSPORT], XCB_CURRENT_TIME);

    xcb_flush(im->conn);
}

typedef enum _xcb_xim_connect_action_t {
    ACTION_ACCEPT,
    ACTION_FAILED,
    ACTION_YIELD,
} xcb_xim_connect_action_t;

xcb_xim_connect_action_t
_xcb_xim_check_server_transport_wait(xcb_xim_t *im,
                                     xcb_generic_event_t *event) {
    xcb_atom_t server_atom =
        im->server_atoms[im->connect_state.check_server.index];
    if (!event) {
        return ACTION_YIELD;
    }

    if ((event->response_type & ~0x80) != XCB_SELECTION_NOTIFY) {
        return ACTION_YIELD;
    }

    xcb_selection_notify_event_t *selection_notify =
        (xcb_selection_notify_event_t *)event;
    if (selection_notify->requestor !=
        im->connect_state.check_server.requestor_window) {
        return ACTION_YIELD;
    }

    if (selection_notify->property == XCB_ATOM_NONE) {
        return ACTION_FAILED;
    }

    xcb_get_property_cookie_t cookie = xcb_get_property(
        im->conn, true, im->connect_state.check_server.requestor_window,
        im->atoms[XIM_ATOM_TRANSPORT], im->atoms[XIM_ATOM_TRANSPORT], 0L,
        100000L);
    xcb_get_property_reply_t *reply =
        xcb_get_property_reply(im->conn, cookie, NULL);
    if (!reply) {
        return ACTION_FAILED;
    }

    char *value = xcb_get_property_value(reply);
    int length = xcb_get_property_value_length(reply);
    char *trans_addr;
    char *address = malloc(length + 1);
    if (!address) {
        free(reply);
        return ACTION_FAILED;
    }
    memcpy(address, value, length);
    address[length] = '\0';
    bool check_transport = _xcb_xim_check_transport(im, address, &trans_addr);
    free(reply);
    if (check_transport) {
        im->trans_addr = strdup(trans_addr);
        free(address);
        if (!im->trans_addr) {
            return ACTION_FAILED;
        }
        xcb_destroy_window(im->conn,
                           im->connect_state.check_server.requestor_window);
        im->connect_state.check_server.requestor_window = XCB_NONE;
        im->im_window = im->connect_state.check_server.window;
        im->atoms[XIM_ATOM_SERVER_NAME] = server_atom;
        return ACTION_ACCEPT;
    }
    free(address);

    return ACTION_FAILED;
}

void _xcb_xim_connect_prepare(xcb_xim_t *im) {
    xcb_window_t w = xcb_generate_id(im->conn);

    xcb_create_window(im->conn, XCB_COPY_FROM_PARENT, w,
                      im->default_screen->root, 0, 0, 1, 1, 1,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      im->default_screen->root_visual, 0, NULL);
    im->im_client_window = w;

    xcb_client_message_event_t ev;
    memset(&ev, 0, sizeof(ev));
    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.format = 32;
    ev.sequence = 0;
    ev.type = im->atoms[XIM_ATOM_XIM_CONNECT];
    ev.window = im->im_window;
    ev.data.data32[0] = im->im_client_window;
    ev.data.data32[1] = 0; // major
    ev.data.data32[2] = 0; // minor
    ev.data.data32[3] = 0;
    ev.data.data32[4] = 0;
    xcb_send_event(im->conn, false, im->im_window, XCB_EVENT_MASK_NO_EVENT,
                   (char *)&ev);
    xcb_flush(im->conn);
}

xcb_xim_connect_action_t _xcb_xim_connect_wait(xcb_xim_t *im,
                                               xcb_generic_event_t *event) {
    if (!event) {
        return ACTION_YIELD;
    }
    if ((event->response_type & ~0x80) != XCB_CLIENT_MESSAGE) {
        return ACTION_YIELD;
    }

    xcb_client_message_event_t *client_message =
        (xcb_client_message_event_t *)event;
    if (client_message->type != im->atoms[XIM_ATOM_XIM_CONNECT]) {
        return ACTION_YIELD;
    }
    event = NULL;
    im->major_code = 0;
    im->minor_code = 0;
    im->accept_win = client_message->data.data32[0];

    xcb_im_connect_fr_t frame;
    frame.byte_order = im->byte_order;
    frame.client_auth_protocol_names.size = 0;
    frame.client_auth_protocol_names.items = NULL;
    frame.client_major_protocol_version = 0;
    frame.client_minor_protocol_version = 0;

    bool fail;
    _xcb_xim_send_frame(im, frame, fail);
    if (fail) {
        return ACTION_FAILED;
    }

    return ACTION_ACCEPT;
}

bool _xcb_xim_send_open(xcb_xim_t *im) {
    xcb_im_open_fr_t frame;
    frame.field0.length_of_string = 0;
    frame.field0.string = 0;

    bool fail;
    _xcb_xim_send_frame(im, frame, fail);
    if (!fail) {
        im->open_state = XIM_OPEN_WAIT_OPEN_REPLY;
    }
    return !fail;
}

xcb_xim_connect_action_t
_xcb_xim_connect_wait_reply(xcb_xim_t *im, xcb_generic_event_t *event) {
    if (!event) {
        return ACTION_YIELD;
    }
    if ((event->response_type & ~0x80) != XCB_CLIENT_MESSAGE) {
        return ACTION_YIELD;
    }
    xcb_client_message_event_t *client_message =
        (xcb_client_message_event_t *)event;
    if (client_message->type != im->atoms[XIM_ATOM_XIM_PROTOCOL]) {
        return ACTION_YIELD;
    }

    xcb_im_packet_header_fr_t hdr;
    uint8_t *message = _xcb_read_xim_message(im->conn, im->accept_win, NULL,
                                             client_message, &hdr, false);

    if (!message) {
        return ACTION_FAILED;
    }

    xcb_xim_connect_action_t result = ACTION_FAILED;
    do {
        if (hdr.major_opcode != XCB_XIM_CONNECT_REPLY) {
            result = ACTION_YIELD;
            break;
        }

        xcb_im_connect_reply_fr_t reply_frame;
        bool fail;
        _xcb_xim_read_frame(reply_frame, message, XIM_MESSAGE_BYTES(&hdr),
                            fail);
        if (fail) {
            break;
        }

        if (!_xcb_xim_send_open(im)) {
            result = ACTION_FAILED;
        } else {
            result = ACTION_ACCEPT;
        }
    } while (0);

    free(message);
    return result;
}

bool _xcb_xim_preconnect_im(xcb_xim_t *im, xcb_generic_event_t *event) {
    while (im->connect_state.phase != XIM_CONNECT_DONE &&
           im->connect_state.phase != XIM_CONNECT_FAIL) {
        if (im->connect_state.phase == XIM_CONNECT_CHECK_SERVER) {
            if (im->connect_state.check_server.index == im->n_server_atoms) {
                im->connect_state.phase = XIM_CONNECT_FAIL;
                continue;
            }
            if (im->connect_state.check_server.subphase ==
                XIM_CONNECT_CHECK_SERVER_PREPARE) {
                if (_xcb_xim_check_server_prepare(im)) {
                    im->connect_state.check_server.subphase =
                        XIM_CONNECT_CHECK_SERVER_LOCALE;
                } else {
                    CHECK_NEXT_SERVER(im);
                    continue;
                }
            } else if (im->connect_state.check_server.subphase ==
                       XIM_CONNECT_CHECK_SERVER_LOCALE) {
                // TODO
                im->connect_state.check_server.subphase =
                    XIM_CONNECT_CHECK_SERVER_TRANSPORT;
            } else if (im->connect_state.check_server.subphase ==
                       XIM_CONNECT_CHECK_SERVER_TRANSPORT) {
                _xcb_xim_check_server_transport(im);
                im->connect_state.check_server.subphase =
                    XIM_CONNECT_CHECK_SERVER_TRANSPORT_WAIT;
            } else if (im->connect_state.check_server.subphase ==
                       XIM_CONNECT_CHECK_SERVER_TRANSPORT_WAIT) {
                xcb_xim_connect_action_t result =
                    _xcb_xim_check_server_transport_wait(im, event);
                switch (result) {
                case ACTION_ACCEPT:
                    event = NULL;
                    im->connect_state.phase = XIM_CONNECT_CONNECT;
                    im->connect_state.connect.subphase =
                        XIM_CONNECT_CONNECT_PREPARE;
                    break;
                case ACTION_FAILED:
                    event = NULL;
                    CHECK_NEXT_SERVER(im);
                    break;
                case ACTION_YIELD:
                    return false;
                }
            }
        } else if (im->connect_state.phase == XIM_CONNECT_CONNECT) {
            if (im->connect_state.connect.subphase ==
                XIM_CONNECT_CONNECT_PREPARE) {
                _xcb_xim_connect_prepare(im);
                im->connect_state.connect.subphase = XIM_CONNECT_CONNECT_WAIT;
            } else if (im->connect_state.connect.subphase ==
                       XIM_CONNECT_CONNECT_WAIT) {
                xcb_xim_connect_action_t result =
                    _xcb_xim_connect_wait(im, event);
                switch (result) {
                case ACTION_ACCEPT:
                    event = NULL;
                    im->connect_state.connect.subphase =
                        XIM_CONNECT_CONNECT_WAIT_REPLY;
                    break;
                case ACTION_FAILED:
                    event = NULL;
                    im->connect_state.phase = XIM_CONNECT_FAIL;
                    break;
                case ACTION_YIELD:
                    return false;
                }
            } else if (im->connect_state.connect.subphase ==
                       XIM_CONNECT_CONNECT_WAIT_REPLY) {
                xcb_xim_connect_action_t result =
                    _xcb_xim_connect_wait_reply(im, event);
                switch (result) {
                case ACTION_ACCEPT:
                    event = NULL;
                    im->connect_state.phase = XIM_CONNECT_DONE;
                    break;
                case ACTION_FAILED:
                    event = NULL;
                    im->connect_state.phase = XIM_CONNECT_FAIL;
                    break;
                case ACTION_YIELD:
                    return false;
                }
            }
        }
    }

    return event == NULL;
}

bool _xcb_xim_init(xcb_xim_t *im) {
    if (im->init) {
        return true;
    }

    const char *atom_names[] = {XIM_SERVERS, XIM_LOCALES, XIM_TRANSPORT,
                                _XIM_PROTOCOL, _XIM_XCONNECT};
    // plus 1 for server name
    if (!_xcb_im_init_atoms(im->conn, ARRAY_SIZE(atom_names), atom_names,
                            im->atoms + 1)) {
        return false;
    }
    im->screen = xcb_aux_get_screen(im->conn, 0);
    im->default_screen = xcb_aux_get_screen(im->conn, im->screen_id);
    if (!im->screen || !im->default_screen) {
        return false;
    }

    im->init = true;

    return true;
}

bool _xcb_xim_get_servers(xcb_xim_t *im) {
    xcb_get_property_cookie_t cookie = xcb_get_property(
        im->conn, false, im->screen->root, im->atoms[XIM_ATOM_XIM_SERVERS],
        XCB_ATOM_ATOM, 0L, 100000L);

    xcb_get_property_reply_t *reply =
        xcb_get_property_reply(im->conn, cookie, NULL);
    if (!reply) {
        return false;
    }

    do {
        if (reply->type != XCB_ATOM_ATOM || reply->format != 32) {
            break;
        }

        int items = xcb_get_property_value_length(reply) / sizeof(xcb_atom_t);
        im->n_server_atoms = items;
        if (items <= 0) {
            break;
        }
        free(im->server_atoms);
        xcb_atom_t *server_atoms = xcb_get_property_value(reply);
        im->server_atoms = calloc(items, sizeof(xcb_atom_t));
        memcpy(im->server_atoms, server_atoms, items * sizeof(xcb_atom_t));
    } while (0);

    free(reply);

    return (im->n_server_atoms > 0);
}

char *_xcb_xim_make_im_name(const char *imname) {
    if (!imname || strncmp(imname, "@im=", strlen("@im=")) != 0) {
        return NULL;
    }

    imname += strlen("@im=");

    return strdup(imname);
}

xcb_xim_t *xcb_xim_create(xcb_connection_t *conn, int screen_id,
                          const char *imname) {
    xcb_xim_t *im = calloc(1, sizeof(xcb_xim_t));

    if (!imname) {
        imname = getenv("XMODIFIERS");
    }
    im->conn = conn;
    im->server_name = _xcb_xim_make_im_name(imname);
    im->screen_id = screen_id;
    im->connect_state.phase = XIM_CONNECT_FAIL;
    list_init(&im->queue);
    uint16_t endian = 1;
    if (*(char *)&endian) {
        im->byte_order = 'l';
    } else {
        im->byte_order = 'B';
    }

    im->use_compound_text = true;
    im->use_utf8_string = false;
    im->encoding = XCB_XIM_COMPOUND_TEXT;
    return im;
}

void xcb_xim_set_log_handler(xcb_xim_t *im, void (*logger)(const char *, ...)) {
    im->logger = logger;
}

void xcb_xim_set_use_compound_text(xcb_xim_t *im, bool enable) {
    im->use_compound_text = enable;
}

void xcb_xim_set_use_utf8_string(xcb_xim_t *im, bool enable) {
    im->use_utf8_string = enable;
}

xcb_xim_encoding_t xcb_xim_get_encoding(xcb_xim_t *im) { return im->encoding; }

bool _xcb_xim_open(xcb_xim_t *im) {
    im->connect_state.phase = XIM_CONNECT_FAIL;
    im->open_state = XIM_OPEN_INVALID;

    if (!_xcb_xim_init(im)) {
        return false;
    }

    if (im->auto_connect) {
        _xcb_change_event_mask(im->conn, im->screen->root,
                               XCB_EVENT_MASK_PROPERTY_CHANGE, false);
    }

    if (!_xcb_xim_get_servers(im)) {
        return false;
    }
    im->connect_state.phase = XIM_CONNECT_CHECK_SERVER;
    im->connect_state.check_server.index = 0;
    im->connect_state.check_server.requestor_window = 0;
    im->connect_state.check_server.window = 0;
    im->connect_state.check_server.subphase = XIM_CONNECT_CHECK_SERVER_PREPARE;

    _xcb_xim_preconnect_im(im, NULL);
    return true;
}

bool xcb_xim_open(xcb_xim_t *im, xcb_xim_open_callback callback,
                  bool auto_connect, void *user_data) {
    im->connect_state.callback = callback;
    im->connect_state.user_data = user_data;
    im->auto_connect = auto_connect;

    return _xcb_xim_open(im);
}

void _xcb_xim_handle_message(xcb_xim_t *im,
                             const xcb_im_packet_header_fr_t *hdr,
                             uint8_t *data) {
    switch (hdr->major_opcode) {
    case XCB_XIM_OPEN_REPLY:
        DebugLog("-- XIM_OPEN_REPLY\n");
        _xcb_xim_handle_open_reply(im, hdr, data);
        break;
    case XCB_XIM_REGISTER_TRIGGERKEYS:
        DebugLog("-- XIM_REGISTER_TRIGGERKEYS\n");
        _xcb_xim_handle_register_triggerkeys(im, hdr, data);
        break;
    case XCB_XIM_QUERY_EXTENSION_REPLY:
        DebugLog("-- XIM_QUERY_EXTENSION_REPLY\n");
        _xcb_xim_handle_query_extension_reply(im, hdr, data);
        break;
    case XCB_XIM_ENCODING_NEGOTIATION_REPLY:
        DebugLog("-- XIM_ENCODING_NEGOTIATION_REPLY\n");
        _xcb_xim_handle_encoding_negotiation_reply(im, hdr, data);
        break;
    case XCB_XIM_GET_IM_VALUES_REPLY:
        DebugLog("-- XIM_GET_IM_VALUES_REPLY\n");
        _xcb_xim_handle_get_im_values_reply(im, hdr, data);
        break;
    case XCB_XIM_SET_EVENT_MASK:
        DebugLog("-- XIM_SET_EVENT_MASK\n");
        _xcb_xim_handle_set_event_mask(im, hdr, data);
        break;
    case XCB_XIM_CREATE_IC_REPLY:
        DebugLog("-- XIM_CREATE_IC_REPLY\n");
        _xcb_xim_handle_create_ic_reply(im, hdr, data);
        break;
    case XCB_XIM_GET_IC_VALUES_REPLY:
        DebugLog("-- XIM_GET_IC_VALUES_REPLY\n");
        _xcb_xim_handle_get_ic_values_reply(im, hdr, data);
        break;
    case XCB_XIM_SET_IC_VALUES_REPLY:
        DebugLog("-- XIM_SET_IC_VALUES_REPLY\n");
        _xcb_xim_handle_set_ic_values_reply(im, hdr, data);
        break;
    case XCB_XIM_FORWARD_EVENT:
        DebugLog("-- XIM_FORWARD_EVENT\n");
        _xcb_xim_handle_forward_event(im, hdr, data);
        break;
    case XCB_XIM_SYNC:
        DebugLog("-- XIM_SYNC\n");
        _xcb_xim_handle_sync(im, hdr, data);
        break;
    case XCB_XIM_COMMIT:
        DebugLog("-- XIM_COMMIT\n");
        _xcb_xim_handle_commit(im, hdr, data);
        break;
    case XCB_XIM_GEOMETRY:
        DebugLog("-- XIM_GEOMETRY\n");
        _xcb_xim_handle_geometry(im, hdr, data);
        break;
    case XCB_XIM_PREEDIT_START:
        DebugLog("-- XIM_PREEDIT_START\n");
        _xcb_xim_handle_preedit_start(im, hdr, data);
        break;
    case XCB_XIM_PREEDIT_DRAW:
        DebugLog("-- XIM_PREEDIT_DRAW\n");
        _xcb_xim_handle_preedit_draw(im, hdr, data);
        break;
    case XCB_XIM_PREEDIT_CARET:
        DebugLog("-- XIM_PREEDIT_CARET\n");
        _xcb_xim_handle_preedit_caret(im, hdr, data);
        break;
    case XCB_XIM_PREEDIT_DONE:
        DebugLog("-- XIM_PREEDIT_DONE\n");
        _xcb_xim_handle_preedit_done(im, hdr, data);
        break;
    case XCB_XIM_STATUS_START:
        DebugLog("-- XIM_STATUS_START\n");
        _xcb_xim_handle_status_start(im, hdr, data);
        break;
    case XCB_XIM_STATUS_DRAW:
        DebugLog("-- XIM_STATUS_DRAW\n");
        _xcb_xim_handle_status_draw(im, hdr, data);
        break;
    case XCB_XIM_STATUS_DONE:
        DebugLog("-- XIM_STATUS_DONE\n");
        _xcb_xim_handle_status_done(im, hdr, data);
        break;
    case XCB_XIM_CLOSE_REPLY:
        DebugLog("-- XIM_CLOSE_REPLY\n");
        _xcb_xim_handle_close_reply(im, hdr, data);
        break;
    case XCB_XIM_DESTROY_IC_REPLY:
        DebugLog("-- XIM_DESTROY_IC_REPLY\n");
        _xcb_xim_handle_destroy_ic_reply(im, hdr, data);
        break;
    case XCB_XIM_RESET_IC_REPLY:
        DebugLog("-- XIM_DESTROY_IC_REPLY\n");
        _xcb_xim_handle_reset_ic_reply(im, hdr, data);
        break;
    case XCB_XIM_ERROR:
        DebugLog("-- XIM_ERROR\n");
        _xcb_xim_handle_error(im, data);
        break;
    }
}

bool _xcb_xim_filter_event(xcb_xim_t *im, xcb_generic_event_t *event) {
    if (im->connect_state.phase != XIM_CONNECT_DONE) {
        return false;
    }

    do {
        if ((event->response_type & ~0x80) != XCB_CLIENT_MESSAGE) {
            break;
        }

        xcb_client_message_event_t *clientmessage =
            (xcb_client_message_event_t *)event;

        if (clientmessage->type != im->atoms[XIM_ATOM_XIM_PROTOCOL]) {
            break;
        }

        xcb_im_packet_header_fr_t hdr;
        uint8_t *message =
            _xcb_read_xim_message(im->conn, im->im_client_window, &im->offsets,
                                  clientmessage, &hdr, false);
        if (message) {
            _xcb_xim_handle_message(im, &hdr, message);
            free(message);
        }

        return true;

    } while (0);

    return false;
}

bool _xcb_xim_send_disconnect(xcb_xim_t *im) {
    xcb_im_disconnect_fr_t frame;
    bool fail;
    _xcb_xim_send_frame(im, frame, fail);
    return !fail;
}

bool _xcb_xim_send_close(xcb_xim_t *im) {
    xcb_im_close_fr_t frame;
    frame.input_method_ID = im->connect_id;
    bool fail;
    _xcb_xim_send_frame(im, frame, fail);
    return !fail;
}

void _xcb_xim_clean_up(xcb_xim_t *im) {
    _xcb_xim_close(im);
    if (im->auto_connect && im->recheck) {
        im->recheck = false;
        _xcb_xim_open(im);
    }
}

void _xcb_xim_disconnected(xcb_xim_t *im) {
    if (im->im_callback.disconnected) {
        im->im_callback.disconnected(im, im->user_data);
    }

    _xcb_xim_clean_up(im);
}

bool _xcb_xim_filter_destroy_window(xcb_xim_t *im, xcb_generic_event_t *event) {
    do {
        if (im->open_state != XIM_OPEN_DONE) {
            return false;
        }

        if ((event->response_type & ~0x80) != XCB_DESTROY_NOTIFY) {
            break;
        }

        xcb_destroy_notify_event_t *destroy_notify =
            (xcb_destroy_notify_event_t *)event;

        if (im->accept_win != destroy_notify->window) {
            break;
        }

        _xcb_xim_disconnected(im);

        return true;
    } while (0);

    return false;
}

bool _xcb_xim_filter_property_changed(xcb_xim_t *im,
                                      xcb_generic_event_t *event) {
    do {
        if (!im->auto_connect) {
            return false;
        }

        if ((event->response_type & ~0x80) != XCB_PROPERTY_NOTIFY) {
            break;
        }

        xcb_property_notify_event_t *property_notify =
            (xcb_property_notify_event_t *)event;

        if (im->screen->root != property_notify->window) {
            break;
        }

        if (property_notify->atom != im->atoms[XIM_ATOM_XIM_SERVERS]) {
            break;
        }

        im->recheck = true;
        if (im->connect_state.phase == XIM_CONNECT_FAIL) {
            im->yield_recheck = true;
        }

        return true;
    } while (0);

    return false;
}

bool xcb_xim_filter_event(xcb_xim_t *im, xcb_generic_event_t *event) {
    im->yield_recheck = false;
    im->event_sequence = event->full_sequence;
    bool result = _xcb_xim_preconnect_im(im, event) ||
                  _xcb_xim_filter_event(im, event) ||
                  _xcb_xim_filter_destroy_window(im, event) ||
                  _xcb_xim_filter_property_changed(im, event);
    if (im->yield_recheck) {
        _xcb_xim_clean_up(im);
    }
    return result;
}

bool xcb_xim_trigger_notify(xcb_xim_t *im, xcb_xic_t ic, uint32_t idx,
                            bool off) {
    xcb_im_trigger_notify_fr_t frame;
    frame.input_method_ID = im->connect_id;
    frame.input_context_ID = ic;
    frame.flag = off ? 1 : 0;
    frame.index_of_keys_list = idx;
    frame.client_select_event_mask =
        _xcb_get_event_mask(im->conn, im->focus_window);
    bool fail;
    _xcb_xim_send_frame(im, frame, fail);
    return !fail;
}

xcb_xim_trigger_key_type_t xcb_xim_check_trigger_key(xcb_xim_t *im,
                                                     xcb_keysym_t keysym,
                                                     uint32_t modifier,
                                                     uint32_t *idx) {
    for (uint32_t i = 0; i < im->onKeys.nKeys; i++) {
        if (im->onKeys.keys[i].keysym == keysym &&
            (modifier & im->onKeys.keys[i].modifier_mask) ==
                im->onKeys.keys[i].modifier) {
            *idx = i;
            return XCB_XIM_TRIGGER_ON_KEY;
        }
    }
    for (uint32_t i = 0; i < im->offKeys.nKeys; i++) {
        if (im->offKeys.keys[i].keysym == keysym &&
            (modifier & im->offKeys.keys[i].modifier_mask) ==
                im->offKeys.keys[i].modifier) {
            *idx = i;
            return XCB_XIM_TRIGGER_OFF_KEY;
        }
    }
    return XCB_XIM_IS_NOT_TRIGGER;
}

bool xcb_xim_check_trigger_on_key(xcb_xim_t *im, xcb_keysym_t keysym,
                                  uint32_t modifier, uint32_t *idx) {
    for (uint32_t i = 0; i < im->onKeys.nKeys; i++) {
        if (im->onKeys.keys[i].keysym == keysym &&
            (modifier & im->onKeys.keys[i].modifier_mask) ==
                im->onKeys.keys[i].modifier) {
            *idx = i;
            return true;
        }
    }
    return false;
}

bool xcb_xim_check_trigger_off_key(xcb_xim_t *im, xcb_keysym_t keysym,
                                   uint32_t modifier, uint32_t *idx) {
    for (uint32_t i = 0; i < im->offKeys.nKeys; i++) {
        if (im->offKeys.keys[i].keysym == keysym &&
            (modifier & im->offKeys.keys[i].modifier_mask) ==
                im->offKeys.keys[i].modifier) {
            *idx = i;
            return XCB_XIM_TRIGGER_OFF_KEY;
        }
    }
    return false;
}

void xcb_xim_destroy(xcb_xim_t *im) {
    free(im->server_name);
    free(im);
}

void _xcb_xim_close(xcb_xim_t *im) {
    im->client_window = XCB_NONE;
    im->focus_window = XCB_NONE;

    if (im->open_state == XIM_OPEN_DONE) {
        _xcb_xim_send_close(im);
    }

    if (im->connect_state.phase == XIM_CONNECT_DONE) {
        _xcb_xim_send_disconnect(im);
    } else if (im->connect_state.phase == XIM_CONNECT_CHECK_SERVER) {
        if (im->connect_state.check_server.requestor_window) {
            xcb_destroy_window(im->conn,
                               im->connect_state.check_server.requestor_window);
        }
    }

    im->open_state = XIM_OPEN_INVALID;

    free(im->extensions);
    im->extensions = NULL;
    im->nExtensions = 0;

    free(im->onKeys.keys);
    im->onKeys.keys = NULL;
    im->onKeys.nKeys = 0;
    free(im->offKeys.keys);
    im->onKeys.keys = NULL;
    im->offKeys.nKeys = 0;

    free(im->server_atoms);
    im->server_atoms = NULL;
    im->n_server_atoms = 0;

    free(im->trans_addr);
    im->trans_addr = NULL;
    im->connect_state.phase = XIM_CONNECT_FAIL;
    if (im->im_client_window != XCB_NONE) {
        xcb_destroy_window(im->conn, im->im_client_window);
        im->im_client_window = XCB_NONE;
        xcb_flush(im->conn);
    }

    while (im->imattr) {
        xcb_xim_imattr_table_t *imattr = im->imattr;
        HASH_DEL(im->imattr, im->imattr);
        free(imattr->attr.im_attribute);
        free(imattr);
    }

    while (im->icattr) {
        xcb_xim_icattr_table_t *icattr = im->icattr;
        HASH_DEL(im->icattr, im->icattr);
        free(icattr->attr.ic_attribute);
        free(icattr);
    }

    if (im->current) {
        _xcb_xim_request_free(im->current);
        im->current = NULL;
    }

    list_entry_foreach_safe(item, xcb_xim_request_queue_t, &im->queue, list) {
        list_remove(&item->list);
        _xcb_xim_request_free(item);
    }

    while (im->offsets) {
        xcb_im_property_offset_t *p = im->offsets;
        HASH_DEL(im->offsets, im->offsets);
        free(p);
    }
}

void xcb_xim_close(xcb_xim_t *im) {
    im->auto_connect = false;
    im->recheck = false;

    _xcb_xim_close(im);
}

void _xcb_xim_request_free(xcb_xim_request_queue_t *request) {
    switch (request->major_code) {
    case XCB_XIM_CREATE_IC: {
        xcb_im_xicattribute_fr_t *items =
            request->frame.create_ic.ic_attributes.items;
        for (uint32_t i = 0; i < request->frame.create_ic.ic_attributes.size;
             i++) {
            free(items[i].value);
        }

        free(items);
        break;
    }
    case XCB_XIM_GET_IM_VALUES:
        free(request->frame.get_im_values.im_attribute_id.items);
        break;
    case XCB_XIM_GET_IC_VALUES:
        free(request->frame.get_ic_values.ic_attribute.items);
        break;
    case XCB_XIM_SET_IC_VALUES: {
        xcb_im_xicattribute_fr_t *items =
            request->frame.set_ic_values.ic_attribute.items;
        for (uint32_t i = 0; i < request->frame.set_ic_values.ic_attribute.size;
             i++) {
            free(items[i].value);
        }

        free(items);
        break;
    }
    case XCB_XIM_DESTROY_IC:
    case XCB_XIM_FORWARD_EVENT:
    case XCB_XIM_RESET_IC:
        break;
    }
    free(request);
}

bool _xcb_xim_send_request_frame(xcb_xim_t *im,
                                 xcb_xim_request_queue_t *request) {
    bool fail = true;
    switch (request->major_code) {
    case XCB_XIM_CREATE_IC:
        _xcb_xim_send_frame(im, request->frame.create_ic, fail);
        break;
    case XCB_XIM_DESTROY_IC:
        _xcb_xim_send_frame(im, request->frame.destroy_ic, fail);
        break;
    case XCB_XIM_GET_IM_VALUES:
        _xcb_xim_send_frame(im, request->frame.get_im_values, fail);
        break;
    case XCB_XIM_GET_IC_VALUES:
        _xcb_xim_send_frame(im, request->frame.get_ic_values, fail);
        break;
    case XCB_XIM_SET_IC_VALUES:
        _xcb_xim_send_frame(im, request->frame.set_ic_values, fail);
        break;
    case XCB_XIM_FORWARD_EVENT:
        fail = !_xcb_xim_send_message(im, request->frame.forward_event,
                                      sizeof(request->frame.forward_event));
        break;
    case XCB_XIM_RESET_IC:
        _xcb_xim_send_frame(im, request->frame.reset_ic, fail);
        break;
    default:
        break;
    }
    return !fail;
}

void _xcb_xim_process_fail_callback(xcb_xim_t *im,
                                    xcb_xim_request_queue_t *request) {
    if (!request->callback.generic) {
        return;
    }
    switch (request->major_code) {
    case XCB_XIM_CREATE_IC:
        request->callback.create_ic(im, 0, request->user_data);
        break;
    case XCB_XIM_DESTROY_IC:
        request->callback.destroy_ic(
            im, request->frame.destroy_ic.input_context_ID, request->user_data);
        break;
    case XCB_XIM_GET_IM_VALUES:
        request->callback.get_im_values(im, NULL, request->user_data);
        break;
    case XCB_XIM_GET_IC_VALUES:
        request->callback.get_ic_values(
            im, request->frame.get_ic_values.input_context_ID, NULL,
            request->user_data);
        break;
    case XCB_XIM_SET_IC_VALUES:
        request->callback.set_ic_values(
            im, request->frame.set_ic_values.input_context_ID,
            request->user_data);
        break;
    case XCB_XIM_RESET_IC:
        request->callback.reset_ic(im, request->frame.reset_ic.input_context_ID,
                                   NULL, request->user_data);
        break;
    case XCB_XIM_FORWARD_EVENT:
        break;
    default:
        break;
    }
}

void _xcb_xim_process_queue(xcb_xim_t *im) {
    while (!im->current && !list_is_empty(&im->queue)) {
        xcb_xim_request_queue_t *request =
            list_container_of((im->queue).next, xcb_xim_request_queue_t, list);
        list_remove(&request->list);
        if (_xcb_xim_send_request_frame(im, request)) {
            if (request->major_code != XCB_XIM_FORWARD_EVENT) {
                im->current = request;
            }
        } else {
            _xcb_xim_process_fail_callback(im, request);
            _xcb_xim_request_free(request);
            im->current = NULL;
        }
    }
}

static inline xcb_xim_icattr_table_t *_xcb_xim_find_icattr(xcb_xim_t *im,
                                                           const char *attr) {
    xcb_xim_icattr_table_t *icattr = NULL;
    HASH_FIND_STR(im->icattr, attr, icattr);
    return icattr;
}

static inline xcb_xim_imattr_table_t *_xcb_xim_find_imattr(xcb_xim_t *im,
                                                           const char *attr) {
    xcb_xim_imattr_table_t *imattr = NULL;
    HASH_FIND_STR(im->imattr, attr, imattr);
    return imattr;
}

size_t _xcb_xim_check_valist(xcb_xim_t *im, va_list var) {
    char *attr;

    size_t count = 0;

    for (attr = va_arg(var, char *); attr; attr = va_arg(var, char *)) {
        (void)va_arg(var, void **);

        xcb_xim_icattr_table_t *icattr = _xcb_xim_find_icattr(im, attr);
        if (!icattr) {
            return 0;
        }
        ++count;
    }
    return count;
}

xcb_xim_nested_list xcb_xim_create_nested_list(xcb_xim_t *im, ...) {
    xcb_xim_nested_list result;
    result.data = 0;
    result.length = 0;

    va_list var;
    bool flag = false;
    size_t totalSize = 0;
    uint8_t *data = NULL;
    for (int round = 0; round < 2; round++) {
        va_start(var, im);
        for (char *attr = va_arg(var, char *); attr;
             attr = va_arg(var, char *)) {
            if (round == 0) {
                (void)va_arg(var, void *);

                xcb_xim_icattr_table_t *icattr = _xcb_xim_find_icattr(im, attr);
                if (!icattr) {
                    flag = true;
                    break;
                }

                xcb_im_xicattribute_fr_t fr;
                fr.value_length =
                    _xcb_im_ic_attr_size(icattr->attr.type_of_the_value);
                if (fr.value_length == 0) {
                    continue;
                }
                totalSize += xcb_im_xicattribute_fr_size(&fr);
            } else {
                void *p = va_arg(var, void *);
                xcb_xim_icattr_table_t *icattr = _xcb_xim_find_icattr(im, attr);
                uint8_t *start = data;
                uint16_t value_length =
                    _xcb_im_ic_attr_size(icattr->attr.type_of_the_value);
                data = uint16_t_write(&icattr->attr.attribute_ID, data, false);
                data = uint16_t_write(&value_length, data, false);
                data = _xcb_im_get_ic_value(p, icattr->attr.type_of_the_value,
                                            data, false);
                data =
                    (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
            }
        }
        va_end(var);
        if (round == 0) {
            if (totalSize) {
                data = malloc(totalSize);
            }
            if (!data) {
                flag = true;
            } else {
                result.data = data;
                result.length = totalSize;
            }
        }
        if (flag) {
            break;
        }
    }

    return result;
}

#define _xcb_xim_new_request(im, major_code, minor_code, callback, user_data)  \
    __xcb_xim_new_request(im, major_code, minor_code,                          \
                          (xcb_xim_callback)callback, user_data)

xcb_xim_request_queue_t *
__xcb_xim_new_request(xcb_xim_t *im, uint8_t major_code, uint8_t minor_code,
                      xcb_xim_callback callback, void *user_data) {
    xcb_xim_request_queue_t *queue = calloc(1, sizeof(xcb_xim_request_queue_t));
    if (!queue) {
        return NULL;
    }
    queue->major_code = major_code;
    queue->minor_code = minor_code;
    queue->callback.generic = callback;
    queue->user_data = user_data;

    return queue;
}

bool xcb_xim_create_ic(xcb_xim_t *im, xcb_xim_create_ic_callback callback,
                       void *user_data, ...) {
    if (im->open_state != XIM_OPEN_DONE) {
        return false;
    }

    xcb_xim_request_queue_t *queue =
        _xcb_xim_new_request(im, XCB_XIM_CREATE_IC, 0, callback, user_data);
    if (!queue) {
        return false;
    }

    va_list var;
    va_start(var, user_data);
    queue->frame.create_ic.input_method_ID = im->connect_id;
    queue->frame.create_ic.ic_attributes.size = _xcb_xim_check_valist(im, var);
    queue->frame.create_ic.ic_attributes.items =
        calloc(queue->frame.create_ic.ic_attributes.size,
               sizeof(xcb_im_xicattribute_fr_t));
    va_end(var);

    if (!queue->frame.create_ic.ic_attributes.items) {
        free(queue);
        return false;
    }

    xcb_im_xicattribute_fr_t *items =
        queue->frame.create_ic.ic_attributes.items;
    uint32_t nItems = queue->frame.create_ic.ic_attributes.size;

    va_start(var, user_data);
    for (uint32_t i = 0; i < nItems; i++) {
        char *attr = va_arg(var, char *);
        void *p = va_arg(var, void *);
        if (strcmp(attr, XCB_XIM_XNClientWindow) == 0) {
            im->client_window = *(xcb_window_t *)p;
        } else if (strcmp(attr, XCB_XIM_XNFocusWindow) == 0) {
            im->client_window = *(xcb_window_t *)p;
        }

        xcb_xim_icattr_table_t *icattr = _xcb_xim_find_icattr(im, attr);
        items[i].attribute_ID = icattr->attr.attribute_ID;
        if (icattr->attr.type_of_the_value == XimType_NEST) {
            xcb_xim_nested_list *nested = p;
            items[i].value_length = nested->length;
            items[i].value = malloc(nested->length);
            memcpy(items[i].value, nested->data, nested->length);
        } else {
            items[i].value_length =
                _xcb_im_ic_attr_size(icattr->attr.type_of_the_value);
            items[i].value = malloc(items[i].value_length);
            _xcb_im_get_ic_value(p, icattr->attr.type_of_the_value,
                                 items[i].value, false);
        }
    }
    va_end(var);

    list_append(&queue->list, &im->queue);

    _xcb_xim_process_queue(im);

    return true;
}

bool xcb_xim_destroy_ic(xcb_xim_t *im, xcb_xic_t ic,
                        xcb_xim_destroy_ic_callback callback, void *user_data) {
    if (im->open_state != XIM_OPEN_DONE) {
        return false;
    }

    xcb_xim_request_queue_t *queue =
        _xcb_xim_new_request(im, XCB_XIM_DESTROY_IC, 0, callback, user_data);
    if (!queue) {
        return false;
    }

    queue->frame.destroy_ic.input_method_ID = im->connect_id;
    queue->frame.destroy_ic.input_context_ID = ic;

    list_append(&queue->list, &im->queue);

    _xcb_xim_process_queue(im);

    return true;
}

bool xcb_xim_get_im_values(xcb_xim_t *im,
                           xcb_xim_get_im_values_callback callback,
                           void *user_data, ...) {
    if (im->open_state != XIM_OPEN_DONE) {
        return false;
    }
    va_list var;
    va_start(var, user_data);
    uint32_t count = 0;
    for (char *attr = va_arg(var, char *); attr; attr = va_arg(var, char *)) {
        xcb_xim_imattr_table_t *imattr = _xcb_xim_find_imattr(im, attr);
        if (imattr) {
            count++;
        }
    }
    va_end(var);

    if (count == 0) {
        return false;
    }

    xcb_xim_request_queue_t *queue =
        _xcb_xim_new_request(im, XCB_XIM_GET_IM_VALUES, 0, callback, user_data);
    if (!queue) {
        return false;
    }

    queue->frame.get_im_values.input_method_ID = im->connect_id;
    queue->frame.get_im_values.im_attribute_id.size = count;
    queue->frame.get_im_values.im_attribute_id.items =
        calloc(count, sizeof(uint16_t));
    va_start(var, user_data);
    count = 0;
    for (char *attr = va_arg(var, char *); attr; attr = va_arg(var, char *)) {
        xcb_xim_imattr_table_t *imattr = _xcb_xim_find_imattr(im, attr);
        if (imattr) {
            queue->frame.get_im_values.im_attribute_id.items[count] =
                imattr->attr.attribute_ID;
            count++;
        }
    }
    va_end(var);

    list_append(&queue->list, &im->queue);

    _xcb_xim_process_queue(im);

    return true;
}

bool xcb_xim_get_ic_values(xcb_xim_t *im, xcb_xic_t ic,
                           xcb_xim_get_ic_values_callback callback,
                           void *user_data, ...) {
    if (im->open_state != XIM_OPEN_DONE) {
        return false;
    }
    va_list var;
    va_start(var, user_data);
    uint32_t count = 0;
    for (char *attr = va_arg(var, char *); attr; attr = va_arg(var, char *)) {
        xcb_xim_icattr_table_t *icattr = _xcb_xim_find_icattr(im, attr);
        if (icattr) {
            count++;
        }
    }
    va_end(var);

    if (count == 0) {
        return false;
    }

    xcb_xim_request_queue_t *queue =
        _xcb_xim_new_request(im, XCB_XIM_GET_IC_VALUES, 0, callback, user_data);
    if (!queue) {
        return false;
    }

    queue->frame.get_ic_values.input_method_ID = im->connect_id;
    queue->frame.get_ic_values.input_context_ID = ic;
    queue->frame.get_ic_values.ic_attribute.size = count;
    queue->frame.get_ic_values.ic_attribute.items =
        calloc(count, sizeof(uint16_t));
    va_start(var, user_data);
    count = 0;
    for (char *attr = va_arg(var, char *); attr; attr = va_arg(var, char *)) {
        xcb_xim_icattr_table_t *icattr = _xcb_xim_find_icattr(im, attr);
        if (icattr) {
            queue->frame.get_ic_values.ic_attribute.items[count] =
                icattr->attr.attribute_ID;
            count++;
        }
    }
    va_end(var);

    list_append(&queue->list, &im->queue);

    _xcb_xim_process_queue(im);

    return true;
}

// not actually enabled by Xlib
bool xcb_xim_ext_move(xcb_xim_t *im, xcb_xic_t ic, int16_t x, int16_t y) {
    xcb_im_ext_move_fr_t frame;
    frame.input_method_ID = im->connect_id;
    frame.input_context_ID = ic;
    frame.X = x;
    frame.Y = y;

    size_t length = xcb_im_ext_move_fr_size(&frame);
    uint8_t reply[XCB_IM_HEADER_SIZE + xcb_im_ext_move_fr_size(&frame)];
    _xcb_write_xim_message_header(reply, XCB_XIM_EXTENSION, XCB_XIM_EXT_MOVE,
                                  length, false);
    xcb_im_ext_move_fr_write(&frame, reply + XCB_IM_HEADER_SIZE, false);
    if (!_xcb_xim_send_message(im, reply, length)) {
        return false;
    }
    return true;
}

bool xcb_xim_set_ic_values(xcb_xim_t *im, xcb_xic_t ic,
                           xcb_xim_set_ic_values_callback callback,
                           void *user_data, ...) {
    if (im->open_state != XIM_OPEN_DONE) {
        return false;
    }

    va_list var;
    va_start(var, user_data);
    size_t attr_size = _xcb_xim_check_valist(im, var);
    va_end(var);
    if (!attr_size) {
        return false;
    }

    xcb_xim_request_queue_t *queue =
        _xcb_xim_new_request(im, XCB_XIM_SET_IC_VALUES, 0, callback, user_data);
    if (!queue) {
        return false;
    }

    queue->frame.set_ic_values.input_method_ID = im->connect_id;
    queue->frame.set_ic_values.input_context_ID = ic;
    queue->frame.set_ic_values.ic_attribute.size = attr_size;
    queue->frame.set_ic_values.ic_attribute.items =
        calloc(queue->frame.set_ic_values.ic_attribute.size,
               sizeof(xcb_im_xicattribute_fr_t));

    if (!queue->frame.set_ic_values.ic_attribute.items) {
        free(queue);
        return false;
    }

    xcb_im_xicattribute_fr_t *items =
        queue->frame.set_ic_values.ic_attribute.items;
    uint32_t nItems = queue->frame.set_ic_values.ic_attribute.size;

    va_start(var, user_data);
    for (uint32_t i = 0; i < nItems; i++) {
        char *attr = va_arg(var, char *);
        void *p = va_arg(var, void *);
        if (strcmp(attr, XCB_XIM_XNClientWindow) == 0) {
            im->client_window = *(xcb_window_t *)p;
        } else if (strcmp(attr, XCB_XIM_XNFocusWindow) == 0) {
            im->client_window = *(xcb_window_t *)p;
        }

        xcb_xim_icattr_table_t *icattr = _xcb_xim_find_icattr(im, attr);
        items[i].attribute_ID = icattr->attr.attribute_ID;
        if (icattr->attr.type_of_the_value == XimType_NEST) {
            xcb_xim_nested_list *nested = p;
            items[i].value_length = nested->length;
            items[i].value = malloc(nested->length);
            memcpy(items[i].value, nested->data, nested->length);
        } else {
            items[i].value_length =
                _xcb_im_ic_attr_size(icattr->attr.type_of_the_value);
            items[i].value = malloc(items[i].value_length);
            _xcb_im_get_ic_value(p, icattr->attr.type_of_the_value,
                                 items[i].value, false);
        }
    }
    va_end(var);

    list_append(&queue->list, &im->queue);

    _xcb_xim_process_queue(im);

    return true;
}

bool xcb_xim_set_ic_focus(xcb_xim_t *im, xcb_xic_t ic) {
    xcb_im_set_ic_focus_fr_t frame;
    frame.input_method_ID = im->connect_id;
    frame.input_context_ID = ic;

    bool fail;
    _xcb_xim_send_frame(im, frame, fail);
    return !fail;
}

bool xcb_xim_unset_ic_focus(xcb_xim_t *im, xcb_xic_t ic) {
    xcb_im_unset_ic_focus_fr_t frame;
    frame.input_method_ID = im->connect_id;
    frame.input_context_ID = ic;

    bool fail;
    _xcb_xim_send_frame(im, frame, fail);
    return !fail;
}

bool xcb_xim_forward_event(xcb_xim_t *im, xcb_xic_t ic,
                           xcb_key_press_event_t *event) {
    return xcb_xim_forward_event_full(im, ic, (im->event_sequence >> 16),
                                      event);
}

bool xcb_xim_forward_event_full(xcb_xim_t *im, xcb_xic_t ic, uint16_t sequence,
                                xcb_key_press_event_t *event) {
    xcb_xim_request_queue_t *queue =
        _xcb_xim_new_request(im, XCB_XIM_FORWARD_EVENT, 0, NULL, NULL);
    if (!queue) {
        return false;
    }
    xcb_im_forward_event_fr_t frame;
    frame.input_method_ID = im->connect_id;
    frame.input_context_ID = ic;
    frame.flag = XCB_XIM_SYNCHRONOUS;
    frame.sequence_number = sequence;

    const size_t length =
        xcb_im_forward_event_fr_size(&frame) + sizeof(xcb_key_press_event_t);
    _xcb_write_xim_message_header(queue->frame.forward_event,
                                  XCB_XIM_FORWARD_EVENT, 0, length, false);
    uint8_t *p = xcb_im_forward_event_fr_write(
        &frame, queue->frame.forward_event + XCB_IM_HEADER_SIZE, false);
    memcpy(p, event, sizeof(xcb_key_press_event_t));

    list_append(&queue->list, &im->queue);

    _xcb_xim_process_queue(im);
    return true;
}

bool xcb_xim_reset_ic(xcb_xim_t *im, xcb_xic_t ic,
                      xcb_xim_reset_ic_callback callback, void *user_data) {
    xcb_xim_request_queue_t *queue =
        _xcb_xim_new_request(im, XCB_XIM_RESET_IC, 0, NULL, NULL);
    if (!queue) {
        return false;
    }

    queue->frame.reset_ic.input_method_ID = im->connect_id;
    queue->frame.reset_ic.input_context_ID = ic;

    list_append(&queue->list, &im->queue);

    _xcb_xim_process_queue(im);
    return true;
}

void xcb_xim_set_im_callback(xcb_xim_t *im,
                             const xcb_xim_im_callback *callbacks,
                             void *user_data) {
    memcpy(&im->im_callback, callbacks, sizeof(xcb_xim_im_callback));
    im->user_data = user_data;
}

bool _xcb_xim_sync(xcb_xim_t *im, xcb_xic_t ic) {
    xcb_im_sync_reply_fr_t frame;
    frame.input_method_ID = im->connect_id;
    frame.input_context_ID = ic;

    bool fail;
    _xcb_xim_send_frame(im, frame, fail);
    return !fail;
}

bool xcb_xim_support_extension(xcb_xim_t *im, uint16_t major_code,
                               uint16_t minor_code) {
    if (im->open_state != XIM_OPEN_DONE) {
        return false;
    }

    for (size_t i = 0; i < im->nExtensions; i++) {
        if (im->extensions[i].major_code == major_code &&
            im->extensions[i].minor_code == minor_code) {
            return true;
        }
    }
    return false;
}
