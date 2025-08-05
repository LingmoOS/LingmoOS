/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#ifndef _XCB_IMDKIT_IMDKIT_H_
#define _XCB_IMDKIT_IMDKIT_H_

#include "xcbimdkit_export.h"
#include "ximcommon.h"
#include "ximproto.h"
#include <stdbool.h>
#include <stdint.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

XCBIMDKIT_DECL_BEGIN

/*
 * `C' and `no' are additional one which cannot be obtained from modern
 * locale.gen. `no' is obsolete, but we keep it here for compatible reason.
 */
#define XCB_IM_ALL_LOCALES                                                     \
    "aa,af,am,an,ar,as,ast,az,be,bem,ber,bg,bho,bn,bo,br,brx,bs,byn,"          \
    "C,ca,crh,cs,csb,cv,cy,da,de,dv,dz,el,en,es,et,eu,fa,ff,fi,fil,fo,"        \
    "fr,fur,fy,ga,gd,gez,gl,gu,gv,ha,he,hi,hne,hr,hsb,ht,hu,hy,id,ig,"         \
    "ik,is,it,iu,iw,ja,ka,kk,kl,km,kn,ko,kok,ks,ku,kw,ky,lb,lg,li,lij,"        \
    "lo,lt,lv,mag,mai,mg,mhr,mi,mk,ml,mn,mr,ms,mt,my,nan,nb,nds,ne,nl,"        \
    "nn,no,nr,nso,oc,om,or,os,pa,pap,pl,ps,pt,ro,ru,rw,sa,sc,sd,se,shs,"       \
    "si,sid,sk,sl,so,sq,sr,ss,st,sv,sw,ta,te,tg,th,ti,tig,tk,tl,tn,tr,"        \
    "ts,tt,ug,uk,unm,ur,uz,ve,vi,wa,wae,wal,wo,xh,yi,yo,yue,zh,zu"

typedef struct _xcb_im_styles_t {
    uint32_t nStyles;
    uint32_t *styles;
} xcb_im_styles_t;

typedef char *xcb_im_encoding_t;

typedef struct _xcb_im_encodings_t {
    unsigned short nEncodings;
    xcb_im_encoding_t *encodings;
} xcb_im_encodings_t;

typedef struct _xcb_im_t xcb_im_t;
typedef struct _xcb_im_input_context_t xcb_im_input_context_t;
typedef struct _xcb_im_client_t xcb_im_client_t;

// Mask to check if the attribute is set.
typedef enum _xcb_im_attr_mask_t {
    XCB_XIM_XNArea_MASK = (1 << 0),
    XCB_XIM_XNAreaNeeded_MASK = (1 << 1),
    XCB_XIM_XNSpotLocation_MASK = (1 << 2),
    XCB_XIM_XNColormap_MASK = (1 << 3),
    XCB_XIM_XNForeground_MASK = (1 << 4),
    XCB_XIM_XNBackground_MASK = (1 << 5),
    XCB_XIM_XNBackgroundPixmap_MASK = (1 << 7),
    XCB_XIM_XNLineSpace_MASK = (1 << 8),
} xcb_im_attr_mask_t;

typedef struct _xcb_im_preedit_attr_t {
    xcb_rectangle_t area;
    xcb_rectangle_t area_needed;
    xcb_point_t spot_location;
    xcb_colormap_t colormap;
    uint32_t foreground;
    uint32_t background;
    xcb_window_t bg_pixmap;
    uint32_t line_space;
} xcb_im_preedit_attr_t;

typedef struct _xcb_im_status_attr_t {
    xcb_rectangle_t area;
    xcb_rectangle_t area_needed;
    xcb_point_t spot_location;
    xcb_colormap_t colormap;
    uint32_t foreground;
    uint32_t background;
    xcb_window_t bg_pixmap;
    uint32_t line_space;
} xcb_im_status_attr_t;

typedef void (*xcb_im_callback)(xcb_im_t *im, xcb_im_client_t *client,
                                xcb_im_input_context_t *ic,
                                const xcb_im_packet_header_fr_t *hdr,
                                void *frame, void *arg, void *user_data);

typedef void (*xcb_im_free_function)(void *memory);

/**
 * Create a XIM server.
 *
 * @param conn xcb connection to be used.
 * @param screen xcb screen to be used.
 * @param serverWindow A server window.
 * @param serverName server name, need to be consistent with XMODIFIERS
 * @param locale locale supported. You may want to use XCB_IM_ALL_LOCALES.
 * @param inputStyles XIM Styles supported by XIM server.
 * @param onKeysList Trigger on key to send to client.
 * @param offKeysList Trigger off key to send to cilent.
 * @param encodingList XIM encoding list.
 * @param event_mask if 0, XCB_EVENT_MASK_KEY_PRESS will be used.
 * @param callback Callback function
 * @param user_data user data to callback function.
 * @return xcb_im_t*
 */
