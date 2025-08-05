/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#include "protocolhandler.h"
#include "common.h"
#include "imdkit_p.h"
#include "parser.h"
#include "uthash.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <xcb/xproto.h>

void _xcb_im_handle_connect(xcb_im_t *im, xcb_im_client_t *client,
                            const xcb_im_packet_header_fr_t *hdr,
                            uint8_t *data) {
    xcb_im_connect_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    if (im->callback) {
        im->callback(im, client, NULL, hdr, &frame, NULL, im->user_data);
    }
    xcb_im_connect_fr_free(&frame);

    xcb_im_connect_reply_fr_t reply_frame;
    reply_frame.server_major_protocol_version =
        frame.client_major_protocol_version;
    reply_frame.server_minor_protocol_version =
        frame.client_minor_protocol_version;

    _xcb_im_send_frame(im, client, reply_frame, true);
}

static void _xcb_im_send_trigger_key(xcb_im_t *im, xcb_im_client_t *client) {
    xcb_im_register_triggerkeys_fr_t frame;
    /* Right now XIM_OPEN_REPLY hasn't been sent to this new client, so
       the input-method-id is still invalid, and should be set to zero...
       Reter to $(XC)/lib/X11/imDefLkup.c:_XimRegisterTriggerKeysCallback
     */
    frame.input_method_ID = 0;
    frame.on_keys_list.size = im->onKeys.nKeys;
    frame.on_keys_list.items = im->onKeys.keys;
    frame.off_keys_list.size = im->offKeys.nKeys;
    frame.off_keys_list.items = im->offKeys.keys;

    _xcb_im_send_frame(im, client, frame, false);
}

void _xcb_im_handle_open(xcb_im_t *im, xcb_im_client_t *client,
                         const xcb_im_packet_header_fr_t *hdr, uint8_t *data) {
    xcb_im_open_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    if (im->callback) {
        im->callback(im, client, NULL, hdr, &frame, NULL, im->user_data);
    }

    xcb_im_open_fr_free(&frame);

    xcb_im_open_reply_fr_t reply_frame;
    reply_frame.input_method_ID = client->connect_id;
    reply_frame.IM_attribute_supported.size = ARRAY_SIZE(im->imattr);
    reply_frame.IC_attribute_supported.size = ARRAY_SIZE(im->icattr);
    reply_frame.IM_attribute_supported.items = im->imattr;
    reply_frame.IC_attribute_supported.items = im->icattr;

    _xcb_im_send_frame(im, client, reply_frame, true);
    if (_xcb_im_has_trigger_key(im)) {
        _xcb_im_send_trigger_key(im, client);
    } else {
        _xcb_im_set_im_event_mask(im, client);
    }
}

void _xcb_im_handle_close(xcb_im_t *im, xcb_im_client_t *client,
                          const xcb_im_packet_header_fr_t *hdr, uint8_t *data) {
    xcb_im_close_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    if (im->callback) {
        im->callback(im, client, NULL, hdr, &frame, NULL, im->user_data);
    }

    xcb_im_close_fr_free(&frame);
    xcb_im_close_reply_fr_t reply_frame;
    reply_frame.input_method_ID = frame.input_method_ID;
    _xcb_im_send_frame(im, client, reply_frame, true);
}

void _xcb_im_handle_query_extension(xcb_im_t *im, xcb_im_client_t *client,
                                    const xcb_im_packet_header_fr_t *hdr,
                                    uint8_t *data) {
    xcb_im_query_extension_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    int nExts = 0;
    xcb_im_ext_fr_t ext_list[ARRAY_SIZE(Default_Extension)];
    for (size_t i = 0; i < frame.extensions_supported_by_the_IM_library.size;
         i++) {
        for (size_t j = 0; j < ARRAY_SIZE(Default_Extension); j++) {
            if (frame.extensions_supported_by_the_IM_library.items[i]
                        .length_of_string ==
                    im->extension[j].length_of_extension_name &&
                strncmp((char *)frame.extensions_supported_by_the_IM_library
                            .items[i]
                            .string,
                        (char *)im->extension[j].extension_name,
                        frame.extensions_supported_by_the_IM_library.items[i]
                            .length_of_string) == 0) {
                ext_list[nExts] = im->extension[j];
                nExts++;
                break;
            }
        }
    }

    xcb_im_query_extension_fr_free(&frame);
    xcb_im_query_extension_reply_fr_t reply_frame;
    reply_frame.input_method_ID = client->connect_id;
    reply_frame.list_of_extensions_supported_by_th.items = ext_list;
    reply_frame.list_of_extensions_supported_by_th.size = nExts;

    _xcb_im_send_frame(im, client, reply_frame, true);
}

