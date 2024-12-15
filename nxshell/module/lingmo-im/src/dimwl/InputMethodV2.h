// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTMETHODV2_H
#define INPUTMETHODV2_H

#include "Listener.h"
#include <memory>

extern "C" {
#define delete delete_
#include <wlr/types/wlr_input_method_v2.h>
#undef delete
}

class Server;
class TextInputV3;
class X11KeyboardGrabber;

class InputMethodV2
{
    friend class TextInputV3;
    friend class Keyboard;;

public:
    InputMethodV2(Server *server, wlr_input_method_v2 *input_method);
    ~InputMethodV2();

    void setCursorRectangle(wlr_box *rectangle);

private:
    void commitNotify(void *data);
    void newPopupSurfaceNotify(void *data);
    void grabKeyboardNotify(void *data);
    void destroyNotify(void *data);
    void popupDestroyNotify(void *data);
    void keyboardGrabDestroyNotify(void *data);
    void grabberKeyNotify(void *data);
    void grabberModifiersNotify(void *data);

    TextInputV3 *getFocusedTextInput() const;

private:
    Server *server_;
    wlr_input_method_v2 *input_method_;

    Listener<&InputMethodV2::commitNotify> commit_;
    Listener<&InputMethodV2::newPopupSurfaceNotify> new_popup_surface_;
    Listener<&InputMethodV2::grabKeyboardNotify> grab_keyboard_;
    Listener<&InputMethodV2::destroyNotify> destroy_;

    wlr_input_popup_surface_v2 *popup_ = nullptr;
    Listener<&InputMethodV2::popupDestroyNotify> popup_destroy_;

    Listener<&InputMethodV2::keyboardGrabDestroyNotify> keyboard_grab_destroy_;

    std::unique_ptr<X11KeyboardGrabber> grabber_;
    Listener<&InputMethodV2::grabberKeyNotify> grabberKey_;
    Listener<&InputMethodV2::grabberModifiersNotify> grabberModifiers_;
};

#endif // !INPUTMETHODV2_H