XCBIMDKIT_EXPORT xcb_im_t *
xcb_im_create(xcb_connection_t *conn, int screen, xcb_window_t serverWindow,
              const char *serverName, const char *locale,
              const xcb_im_styles_t *inputStyles,
              const xcb_im_trigger_keys_t *onKeysList,
              const xcb_im_trigger_keys_t *offKeysList,
              const xcb_im_encodings_t *encodingList, uint32_t event_mask,
              xcb_im_callback callback, void *user_data);

/**
 * @brief Set a logger handler.
 *
 * @param im XIM server.
 * @param logger logger function.
 */
XCBIMDKIT_EXPORT void xcb_im_set_log_handler(xcb_im_t *im,
                                             void (*logger)(const char *, ...));

/**
 * Whether to use sync mode, it will affect certain behavior of XIM.
 *
 * Forward event and commit string behavior will be changed. If sync mode is
 * true, every request need to be replied with sync_reply. The library is still
 * working under async mode, just no more request will be send to client.
 *
 * @param im XIM server
 * @param sync sync mode or not.
 */
XCBIMDKIT_EXPORT void xcb_im_set_use_sync_mode(xcb_im_t *im, bool sync);

/**
 * Set whether the event defined by event mask is handled in a synchronous way.
 *
 * @param im XIM server
 * @param sync sync on event or not.
 */
XCBIMDKIT_EXPORT void xcb_im_set_use_sync_event(xcb_im_t *im, bool sync);

/**
 * Start a XIM server synchronously.
 *
 * It only does the minimum initialization and try to grab the server name. When
 * it fails, it means there might be another server with the same name running.
 *
 * To finish the initialization, you will need to use xcb_im_filter_event on all
 * the event recevied.
 *
 * You may also call this function again if it fails or after xcb_im_close_im.
 *
 * @param im XIM server.
 * @return whether XIM server is started successfully.
 */
XCBIMDKIT_EXPORT bool xcb_im_open_im(xcb_im_t *im);

/**
 * Handle XIM related event, most relevant event will be client message.
 *
 * @param im XIM server
 * @param event X event.
 * @return Whether the event is handled by XIM.
 */
XCBIMDKIT_EXPORT bool xcb_im_filter_event(xcb_im_t *im,
                                          xcb_generic_event_t *event);

/**
 * Shutdown the XIM server and free all the resources.
 *
 * @param im XIM server
 *
 * @see xcb_im_open_im
 */
XCBIMDKIT_EXPORT void xcb_im_close_im(xcb_im_t *im);
/**
 * Destroy the XIM server.
 *
 * xcb_im_close_im need to be called if it is opened successfully.
 *
 * @param im XIM server.
 */
XCBIMDKIT_EXPORT void xcb_im_destroy(xcb_im_t *im);

/**
 * Send a key event to the client.
 *
 * @param im XIM server
 * @param ic Input context.
 * @param event key event.
 */
XCBIMDKIT_EXPORT void xcb_im_forward_event(xcb_im_t *im,
                                           xcb_im_input_context_t *ic,
                                           xcb_key_press_event_t *event);

/**
 * Send a key event to the client.
 *
 * @param im XIM server
 * @param ic Input context.
 * @param sequence sequence number, when re-inject, should be the value from
 * forward_event
 * @param event key event.
 * @see xcb_im_forward_event_fr_t.
 */
XCBIMDKIT_EXPORT void xcb_im_forward_event_full(xcb_im_t *im,
                                                xcb_im_input_context_t *ic,
                                                uint16_t sequence,
                                                xcb_key_press_event_t *event);

/**
 * Commit a string to the client.
 *
 * @param im XIM server
 * @param ic Input Context
 * @param flag a bit flag of xcb_xim_lookup_flags_t, XCB_XIM_LOOKUP_CHARS is the
 * most common value to be used.
 * @param str string to be committed, encoding is usually COMPOUND_TEXT.
 * @param length byte length of the string
 * @param keysym key symbol.
 *
 * @see xcb_xim_lookup_flags_t
 * @see xcb_utf8_to_compound_text
 */
XCBIMDKIT_EXPORT void xcb_im_commit_string(xcb_im_t *im,
                                           xcb_im_input_context_t *ic,
                                           uint32_t flag, const char *str,
                                           uint32_t length, uint32_t keysym);

/**
 * Start geometry negotiation, if XIMStyle has XIMPreeditArea or XIMStatusArea
 * set.
 *
 * This is rarely used nowadays. Xlib doesn't have relevant code for it.
 *
 * @param im XIM server
 * @param ic Input context
 */
XCBIMDKIT_EXPORT void xcb_im_geometry_callback(xcb_im_t *im,
                                               xcb_im_input_context_t *ic);

/**
 * Sends XIM_PREEDIT_START message to call the XIMPreeditStartCallback function.
 *
 * @param im XIM server
 * @param ic Input context
 */