void _xcb_im_handle_encoding_negotiation(xcb_im_t *im, xcb_im_client_t *client,
                                         const xcb_im_packet_header_fr_t *hdr,
                                         uint8_t *data) {
    xcb_im_encoding_negotiation_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    size_t i, j;
    for (i = 0; i < frame.supported_list_of_encoding_in_IM_library.size; i++) {
        for (j = 0; j < im->encodings.nEncodings; j++) {
            if (frame.supported_list_of_encoding_in_IM_library.items[i]
                        .length_of_string ==
                    strlen(im->encodings.encodings[j]) &&
                strncmp((char *)frame.supported_list_of_encoding_in_IM_library
                            .items[i]
                            .string,
                        im->encodings.encodings[j],
                        frame.supported_list_of_encoding_in_IM_library.items[i]
                            .length_of_string) == 0) {
                break;
            }
        }
        if (j != im->encodings.nEncodings) {
            break;
        }
    }

    // no match then we use 0.
    if (i == frame.supported_list_of_encoding_in_IM_library.size) {
        i = 0;
        j = 0;
    }

    xcb_im_encoding_negotiation_fr_free(&frame);
    xcb_im_encoding_negotiation_reply_fr_t reply_frame;
    reply_frame.input_method_ID = client->connect_id;
    reply_frame.index_of_the_encoding_determined = i;
    reply_frame.category_of_the_encoding_determined = 0;

    uint16_t index = j;

    if (im->callback) {
        im->callback(im, client, NULL, hdr, &frame, &index, im->user_data);
    }

    _xcb_im_send_frame(im, client, reply_frame, true);
}

void _xcb_im_handle_get_im_values(xcb_im_t *im, xcb_im_client_t *client,
                                  const xcb_im_packet_header_fr_t *hdr,
                                  uint8_t *data) {
    xcb_im_get_im_values_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    xcb_im_get_im_values_reply_fr_t reply_frame;
    size_t nBuffers = 0;
    xcb_im_ximattribute_fr_t buffers[ARRAY_SIZE(Default_IMattr)];
    bool met[ARRAY_SIZE(Default_IMattr)];
    memset(met, 0, sizeof(met));
    for (size_t i = 0; i < frame.im_attribute_id.size; i++) {
        if (frame.im_attribute_id.items[i] >= ARRAY_SIZE(im->id2attr)) {
            continue;
        }
        xcb_im_ximattr_fr_t *attr = im->id2attr[frame.im_attribute_id.items[i]];
        if ((attr < im->imattr) ||
            (attr >= im->imattr + ARRAY_SIZE(Default_IMattr))) {
            continue;
        }

        if (met[attr - im->imattr]) {
            continue;
        }

        if (!Default_IMattr[attr - im->imattr].get_value) {
            continue;
        }

        if (!Default_IMattr[attr - im->imattr].get_value(im, client,
                                                         &buffers[nBuffers])) {
            continue;
        }
        buffers[nBuffers].attribute_ID = frame.im_attribute_id.items[i];
        met[attr - im->imattr] = true;
        nBuffers++;
    }

    reply_frame.input_method_ID = client->connect_id;
    reply_frame.im_attribute_returned.items = buffers;
    reply_frame.im_attribute_returned.size = nBuffers;

    xcb_im_get_im_values_fr_free(&frame);
    _xcb_im_send_frame(im, client, reply_frame, true);

    for (size_t i = 0; i < nBuffers; i++) {
        free(buffers[i].value);
    }
}

