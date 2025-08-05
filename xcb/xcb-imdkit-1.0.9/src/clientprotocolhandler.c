/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#include "clientprotocolhandler.h"
#include "common.h"
#include "imclient_p.h"
#include "parser.h"
#include "uthash.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xproto.h>

bool _xcb_xim_send_query_extension(xcb_xim_t *im) {
    xcb_im_query_extension_fr_t frame;
    xcb_im_str_fr_t extensions[ARRAY_SIZE(Default_Extension)];
    frame.input_method_ID = im->connect_id;
    frame.extensions_supported_by_the_IM_library.size =
        ARRAY_SIZE(Default_Extension);
    frame.extensions_supported_by_the_IM_library.items = extensions;
    for (uint32_t i = 0; i < frame.extensions_supported_by_the_IM_library.size;
         i++) {
        extensions[i].length_of_string = strlen(Default_Extension[i].name);
        extensions[i].string = (uint8_t *)Default_Extension[i].name;
    }

    bool fail;
    _xcb_xim_send_frame(im, frame, fail);
    if (!fail) {
        im->open_state = XIM_OPEN_WAIT_EXTENSION_REPLY;
    }
    return !fail;
}

bool _xcb_xim_send_encoding_negotiation(xcb_xim_t *im) {
    xcb_im_encoding_negotiation_fr_t frame;
    xcb_im_str_fr_t encodings[2];

    int idx = 0;
    if (im->use_compound_text) {
        encodings[idx].length_of_string = strlen("COMPOUND_TEXT");
        encodings[idx].string = (uint8_t *)"COMPOUND_TEXT";
        idx++;
    }
    if (im->use_utf8_string) {
        encodings[idx].length_of_string = strlen("UTF8_STRING");
        encodings[idx].string = (uint8_t *)"UTF8_STRING";
        idx++;
    }

    frame.input_method_ID = im->connect_id;
    frame.supported_list_of_encoding_in_IM_library.size = idx;
    frame.supported_list_of_encoding_in_IM_library.items = encodings;
    frame.list_of_encodings_supported_in_th.size = 0;
    frame.list_of_encodings_supported_in_th.items = 0;

    bool fail;
    _xcb_xim_send_frame(im, frame, fail);
    if (!fail) {
        im->open_state = XIM_OPEN_WAIT_ENCODING_REPLY;
    }
    return !fail;
}

void _xcb_xim_handle_open_reply(xcb_xim_t *im,
                                const xcb_im_packet_header_fr_t *hdr,
                                uint8_t *data) {
    if (im->open_state != XIM_OPEN_WAIT_OPEN_REPLY) {
        return;
    }
    xcb_im_open_reply_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    im->connect_id = frame.input_method_ID;
    // construct a name based attr table
    for (uint32_t i = 0; i < frame.IM_attribute_supported.size; i++) {
        xcb_im_ximattr_fr_t *subframe = &frame.IM_attribute_supported.items[i];

        if (subframe->length_of_im_attribute == 0) {
            continue;
        }

        xcb_xim_imattr_table_t *imattr = NULL;
        HASH_FIND(hh, im->imattr, subframe->im_attribute,
                  subframe->length_of_im_attribute, imattr);
        if (imattr) {
            continue;
        }

        imattr = malloc(sizeof(xcb_xim_imattr_table_t));
        imattr->attr.attribute_ID = subframe->attribute_ID;
        imattr->attr.type_of_the_value = subframe->type_of_the_value;
        imattr->attr.length_of_im_attribute = subframe->length_of_im_attribute;
        imattr->attr.im_attribute =
            malloc((imattr->attr.length_of_im_attribute + 1) * sizeof(uint8_t));
        imattr->attr.im_attribute[imattr->attr.length_of_im_attribute] = 0;
        memcpy(imattr->attr.im_attribute, subframe->im_attribute,
               imattr->attr.length_of_im_attribute);

        HASH_ADD_KEYPTR(hh, im->imattr, imattr->attr.im_attribute,
                        imattr->attr.length_of_im_attribute, imattr);
    }
    for (uint32_t i = 0; i < frame.IC_attribute_supported.size; i++) {
        xcb_im_xicattr_fr_t *subframe = &frame.IC_attribute_supported.items[i];
        if (subframe->length_of_ic_attribute == 0) {
            continue;
        }
        xcb_xim_icattr_table_t *icattr = NULL;
        HASH_FIND(hh, im->icattr, subframe->ic_attribute,
                  subframe->length_of_ic_attribute, icattr);
        if (icattr) {
            continue;
        }

        icattr = malloc(sizeof(xcb_xim_icattr_table_t));
        icattr->attr.attribute_ID = subframe->attribute_ID;
        icattr->attr.type_of_the_value = subframe->type_of_the_value;
        icattr->attr.length_of_ic_attribute = subframe->length_of_ic_attribute;
        icattr->attr.ic_attribute =
            malloc((icattr->attr.length_of_ic_attribute + 1) * sizeof(uint8_t));
        icattr->attr.ic_attribute[icattr->attr.length_of_ic_attribute] = 0;
        memcpy(icattr->attr.ic_attribute, subframe->ic_attribute,
               icattr->attr.length_of_ic_attribute);

        HASH_ADD_KEYPTR(hh, im->icattr, icattr->attr.ic_attribute,
                        icattr->attr.length_of_ic_attribute, icattr);
    }
    xcb_im_open_reply_fr_free(&frame);

    _xcb_xim_send_query_extension(im);
}

