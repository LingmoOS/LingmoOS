// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_ZWPTEXTINPUTV3_H
#define WL_CLIENT_ZWPTEXTINPUTV3_H

#include "Type.h"
#include "wayland-text-input-unstable-v3-client-protocol.h"

namespace wl {
namespace client {

class ZwpTextInputV3 : public Type<zwp_text_input_v3>
{
public:
    ZwpTextInputV3(zwp_text_input_v3 *val);
    ~ZwpTextInputV3();

    void enable();
    void disable();
    void set_surrounding_text(const char *text, int32_t cursor, int32_t anchor);
    void set_text_change_cause(uint32_t cause);
    void set_content_type(uint32_t hint, uint32_t purpose);
    void set_cursor_rectangle(int32_t x, int32_t y, int32_t width, int32_t height);
    void commit();

protected:
    virtual void zwp_text_input_v3_enter(struct wl_surface *surface) = 0;
    virtual void zwp_text_input_v3_leave(struct wl_surface *surface) = 0;
    virtual void zwp_text_input_v3_preedit_string(const char *text, int32_t cursor_begin, int32_t cursor_end) = 0;
    virtual void zwp_text_input_v3_commit_string(const char *text) = 0;
    virtual void zwp_text_input_v3_delete_surrounding_text(uint32_t before_length, uint32_t after_length) = 0;
    virtual void zwp_text_input_v3_done(uint32_t serial) = 0;

private:
    static const zwp_text_input_v3_listener listener_;
};

} // namespace client
} // namespace wl

#endif // !WL_CLIENT_ZWPTEXTINPUTV3_H