void _xcb_im_handle_disconnect(xcb_im_t *im, xcb_im_client_t *client,
                               const xcb_im_packet_header_fr_t *hdr,
                               uint8_t *data) {
    _xcb_im_destroy_client(im, client);

    xcb_im_disconnect_reply_fr_t reply_frame;
    _xcb_im_send_frame(im, client, reply_frame, true);
}

void _xcb_im_parse_ic_value(xcb_im_t *im, xcb_im_input_context_t *ic, void *p,
                            const xcb_im_default_ic_attr_t *icattr,
                            uint8_t *data, size_t length) {
    switch (icattr->type) {
    case XimType_CARD32:
    case XimType_Window: {
        uint32_t *result = p;
        uint32_t_read(result, &data, &length,
                      im->byte_order != ic->client->byte_order);
        break;
    }
    case XimType_XRectangle: {
        xcb_rectangle_t *result = p;
        xcb_im_xrectangle_fr_t fr;
        _xcb_im_read_frame(im, ic->client, fr, data, length);
        result->x = fr.x;
        result->y = fr.y;
        result->height = fr.height;
        result->width = fr.width;
        break;
    }
    case XimType_XPoint: {
        xcb_point_t *result = p;
        xcb_im_xpoint_fr_t fr;
        _xcb_im_read_frame(im, ic->client, fr, data, length);
        result->x = fr.x;
        result->y = fr.y;
        break;
    }
    case XimType_XFontSet: {
        break;
    }
    }
}

void _xcb_im_parse_nested_ic_values(xcb_im_t *im, xcb_im_input_context_t *ic,
                                    void *p, uint32_t *mask, ssize_t *offsets,
                                    uint32_t *masks, uint8_t *data,
                                    size_t length) {
    while (length != 0) {
        xcb_im_xicattribute_fr_t fr;
        xcb_im_xicattribute_fr_read(&fr, &data, &length,
                                    im->byte_order != ic->client->byte_order);
        if (!data) {
            return;
        }

        if (fr.attribute_ID >= ARRAY_SIZE(im->id2attr)) {
            continue;
        }

        const xcb_im_default_ic_attr_t *entry =
            _xcb_im_default_ic_attr_entry(im, fr.attribute_ID);
        if (!entry || offsets[fr.attribute_ID] < 0) {
            continue;
        }

        *mask |= masks[fr.attribute_ID];
        _xcb_im_parse_ic_value(im, ic,
                               (((uint8_t *)p) + offsets[fr.attribute_ID]),
                               entry, fr.value, fr.value_length);
    }
}

void _xcb_im_parse_ic_values(xcb_im_t *im, xcb_im_input_context_t *ic,
                             uint32_t nAttr,
                             xcb_im_xicattribute_fr_t *icattrs) {
    for (uint32_t i = 0; i < nAttr; i++) {
        if (icattrs[i].attribute_ID == im->separatorAttr_id) {
            continue;
        }

        if (icattrs[i].attribute_ID == im->statusAttr_id) {
            _xcb_im_parse_nested_ic_values(
                im, ic, &ic->status, &ic->status_mask, im->id2statusoffset,
                im->id2statusmask, icattrs[i].value, icattrs[i].value_length);
        } else if (icattrs[i].attribute_ID == im->preeditAttr_id) {
            _xcb_im_parse_nested_ic_values(
                im, ic, &ic->preedit, &ic->preedit_mask, im->id2preeditoffset,
                im->id2preeditmask, icattrs[i].value, icattrs[i].value_length);
        } else {
            const xcb_im_default_ic_attr_t *entry =
                _xcb_im_default_ic_attr_entry(im, icattrs[i].attribute_ID);
            if (!entry || im->id2icoffset[icattrs[i].attribute_ID] < 0) {
                continue;
            }
            _xcb_im_parse_ic_value(
                im, ic,
                (((uint8_t *)ic) + im->id2icoffset[icattrs[i].attribute_ID]),
                entry, icattrs[i].value, icattrs[i].value_length);
        }
    }
}

