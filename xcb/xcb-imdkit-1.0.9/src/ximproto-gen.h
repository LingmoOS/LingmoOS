/*
 * SPDX-FileCopyrightText: 2014-2020 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#ifndef XIMPROTO_GEN_H
#define XIMPROTO_GEN_H

#include "ximcommon.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

XCBIMDKIT_DECL_BEGIN

typedef uint8_t *xcb_im_bytearray_t;

typedef struct _xcb_im_ximattr_fr_t
{
    uint16_t attribute_ID;
    uint16_t type_of_the_value;
    uint16_t length_of_im_attribute;
    xcb_im_bytearray_t im_attribute;
} xcb_im_ximattr_fr_t;

typedef struct _xcb_im_xicattr_fr_t
{
    uint16_t attribute_ID;
    uint16_t type_of_the_value;
    uint16_t length_of_ic_attribute;
    xcb_im_bytearray_t ic_attribute;
} xcb_im_xicattr_fr_t;

typedef struct _xcb_im_ximattribute_fr_t
{
    uint16_t attribute_ID;
    uint16_t value_length;
    xcb_im_bytearray_t value;
} xcb_im_ximattribute_fr_t;

typedef struct _xcb_im_xicattribute_fr_t
{
    uint16_t attribute_ID;
    uint16_t value_length;
    xcb_im_bytearray_t value;
} xcb_im_xicattribute_fr_t;

typedef struct _xcb_im_ximtriggerkey_fr_t
{
    uint32_t keysym;
    uint32_t modifier;
    uint32_t modifier_mask;
} xcb_im_ximtriggerkey_fr_t;

typedef struct _xcb_im_encodinginfo_fr_t
{
    uint16_t length_of_encoding_info;
    xcb_im_bytearray_t encoding_info;
} xcb_im_encodinginfo_fr_t;

typedef struct _xcb_im_str_fr_t
{
    uint8_t length_of_string;
    xcb_im_bytearray_t string;
} xcb_im_str_fr_t;

typedef struct _xcb_im_xpcs_fr_t
{
    uint16_t length_of_string_in_bytes;
    xcb_im_bytearray_t string;
} xcb_im_xpcs_fr_t;

typedef struct _xcb_im_ext_fr_t
{
    uint8_t extension_major_opcode;
    uint8_t extension_minor_opcode;
    uint16_t length_of_extension_name;
    xcb_im_bytearray_t extension_name;
} xcb_im_ext_fr_t;

typedef struct _xcb_im_inputstyle_fr_t
{
    uint32_t inputstyle;
} xcb_im_inputstyle_fr_t;

typedef struct _xcb_im_attr_head_fr_t
{
    uint16_t attribute_id;
    uint16_t attribute_length;
} xcb_im_attr_head_fr_t;

typedef struct _xcb_im_short_fr_t
{
    uint16_t value;
} xcb_im_short_fr_t;

typedef struct _xcb_im_long_fr_t
{
    uint32_t value;
} xcb_im_long_fr_t;

typedef struct _xcb_im_xrectangle_fr_t
{
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} xcb_im_xrectangle_fr_t;

typedef struct _xcb_im_xpoint_fr_t
{
    uint16_t x;
    uint16_t y;
} xcb_im_xpoint_fr_t;

typedef struct _xcb_im_fontset_fr_t
{
    uint16_t length_of_base_font_name;
    xcb_im_bytearray_t base_font_name_list;
} xcb_im_fontset_fr_t;

typedef struct _xcb_im_input_styles_fr_t
{
    struct {
        uint32_t size;
        xcb_im_inputstyle_fr_t* items;
    } XIMStyle_list;
} xcb_im_input_styles_fr_t;

typedef struct _xcb_im_packet_header_fr_t
{
    uint8_t major_opcode;
    uint8_t minor_opcode;
    uint16_t length;
} xcb_im_packet_header_fr_t;

typedef struct _xcb_im_error_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint16_t flag;
    uint16_t Error_Code;
    uint16_t length_of_error_detail;
    uint16_t type_of_error_detail;
    xcb_im_bytearray_t error_detail;
} xcb_im_error_fr_t;

typedef struct _xcb_im_connect_fr_t
{
    uint8_t byte_order;
    uint16_t client_major_protocol_version;
    uint16_t client_minor_protocol_version;
    struct {
        uint32_t size;
        xcb_im_xpcs_fr_t* items;
    } client_auth_protocol_names;
} xcb_im_connect_fr_t;

typedef struct _xcb_im_connect_reply_fr_t
{
    uint16_t server_major_protocol_version;
    uint16_t server_minor_protocol_version;
} xcb_im_connect_reply_fr_t;

typedef struct _xcb_im_auth_required_fr_t
{
    uint8_t auth_protocol_index;
    uint8_t auth_data1;
    xcb_im_bytearray_t auth_data2;
} xcb_im_auth_required_fr_t;

typedef struct _xcb_im_auth_reply_fr_t
{
    uint8_t field0;
    xcb_im_bytearray_t field1;
} xcb_im_auth_reply_fr_t;

typedef struct _xcb_im_auth_next_fr_t
{
    uint8_t auth_data1;
    xcb_im_bytearray_t auth_data2;
} xcb_im_auth_next_fr_t;

typedef struct _xcb_im_auth_setup_fr_t
{
    struct {
        uint32_t size;
        xcb_im_xpcs_fr_t* items;
    } server_auth_protocol_names;
} xcb_im_auth_setup_fr_t;

typedef struct _xcb_im_auth_ng_fr_t
{
} xcb_im_auth_ng_fr_t;

typedef struct _xcb_im_disconnect_fr_t
{
} xcb_im_disconnect_fr_t;

typedef struct _xcb_im_disconnect_reply_fr_t
{
} xcb_im_disconnect_reply_fr_t;

typedef struct _xcb_im_open_fr_t
{
    xcb_im_str_fr_t field0;
} xcb_im_open_fr_t;

typedef struct _xcb_im_open_reply_fr_t
{
    uint16_t input_method_ID;
    struct {
        uint32_t size;
        xcb_im_ximattr_fr_t* items;
    } IM_attribute_supported;
    struct {
        uint32_t size;
        xcb_im_xicattr_fr_t* items;
    } IC_attribute_supported;
} xcb_im_open_reply_fr_t;

typedef struct _xcb_im_close_fr_t
{
    uint16_t input_method_ID;
} xcb_im_close_fr_t;

typedef struct _xcb_im_close_reply_fr_t
{
    uint16_t input_method_ID;
} xcb_im_close_reply_fr_t;

typedef struct _xcb_im_register_triggerkeys_fr_t
{
    uint16_t input_method_ID;
    struct {
        uint32_t size;
        xcb_im_ximtriggerkey_fr_t* items;
    } on_keys_list;
    struct {
        uint32_t size;
        xcb_im_ximtriggerkey_fr_t* items;
    } off_keys_list;
} xcb_im_register_triggerkeys_fr_t;

typedef struct _xcb_im_trigger_notify_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint32_t flag;
    uint32_t index_of_keys_list;
    uint32_t client_select_event_mask;
} xcb_im_trigger_notify_fr_t;

typedef struct _xcb_im_trigger_notify_reply_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_trigger_notify_reply_fr_t;

typedef struct _xcb_im_set_event_mask_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint32_t forward_event_mask;
    uint32_t synchronous_event_mask;
} xcb_im_set_event_mask_fr_t;

typedef struct _xcb_im_encoding_negotiation_fr_t
{
    uint16_t input_method_ID;
    struct {
        uint32_t size;
        xcb_im_str_fr_t* items;
    } supported_list_of_encoding_in_IM_library;
    struct {
        uint32_t size;
        xcb_im_encodinginfo_fr_t* items;
    } list_of_encodings_supported_in_th;
} xcb_im_encoding_negotiation_fr_t;

typedef struct _xcb_im_encoding_negotiation_reply_fr_t
{
    uint16_t input_method_ID;
    uint16_t category_of_the_encoding_determined;
    uint16_t index_of_the_encoding_determined;
} xcb_im_encoding_negotiation_reply_fr_t;

typedef struct _xcb_im_query_extension_fr_t
{
    uint16_t input_method_ID;
    struct {
        uint32_t size;
        xcb_im_str_fr_t* items;
    } extensions_supported_by_the_IM_library;
} xcb_im_query_extension_fr_t;

typedef struct _xcb_im_query_extension_reply_fr_t
{
    uint16_t input_method_ID;
    struct {
        uint32_t size;
        xcb_im_ext_fr_t* items;
    } list_of_extensions_supported_by_th;
} xcb_im_query_extension_reply_fr_t;

typedef struct _xcb_im_get_im_values_fr_t
{
    uint16_t input_method_ID;
    struct {
        uint32_t size;
        uint16_t* items;
    } im_attribute_id;
} xcb_im_get_im_values_fr_t;

typedef struct _xcb_im_get_im_values_reply_fr_t
{
    uint16_t input_method_ID;
    struct {
        uint32_t size;
        xcb_im_ximattribute_fr_t* items;
    } im_attribute_returned;
} xcb_im_get_im_values_reply_fr_t;

typedef struct _xcb_im_create_ic_fr_t
{
    uint16_t input_method_ID;
    struct {
        uint32_t size;
        xcb_im_xicattribute_fr_t* items;
    } ic_attributes;
} xcb_im_create_ic_fr_t;

typedef struct _xcb_im_create_ic_reply_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_create_ic_reply_fr_t;

typedef struct _xcb_im_destroy_ic_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_destroy_ic_fr_t;

typedef struct _xcb_im_destroy_ic_reply_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_destroy_ic_reply_fr_t;

typedef struct _xcb_im_set_ic_values_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    struct {
        uint32_t size;
        xcb_im_xicattribute_fr_t* items;
    } ic_attribute;
} xcb_im_set_ic_values_fr_t;

typedef struct _xcb_im_set_ic_values_reply_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_set_ic_values_reply_fr_t;

typedef struct _xcb_im_get_ic_values_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    struct {
        uint32_t size;
        uint16_t* items;
    } ic_attribute;
} xcb_im_get_ic_values_fr_t;

typedef struct _xcb_im_get_ic_values_reply_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    struct {
        uint32_t size;
        xcb_im_xicattribute_fr_t* items;
    } ic_attribute;
} xcb_im_get_ic_values_reply_fr_t;

typedef struct _xcb_im_set_ic_focus_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_set_ic_focus_fr_t;

typedef struct _xcb_im_unset_ic_focus_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_unset_ic_focus_fr_t;

typedef struct _xcb_im_forward_event_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint16_t flag;
    uint16_t sequence_number;
} xcb_im_forward_event_fr_t;

typedef struct _xcb_im_sync_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_sync_fr_t;

typedef struct _xcb_im_sync_reply_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_sync_reply_fr_t;

typedef struct _xcb_im_commit_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint16_t flag;
    uint16_t byte_length_of_committed_string;
    xcb_im_bytearray_t committed_string;
    struct {
        uint32_t size;
        uint32_t* items;
    } keysym;
} xcb_im_commit_fr_t;

typedef struct _xcb_im_commit_chars_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint16_t flag;
    uint16_t byte_length_of_committed_string;
    xcb_im_bytearray_t committed_string;
} xcb_im_commit_chars_fr_t;

typedef struct _xcb_im_commit_both_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint16_t flag;
    uint32_t keysym;
    uint16_t byte_length_of_committed_string;
    xcb_im_bytearray_t committed_string;
} xcb_im_commit_both_fr_t;

typedef struct _xcb_im_reset_ic_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_reset_ic_fr_t;

typedef struct _xcb_im_reset_ic_reply_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint16_t byte_length_of_committed_string;
    xcb_im_bytearray_t committed_string;
} xcb_im_reset_ic_reply_fr_t;

typedef struct _xcb_im_geometry_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_geometry_fr_t;

typedef struct _xcb_im_str_conversion_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint32_t XIMStringConversionPosition;
    uint32_t XIMStringConversionType;
    uint32_t XIMStringConversionOperation;
    uint16_t length_to_multiply_th;
    uint16_t length_of_the_string_to_b;
    xcb_im_bytearray_t string;
} xcb_im_str_conversion_fr_t;

typedef struct _xcb_im_str_conversion_reply_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint32_t XIMStringConversionFeedback;
    uint16_t length_of_the_retrieved_string;
    xcb_im_bytearray_t retrieved_string;
    struct {
        uint32_t size;
        uint32_t* items;
    } feedback_array;
} xcb_im_str_conversion_reply_fr_t;

typedef struct _xcb_im_preedit_start_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_preedit_start_fr_t;

typedef struct _xcb_im_preedit_start_reply_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint32_t return_value;
} xcb_im_preedit_start_reply_fr_t;

typedef struct _xcb_im_preedit_draw_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint32_t caret;
    uint32_t chg_first;
    uint32_t chg_length;
    uint32_t status;
    uint16_t length_of_preedit_string;
    xcb_im_bytearray_t preedit_string;
    struct {
        uint32_t size;
        uint32_t* items;
    } feedback_array;
} xcb_im_preedit_draw_fr_t;

typedef struct _xcb_im_preedit_caret_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint32_t position;
    uint32_t direction;
    uint32_t style;
} xcb_im_preedit_caret_fr_t;

typedef struct _xcb_im_preedit_caret_reply_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint32_t position;
} xcb_im_preedit_caret_reply_fr_t;

typedef struct _xcb_im_preedit_done_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_preedit_done_fr_t;

typedef struct _xcb_im_status_start_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_status_start_fr_t;

typedef struct _xcb_im_status_draw_text_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint32_t type;
    uint32_t status;
    uint16_t length_of_status_string;
    xcb_im_bytearray_t status_string;
    struct {
        uint32_t size;
        uint32_t* items;
    } feedback_array;
} xcb_im_status_draw_text_fr_t;

typedef struct _xcb_im_status_draw_bitmap_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint32_t type;
    uint32_t pixmap_data;
} xcb_im_status_draw_bitmap_fr_t;

typedef struct _xcb_im_status_done_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
} xcb_im_status_done_fr_t;

typedef struct _xcb_im_ext_set_event_mask_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint32_t filter_event_mask;
    uint32_t intercept_event_mask;
    uint32_t select_event_mask;
    uint32_t forward_event_mask;
    uint32_t synchronous_event_mask;
} xcb_im_ext_set_event_mask_fr_t;

typedef struct _xcb_im_ext_forward_keyevent_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint16_t flag;
    uint16_t sequence_number;
    uint8_t xEvent_u_u_type;
    uint8_t keycode;
    uint16_t state;
    uint32_t time;
    uint32_t window;
} xcb_im_ext_forward_keyevent_fr_t;

typedef struct _xcb_im_ext_move_fr_t
{
    uint16_t input_method_ID;
    uint16_t input_context_ID;
    uint16_t X;
    uint16_t Y;
} xcb_im_ext_move_fr_t;

#define frame_read_func(FRAME) _Generic((FRAME), \
    xcb_im_ximattr_fr_t : xcb_im_ximattr_fr_read,\
    xcb_im_xicattr_fr_t : xcb_im_xicattr_fr_read,\
    xcb_im_ximattribute_fr_t : xcb_im_ximattribute_fr_read,\
    xcb_im_xicattribute_fr_t : xcb_im_xicattribute_fr_read,\
    xcb_im_ximtriggerkey_fr_t : xcb_im_ximtriggerkey_fr_read,\
    xcb_im_encodinginfo_fr_t : xcb_im_encodinginfo_fr_read,\
    xcb_im_str_fr_t : xcb_im_str_fr_read,\
    xcb_im_xpcs_fr_t : xcb_im_xpcs_fr_read,\
    xcb_im_ext_fr_t : xcb_im_ext_fr_read,\
    xcb_im_inputstyle_fr_t : xcb_im_inputstyle_fr_read,\
    xcb_im_attr_head_fr_t : xcb_im_attr_head_fr_read,\
    xcb_im_short_fr_t : xcb_im_short_fr_read,\
    xcb_im_long_fr_t : xcb_im_long_fr_read,\
    xcb_im_xrectangle_fr_t : xcb_im_xrectangle_fr_read,\
    xcb_im_xpoint_fr_t : xcb_im_xpoint_fr_read,\
    xcb_im_fontset_fr_t : xcb_im_fontset_fr_read,\
    xcb_im_input_styles_fr_t : xcb_im_input_styles_fr_read,\
    xcb_im_packet_header_fr_t : xcb_im_packet_header_fr_read,\
    xcb_im_error_fr_t : xcb_im_error_fr_read,\
    xcb_im_connect_fr_t : xcb_im_connect_fr_read,\
    xcb_im_connect_reply_fr_t : xcb_im_connect_reply_fr_read,\
    xcb_im_auth_required_fr_t : xcb_im_auth_required_fr_read,\
    xcb_im_auth_reply_fr_t : xcb_im_auth_reply_fr_read,\
    xcb_im_auth_next_fr_t : xcb_im_auth_next_fr_read,\
    xcb_im_auth_setup_fr_t : xcb_im_auth_setup_fr_read,\
    xcb_im_auth_ng_fr_t : xcb_im_auth_ng_fr_read,\
    xcb_im_disconnect_fr_t : xcb_im_disconnect_fr_read,\
    xcb_im_disconnect_reply_fr_t : xcb_im_disconnect_reply_fr_read,\
    xcb_im_open_fr_t : xcb_im_open_fr_read,\
    xcb_im_open_reply_fr_t : xcb_im_open_reply_fr_read,\
    xcb_im_close_fr_t : xcb_im_close_fr_read,\
    xcb_im_close_reply_fr_t : xcb_im_close_reply_fr_read,\
    xcb_im_register_triggerkeys_fr_t : xcb_im_register_triggerkeys_fr_read,\
    xcb_im_trigger_notify_fr_t : xcb_im_trigger_notify_fr_read,\
    xcb_im_trigger_notify_reply_fr_t : xcb_im_trigger_notify_reply_fr_read,\
    xcb_im_set_event_mask_fr_t : xcb_im_set_event_mask_fr_read,\
    xcb_im_encoding_negotiation_fr_t : xcb_im_encoding_negotiation_fr_read,\
    xcb_im_encoding_negotiation_reply_fr_t : xcb_im_encoding_negotiation_reply_fr_read,\
    xcb_im_query_extension_fr_t : xcb_im_query_extension_fr_read,\
    xcb_im_query_extension_reply_fr_t : xcb_im_query_extension_reply_fr_read,\
    xcb_im_get_im_values_fr_t : xcb_im_get_im_values_fr_read,\
    xcb_im_get_im_values_reply_fr_t : xcb_im_get_im_values_reply_fr_read,\
    xcb_im_create_ic_fr_t : xcb_im_create_ic_fr_read,\
    xcb_im_create_ic_reply_fr_t : xcb_im_create_ic_reply_fr_read,\
    xcb_im_destroy_ic_fr_t : xcb_im_destroy_ic_fr_read,\
    xcb_im_destroy_ic_reply_fr_t : xcb_im_destroy_ic_reply_fr_read,\
    xcb_im_set_ic_values_fr_t : xcb_im_set_ic_values_fr_read,\
    xcb_im_set_ic_values_reply_fr_t : xcb_im_set_ic_values_reply_fr_read,\
    xcb_im_get_ic_values_fr_t : xcb_im_get_ic_values_fr_read,\
    xcb_im_get_ic_values_reply_fr_t : xcb_im_get_ic_values_reply_fr_read,\
    xcb_im_set_ic_focus_fr_t : xcb_im_set_ic_focus_fr_read,\
    xcb_im_unset_ic_focus_fr_t : xcb_im_unset_ic_focus_fr_read,\
    xcb_im_forward_event_fr_t : xcb_im_forward_event_fr_read,\
    xcb_im_sync_fr_t : xcb_im_sync_fr_read,\
    xcb_im_sync_reply_fr_t : xcb_im_sync_reply_fr_read,\
    xcb_im_commit_fr_t : xcb_im_commit_fr_read,\
    xcb_im_commit_chars_fr_t : xcb_im_commit_chars_fr_read,\
    xcb_im_commit_both_fr_t : xcb_im_commit_both_fr_read,\
    xcb_im_reset_ic_fr_t : xcb_im_reset_ic_fr_read,\
    xcb_im_reset_ic_reply_fr_t : xcb_im_reset_ic_reply_fr_read,\
    xcb_im_geometry_fr_t : xcb_im_geometry_fr_read,\
    xcb_im_str_conversion_fr_t : xcb_im_str_conversion_fr_read,\
    xcb_im_str_conversion_reply_fr_t : xcb_im_str_conversion_reply_fr_read,\
    xcb_im_preedit_start_fr_t : xcb_im_preedit_start_fr_read,\
    xcb_im_preedit_start_reply_fr_t : xcb_im_preedit_start_reply_fr_read,\
    xcb_im_preedit_draw_fr_t : xcb_im_preedit_draw_fr_read,\
    xcb_im_preedit_caret_fr_t : xcb_im_preedit_caret_fr_read,\
    xcb_im_preedit_caret_reply_fr_t : xcb_im_preedit_caret_reply_fr_read,\
    xcb_im_preedit_done_fr_t : xcb_im_preedit_done_fr_read,\
    xcb_im_status_start_fr_t : xcb_im_status_start_fr_read,\
    xcb_im_status_draw_text_fr_t : xcb_im_status_draw_text_fr_read,\
    xcb_im_status_draw_bitmap_fr_t : xcb_im_status_draw_bitmap_fr_read,\
    xcb_im_status_done_fr_t : xcb_im_status_done_fr_read,\
    xcb_im_ext_set_event_mask_fr_t : xcb_im_ext_set_event_mask_fr_read,\
    xcb_im_ext_forward_keyevent_fr_t : xcb_im_ext_forward_keyevent_fr_read,\
    xcb_im_ext_move_fr_t : xcb_im_ext_move_fr_read)

#define frame_write_func(FRAME) _Generic((FRAME), \
    xcb_im_ximattr_fr_t : xcb_im_ximattr_fr_write,\
    xcb_im_xicattr_fr_t : xcb_im_xicattr_fr_write,\
    xcb_im_ximattribute_fr_t : xcb_im_ximattribute_fr_write,\
    xcb_im_xicattribute_fr_t : xcb_im_xicattribute_fr_write,\
    xcb_im_ximtriggerkey_fr_t : xcb_im_ximtriggerkey_fr_write,\
    xcb_im_encodinginfo_fr_t : xcb_im_encodinginfo_fr_write,\
    xcb_im_str_fr_t : xcb_im_str_fr_write,\
    xcb_im_xpcs_fr_t : xcb_im_xpcs_fr_write,\
    xcb_im_ext_fr_t : xcb_im_ext_fr_write,\
    xcb_im_inputstyle_fr_t : xcb_im_inputstyle_fr_write,\
    xcb_im_attr_head_fr_t : xcb_im_attr_head_fr_write,\
    xcb_im_short_fr_t : xcb_im_short_fr_write,\
    xcb_im_long_fr_t : xcb_im_long_fr_write,\
    xcb_im_xrectangle_fr_t : xcb_im_xrectangle_fr_write,\
    xcb_im_xpoint_fr_t : xcb_im_xpoint_fr_write,\
    xcb_im_fontset_fr_t : xcb_im_fontset_fr_write,\
    xcb_im_input_styles_fr_t : xcb_im_input_styles_fr_write,\
    xcb_im_packet_header_fr_t : xcb_im_packet_header_fr_write,\
    xcb_im_error_fr_t : xcb_im_error_fr_write,\
    xcb_im_connect_fr_t : xcb_im_connect_fr_write,\
    xcb_im_connect_reply_fr_t : xcb_im_connect_reply_fr_write,\
    xcb_im_auth_required_fr_t : xcb_im_auth_required_fr_write,\
    xcb_im_auth_reply_fr_t : xcb_im_auth_reply_fr_write,\
    xcb_im_auth_next_fr_t : xcb_im_auth_next_fr_write,\
    xcb_im_auth_setup_fr_t : xcb_im_auth_setup_fr_write,\
    xcb_im_auth_ng_fr_t : xcb_im_auth_ng_fr_write,\
    xcb_im_disconnect_fr_t : xcb_im_disconnect_fr_write,\
    xcb_im_disconnect_reply_fr_t : xcb_im_disconnect_reply_fr_write,\
    xcb_im_open_fr_t : xcb_im_open_fr_write,\
    xcb_im_open_reply_fr_t : xcb_im_open_reply_fr_write,\
    xcb_im_close_fr_t : xcb_im_close_fr_write,\
    xcb_im_close_reply_fr_t : xcb_im_close_reply_fr_write,\
    xcb_im_register_triggerkeys_fr_t : xcb_im_register_triggerkeys_fr_write,\
    xcb_im_trigger_notify_fr_t : xcb_im_trigger_notify_fr_write,\
    xcb_im_trigger_notify_reply_fr_t : xcb_im_trigger_notify_reply_fr_write,\
    xcb_im_set_event_mask_fr_t : xcb_im_set_event_mask_fr_write,\
    xcb_im_encoding_negotiation_fr_t : xcb_im_encoding_negotiation_fr_write,\
    xcb_im_encoding_negotiation_reply_fr_t : xcb_im_encoding_negotiation_reply_fr_write,\
    xcb_im_query_extension_fr_t : xcb_im_query_extension_fr_write,\
    xcb_im_query_extension_reply_fr_t : xcb_im_query_extension_reply_fr_write,\
    xcb_im_get_im_values_fr_t : xcb_im_get_im_values_fr_write,\
    xcb_im_get_im_values_reply_fr_t : xcb_im_get_im_values_reply_fr_write,\
    xcb_im_create_ic_fr_t : xcb_im_create_ic_fr_write,\
    xcb_im_create_ic_reply_fr_t : xcb_im_create_ic_reply_fr_write,\
    xcb_im_destroy_ic_fr_t : xcb_im_destroy_ic_fr_write,\
    xcb_im_destroy_ic_reply_fr_t : xcb_im_destroy_ic_reply_fr_write,\
    xcb_im_set_ic_values_fr_t : xcb_im_set_ic_values_fr_write,\
    xcb_im_set_ic_values_reply_fr_t : xcb_im_set_ic_values_reply_fr_write,\
    xcb_im_get_ic_values_fr_t : xcb_im_get_ic_values_fr_write,\
    xcb_im_get_ic_values_reply_fr_t : xcb_im_get_ic_values_reply_fr_write,\
    xcb_im_set_ic_focus_fr_t : xcb_im_set_ic_focus_fr_write,\
    xcb_im_unset_ic_focus_fr_t : xcb_im_unset_ic_focus_fr_write,\
    xcb_im_forward_event_fr_t : xcb_im_forward_event_fr_write,\
    xcb_im_sync_fr_t : xcb_im_sync_fr_write,\
    xcb_im_sync_reply_fr_t : xcb_im_sync_reply_fr_write,\
    xcb_im_commit_fr_t : xcb_im_commit_fr_write,\
    xcb_im_commit_chars_fr_t : xcb_im_commit_chars_fr_write,\
    xcb_im_commit_both_fr_t : xcb_im_commit_both_fr_write,\
    xcb_im_reset_ic_fr_t : xcb_im_reset_ic_fr_write,\
    xcb_im_reset_ic_reply_fr_t : xcb_im_reset_ic_reply_fr_write,\
    xcb_im_geometry_fr_t : xcb_im_geometry_fr_write,\
    xcb_im_str_conversion_fr_t : xcb_im_str_conversion_fr_write,\
    xcb_im_str_conversion_reply_fr_t : xcb_im_str_conversion_reply_fr_write,\
    xcb_im_preedit_start_fr_t : xcb_im_preedit_start_fr_write,\
    xcb_im_preedit_start_reply_fr_t : xcb_im_preedit_start_reply_fr_write,\
    xcb_im_preedit_draw_fr_t : xcb_im_preedit_draw_fr_write,\
    xcb_im_preedit_caret_fr_t : xcb_im_preedit_caret_fr_write,\
    xcb_im_preedit_caret_reply_fr_t : xcb_im_preedit_caret_reply_fr_write,\
    xcb_im_preedit_done_fr_t : xcb_im_preedit_done_fr_write,\
    xcb_im_status_start_fr_t : xcb_im_status_start_fr_write,\
    xcb_im_status_draw_text_fr_t : xcb_im_status_draw_text_fr_write,\
    xcb_im_status_draw_bitmap_fr_t : xcb_im_status_draw_bitmap_fr_write,\
    xcb_im_status_done_fr_t : xcb_im_status_done_fr_write,\
    xcb_im_ext_set_event_mask_fr_t : xcb_im_ext_set_event_mask_fr_write,\
    xcb_im_ext_forward_keyevent_fr_t : xcb_im_ext_forward_keyevent_fr_write,\
    xcb_im_ext_move_fr_t : xcb_im_ext_move_fr_write)

#define frame_size_func(FRAME) _Generic((FRAME), \
    xcb_im_ximattr_fr_t : xcb_im_ximattr_fr_size((xcb_im_ximattr_fr_t*) &(FRAME)),\
    xcb_im_xicattr_fr_t : xcb_im_xicattr_fr_size((xcb_im_xicattr_fr_t*) &(FRAME)),\
    xcb_im_ximattribute_fr_t : xcb_im_ximattribute_fr_size((xcb_im_ximattribute_fr_t*) &(FRAME)),\
    xcb_im_xicattribute_fr_t : xcb_im_xicattribute_fr_size((xcb_im_xicattribute_fr_t*) &(FRAME)),\
    xcb_im_ximtriggerkey_fr_t : 12,\
    xcb_im_encodinginfo_fr_t : xcb_im_encodinginfo_fr_size((xcb_im_encodinginfo_fr_t*) &(FRAME)),\
    xcb_im_str_fr_t : xcb_im_str_fr_size((xcb_im_str_fr_t*) &(FRAME)),\
    xcb_im_xpcs_fr_t : xcb_im_xpcs_fr_size((xcb_im_xpcs_fr_t*) &(FRAME)),\
    xcb_im_ext_fr_t : xcb_im_ext_fr_size((xcb_im_ext_fr_t*) &(FRAME)),\
    xcb_im_inputstyle_fr_t : 4,\
    xcb_im_attr_head_fr_t : 4,\
    xcb_im_short_fr_t : 2,\
    xcb_im_long_fr_t : 4,\
    xcb_im_xrectangle_fr_t : 8,\
    xcb_im_xpoint_fr_t : 4,\
    xcb_im_fontset_fr_t : xcb_im_fontset_fr_size((xcb_im_fontset_fr_t*) &(FRAME)),\
    xcb_im_input_styles_fr_t : xcb_im_input_styles_fr_size((xcb_im_input_styles_fr_t*) &(FRAME)),\
    xcb_im_packet_header_fr_t : 4,\
    xcb_im_error_fr_t : xcb_im_error_fr_size((xcb_im_error_fr_t*) &(FRAME)),\
    xcb_im_connect_fr_t : xcb_im_connect_fr_size((xcb_im_connect_fr_t*) &(FRAME)),\
    xcb_im_connect_reply_fr_t : 4,\
    xcb_im_auth_required_fr_t : xcb_im_auth_required_fr_size((xcb_im_auth_required_fr_t*) &(FRAME)),\
    xcb_im_auth_reply_fr_t : xcb_im_auth_reply_fr_size((xcb_im_auth_reply_fr_t*) &(FRAME)),\
    xcb_im_auth_next_fr_t : xcb_im_auth_next_fr_size((xcb_im_auth_next_fr_t*) &(FRAME)),\
    xcb_im_auth_setup_fr_t : xcb_im_auth_setup_fr_size((xcb_im_auth_setup_fr_t*) &(FRAME)),\
    xcb_im_auth_ng_fr_t : 0,\
    xcb_im_disconnect_fr_t : 0,\
    xcb_im_disconnect_reply_fr_t : 0,\
    xcb_im_open_fr_t : xcb_im_open_fr_size((xcb_im_open_fr_t*) &(FRAME)),\
    xcb_im_open_reply_fr_t : xcb_im_open_reply_fr_size((xcb_im_open_reply_fr_t*) &(FRAME)),\
    xcb_im_close_fr_t : 4,\
    xcb_im_close_reply_fr_t : 4,\
    xcb_im_register_triggerkeys_fr_t : xcb_im_register_triggerkeys_fr_size((xcb_im_register_triggerkeys_fr_t*) &(FRAME)),\
    xcb_im_trigger_notify_fr_t : 16,\
    xcb_im_trigger_notify_reply_fr_t : 4,\
    xcb_im_set_event_mask_fr_t : 12,\
    xcb_im_encoding_negotiation_fr_t : xcb_im_encoding_negotiation_fr_size((xcb_im_encoding_negotiation_fr_t*) &(FRAME)),\
    xcb_im_encoding_negotiation_reply_fr_t : 8,\
    xcb_im_query_extension_fr_t : xcb_im_query_extension_fr_size((xcb_im_query_extension_fr_t*) &(FRAME)),\
    xcb_im_query_extension_reply_fr_t : xcb_im_query_extension_reply_fr_size((xcb_im_query_extension_reply_fr_t*) &(FRAME)),\
    xcb_im_get_im_values_fr_t : xcb_im_get_im_values_fr_size((xcb_im_get_im_values_fr_t*) &(FRAME)),\
    xcb_im_get_im_values_reply_fr_t : xcb_im_get_im_values_reply_fr_size((xcb_im_get_im_values_reply_fr_t*) &(FRAME)),\
    xcb_im_create_ic_fr_t : xcb_im_create_ic_fr_size((xcb_im_create_ic_fr_t*) &(FRAME)),\
    xcb_im_create_ic_reply_fr_t : 4,\
    xcb_im_destroy_ic_fr_t : 4,\
    xcb_im_destroy_ic_reply_fr_t : 4,\
    xcb_im_set_ic_values_fr_t : xcb_im_set_ic_values_fr_size((xcb_im_set_ic_values_fr_t*) &(FRAME)),\
    xcb_im_set_ic_values_reply_fr_t : 4,\
    xcb_im_get_ic_values_fr_t : xcb_im_get_ic_values_fr_size((xcb_im_get_ic_values_fr_t*) &(FRAME)),\
    xcb_im_get_ic_values_reply_fr_t : xcb_im_get_ic_values_reply_fr_size((xcb_im_get_ic_values_reply_fr_t*) &(FRAME)),\
    xcb_im_set_ic_focus_fr_t : 4,\
    xcb_im_unset_ic_focus_fr_t : 4,\
    xcb_im_forward_event_fr_t : 8,\
    xcb_im_sync_fr_t : 4,\
    xcb_im_sync_reply_fr_t : 4,\
    xcb_im_commit_fr_t : xcb_im_commit_fr_size((xcb_im_commit_fr_t*) &(FRAME)),\
    xcb_im_commit_chars_fr_t : xcb_im_commit_chars_fr_size((xcb_im_commit_chars_fr_t*) &(FRAME)),\
    xcb_im_commit_both_fr_t : xcb_im_commit_both_fr_size((xcb_im_commit_both_fr_t*) &(FRAME)),\
    xcb_im_reset_ic_fr_t : 4,\
    xcb_im_reset_ic_reply_fr_t : xcb_im_reset_ic_reply_fr_size((xcb_im_reset_ic_reply_fr_t*) &(FRAME)),\
    xcb_im_geometry_fr_t : 4,\
    xcb_im_str_conversion_fr_t : xcb_im_str_conversion_fr_size((xcb_im_str_conversion_fr_t*) &(FRAME)),\
    xcb_im_str_conversion_reply_fr_t : xcb_im_str_conversion_reply_fr_size((xcb_im_str_conversion_reply_fr_t*) &(FRAME)),\
    xcb_im_preedit_start_fr_t : 4,\
    xcb_im_preedit_start_reply_fr_t : 8,\
    xcb_im_preedit_draw_fr_t : xcb_im_preedit_draw_fr_size((xcb_im_preedit_draw_fr_t*) &(FRAME)),\
    xcb_im_preedit_caret_fr_t : 16,\
    xcb_im_preedit_caret_reply_fr_t : 8,\
    xcb_im_preedit_done_fr_t : 4,\
    xcb_im_status_start_fr_t : 4,\
    xcb_im_status_draw_text_fr_t : xcb_im_status_draw_text_fr_size((xcb_im_status_draw_text_fr_t*) &(FRAME)),\
    xcb_im_status_draw_bitmap_fr_t : 12,\
    xcb_im_status_done_fr_t : 4,\
    xcb_im_ext_set_event_mask_fr_t : 24,\
    xcb_im_ext_forward_keyevent_fr_t : 20,\
    xcb_im_ext_move_fr_t : 8)

#define frame_free_func(FRAME) _Generic((FRAME), \
    xcb_im_ximattr_fr_t : xcb_im_ximattr_fr_free,\
    xcb_im_xicattr_fr_t : xcb_im_xicattr_fr_free,\
    xcb_im_ximattribute_fr_t : xcb_im_ximattribute_fr_free,\
    xcb_im_xicattribute_fr_t : xcb_im_xicattribute_fr_free,\
    xcb_im_ximtriggerkey_fr_t : xcb_im_ximtriggerkey_fr_free,\
    xcb_im_encodinginfo_fr_t : xcb_im_encodinginfo_fr_free,\
    xcb_im_str_fr_t : xcb_im_str_fr_free,\
    xcb_im_xpcs_fr_t : xcb_im_xpcs_fr_free,\
    xcb_im_ext_fr_t : xcb_im_ext_fr_free,\
    xcb_im_inputstyle_fr_t : xcb_im_inputstyle_fr_free,\
    xcb_im_attr_head_fr_t : xcb_im_attr_head_fr_free,\
    xcb_im_short_fr_t : xcb_im_short_fr_free,\
    xcb_im_long_fr_t : xcb_im_long_fr_free,\
    xcb_im_xrectangle_fr_t : xcb_im_xrectangle_fr_free,\
    xcb_im_xpoint_fr_t : xcb_im_xpoint_fr_free,\
    xcb_im_fontset_fr_t : xcb_im_fontset_fr_free,\
    xcb_im_input_styles_fr_t : xcb_im_input_styles_fr_free,\
    xcb_im_packet_header_fr_t : xcb_im_packet_header_fr_free,\
    xcb_im_error_fr_t : xcb_im_error_fr_free,\
    xcb_im_connect_fr_t : xcb_im_connect_fr_free,\
    xcb_im_connect_reply_fr_t : xcb_im_connect_reply_fr_free,\
    xcb_im_auth_required_fr_t : xcb_im_auth_required_fr_free,\
    xcb_im_auth_reply_fr_t : xcb_im_auth_reply_fr_free,\
    xcb_im_auth_next_fr_t : xcb_im_auth_next_fr_free,\
    xcb_im_auth_setup_fr_t : xcb_im_auth_setup_fr_free,\
    xcb_im_auth_ng_fr_t : xcb_im_auth_ng_fr_free,\
    xcb_im_disconnect_fr_t : xcb_im_disconnect_fr_free,\
    xcb_im_disconnect_reply_fr_t : xcb_im_disconnect_reply_fr_free,\
    xcb_im_open_fr_t : xcb_im_open_fr_free,\
    xcb_im_open_reply_fr_t : xcb_im_open_reply_fr_free,\
    xcb_im_close_fr_t : xcb_im_close_fr_free,\
    xcb_im_close_reply_fr_t : xcb_im_close_reply_fr_free,\
    xcb_im_register_triggerkeys_fr_t : xcb_im_register_triggerkeys_fr_free,\
    xcb_im_trigger_notify_fr_t : xcb_im_trigger_notify_fr_free,\
    xcb_im_trigger_notify_reply_fr_t : xcb_im_trigger_notify_reply_fr_free,\
    xcb_im_set_event_mask_fr_t : xcb_im_set_event_mask_fr_free,\
    xcb_im_encoding_negotiation_fr_t : xcb_im_encoding_negotiation_fr_free,\
    xcb_im_encoding_negotiation_reply_fr_t : xcb_im_encoding_negotiation_reply_fr_free,\
    xcb_im_query_extension_fr_t : xcb_im_query_extension_fr_free,\
    xcb_im_query_extension_reply_fr_t : xcb_im_query_extension_reply_fr_free,\
    xcb_im_get_im_values_fr_t : xcb_im_get_im_values_fr_free,\
    xcb_im_get_im_values_reply_fr_t : xcb_im_get_im_values_reply_fr_free,\
    xcb_im_create_ic_fr_t : xcb_im_create_ic_fr_free,\
    xcb_im_create_ic_reply_fr_t : xcb_im_create_ic_reply_fr_free,\
    xcb_im_destroy_ic_fr_t : xcb_im_destroy_ic_fr_free,\
    xcb_im_destroy_ic_reply_fr_t : xcb_im_destroy_ic_reply_fr_free,\
    xcb_im_set_ic_values_fr_t : xcb_im_set_ic_values_fr_free,\
    xcb_im_set_ic_values_reply_fr_t : xcb_im_set_ic_values_reply_fr_free,\
    xcb_im_get_ic_values_fr_t : xcb_im_get_ic_values_fr_free,\
    xcb_im_get_ic_values_reply_fr_t : xcb_im_get_ic_values_reply_fr_free,\
    xcb_im_set_ic_focus_fr_t : xcb_im_set_ic_focus_fr_free,\
    xcb_im_unset_ic_focus_fr_t : xcb_im_unset_ic_focus_fr_free,\
    xcb_im_forward_event_fr_t : xcb_im_forward_event_fr_free,\
    xcb_im_sync_fr_t : xcb_im_sync_fr_free,\
    xcb_im_sync_reply_fr_t : xcb_im_sync_reply_fr_free,\
    xcb_im_commit_fr_t : xcb_im_commit_fr_free,\
    xcb_im_commit_chars_fr_t : xcb_im_commit_chars_fr_free,\
    xcb_im_commit_both_fr_t : xcb_im_commit_both_fr_free,\
    xcb_im_reset_ic_fr_t : xcb_im_reset_ic_fr_free,\
    xcb_im_reset_ic_reply_fr_t : xcb_im_reset_ic_reply_fr_free,\
    xcb_im_geometry_fr_t : xcb_im_geometry_fr_free,\
    xcb_im_str_conversion_fr_t : xcb_im_str_conversion_fr_free,\
    xcb_im_str_conversion_reply_fr_t : xcb_im_str_conversion_reply_fr_free,\
    xcb_im_preedit_start_fr_t : xcb_im_preedit_start_fr_free,\
    xcb_im_preedit_start_reply_fr_t : xcb_im_preedit_start_reply_fr_free,\
    xcb_im_preedit_draw_fr_t : xcb_im_preedit_draw_fr_free,\
    xcb_im_preedit_caret_fr_t : xcb_im_preedit_caret_fr_free,\
    xcb_im_preedit_caret_reply_fr_t : xcb_im_preedit_caret_reply_fr_free,\
    xcb_im_preedit_done_fr_t : xcb_im_preedit_done_fr_free,\
    xcb_im_status_start_fr_t : xcb_im_status_start_fr_free,\
    xcb_im_status_draw_text_fr_t : xcb_im_status_draw_text_fr_free,\
    xcb_im_status_draw_bitmap_fr_t : xcb_im_status_draw_bitmap_fr_free,\
    xcb_im_status_done_fr_t : xcb_im_status_done_fr_free,\
    xcb_im_ext_set_event_mask_fr_t : xcb_im_ext_set_event_mask_fr_free,\
    xcb_im_ext_forward_keyevent_fr_t : xcb_im_ext_forward_keyevent_fr_free,\
    xcb_im_ext_move_fr_t : xcb_im_ext_move_fr_free)

#define frame_has_static_size(FRAME) _Generic((FRAME), \
    xcb_im_ximattr_fr_t : false,\
    xcb_im_xicattr_fr_t : false,\
    xcb_im_ximattribute_fr_t : false,\
    xcb_im_xicattribute_fr_t : false,\
    xcb_im_ximtriggerkey_fr_t : true,\
    xcb_im_encodinginfo_fr_t : false,\
    xcb_im_str_fr_t : false,\
    xcb_im_xpcs_fr_t : false,\
    xcb_im_ext_fr_t : false,\
    xcb_im_inputstyle_fr_t : true,\
    xcb_im_attr_head_fr_t : true,\
    xcb_im_short_fr_t : true,\
    xcb_im_long_fr_t : true,\
    xcb_im_xrectangle_fr_t : true,\
    xcb_im_xpoint_fr_t : true,\
    xcb_im_fontset_fr_t : false,\
    xcb_im_input_styles_fr_t : false,\
    xcb_im_packet_header_fr_t : true,\
    xcb_im_error_fr_t : false,\
    xcb_im_connect_fr_t : false,\
    xcb_im_connect_reply_fr_t : true,\
    xcb_im_auth_required_fr_t : false,\
    xcb_im_auth_reply_fr_t : false,\
    xcb_im_auth_next_fr_t : false,\
    xcb_im_auth_setup_fr_t : false,\
    xcb_im_auth_ng_fr_t : true,\
    xcb_im_disconnect_fr_t : true,\
    xcb_im_disconnect_reply_fr_t : true,\
    xcb_im_open_fr_t : false,\
    xcb_im_open_reply_fr_t : false,\
    xcb_im_close_fr_t : true,\
    xcb_im_close_reply_fr_t : true,\
    xcb_im_register_triggerkeys_fr_t : false,\
    xcb_im_trigger_notify_fr_t : true,\
    xcb_im_trigger_notify_reply_fr_t : true,\
    xcb_im_set_event_mask_fr_t : true,\
    xcb_im_encoding_negotiation_fr_t : false,\
    xcb_im_encoding_negotiation_reply_fr_t : true,\
    xcb_im_query_extension_fr_t : false,\
    xcb_im_query_extension_reply_fr_t : false,\
    xcb_im_get_im_values_fr_t : false,\
    xcb_im_get_im_values_reply_fr_t : false,\
    xcb_im_create_ic_fr_t : false,\
    xcb_im_create_ic_reply_fr_t : true,\
    xcb_im_destroy_ic_fr_t : true,\
    xcb_im_destroy_ic_reply_fr_t : true,\
    xcb_im_set_ic_values_fr_t : false,\
    xcb_im_set_ic_values_reply_fr_t : true,\
    xcb_im_get_ic_values_fr_t : false,\
    xcb_im_get_ic_values_reply_fr_t : false,\
    xcb_im_set_ic_focus_fr_t : true,\
    xcb_im_unset_ic_focus_fr_t : true,\
    xcb_im_forward_event_fr_t : true,\
    xcb_im_sync_fr_t : true,\
    xcb_im_sync_reply_fr_t : true,\
    xcb_im_commit_fr_t : false,\
    xcb_im_commit_chars_fr_t : false,\
    xcb_im_commit_both_fr_t : false,\
    xcb_im_reset_ic_fr_t : true,\
    xcb_im_reset_ic_reply_fr_t : false,\
    xcb_im_geometry_fr_t : true,\
    xcb_im_str_conversion_fr_t : false,\
    xcb_im_str_conversion_reply_fr_t : false,\
    xcb_im_preedit_start_fr_t : true,\
    xcb_im_preedit_start_reply_fr_t : true,\
    xcb_im_preedit_draw_fr_t : false,\
    xcb_im_preedit_caret_fr_t : true,\
    xcb_im_preedit_caret_reply_fr_t : true,\
    xcb_im_preedit_done_fr_t : true,\
    xcb_im_status_start_fr_t : true,\
    xcb_im_status_draw_text_fr_t : false,\
    xcb_im_status_draw_bitmap_fr_t : true,\
    xcb_im_status_done_fr_t : true,\
    xcb_im_ext_set_event_mask_fr_t : true,\
    xcb_im_ext_forward_keyevent_fr_t : true,\
    xcb_im_ext_move_fr_t : true)

void xcb_im_ximattr_fr_read(xcb_im_ximattr_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_ximattr_fr_write(xcb_im_ximattr_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_ximattr_fr_free(xcb_im_ximattr_fr_t *frame);
size_t xcb_im_ximattr_fr_size(xcb_im_ximattr_fr_t *frame);

void xcb_im_xicattr_fr_read(xcb_im_xicattr_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_xicattr_fr_write(xcb_im_xicattr_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_xicattr_fr_free(xcb_im_xicattr_fr_t *frame);
size_t xcb_im_xicattr_fr_size(xcb_im_xicattr_fr_t *frame);

void xcb_im_ximattribute_fr_read(xcb_im_ximattribute_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_ximattribute_fr_write(xcb_im_ximattribute_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_ximattribute_fr_free(xcb_im_ximattribute_fr_t *frame);
size_t xcb_im_ximattribute_fr_size(xcb_im_ximattribute_fr_t *frame);

void xcb_im_xicattribute_fr_read(xcb_im_xicattribute_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_xicattribute_fr_write(xcb_im_xicattribute_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_xicattribute_fr_free(xcb_im_xicattribute_fr_t *frame);
size_t xcb_im_xicattribute_fr_size(xcb_im_xicattribute_fr_t *frame);

void xcb_im_ximtriggerkey_fr_read(xcb_im_ximtriggerkey_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_ximtriggerkey_fr_write(xcb_im_ximtriggerkey_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_ximtriggerkey_fr_free(xcb_im_ximtriggerkey_fr_t *frame);
#define xcb_im_ximtriggerkey_fr_size(ARG...) (((void)(ARG)), (12))

void xcb_im_encodinginfo_fr_read(xcb_im_encodinginfo_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_encodinginfo_fr_write(xcb_im_encodinginfo_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_encodinginfo_fr_free(xcb_im_encodinginfo_fr_t *frame);
size_t xcb_im_encodinginfo_fr_size(xcb_im_encodinginfo_fr_t *frame);

void xcb_im_str_fr_read(xcb_im_str_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_str_fr_write(xcb_im_str_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_str_fr_free(xcb_im_str_fr_t *frame);
size_t xcb_im_str_fr_size(xcb_im_str_fr_t *frame);

void xcb_im_xpcs_fr_read(xcb_im_xpcs_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_xpcs_fr_write(xcb_im_xpcs_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_xpcs_fr_free(xcb_im_xpcs_fr_t *frame);
size_t xcb_im_xpcs_fr_size(xcb_im_xpcs_fr_t *frame);

void xcb_im_ext_fr_read(xcb_im_ext_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_ext_fr_write(xcb_im_ext_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_ext_fr_free(xcb_im_ext_fr_t *frame);
size_t xcb_im_ext_fr_size(xcb_im_ext_fr_t *frame);

void xcb_im_inputstyle_fr_read(xcb_im_inputstyle_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_inputstyle_fr_write(xcb_im_inputstyle_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_inputstyle_fr_free(xcb_im_inputstyle_fr_t *frame);
#define xcb_im_inputstyle_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_attr_head_fr_read(xcb_im_attr_head_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_attr_head_fr_write(xcb_im_attr_head_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_attr_head_fr_free(xcb_im_attr_head_fr_t *frame);
#define xcb_im_attr_head_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_short_fr_read(xcb_im_short_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_short_fr_write(xcb_im_short_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_short_fr_free(xcb_im_short_fr_t *frame);
#define xcb_im_short_fr_size(ARG...) (((void)(ARG)), (2))

void xcb_im_long_fr_read(xcb_im_long_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_long_fr_write(xcb_im_long_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_long_fr_free(xcb_im_long_fr_t *frame);
#define xcb_im_long_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_xrectangle_fr_read(xcb_im_xrectangle_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_xrectangle_fr_write(xcb_im_xrectangle_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_xrectangle_fr_free(xcb_im_xrectangle_fr_t *frame);
#define xcb_im_xrectangle_fr_size(ARG...) (((void)(ARG)), (8))

void xcb_im_xpoint_fr_read(xcb_im_xpoint_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_xpoint_fr_write(xcb_im_xpoint_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_xpoint_fr_free(xcb_im_xpoint_fr_t *frame);
#define xcb_im_xpoint_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_fontset_fr_read(xcb_im_fontset_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_fontset_fr_write(xcb_im_fontset_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_fontset_fr_free(xcb_im_fontset_fr_t *frame);
size_t xcb_im_fontset_fr_size(xcb_im_fontset_fr_t *frame);

void xcb_im_input_styles_fr_read(xcb_im_input_styles_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_input_styles_fr_write(xcb_im_input_styles_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_input_styles_fr_free(xcb_im_input_styles_fr_t *frame);
size_t xcb_im_input_styles_fr_size(xcb_im_input_styles_fr_t *frame);

void xcb_im_packet_header_fr_read(xcb_im_packet_header_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_packet_header_fr_write(xcb_im_packet_header_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_packet_header_fr_free(xcb_im_packet_header_fr_t *frame);
#define xcb_im_packet_header_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_error_fr_read(xcb_im_error_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_error_fr_write(xcb_im_error_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_error_fr_free(xcb_im_error_fr_t *frame);
size_t xcb_im_error_fr_size(xcb_im_error_fr_t *frame);

void xcb_im_connect_fr_read(xcb_im_connect_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_connect_fr_write(xcb_im_connect_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_connect_fr_free(xcb_im_connect_fr_t *frame);
size_t xcb_im_connect_fr_size(xcb_im_connect_fr_t *frame);

void xcb_im_connect_reply_fr_read(xcb_im_connect_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_connect_reply_fr_write(xcb_im_connect_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_connect_reply_fr_free(xcb_im_connect_reply_fr_t *frame);
#define xcb_im_connect_reply_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_auth_required_fr_read(xcb_im_auth_required_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_auth_required_fr_write(xcb_im_auth_required_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_auth_required_fr_free(xcb_im_auth_required_fr_t *frame);
size_t xcb_im_auth_required_fr_size(xcb_im_auth_required_fr_t *frame);

void xcb_im_auth_reply_fr_read(xcb_im_auth_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_auth_reply_fr_write(xcb_im_auth_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_auth_reply_fr_free(xcb_im_auth_reply_fr_t *frame);
size_t xcb_im_auth_reply_fr_size(xcb_im_auth_reply_fr_t *frame);

void xcb_im_auth_next_fr_read(xcb_im_auth_next_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_auth_next_fr_write(xcb_im_auth_next_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_auth_next_fr_free(xcb_im_auth_next_fr_t *frame);
size_t xcb_im_auth_next_fr_size(xcb_im_auth_next_fr_t *frame);

void xcb_im_auth_setup_fr_read(xcb_im_auth_setup_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_auth_setup_fr_write(xcb_im_auth_setup_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_auth_setup_fr_free(xcb_im_auth_setup_fr_t *frame);
size_t xcb_im_auth_setup_fr_size(xcb_im_auth_setup_fr_t *frame);

void xcb_im_auth_ng_fr_read(xcb_im_auth_ng_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_auth_ng_fr_write(xcb_im_auth_ng_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_auth_ng_fr_free(xcb_im_auth_ng_fr_t *frame);
#define xcb_im_auth_ng_fr_size(ARG...) (((void)(ARG)), (0))

void xcb_im_disconnect_fr_read(xcb_im_disconnect_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_disconnect_fr_write(xcb_im_disconnect_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_disconnect_fr_free(xcb_im_disconnect_fr_t *frame);
#define xcb_im_disconnect_fr_size(ARG...) (((void)(ARG)), (0))

void xcb_im_disconnect_reply_fr_read(xcb_im_disconnect_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_disconnect_reply_fr_write(xcb_im_disconnect_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_disconnect_reply_fr_free(xcb_im_disconnect_reply_fr_t *frame);
#define xcb_im_disconnect_reply_fr_size(ARG...) (((void)(ARG)), (0))

void xcb_im_open_fr_read(xcb_im_open_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_open_fr_write(xcb_im_open_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_open_fr_free(xcb_im_open_fr_t *frame);
size_t xcb_im_open_fr_size(xcb_im_open_fr_t *frame);

void xcb_im_open_reply_fr_read(xcb_im_open_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_open_reply_fr_write(xcb_im_open_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_open_reply_fr_free(xcb_im_open_reply_fr_t *frame);
size_t xcb_im_open_reply_fr_size(xcb_im_open_reply_fr_t *frame);

void xcb_im_close_fr_read(xcb_im_close_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_close_fr_write(xcb_im_close_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_close_fr_free(xcb_im_close_fr_t *frame);
#define xcb_im_close_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_close_reply_fr_read(xcb_im_close_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_close_reply_fr_write(xcb_im_close_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_close_reply_fr_free(xcb_im_close_reply_fr_t *frame);
#define xcb_im_close_reply_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_register_triggerkeys_fr_read(xcb_im_register_triggerkeys_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_register_triggerkeys_fr_write(xcb_im_register_triggerkeys_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_register_triggerkeys_fr_free(xcb_im_register_triggerkeys_fr_t *frame);
size_t xcb_im_register_triggerkeys_fr_size(xcb_im_register_triggerkeys_fr_t *frame);

void xcb_im_trigger_notify_fr_read(xcb_im_trigger_notify_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_trigger_notify_fr_write(xcb_im_trigger_notify_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_trigger_notify_fr_free(xcb_im_trigger_notify_fr_t *frame);
#define xcb_im_trigger_notify_fr_size(ARG...) (((void)(ARG)), (16))

void xcb_im_trigger_notify_reply_fr_read(xcb_im_trigger_notify_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_trigger_notify_reply_fr_write(xcb_im_trigger_notify_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_trigger_notify_reply_fr_free(xcb_im_trigger_notify_reply_fr_t *frame);
#define xcb_im_trigger_notify_reply_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_set_event_mask_fr_read(xcb_im_set_event_mask_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_set_event_mask_fr_write(xcb_im_set_event_mask_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_set_event_mask_fr_free(xcb_im_set_event_mask_fr_t *frame);
#define xcb_im_set_event_mask_fr_size(ARG...) (((void)(ARG)), (12))

void xcb_im_encoding_negotiation_fr_read(xcb_im_encoding_negotiation_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_encoding_negotiation_fr_write(xcb_im_encoding_negotiation_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_encoding_negotiation_fr_free(xcb_im_encoding_negotiation_fr_t *frame);
size_t xcb_im_encoding_negotiation_fr_size(xcb_im_encoding_negotiation_fr_t *frame);

void xcb_im_encoding_negotiation_reply_fr_read(xcb_im_encoding_negotiation_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_encoding_negotiation_reply_fr_write(xcb_im_encoding_negotiation_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_encoding_negotiation_reply_fr_free(xcb_im_encoding_negotiation_reply_fr_t *frame);
#define xcb_im_encoding_negotiation_reply_fr_size(ARG...) (((void)(ARG)), (8))

void xcb_im_query_extension_fr_read(xcb_im_query_extension_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_query_extension_fr_write(xcb_im_query_extension_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_query_extension_fr_free(xcb_im_query_extension_fr_t *frame);
size_t xcb_im_query_extension_fr_size(xcb_im_query_extension_fr_t *frame);

void xcb_im_query_extension_reply_fr_read(xcb_im_query_extension_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_query_extension_reply_fr_write(xcb_im_query_extension_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_query_extension_reply_fr_free(xcb_im_query_extension_reply_fr_t *frame);
size_t xcb_im_query_extension_reply_fr_size(xcb_im_query_extension_reply_fr_t *frame);

void xcb_im_get_im_values_fr_read(xcb_im_get_im_values_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_get_im_values_fr_write(xcb_im_get_im_values_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_get_im_values_fr_free(xcb_im_get_im_values_fr_t *frame);
size_t xcb_im_get_im_values_fr_size(xcb_im_get_im_values_fr_t *frame);

void xcb_im_get_im_values_reply_fr_read(xcb_im_get_im_values_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_get_im_values_reply_fr_write(xcb_im_get_im_values_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_get_im_values_reply_fr_free(xcb_im_get_im_values_reply_fr_t *frame);
size_t xcb_im_get_im_values_reply_fr_size(xcb_im_get_im_values_reply_fr_t *frame);

void xcb_im_create_ic_fr_read(xcb_im_create_ic_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_create_ic_fr_write(xcb_im_create_ic_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_create_ic_fr_free(xcb_im_create_ic_fr_t *frame);
size_t xcb_im_create_ic_fr_size(xcb_im_create_ic_fr_t *frame);

void xcb_im_create_ic_reply_fr_read(xcb_im_create_ic_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_create_ic_reply_fr_write(xcb_im_create_ic_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_create_ic_reply_fr_free(xcb_im_create_ic_reply_fr_t *frame);
#define xcb_im_create_ic_reply_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_destroy_ic_fr_read(xcb_im_destroy_ic_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_destroy_ic_fr_write(xcb_im_destroy_ic_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_destroy_ic_fr_free(xcb_im_destroy_ic_fr_t *frame);
#define xcb_im_destroy_ic_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_destroy_ic_reply_fr_read(xcb_im_destroy_ic_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_destroy_ic_reply_fr_write(xcb_im_destroy_ic_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_destroy_ic_reply_fr_free(xcb_im_destroy_ic_reply_fr_t *frame);
#define xcb_im_destroy_ic_reply_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_set_ic_values_fr_read(xcb_im_set_ic_values_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_set_ic_values_fr_write(xcb_im_set_ic_values_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_set_ic_values_fr_free(xcb_im_set_ic_values_fr_t *frame);
size_t xcb_im_set_ic_values_fr_size(xcb_im_set_ic_values_fr_t *frame);

void xcb_im_set_ic_values_reply_fr_read(xcb_im_set_ic_values_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_set_ic_values_reply_fr_write(xcb_im_set_ic_values_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_set_ic_values_reply_fr_free(xcb_im_set_ic_values_reply_fr_t *frame);
#define xcb_im_set_ic_values_reply_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_get_ic_values_fr_read(xcb_im_get_ic_values_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_get_ic_values_fr_write(xcb_im_get_ic_values_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_get_ic_values_fr_free(xcb_im_get_ic_values_fr_t *frame);
size_t xcb_im_get_ic_values_fr_size(xcb_im_get_ic_values_fr_t *frame);

void xcb_im_get_ic_values_reply_fr_read(xcb_im_get_ic_values_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_get_ic_values_reply_fr_write(xcb_im_get_ic_values_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_get_ic_values_reply_fr_free(xcb_im_get_ic_values_reply_fr_t *frame);
size_t xcb_im_get_ic_values_reply_fr_size(xcb_im_get_ic_values_reply_fr_t *frame);

void xcb_im_set_ic_focus_fr_read(xcb_im_set_ic_focus_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_set_ic_focus_fr_write(xcb_im_set_ic_focus_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_set_ic_focus_fr_free(xcb_im_set_ic_focus_fr_t *frame);
#define xcb_im_set_ic_focus_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_unset_ic_focus_fr_read(xcb_im_unset_ic_focus_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_unset_ic_focus_fr_write(xcb_im_unset_ic_focus_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_unset_ic_focus_fr_free(xcb_im_unset_ic_focus_fr_t *frame);
#define xcb_im_unset_ic_focus_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_forward_event_fr_read(xcb_im_forward_event_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_forward_event_fr_write(xcb_im_forward_event_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_forward_event_fr_free(xcb_im_forward_event_fr_t *frame);
#define xcb_im_forward_event_fr_size(ARG...) (((void)(ARG)), (8))

void xcb_im_sync_fr_read(xcb_im_sync_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_sync_fr_write(xcb_im_sync_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_sync_fr_free(xcb_im_sync_fr_t *frame);
#define xcb_im_sync_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_sync_reply_fr_read(xcb_im_sync_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_sync_reply_fr_write(xcb_im_sync_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_sync_reply_fr_free(xcb_im_sync_reply_fr_t *frame);
#define xcb_im_sync_reply_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_commit_fr_read(xcb_im_commit_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_commit_fr_write(xcb_im_commit_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_commit_fr_free(xcb_im_commit_fr_t *frame);
size_t xcb_im_commit_fr_size(xcb_im_commit_fr_t *frame);

void xcb_im_commit_chars_fr_read(xcb_im_commit_chars_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_commit_chars_fr_write(xcb_im_commit_chars_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_commit_chars_fr_free(xcb_im_commit_chars_fr_t *frame);
size_t xcb_im_commit_chars_fr_size(xcb_im_commit_chars_fr_t *frame);

void xcb_im_commit_both_fr_read(xcb_im_commit_both_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_commit_both_fr_write(xcb_im_commit_both_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_commit_both_fr_free(xcb_im_commit_both_fr_t *frame);
size_t xcb_im_commit_both_fr_size(xcb_im_commit_both_fr_t *frame);

void xcb_im_reset_ic_fr_read(xcb_im_reset_ic_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_reset_ic_fr_write(xcb_im_reset_ic_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_reset_ic_fr_free(xcb_im_reset_ic_fr_t *frame);
#define xcb_im_reset_ic_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_reset_ic_reply_fr_read(xcb_im_reset_ic_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_reset_ic_reply_fr_write(xcb_im_reset_ic_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_reset_ic_reply_fr_free(xcb_im_reset_ic_reply_fr_t *frame);
size_t xcb_im_reset_ic_reply_fr_size(xcb_im_reset_ic_reply_fr_t *frame);

void xcb_im_geometry_fr_read(xcb_im_geometry_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_geometry_fr_write(xcb_im_geometry_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_geometry_fr_free(xcb_im_geometry_fr_t *frame);
#define xcb_im_geometry_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_str_conversion_fr_read(xcb_im_str_conversion_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_str_conversion_fr_write(xcb_im_str_conversion_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_str_conversion_fr_free(xcb_im_str_conversion_fr_t *frame);
size_t xcb_im_str_conversion_fr_size(xcb_im_str_conversion_fr_t *frame);

void xcb_im_str_conversion_reply_fr_read(xcb_im_str_conversion_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_str_conversion_reply_fr_write(xcb_im_str_conversion_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_str_conversion_reply_fr_free(xcb_im_str_conversion_reply_fr_t *frame);
size_t xcb_im_str_conversion_reply_fr_size(xcb_im_str_conversion_reply_fr_t *frame);

void xcb_im_preedit_start_fr_read(xcb_im_preedit_start_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_preedit_start_fr_write(xcb_im_preedit_start_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_preedit_start_fr_free(xcb_im_preedit_start_fr_t *frame);
#define xcb_im_preedit_start_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_preedit_start_reply_fr_read(xcb_im_preedit_start_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_preedit_start_reply_fr_write(xcb_im_preedit_start_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_preedit_start_reply_fr_free(xcb_im_preedit_start_reply_fr_t *frame);
#define xcb_im_preedit_start_reply_fr_size(ARG...) (((void)(ARG)), (8))

void xcb_im_preedit_draw_fr_read(xcb_im_preedit_draw_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_preedit_draw_fr_write(xcb_im_preedit_draw_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_preedit_draw_fr_free(xcb_im_preedit_draw_fr_t *frame);
size_t xcb_im_preedit_draw_fr_size(xcb_im_preedit_draw_fr_t *frame);

void xcb_im_preedit_caret_fr_read(xcb_im_preedit_caret_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_preedit_caret_fr_write(xcb_im_preedit_caret_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_preedit_caret_fr_free(xcb_im_preedit_caret_fr_t *frame);
#define xcb_im_preedit_caret_fr_size(ARG...) (((void)(ARG)), (16))

void xcb_im_preedit_caret_reply_fr_read(xcb_im_preedit_caret_reply_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_preedit_caret_reply_fr_write(xcb_im_preedit_caret_reply_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_preedit_caret_reply_fr_free(xcb_im_preedit_caret_reply_fr_t *frame);
#define xcb_im_preedit_caret_reply_fr_size(ARG...) (((void)(ARG)), (8))

void xcb_im_preedit_done_fr_read(xcb_im_preedit_done_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_preedit_done_fr_write(xcb_im_preedit_done_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_preedit_done_fr_free(xcb_im_preedit_done_fr_t *frame);
#define xcb_im_preedit_done_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_status_start_fr_read(xcb_im_status_start_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_status_start_fr_write(xcb_im_status_start_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_status_start_fr_free(xcb_im_status_start_fr_t *frame);
#define xcb_im_status_start_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_status_draw_text_fr_read(xcb_im_status_draw_text_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_status_draw_text_fr_write(xcb_im_status_draw_text_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_status_draw_text_fr_free(xcb_im_status_draw_text_fr_t *frame);
size_t xcb_im_status_draw_text_fr_size(xcb_im_status_draw_text_fr_t *frame);

void xcb_im_status_draw_bitmap_fr_read(xcb_im_status_draw_bitmap_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_status_draw_bitmap_fr_write(xcb_im_status_draw_bitmap_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_status_draw_bitmap_fr_free(xcb_im_status_draw_bitmap_fr_t *frame);
#define xcb_im_status_draw_bitmap_fr_size(ARG...) (((void)(ARG)), (12))

void xcb_im_status_done_fr_read(xcb_im_status_done_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_status_done_fr_write(xcb_im_status_done_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_status_done_fr_free(xcb_im_status_done_fr_t *frame);
#define xcb_im_status_done_fr_size(ARG...) (((void)(ARG)), (4))

void xcb_im_ext_set_event_mask_fr_read(xcb_im_ext_set_event_mask_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_ext_set_event_mask_fr_write(xcb_im_ext_set_event_mask_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_ext_set_event_mask_fr_free(xcb_im_ext_set_event_mask_fr_t *frame);
#define xcb_im_ext_set_event_mask_fr_size(ARG...) (((void)(ARG)), (24))

void xcb_im_ext_forward_keyevent_fr_read(xcb_im_ext_forward_keyevent_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_ext_forward_keyevent_fr_write(xcb_im_ext_forward_keyevent_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_ext_forward_keyevent_fr_free(xcb_im_ext_forward_keyevent_fr_t *frame);
#define xcb_im_ext_forward_keyevent_fr_size(ARG...) (((void)(ARG)), (20))

void xcb_im_ext_move_fr_read(xcb_im_ext_move_fr_t *frame, uint8_t **data, size_t *len, bool swap);
uint8_t* xcb_im_ext_move_fr_write(xcb_im_ext_move_fr_t *frame, uint8_t *data, bool swap);
void xcb_im_ext_move_fr_free(xcb_im_ext_move_fr_t *frame);
#define xcb_im_ext_move_fr_size(ARG...) (((void)(ARG)), (8))

XCBIMDKIT_DECL_END

#endif

// IWYU pragma: private, include "ximproto.h"