void _xcb_xim_handle_query_extension_reply(xcb_xim_t *im,
                                           const xcb_im_packet_header_fr_t *hdr,
                                           uint8_t *data) {
    if (im->open_state != XIM_OPEN_WAIT_EXTENSION_REPLY) {
        return;
    }
    xcb_im_query_extension_reply_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);

    do {
        if (frame.input_method_ID != im->connect_id) {
            break;
        }
        free(im->extensions);
        im->extensions = NULL;
        im->nExtensions = 0;
        // There are some wrong implementation around that use a different wire
        // format, thus do not hard fail on this.
        if (fail) {
            break;
        }

        im->extensions =
            frame.list_of_extensions_supported_by_th.size
                ? calloc(frame.list_of_extensions_supported_by_th.size,
                         sizeof(xcb_xim_extension_t))
                : NULL;
        im->nExtensions = frame.list_of_extensions_supported_by_th.size;
        for (uint32_t i = 0; i < frame.list_of_extensions_supported_by_th.size;
             i++) {
            im->extensions[i].major_code =
                frame.list_of_extensions_supported_by_th.items[i]
                    .extension_major_opcode;
            im->extensions[i].minor_code =
                frame.list_of_extensions_supported_by_th.items[i]
                    .extension_minor_opcode;
        }
    } while (0);

    _xcb_xim_send_encoding_negotiation(im);
    if (!fail) {
        xcb_im_query_extension_reply_fr_free(&frame);
    }
}

void _xcb_xim_handle_encoding_negotiation_reply(
    xcb_xim_t *im, const xcb_im_packet_header_fr_t *hdr, uint8_t *data) {
    if (im->open_state != XIM_OPEN_WAIT_ENCODING_REPLY) {
        return;
    }
    xcb_im_encoding_negotiation_reply_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        xcb_xim_encoding_t encodings[2] = {XCB_XIM_COMPOUND_TEXT,
                                           XCB_XIM_COMPOUND_TEXT};
        size_t nEncodings = 0;
        if (im->use_compound_text) {
            encodings[nEncodings++] = XCB_XIM_COMPOUND_TEXT;
        }
        if (im->use_utf8_string) {
            encodings[nEncodings++] = XCB_XIM_UTF8_STRING;
        }
        // we only send compound
        if (frame.input_method_ID != im->connect_id &&
            frame.index_of_the_encoding_determined >= nEncodings) {
            break;
        }

        im->open_state = XIM_OPEN_DONE;
        im->encoding = encodings[frame.index_of_the_encoding_determined];

        if (im->connect_state.callback) {
            im->connect_state.callback(im, im->connect_state.user_data);
        }

        _xcb_change_event_mask(im->conn, im->accept_win,
                               XCB_EVENT_MASK_STRUCTURE_NOTIFY, false);
    } while (0);
    xcb_im_encoding_negotiation_reply_fr_free(&frame);
}