void _xcb_im_handle_create_ic(xcb_im_t *im, xcb_im_client_t *client,
                              const xcb_im_packet_header_fr_t *hdr,
                              uint8_t *data) {
    xcb_im_create_ic_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (frame.input_method_ID != client->connect_id) {
            break;
        }

        xcb_im_input_context_t *ic = _xcb_im_new_input_context(im, client);
        if (!ic) {
            break;
        }

        _xcb_im_parse_ic_values(im, ic, frame.ic_attributes.size,
                                frame.ic_attributes.items);

        xcb_im_create_ic_reply_fr_t reply_frame;
        reply_frame.input_method_ID = client->connect_id;
        reply_frame.input_context_ID = ic->id;

        if (im->callback) {
            im->callback(im, client, ic, hdr, &frame, &reply_frame,
                         im->user_data);
        }

        _xcb_im_send_frame(im, client, reply_frame, true);
        if (!_xcb_im_has_trigger_key(im)) {
            _xcb_im_set_ic_event_mask(im, ic);
        }
        xcb_im_create_ic_fr_free(&frame);
        return;
    } while (0);
    // error
    xcb_im_create_ic_fr_free(&frame);
    _xcb_im_send_error_message(im, client);
}

void _xcb_im_handle_destroy_ic(xcb_im_t *im, xcb_im_client_t *client,
                               const xcb_im_packet_header_fr_t *hdr,
                               uint8_t *data) {
    xcb_im_destroy_ic_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (frame.input_method_ID != client->connect_id) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        xcb_im_destroy_ic_reply_fr_t reply_frame;
        reply_frame.input_method_ID = client->connect_id;
        reply_frame.input_context_ID = frame.input_context_ID;
        xcb_im_destroy_ic_fr_free(&frame);

        _xcb_im_destroy_ic(im, ic);

        _xcb_im_send_frame(im, client, reply_frame, true);
        return;
    } while (0);
    // error
    xcb_im_destroy_ic_fr_free(&frame);
    _xcb_im_send_error_message(im, client);
}

void _xcb_im_handle_set_ic_values(xcb_im_t *im, xcb_im_client_t *client,
                                  const xcb_im_packet_header_fr_t *hdr,
                                  uint8_t *data) {
    xcb_im_set_ic_values_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (frame.input_method_ID != client->connect_id) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        _xcb_im_parse_ic_values(im, ic, frame.ic_attribute.size,
                                frame.ic_attribute.items);

        if (im->callback) {
            im->callback(im, client, ic, hdr, &frame, NULL, im->user_data);
        }
        xcb_im_set_ic_values_fr_free(&frame);

        xcb_im_set_ic_values_reply_fr_t reply_frame;
        reply_frame.input_method_ID = client->connect_id;
        reply_frame.input_context_ID = frame.input_context_ID;
        _xcb_im_send_frame(im, client, reply_frame, true);
        return;
    } while (0);
    // error
    xcb_im_set_ic_values_fr_free(&frame);
    _xcb_im_send_error_message(im, client);
}

uint32_t _xcb_im_get_nested_ic_values(xcb_im_t *im, xcb_im_input_context_t *ic,
                                      void *p, ssize_t *offsets,
                                      uint16_t *attrIDs, uint32_t i,
                                      uint32_t size,
                                      xcb_im_xicattribute_fr_t *attr) {
    size_t totalSize = 0;
    uint8_t *data = NULL;
    uint32_t start = i;
    for (int round = 0; round < 2; round++) {
        i = start;
        while (i < size && attrIDs[i] != im->separatorAttr_id) {
            uint16_t attrID = attrIDs[i];
            i++;
            const xcb_im_default_ic_attr_t *entry =
                _xcb_im_default_ic_attr_entry(im, attrID);
            if (!entry || !entry->read || offsets[attrID] < 0) {
                continue;
            }

            // calculate size
            if (round == 0) {
                xcb_im_xicattribute_fr_t fr;
                fr.value_length = _xcb_im_ic_attr_size(entry->type);
                totalSize += xcb_im_xicattribute_fr_size(&fr);
            } else {
                uint8_t *start = data;
                uint16_t value_length = _xcb_im_ic_attr_size(entry->type);
                bool swap = im->byte_order != ic->client->byte_order;
                data = uint16_t_write(&attrID, data, swap);
                data = uint16_t_write(&value_length, data, swap);
                data = _xcb_im_get_ic_value((((uint8_t *)p) + offsets[attrID]),
                                            entry->type, data, swap);
                data =
                    (uint8_t *)align_to_4((uintptr_t)data, data - start, NULL);
            }
        }
        if (round == 0) {
            data = malloc(totalSize);
            if (!data) {
                break;
            }
            attr->value_length = totalSize;
            attr->value = data;
        }
    }

    return i;
}

