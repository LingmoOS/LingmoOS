/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#ifndef _XCB_IMDKIT_IMCLIENT_H_
#define _XCB_IMDKIT_IMCLIENT_H_

#include "ximcommon.h"
#include "ximproto.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

XCBIMDKIT_DECL_BEGIN

typedef struct _xcb_xim_t xcb_xim_t;
typedef uint16_t xcb_xic_t;

typedef struct _xcb_xim_nested_list {
    uint8_t *data;
    size_t length;
} xcb_xim_nested_list;

typedef void (*xcb_xim_set_event_mask_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                                uint32_t forward_event_mask,
                                                uint32_t synchronous_event_mask,
                                                void *user_data);
typedef void (*xcb_xim_forward_event_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                               xcb_key_press_event_t *event,
                                               void *user_data);
typedef void (*xcb_xim_commit_string_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                               uint32_t flag, char *str,
                                               uint32_t length,
                                               uint32_t *keysym, size_t nKeySym,
                                               void *user_data);
typedef void (*xcb_xim_geometry_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                          void *user_data);
typedef void (*xcb_xim_preedit_start_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                               void *user_data);
typedef void (*xcb_xim_preedit_draw_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                              xcb_im_preedit_draw_fr_t *frame,
                                              void *user_data);
typedef void (*xcb_xim_preedit_caret_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                               xcb_im_preedit_caret_fr_t *frame,
                                               void *user_data);
typedef void (*xcb_xim_preedit_done_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                              void *user_data);
typedef void (*xcb_xim_status_start_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                              void *user_data);
typedef void (*xcb_xim_status_draw_text_callback)(
    xcb_xim_t *im, xcb_xic_t ic, xcb_im_status_draw_text_fr_t *frame,
    void *user_data);
typedef void (*xcb_xim_status_draw_bitmap_callback)(
    xcb_xim_t *im, xcb_xic_t ic, xcb_im_status_draw_bitmap_fr_t *frame,
    void *user_data);
typedef void (*xcb_xim_status_done_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                             void *user_data);
typedef void (*xcb_xim_sync_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                      void *user_data);
typedef void (*xcb_xim_disconnected_callback)(xcb_xim_t *im, void *user_data);

typedef struct _xcb_xim_im_callback {
    xcb_xim_set_event_mask_callback set_event_mask;
    xcb_xim_forward_event_callback forward_event;
    xcb_xim_commit_string_callback commit_string;
    xcb_xim_geometry_callback geometry;
    xcb_xim_preedit_start_callback preedit_start;
    xcb_xim_preedit_draw_callback preedit_draw;
    xcb_xim_preedit_caret_callback preedit_caret;
    xcb_xim_preedit_done_callback preedit_done;
    xcb_xim_status_start_callback status_start;
    xcb_xim_status_draw_text_callback status_draw_text;
    xcb_xim_status_draw_bitmap_callback status_draw_bitmap;
    xcb_xim_status_done_callback status_done;
    xcb_xim_sync_callback sync;
    xcb_xim_disconnected_callback disconnected;
} xcb_xim_im_callback;

typedef enum _xcb_xim_trigger_key_type_t {
    XCB_XIM_IS_NOT_TRIGGER,
    XCB_XIM_TRIGGER_ON_KEY,
    XCB_XIM_TRIGGER_OFF_KEY,
} xcb_xim_trigger_key_type_t;

typedef enum _xcb_xim_encoding_t {
    XCB_XIM_COMPOUND_TEXT,
    XCB_XIM_UTF8_STRING,
} xcb_xim_encoding_t;

typedef void (*xcb_xim_open_callback)(xcb_xim_t *im, void *user_data);
typedef void (*xcb_xim_create_ic_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                           void *user_data);
typedef void (*xcb_xim_get_im_values_callback)(
    xcb_xim_t *im, xcb_im_get_im_values_reply_fr_t *reply, void *user_data);
typedef void (*xcb_xim_get_ic_values_callback)(
    xcb_xim_t *im, xcb_xic_t ic, xcb_im_get_ic_values_reply_fr_t *reply,
    void *user_data);
typedef void (*xcb_xim_set_ic_values_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                               void *user_data);
typedef void (*xcb_xim_destroy_ic_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                            void *user_data);
typedef void (*xcb_xim_reset_ic_callback)(xcb_xim_t *im, xcb_xic_t ic,
                                          xcb_im_reset_ic_reply_fr_t *reply,
                                          void *user_data);

XCBIMDKIT_EXPORT xcb_xim_t *xcb_xim_create(xcb_connection_t *conn,
                                           int screen_id, const char *imname);