void _xcb_xim_handle_register_triggerkeys(xcb_xim_t *im,
                                          const xcb_im_packet_header_fr_t *hdr,
                                          uint8_t *data) {
    xcb_im_register_triggerkeys_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        // we dont check (frame.input_method_ID != im->connect_id);
        // here because this is send with open_reply
        free(im->onKeys.keys);
        free(im->offKeys.keys);

        im->onKeys.keys = frame.on_keys_list.size
                              ? calloc(frame.on_keys_list.size,
                                       sizeof(xcb_im_ximtriggerkey_fr_t))
                              : NULL;
        im->offKeys.keys = frame.off_keys_list.size
                               ? calloc(frame.off_keys_list.size,
                                        sizeof(xcb_im_ximtriggerkey_fr_t))
                               : NULL;
        if ((frame.on_keys_list.size && !im->onKeys.keys) ||
            (frame.off_keys_list.size && !im->offKeys.keys)) {
            free(im->onKeys.keys);
            free(im->offKeys.keys);
            im->onKeys.keys = im->offKeys.keys = NULL;
            im->onKeys.nKeys = im->offKeys.nKeys = 0;
            break;
        }

        im->onKeys.nKeys = frame.on_keys_list.size;
        im->offKeys.nKeys = frame.off_keys_list.size;
        if (frame.on_keys_list.size) {
            memcpy(im->onKeys.keys, frame.on_keys_list.items,
                   frame.on_keys_list.size * sizeof(xcb_im_ximtriggerkey_fr_t));
        }
        if (frame.off_keys_list.size) {
            memcpy(im->offKeys.keys, frame.off_keys_list.items,
                   frame.off_keys_list.size *
                       sizeof(xcb_im_ximtriggerkey_fr_t));
        }
    } while (0);
    xcb_im_register_triggerkeys_fr_free(&frame);
}

void _xcb_xim_handle_create_ic_reply(xcb_xim_t *im,
                                     const xcb_im_packet_header_fr_t *hdr,
                                     uint8_t *data) {
    xcb_im_create_ic_reply_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (!im->current) {
            break;
        }

        if (im->current->major_code != XCB_XIM_CREATE_IC &&
            im->connect_id != frame.input_method_ID) {
            break;
        }

        xcb_xim_request_queue_t *request = im->current;
        im->current = NULL;

        if (request->callback.create_ic) {
            request->callback.create_ic(im, frame.input_context_ID,
                                        request->user_data);
        }

        _xcb_xim_request_free(request);
    } while (0);
    xcb_im_create_ic_reply_fr_free(&frame);
}

void _xcb_xim_handle_destroy_ic_reply(xcb_xim_t *im,
                                      const xcb_im_packet_header_fr_t *hdr,
                                      uint8_t *data) {
    xcb_im_destroy_ic_reply_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (!im->current) {
            break;
        }

        if (im->current->major_code != XCB_XIM_DESTROY_IC &&
            im->connect_id != frame.input_method_ID &&
            im->current->frame.destroy_ic.input_context_ID !=
                frame.input_context_ID) {
            break;
        }

        xcb_xim_request_queue_t *request = im->current;
        im->current = NULL;

        if (request->callback.destroy_ic) {
            request->callback.destroy_ic(im, frame.input_context_ID,
                                         request->user_data);
        }

        _xcb_xim_request_free(request);
    } while (0);
    xcb_im_destroy_ic_reply_fr_free(&frame);
}

void _xcb_xim_handle_get_im_values_reply(xcb_xim_t *im,
                                         const xcb_im_packet_header_fr_t *hdr,
                                         uint8_t *data) {
    xcb_im_get_im_values_reply_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (!im->current) {
            break;
        }

        if (im->current->major_code != XCB_XIM_GET_IM_VALUES &&
            im->connect_id != frame.input_method_ID) {
            break;
        }

        xcb_xim_request_queue_t *request = im->current;
        im->current = NULL;

        if (request->callback.get_im_values) {
            request->callback.get_im_values(im, &frame, request->user_data);
        }

        _xcb_xim_request_free(request);
    } while (0);
    xcb_im_get_im_values_reply_fr_free(&frame);
}