void _xcb_im_handle_get_ic_values(xcb_im_t *im, xcb_im_client_t *client,
                                  const xcb_im_packet_header_fr_t *hdr,
                                  uint8_t *data) {
    xcb_im_get_ic_values_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (frame.input_method_ID != client->connect_id) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        if (im->callback) {
            im->callback(im, client, ic, hdr, &frame, NULL, im->user_data);
        }

        xcb_im_get_ic_values_reply_fr_t reply_frame;
        reply_frame.input_method_ID = client->connect_id;
        reply_frame.input_context_ID = frame.input_context_ID;
        reply_frame.ic_attribute.size = 0;
        uint32_t nBuffers = 0;
        xcb_im_xicattribute_fr_t buffers[ARRAY_SIZE(Default_ICattr)];
        memset(buffers, 0, sizeof(buffers));
        uint32_t i = 0;
        while (i < frame.ic_attribute.size) {
            buffers[nBuffers].attribute_ID = frame.ic_attribute.items[i];

            if (frame.ic_attribute.items[i] == im->statusAttr_id) {
                i = _xcb_im_get_nested_ic_values(
                    im, ic, &ic->status, im->id2statusoffset,
                    frame.ic_attribute.items, i + 1, frame.ic_attribute.size,
                    &buffers[nBuffers]);
            } else if (frame.ic_attribute.items[i] == im->preeditAttr_id) {
                i = _xcb_im_get_nested_ic_values(
                    im, ic, &ic->preedit, im->id2preeditoffset,
                    frame.ic_attribute.items, i + 1, frame.ic_attribute.size,
                    &buffers[nBuffers]);
            } else {
                const xcb_im_default_ic_attr_t *entry =
                    _xcb_im_default_ic_attr_entry(im,
                                                  frame.ic_attribute.items[i]);
                i++;
                if (!entry) {
                    continue;
                }

                if (strcmp(entry->name, XCB_XIM_XNFilterEvents) == 0) {
                    uint32_t result =
                        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE;
                    buffers[nBuffers].value = malloc(sizeof(uint32_t));
                    buffers[nBuffers].value_length = sizeof(uint32_t);
                    if (data) {
                        uint32_t_write(&result, buffers[nBuffers].value,
                                       im->byte_order !=
                                           ic->client->byte_order);
                    }
                }
            }

            if (buffers[nBuffers].value) {
                nBuffers++;
            }
        }

        reply_frame.ic_attribute.size = nBuffers;
        reply_frame.ic_attribute.items = buffers;
        xcb_im_get_ic_values_fr_free(&frame);
        _xcb_im_send_frame(im, client, reply_frame, true);
        for (uint32_t i = 0; i < nBuffers; i++) {
            free(buffers[i].value);
        }
        return;
    } while (0);
    // error
    xcb_im_get_ic_values_fr_free(&frame);
    _xcb_im_send_error_message(im, client);
}

void _xcb_im_handle_set_ic_focus(xcb_im_t *im, xcb_im_client_t *client,
                                 const xcb_im_packet_header_fr_t *hdr,
                                 uint8_t *data) {
    xcb_im_set_ic_focus_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (client->connect_id != frame.input_method_ID) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        if (!_xcb_im_has_trigger_key(im)) {
            _xcb_im_set_ic_event_mask(im, ic);
        }
        if (im->callback) {
            im->callback(im, client, ic, hdr, &frame, NULL, im->user_data);
        }
    } while (0);

    xcb_im_set_ic_focus_fr_free(&frame);
}