XCBIMDKIT_EXPORT void xcb_im_preedit_start_callback(xcb_im_t *im,
                                                    xcb_im_input_context_t *ic);

/**
 * Sends XIM_PREEDIT_DRAW message to call the XIMPreeditDrawCallback function.
 *
 * @param im XIM server
 * @param ic Input context
 * @param frame information about preedit string.
 */
XCBIMDKIT_EXPORT void
xcb_im_preedit_draw_callback(xcb_im_t *im, xcb_im_input_context_t *ic,
                             xcb_im_preedit_draw_fr_t *frame);

/**
 * Sends XIM_PREEDIT_CARET message to call the PreeditCaretCallback function.
 *
 * @param im XIM server
 * @param ic Input context
 * @param frame information about preedit caret.
 */
XCBIMDKIT_EXPORT void
xcb_im_preedit_caret_callback(xcb_im_t *im, xcb_im_input_context_t *ic,
                              xcb_im_preedit_caret_fr_t *frame);

/**
 * sends XIM_PREEDIT_DONE message to call the XIMPreeditDoneCallback function.
 *
 * This should only be called after calling xcb_im_preedit_start_callback.
 *
 * @param im XIM server
 * @param ic Input context
 */
XCBIMDKIT_EXPORT void xcb_im_preedit_done_callback(xcb_im_t *im,
                                                   xcb_im_input_context_t *ic);

/**
 * Sends XIM_STATUS_START message to call the XIMStatusStartCallback function.
 *
 * @param im XIM server
 * @param ic Input context
 */
XCBIMDKIT_EXPORT void xcb_im_status_start_callback(xcb_im_t *im,
                                                   xcb_im_input_context_t *ic);

/**
 * Sends XIM_STATUS_DRAW message to call the XIMStatusDrawCallback function with
 * text.
 *
 * @param im XIM server
 * @param ic Input context
 * @param frame text to be drawn by client.
 */
XCBIMDKIT_EXPORT void
xcb_im_status_draw_text_callback(xcb_im_t *im, xcb_im_input_context_t *ic,
                                 xcb_im_status_draw_text_fr_t *frame);

/**
 * Sends XIM_STATUS_DRAW message to call the XIMStatusDrawCallback function with
 * bitmap.
 *
 * @param im XIM server
 * @param ic Input context
 * @param frame bitmap to be drawn by client.
 */
XCBIMDKIT_EXPORT void
xcb_im_status_draw_bitmap_callback(xcb_im_t *im, xcb_im_input_context_t *ic,
                                   xcb_im_status_draw_bitmap_fr_t *frame);

/**
 * Sends XIM_STATUS_DONE message to call the XIMStatusDoneCallback function.
 *
 * @param im XIM server
 * @param ic Input context
 */
XCBIMDKIT_EXPORT void xcb_im_status_done_callback(xcb_im_t *im,
                                                  xcb_im_input_context_t *ic);
XCBIMDKIT_EXPORT void xcb_im_preedit_start(xcb_im_t *im,
                                           xcb_im_input_context_t *ic);
XCBIMDKIT_EXPORT void xcb_im_preedit_end(xcb_im_t *im,
                                         xcb_im_input_context_t *ic);
XCBIMDKIT_EXPORT void xcb_im_sync_xlib(xcb_im_t *im,
                                       xcb_im_input_context_t *ic);
XCBIMDKIT_EXPORT bool xcb_im_support_extension(xcb_im_t *im,
                                               uint16_t major_code,
                                               uint16_t minor_code);
XCBIMDKIT_EXPORT void
xcb_im_input_context_set_data(xcb_im_input_context_t *ic, void *data,
                              xcb_im_free_function free_data_function);
XCBIMDKIT_EXPORT void *
xcb_im_input_context_get_data(xcb_im_input_context_t *ic);
XCBIMDKIT_EXPORT uint32_t
xcb_im_input_context_get_input_style(xcb_im_input_context_t *ic);
XCBIMDKIT_EXPORT xcb_window_t
xcb_im_input_context_get_client_window(xcb_im_input_context_t *ic);
XCBIMDKIT_EXPORT xcb_window_t
xcb_im_input_context_get_focus_window(xcb_im_input_context_t *ic);
XCBIMDKIT_EXPORT const xcb_im_preedit_attr_t *
xcb_im_input_context_get_preedit_attr(xcb_im_input_context_t *ic);
XCBIMDKIT_EXPORT const xcb_im_status_attr_t *
xcb_im_input_context_get_status_attr(xcb_im_input_context_t *ic);
XCBIMDKIT_EXPORT uint32_t
xcb_im_input_context_get_preedit_attr_mask(xcb_im_input_context_t *ic);
XCBIMDKIT_EXPORT uint32_t
xcb_im_input_context_get_status_attr_mask(xcb_im_input_context_t *ic);

XCBIMDKIT_DECL_END

#endif // _XCB_IMDKIT_IMDKIT_H_