void _xcb_xim_handle_get_ic_values_reply(xcb_xim_t *im,
                                         const xcb_im_packet_header_fr_t *hdr,
                                         uint8_t *data) {
    xcb_im_get_ic_values_reply_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (!im->current) {
            break;
        }

        if (im->current->major_code != XCB_XIM_GET_IC_VALUES &&
            im->connect_id != frame.input_method_ID &&
            im->current->frame.get_ic_values.input_context_ID !=
                frame.input_context_ID) {
            break;
        }

        xcb_xim_request_queue_t *request = im->current;
        im->current = NULL;

        if (request->callback.get_ic_values) {
            request->callback.get_ic_values(im, frame.input_context_ID, &frame,
                                            request->user_data);
        }

        _xcb_xim_request_free(request);
    } while (0);
    xcb_im_get_ic_values_reply_fr_free(&frame);
}

void _xcb_xim_handle_set_ic_values_reply(xcb_xim_t *im,
                                         const xcb_im_packet_header_fr_t *hdr,
                                         uint8_t *data) {
    xcb_im_set_ic_values_reply_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (!im->current) {
            break;
        }

        if (im->current->major_code != XCB_XIM_SET_IC_VALUES &&
            im->connect_id != frame.input_method_ID &&
            im->current->frame.set_ic_values.input_context_ID !=
                frame.input_context_ID) {
            break;
        }

        xcb_xim_request_queue_t *request = im->current;
        im->current = NULL;

        if (request->callback.set_ic_values) {
            request->callback.set_ic_values(im, frame.input_context_ID,
                                            request->user_data);
        }

        _xcb_xim_request_free(request);
    } while (0);
    xcb_im_set_ic_values_reply_fr_free(&frame);
}

void _xcb_xim_handle_reset_ic_reply(xcb_xim_t *im,
                                    const xcb_im_packet_header_fr_t *hdr,
                                    uint8_t *data) {
    xcb_im_reset_ic_reply_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (!im->current) {
            break;
        }

        if (im->current->major_code != XCB_XIM_RESET_IC &&
            im->connect_id != frame.input_method_ID &&
            im->current->frame.reset_ic.input_context_ID !=
                frame.input_context_ID) {
            break;
        }

        xcb_xim_request_queue_t *request = im->current;
        im->current = NULL;

        if (request->callback.reset_ic) {
            request->callback.reset_ic(im, frame.input_context_ID, &frame,
                                       request->user_data);
        }

        _xcb_xim_request_free(request);
    } while (0);
    xcb_im_reset_ic_reply_fr_free(&frame);
}

void _xcb_xim_handle_forward_event(xcb_xim_t *im,
                                   const xcb_im_packet_header_fr_t *hdr,
                                   uint8_t *data) {
    xcb_im_forward_event_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (XIM_MESSAGE_BYTES(hdr) < xcb_im_forward_event_fr_size(&frame) +
                                         sizeof(xcb_key_press_event_t)) {
            break;
        }
        if (im->connect_id != frame.input_method_ID) {
            break;
        }
        xcb_key_press_event_t key_event;
        data += xcb_im_forward_event_fr_size(&frame);
        memcpy(&key_event, data, sizeof(xcb_key_press_event_t));

        if (im->im_callback.forward_event) {
            im->im_callback.forward_event(im, frame.input_context_ID,
                                          &key_event, im->user_data);
        }

        if (frame.flag & XCB_XIM_SYNCHRONOUS) {
            _xcb_xim_sync(im, frame.input_context_ID);
        }
    } while (0);
    xcb_im_forward_event_fr_free(&frame);
}

