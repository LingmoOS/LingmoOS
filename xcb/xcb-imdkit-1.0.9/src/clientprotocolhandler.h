/*
 * SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */
#ifndef CLIENTPROTOCOLHANDLER_H
#define CLIENTPROTOCOLHANDLER_H
#include "imclient.h"
#include "ximproto.h"
#include <stdint.h>

void _xcb_xim_handle_open_reply(xcb_xim_t *im,
                                const xcb_im_packet_header_fr_t *hdr,
                                uint8_t *data);
void _xcb_xim_handle_query_extension_reply(xcb_xim_t *im,
                                           const xcb_im_packet_header_fr_t *hdr,
                                           uint8_t *data);
void _xcb_xim_handle_encoding_negotiation_reply(
    xcb_xim_t *im, const xcb_im_packet_header_fr_t *hdr, uint8_t *data);
void _xcb_xim_handle_create_ic_reply(xcb_xim_t *im,
                                     const xcb_im_packet_header_fr_t *hdr,
                                     uint8_t *data);
void _xcb_xim_handle_destroy_ic_reply(xcb_xim_t *im,
                                      const xcb_im_packet_header_fr_t *hdr,
                                      uint8_t *data);
void _xcb_xim_handle_get_im_values_reply(xcb_xim_t *im,
                                         const xcb_im_packet_header_fr_t *hdr,
                                         uint8_t *data);
void _xcb_xim_handle_register_triggerkeys(xcb_xim_t *im,
                                          const xcb_im_packet_header_fr_t *hdr,
                                          uint8_t *data);
void _xcb_xim_handle_get_ic_values_reply(xcb_xim_t *im,
                                         const xcb_im_packet_header_fr_t *hdr,
                                         uint8_t *data);
void _xcb_xim_handle_set_ic_values_reply(xcb_xim_t *im,
                                         const xcb_im_packet_header_fr_t *hdr,
                                         uint8_t *data);
void _xcb_xim_handle_reset_ic_reply(xcb_xim_t *im,
                                    const xcb_im_packet_header_fr_t *hdr,
                                    uint8_t *data);
void _xcb_xim_handle_forward_event(xcb_xim_t *im,
                                   const xcb_im_packet_header_fr_t *hdr,
                                   uint8_t *data);
void _xcb_xim_handle_set_event_mask(xcb_xim_t *im,
                                    const xcb_im_packet_header_fr_t *hdr,
                                    uint8_t *data);
void _xcb_xim_handle_sync(xcb_xim_t *im, const xcb_im_packet_header_fr_t *hdr,
                          uint8_t *data);
void _xcb_xim_handle_commit(xcb_xim_t *im, const xcb_im_packet_header_fr_t *hdr,
                            uint8_t *data);
void _xcb_xim_handle_geometry(xcb_xim_t *im,
                              const xcb_im_packet_header_fr_t *hdr,
                              uint8_t *data);
void _xcb_xim_handle_preedit_start(xcb_xim_t *im,
                                   const xcb_im_packet_header_fr_t *hdr,
                                   uint8_t *data);
void _xcb_xim_handle_preedit_draw(xcb_xim_t *im,
                                  const xcb_im_packet_header_fr_t *hdr,
                                  uint8_t *data);
void _xcb_xim_handle_preedit_caret(xcb_xim_t *im,
                                   const xcb_im_packet_header_fr_t *hdr,
                                   uint8_t *data);
void _xcb_xim_handle_preedit_done(xcb_xim_t *im,
                                  const xcb_im_packet_header_fr_t *hdr,
                                  uint8_t *data);
void _xcb_xim_handle_status_start(xcb_xim_t *im,
                                  const xcb_im_packet_header_fr_t *hdr,
                                  uint8_t *data);
void _xcb_xim_handle_status_draw(xcb_xim_t *im,
                                 const xcb_im_packet_header_fr_t *hdr,
                                 uint8_t *data);
void _xcb_xim_handle_status_done(xcb_xim_t *im,
                                 const xcb_im_packet_header_fr_t *hdr,
                                 uint8_t *data);
void _xcb_xim_handle_close_reply(xcb_xim_t *im,
                                 const xcb_im_packet_header_fr_t *hdr,
                                 uint8_t *data);
void _xcb_xim_handle_error(xcb_xim_t *im, uint8_t *data);

#endif // CLIENTPROTOCOLHANDLER_H