void _xcb_im_handle_unset_ic_focus(xcb_im_t *im, xcb_im_client_t *client,
                                   const xcb_im_packet_header_fr_t *hdr,
                                   uint8_t *data) {
    xcb_im_unset_ic_focus_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (client->connect_id != frame.input_method_ID) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        if (im->callback) {
            im->callback(im, client, ic, hdr, &frame, NULL, im->user_data);
        }
    } while (0);

    xcb_im_unset_ic_focus_fr_free(&frame);
}

void _xcb_im_handle_preedit_caret_reply(xcb_im_t *im, xcb_im_client_t *client,
                                        const xcb_im_packet_header_fr_t *hdr,
                                        uint8_t *data) {
    xcb_im_preedit_caret_reply_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (client->connect_id != frame.input_method_ID) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        if (im->callback) {
            im->callback(im, client, ic, hdr, &frame, NULL, im->user_data);
        }
    } while (0);

    xcb_im_preedit_caret_reply_fr_free(&frame);
}

void _xcb_im_handle_reset_ic(xcb_im_t *im, xcb_im_client_t *client,
                             const xcb_im_packet_header_fr_t *hdr,
                             uint8_t *data) {
    xcb_im_reset_ic_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (client->connect_id != frame.input_method_ID) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        xcb_im_reset_ic_reply_fr_t reply_frame;
        reply_frame.input_method_ID = frame.input_method_ID;
        reply_frame.input_context_ID = frame.input_context_ID;
        reply_frame.committed_string = NULL;
        reply_frame.byte_length_of_committed_string = 0;

        if (im->callback) {
            im->callback(im, client, ic, hdr, &frame, &reply_frame,
                         im->user_data);
        }

        _xcb_im_send_frame(im, client, reply_frame, true);
        free(reply_frame.committed_string);
    } while (0);

    xcb_im_reset_ic_fr_free(&frame);
}

void _xcb_im_handle_forward_event(xcb_im_t *im, xcb_im_client_t *client,
                                  const xcb_im_packet_header_fr_t *hdr,
                                  uint8_t *data) {
    xcb_im_forward_event_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (client->connect_id != frame.input_method_ID) {
            break;
        }

        if (XIM_MESSAGE_BYTES(hdr) < xcb_im_forward_event_fr_size(&frame) +
                                         sizeof(xcb_key_press_event_t)) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }
        ic->forward_event_sequence = frame.sequence_number;
        if (client->sync) {
            _xcb_im_add_queue(im, client, ic->id, hdr, &frame, data);
        } else {
            xcb_key_press_event_t key_event;
            memcpy(&key_event, data, sizeof(xcb_key_press_event_t));

            if (im->callback) {
                im->callback(im, client, ic, hdr, &frame, &key_event,
                             im->user_data);
            }

            if (im->use_sync_event) {
                xcb_im_sync_reply_fr_t reply_frame;
                reply_frame.input_method_ID = frame.input_method_ID;
                reply_frame.input_context_ID = frame.input_context_ID;

                _xcb_im_send_frame(im, client, reply_frame, true);
            }
        }
    } while (0);

    xcb_im_forward_event_fr_free(&frame);
}

void _xcb_im_handle_ext_forward_keyevent(xcb_im_t *im, xcb_im_client_t *client,
                                         const xcb_im_packet_header_fr_t *hdr,
                                         uint8_t *data) {
    xcb_im_ext_forward_keyevent_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (client->connect_id != frame.input_method_ID) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        xcb_key_press_event_t key_event;
        memset(&key_event, 0, sizeof(key_event));
        key_event.response_type = frame.xEvent_u_u_type;
        key_event.sequence = frame.sequence_number;
        key_event.root = im->default_screen->root;
        key_event.time = frame.time;
        key_event.detail = frame.keycode;
        key_event.state = frame.state;
        key_event.event = frame.window;

        if (im->callback) {
            im->callback(im, client, ic, hdr, &frame, &key_event,
                         im->user_data);
        }
    } while (0);

    xcb_im_ext_forward_keyevent_fr_free(&frame);
}