void _xcb_xim_handle_commit(xcb_xim_t *im, const xcb_im_packet_header_fr_t *hdr,
                            uint8_t *data) {
    if (XIM_MESSAGE_BYTES(hdr) < 6) {
        return;
    }

    uint8_t *pflag = data + 4;
    uint16_t flag;
    size_t len = XIM_MESSAGE_BYTES(hdr);
    uint16_t_read(&flag, &pflag, &len, false);

    if (flag & XCB_XIM_LOOKUP_KEYSYM) {
        xcb_im_commit_both_fr_t frame;
        bool fail;
        _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
        if (fail) {
            return;
        }

        do {
            if (im->connect_id != frame.input_method_ID) {
                break;
            }

            if (im->im_callback.commit_string) {
                im->im_callback.commit_string(
                    im, frame.input_context_ID, frame.flag,
                    (char *)frame.committed_string,
                    frame.byte_length_of_committed_string, &frame.keysym, 1,
                    im->user_data);
            }

            if (flag & XCB_XIM_SYNCHRONOUS) {
                _xcb_xim_sync(im, frame.input_context_ID);
            }
        } while (0);
        xcb_im_commit_both_fr_free(&frame);
    } else if ((flag & XCB_XIM_LOOKUP_BOTH) == XCB_XIM_LOOKUP_CHARS) {
        xcb_im_commit_chars_fr_t frame;
        bool fail;
        _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
        if (fail) {
            return;
        }

        do {
            if (im->connect_id != frame.input_method_ID) {
                break;
            }

            if (im->im_callback.commit_string) {
                im->im_callback.commit_string(
                    im, frame.input_context_ID, frame.flag,
                    (char *)frame.committed_string,
                    frame.byte_length_of_committed_string, NULL, 0,
                    im->user_data);
            }

            if (flag & XCB_XIM_SYNCHRONOUS) {
                _xcb_xim_sync(im, frame.input_context_ID);
            }
        } while (0);
        xcb_im_commit_chars_fr_free(&frame);
    }
}

void _xcb_xim_handle_close_reply(xcb_xim_t *im,
                                 const xcb_im_packet_header_fr_t *hdr,
                                 uint8_t *data) {
    // why we need to wait server :?
}

void _xcb_xim_handle_geometry(xcb_xim_t *im,
                              const xcb_im_packet_header_fr_t *hdr,
                              uint8_t *data) {
    xcb_im_geometry_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (im->connect_id != frame.input_method_ID) {
            break;
        }

        if (im->im_callback.geometry) {
            im->im_callback.geometry(im, frame.input_context_ID, im->user_data);
        }
    } while (0);
    xcb_im_geometry_fr_free(&frame);
}

void _xcb_xim_handle_preedit_start(xcb_xim_t *im,
                                   const xcb_im_packet_header_fr_t *hdr,
                                   uint8_t *data) {
    xcb_im_preedit_start_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (im->connect_id != frame.input_method_ID) {
            break;
        }

        if (im->im_callback.preedit_start) {
            im->im_callback.preedit_start(im, frame.input_context_ID,
                                          im->user_data);
        }
    } while (0);
    xcb_im_preedit_start_fr_free(&frame);
}

void _xcb_xim_handle_preedit_draw(xcb_xim_t *im,
                                  const xcb_im_packet_header_fr_t *hdr,
                                  uint8_t *data) {
    xcb_im_preedit_draw_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (im->connect_id != frame.input_method_ID) {
            break;
        }

        if (im->im_callback.preedit_draw) {
            im->im_callback.preedit_draw(im, frame.input_context_ID, &frame,
                                         im->user_data);
        }
    } while (0);
    xcb_im_preedit_draw_fr_free(&frame);
}

void _xcb_xim_handle_preedit_caret(xcb_xim_t *im,
                                   const xcb_im_packet_header_fr_t *hdr,
                                   uint8_t *data) {
    xcb_im_preedit_caret_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (im->connect_id != frame.input_method_ID) {
            break;
        }

        if (im->im_callback.preedit_caret) {
            im->im_callback.preedit_caret(im, frame.input_context_ID, &frame,
                                          im->user_data);
        }
    } while (0);
    xcb_im_preedit_caret_fr_free(&frame);
}

void _xcb_xim_handle_preedit_done(xcb_xim_t *im,
                                  const xcb_im_packet_header_fr_t *hdr,
                                  uint8_t *data) {
    xcb_im_preedit_done_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (im->connect_id != frame.input_method_ID) {
            break;
        }

        if (im->im_callback.preedit_done) {
            im->im_callback.preedit_done(im, frame.input_context_ID,
                                         im->user_data);
        }
    } while (0);
    xcb_im_preedit_done_fr_free(&frame);
}

