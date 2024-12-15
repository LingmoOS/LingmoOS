// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTMETHODV2_H
#define INPUTMETHODV2_H

#include "Listener.h"

#include <functional>
#include <memory>

extern "C" {
#define delete delete_
#include <wlr/types/wlr_input_method_v2.h>
#undef delete
}

WL_ADDONS_BASE_BEGIN_NAMESPACE

class Server;

class InputMethodV2
{
    friend class Keyboard;

public:
    InputMethodV2(Server *server, wlr_input_method_v2 *input_method);
    ~InputMethodV2();

    bool is(wlr_input_method_v2 *im2) { return input_method_ == im2; }

    void setCommitCallback(const std::function<void()> &callback) { commitCallback_ = callback; }

    void setPopupCreateCallback(const std::function<void()> &callback)
    {
        popupCreateCallback_ = callback;
    }

    void setPopupDestroyCallback(const std::function<void()> &callback)
    {
        popupDestroyCallback_ = callback;
    }

    void sendActivate();
    void sendDeactivate();
    void sendContentType(uint32_t hint, uint32_t purpose);
    void sendSurroundingText(const char *text, uint32_t cursor, uint32_t anchor);
    void sendDone();
    void setCursorRectangle(int x, int y, int width, int height);
    void sendKey(uint32_t keycode, bool isRelease);

    wlr_input_method_v2_preedit_string &preeditString() { return input_method_->current.preedit; }

    char *commitText() { return input_method_->current.commit_text; }

    wlr_input_method_v2_delete_surrounding_text &deleteSurroundingText()
    {
        return input_method_->current.delete_;
    }

private:
    void commitNotify(void *data);
    void newPopupSurfaceNotify(void *data);
    void grabKeyboardNotify(void *data);
    void destroyNotify(void *data);
    void popupDestroyNotify(void *data);
    void popupSurfaceCommitNotify(void *data);
    void keyboardGrabDestroyNotify(void *data);
    void grabberKeyNotify(void *data);
    void grabberModifiersNotify(void *data);

private:
    Server *server_;
    wlr_input_method_v2 *input_method_;

    Listener<&InputMethodV2::commitNotify> commit_;
    Listener<&InputMethodV2::newPopupSurfaceNotify> new_popup_surface_;
    Listener<&InputMethodV2::grabKeyboardNotify> grab_keyboard_;
    Listener<&InputMethodV2::destroyNotify> destroy_;

    wlr_input_popup_surface_v2 *popup_ = nullptr;
    Listener<&InputMethodV2::popupDestroyNotify> popup_destroy_;
    Listener<&InputMethodV2::popupSurfaceCommitNotify> popup_surface_commit_;

    Listener<&InputMethodV2::keyboardGrabDestroyNotify> keyboard_grab_destroy_;

    std::function<void()> commitCallback_;
    std::function<void()> popupCreateCallback_;
    std::function<void()> popupDestroyCallback_;
};

WL_ADDONS_BASE_END_NAMESPACE

#endif // !INPUTMETHODV2_H
