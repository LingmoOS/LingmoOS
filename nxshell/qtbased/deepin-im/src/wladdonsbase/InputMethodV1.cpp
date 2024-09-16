// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InputMethodV1.h"

#include "InputMethodContextV1.h"
#include "Server.h"

WL_ADDONS_BASE_USE_NAMESPACE

InputMethodV1::InputMethodV1(Server *server)
    : server_(server)
{
    const auto display = server->display();
    global(display);

    inputPanelV1_.reset(new ZwpInputPanelV1(this));
    inputPanelV1_->global(display);
}

InputMethodV1::~InputMethodV1() = default;

void InputMethodV1::sendActivate()
{
    if (!getResource()) {
        return;
    }

    if (inputMethodContextV1_) {
        return;
    }

    inputMethodContextV1_.reset(new InputMethodContextV1(this));

    if (!getResource()->handle || !inputMethodContextV1_->getResource()->handle) {
        return;
    }

    zwp_input_method_v1_send_activate(getResource()->handle,
                                      inputMethodContextV1_->getResource()->handle);
}

void InputMethodV1::sendDeactivate()
{
    if (!getResource()->handle || inputMethodContextV1_->getResource()->handle) {
        return;
    }

    zwp_input_method_v1_send_deactivate(getResource()->handle,
                                        inputMethodContextV1_->getResource()->handle);
    inputMethodContextV1_.reset();
}