XCBIMDKIT_EXPORT void
xcb_xim_set_log_handler(xcb_xim_t *im, void (*logger)(const char *, ...));
XCBIMDKIT_EXPORT void xcb_xim_set_use_compound_text(xcb_xim_t *im, bool enable);
XCBIMDKIT_EXPORT void xcb_xim_set_use_utf8_string(xcb_xim_t *im, bool enable);
XCBIMDKIT_EXPORT
xcb_xim_encoding_t xcb_xim_get_encoding(xcb_xim_t *im);
XCBIMDKIT_EXPORT void xcb_xim_destroy(xcb_xim_t *im);
XCBIMDKIT_EXPORT bool xcb_xim_open(xcb_xim_t *im,
                                   xcb_xim_open_callback callback,
                                   bool auto_connect, void *user_data);
XCBIMDKIT_EXPORT void
xcb_xim_set_im_callback(xcb_xim_t *im, const xcb_xim_im_callback *callbacks,
                        void *user_data);
XCBIMDKIT_EXPORT void
xcb_xim_set_im_callback(xcb_xim_t *im, const xcb_xim_im_callback *callbacks,
                        void *user_data);
XCBIMDKIT_EXPORT bool xcb_xim_filter_event(xcb_xim_t *im,
                                           xcb_generic_event_t *event);

XCBIMDKIT_DEPRECATED_EXPORT xcb_xim_trigger_key_type_t
xcb_xim_check_trigger_key(xcb_xim_t *im, xcb_keysym_t keysym, uint32_t modifier,
                          uint32_t *idx);
XCBIMDKIT_EXPORT bool xcb_xim_check_trigger_on_key(xcb_xim_t *im,
                                                   xcb_keysym_t keysym,
                                                   uint32_t modifier,
                                                   uint32_t *idx);
XCBIMDKIT_EXPORT bool xcb_xim_check_trigger_off_key(xcb_xim_t *im,
                                                    xcb_keysym_t keysym,
                                                    uint32_t modifier,
                                                    uint32_t *idx);
XCBIMDKIT_EXPORT bool xcb_xim_trigger_notify(xcb_xim_t *im, xcb_xic_t ic,
                                             uint32_t idx, bool off);

XCBIMDKIT_EXPORT bool
xcb_xim_get_im_values(xcb_xim_t *im, xcb_xim_get_im_values_callback callback,
                      void *user_data, ...);
XCBIMDKIT_EXPORT xcb_xim_nested_list xcb_xim_create_nested_list(xcb_xim_t *im,
                                                                ...);
XCBIMDKIT_EXPORT bool xcb_xim_create_ic(xcb_xim_t *im,
                                        xcb_xim_create_ic_callback callback,
                                        void *user_data, ...);
XCBIMDKIT_EXPORT bool
xcb_xim_get_ic_values(xcb_xim_t *im, xcb_xic_t ic,
                      xcb_xim_get_ic_values_callback callback, void *user_data,
                      ...);
XCBIMDKIT_EXPORT bool
xcb_xim_set_ic_values(xcb_xim_t *im, xcb_xic_t ic,
                      xcb_xim_set_ic_values_callback callback, void *user_data,
                      ...);
XCBIMDKIT_EXPORT bool xcb_xim_set_ic_focus(xcb_xim_t *im, xcb_xic_t ic);
XCBIMDKIT_EXPORT bool xcb_xim_unset_ic_focus(xcb_xim_t *im, xcb_xic_t ic);
XCBIMDKIT_EXPORT bool xcb_xim_forward_event(xcb_xim_t *im, xcb_xic_t ic,
                                            xcb_key_press_event_t *event);
XCBIMDKIT_EXPORT bool xcb_xim_forward_event_full(xcb_xim_t *im, xcb_xic_t ic,
                                                 uint16_t sequence,
                                                 xcb_key_press_event_t *event);
XCBIMDKIT_EXPORT bool xcb_xim_reset_ic(xcb_xim_t *im, xcb_xic_t ic,
                                       xcb_xim_reset_ic_callback callback,
                                       void *user_data);
XCBIMDKIT_EXPORT bool xcb_xim_destroy_ic(xcb_xim_t *im, xcb_xic_t ic,
                                         xcb_xim_destroy_ic_callback callback,
                                         void *user_data);
XCBIMDKIT_EXPORT void xcb_xim_close(xcb_xim_t *im);
XCBIMDKIT_EXPORT bool xcb_xim_support_extension(xcb_xim_t *im,
                                                uint16_t major_code,
                                                uint16_t minor_code);
XCBIMDKIT_EXPORT bool xcb_xim_ext_move(xcb_xim_t *im, xcb_xic_t ic, int16_t x,
                                       int16_t y);

XCBIMDKIT_DECL_END

#endif // _XCB_IMDKIT_IMCLIENT_H_
