// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINPUTV3_H
#define TEXTINPUTV3_H

#include "Listener.h"

extern "C" {
#include <wlr/types/wlr_text_input_v3.h>
}

class Server;
class InputMethodV2;

class TextInputV3
{
    friend class Server;
    friend class InputMethodV2;

public:
    TextInputV3(Server *server, wlr_text_input_v3 *text_input, wl_list *list);
    ~TextInputV3();

    wlr_text_input_v3 *raw() { return text_input_; }

private:
    void enableNotify(void *data);
    void commitNotify(void *data);
    void disableNotify(void *data);
    void destroyNotify(void *data);

    void sendIMState(InputMethodV2 *inputMethod);
    void disableTextInput(InputMethodV2 *inputMethod);

public:
    struct
    {
        struct wl_signal cursorRectangle;
    } events;

private:
    Server *server_;
    wl_list link_;
    wlr_text_input_v3 *text_input_;

    Listener<&TextInputV3::enableNotify> enable_;
    Listener<&TextInputV3::commitNotify> commit_;
    Listener<&TextInputV3::disableNotify> disable_;
    Listener<&TextInputV3::destroyNotify> destroy_;
};

#endif // !TEXTINPUTV3_H
