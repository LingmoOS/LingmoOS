// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_ZWPINPUTMETHODV2_H
#define WL_CLIENT_ZWPINPUTMETHODV2_H

#include "Type.h"

#include <memory>

namespace wl {
namespace client {

class Surface;
class ZwpInputMethodKeyboardGrabV2;

class ZwpInputMethodV2 : public Type<zwp_input_method_v2>
{
public:
    ZwpInputMethodV2(zwp_input_method_v2 *val);
    virtual ~ZwpInputMethodV2();

    zwp_input_method_keyboard_grab_v2 *grabKeyboard();

    void commit_string(const char *text);
    void set_preedit_string(const char *text, int32_t cursorBegin, int32_t cursorEnd);
    void commit(uint32_t serial);
    zwp_input_popup_surface_v2 *get_input_popup_surface(wl_surface *surface);

protected:
    virtual void zwp_input_method_v2_activate() = 0;
    virtual void zwp_input_method_v2_deactivate() = 0;
    virtual void zwp_input_method_v2_surrounding_text(const char *text, uint32_t cursor, uint32_t anchor) = 0;
    virtual void zwp_input_method_v2_text_change_cause(uint32_t cause) = 0;
    virtual void zwp_input_method_v2_content_type(uint32_t hint, uint32_t purpose) = 0;
    virtual void zwp_input_method_v2_done() = 0;
    virtual void zwp_input_method_v2_unavailable() = 0;

private:
    static const zwp_input_method_v2_listener listener_;
};

} // namespace client
} // namespace wl

#endif // !WL_CLIENT_ZWPINPUTMETHODV2_H