void _xcb_xim_handle_status_start(xcb_xim_t *im,
                                  const xcb_im_packet_header_fr_t *hdr,
                                  uint8_t *data) {
    xcb_im_status_start_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (im->connect_id != frame.input_method_ID) {
            break;
        }

        if (im->im_callback.status_start) {
            im->im_callback.status_start(im, frame.input_context_ID,
                                         im->user_data);
        }
    } while (0);
    xcb_im_status_start_fr_free(&frame);
}

void _xcb_xim_handle_status_draw(xcb_xim_t *im,
                                 const xcb_im_packet_header_fr_t *hdr,
                                 uint8_t *data) {
    if (XIM_MESSAGE_BYTES(hdr) < 8) {
        return;
    }

    uint8_t *ptype = data + 4;
    uint32_t type;
    size_t len = XIM_MESSAGE_BYTES(hdr);
    uint32_t_read(&type, &ptype, &len, false);

    if (type == XCB_IM_TextType) {
        xcb_im_status_draw_text_fr_t frame;
        bool fail;
        _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
        if (fail) {
            return;
        }

        do {
            if (im->connect_id != frame.input_method_ID) {
                break;
            }

            if (im->im_callback.status_draw_text) {
                im->im_callback.status_draw_text(im, frame.input_context_ID,
                                                 &frame, im->user_data);
            }
        } while (0);
        xcb_im_status_draw_text_fr_free(&frame);
    } else if (type == XCB_IM_BitmapType) {
        xcb_im_status_draw_bitmap_fr_t frame;
        bool fail;
        _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
        if (fail) {
            return;
        }

        do {
            if (im->connect_id != frame.input_method_ID) {
                break;
            }

            if (im->im_callback.status_draw_bitmap) {
                im->im_callback.status_draw_bitmap(im, frame.input_context_ID,
                                                   &frame, im->user_data);
            }
        } while (0);
        xcb_im_status_draw_bitmap_fr_free(&frame);
    }
}

void _xcb_xim_handle_status_done(xcb_xim_t *im,
                                 const xcb_im_packet_header_fr_t *hdr,
                                 uint8_t *data) {
    xcb_im_status_done_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (im->connect_id != frame.input_method_ID) {
            break;
        }

        if (im->im_callback.status_done) {
            im->im_callback.status_done(im, frame.input_context_ID,
                                        im->user_data);
        }
    } while (0);
    xcb_im_status_done_fr_free(&frame);
}

void _xcb_xim_handle_set_event_mask(xcb_xim_t *im,
                                    const xcb_im_packet_header_fr_t *hdr,
                                    uint8_t *data) {
    xcb_im_set_event_mask_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (im->connect_id != frame.input_method_ID) {
            break;
        }

        if (im->im_callback.set_event_mask) {
            im->im_callback.set_event_mask(
                im, frame.input_context_ID, frame.forward_event_mask,
                frame.synchronous_event_mask, im->user_data);
        }
    } while (0);
    xcb_im_set_event_mask_fr_free(&frame);
}

void _xcb_xim_handle_sync(xcb_xim_t *im, const xcb_im_packet_header_fr_t *hdr,
                          uint8_t *data) {
    xcb_im_sync_fr_t frame;
    bool fail;
    _xcb_xim_read_frame(frame, data, XIM_MESSAGE_BYTES(hdr), fail);
    if (fail) {
        return;
    }

    do {
        if (im->connect_id != frame.input_method_ID) {
            break;
        }

        _xcb_xim_sync(im, frame.input_context_ID);
    } while (0);
    xcb_im_sync_fr_free(&frame);
}

void _xcb_xim_handle_error(xcb_xim_t *im, uint8_t *data) {
    if (im->open_state == XIM_OPEN_DONE) {
        if (im->current) {
            _xcb_xim_process_fail_callback(im, im->current);
            _xcb_xim_request_free(im->current);
            im->current = NULL;
        }
    } else {
        if (im->open_state != XIM_OPEN_INVALID) {
            im->open_state = XIM_OPEN_INVALID;
            im->yield_recheck = true;
        }
    }
}