void _xcb_im_handle_ext_move(xcb_im_t *im, xcb_im_client_t *client,
                             const xcb_im_packet_header_fr_t *hdr,
                             uint8_t *data) {
    xcb_im_ext_move_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (client->connect_id != frame.input_method_ID) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        ic->preedit.spot_location.x = frame.X;
        ic->preedit.spot_location.y = frame.Y;
    } while (0);

    xcb_im_ext_move_fr_free(&frame);
}

void _xcb_im_handle_extension(xcb_im_t *im, xcb_im_client_t *client,
                              const xcb_im_packet_header_fr_t *hdr,
                              uint8_t *data) {
    switch (hdr->minor_opcode) {
    case XCB_XIM_EXT_FORWARD_KEYEVENT:
        _xcb_im_handle_ext_forward_keyevent(im, client, hdr, data);
        break;
    case XCB_XIM_EXT_MOVE:
        _xcb_im_handle_ext_move(im, client, hdr, data);
        break;
    }
}

void _xcb_im_handle_sync_reply(xcb_im_t *im, xcb_im_client_t *client,
                               const xcb_im_packet_header_fr_t *hdr,
                               uint8_t *data) {
    xcb_im_sync_reply_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (client->connect_id != frame.input_method_ID) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        client->sync = false;
        if (im->sync) {
            im->sync = false;

            if (im->callback) {
                im->callback(im, client, ic, hdr, &frame, NULL, im->user_data);
            }
        }

        _xcb_im_process_queue(im, client);
    } while (0);

    xcb_im_sync_reply_fr_free(&frame);
}

void _xcb_im_handle_trigger_notify(xcb_im_t *im, xcb_im_client_t *client,
                                   const xcb_im_packet_header_fr_t *hdr,
                                   uint8_t *data) {
    xcb_im_trigger_notify_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (client->connect_id != frame.input_method_ID) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        xcb_im_trigger_notify_reply_fr_t reply_frame;
        reply_frame.input_method_ID = frame.input_method_ID;
        reply_frame.input_context_ID = frame.input_context_ID;

        /* NOTE:
         * XIM_TRIGGER_NOTIFY_REPLY should be sent before XIM_SET_EVENT_MASK
         * in case of XIM_TRIGGER_NOTIFY(flag == ON), while it should be
         * sent after XIM_SET_EVENT_MASK in case of
         * XIM_TRIGGER_NOTIFY(flag == OFF).
         */
        if (frame.flag == 0) {
            _xcb_im_send_frame(im, client, reply_frame, false);
            xcb_im_preedit_start(im, ic);
        }

        if (im->callback) {
            im->callback(im, client, ic, hdr, &frame, NULL, im->user_data);
        }

        if (frame.flag == 1) {
            xcb_im_preedit_end(im, ic);
            _xcb_im_send_frame(im, client, reply_frame, false);
        }

    } while (0);

    xcb_im_trigger_notify_fr_free(&frame);
}

void _xcb_im_handle_preedit_start_reply(xcb_im_t *im, xcb_im_client_t *client,
                                        const xcb_im_packet_header_fr_t *hdr,
                                        uint8_t *data) {
    xcb_im_preedit_start_reply_fr_t frame;
    _xcb_im_read_frame_with_error(im, client, frame, data,
                                  XIM_MESSAGE_BYTES(hdr));

    do {
        if (client->connect_id != frame.input_method_ID) {
            break;
        }

        xcb_im_input_context_t *ic = NULL;
        HASH_FIND(hh, client->input_contexts, &frame.input_context_ID,
                  sizeof(uint16_t), ic);
        if (!ic) {
            break;
        }

        if (im->callback) {
            im->callback(im, client, ic, hdr, &frame, NULL, im->user_data);
        }
    } while (0);

    xcb_im_preedit_start_reply_fr_free(&frame);
}
