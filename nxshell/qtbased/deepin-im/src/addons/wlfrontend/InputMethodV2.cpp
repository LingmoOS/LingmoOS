// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InputMethodV2.h"

#include <QDebug>

using namespace org::deepin::dim;

InputMethodV2::InputMethodV2(zwp_input_method_v2 *val, Dim *dim)
    : wl::client::ZwpInputMethodV2(val)
    , qobject_(std::make_unique<InputMethodV2QObject>())
{
}

InputMethodV2::~InputMethodV2() = default;

void InputMethodV2::zwp_input_method_v2_activate()
{
    emit qobject_->activate();
}

void InputMethodV2::zwp_input_method_v2_deactivate()
{
    emit qobject_->deactivate();
}

void InputMethodV2::zwp_input_method_v2_surrounding_text(const char *text,
                                                         uint32_t cursor,
                                                         uint32_t anchor)
{
    emit qobject_->surroundingText(text, cursor, anchor);
}

void InputMethodV2::zwp_input_method_v2_text_change_cause(uint32_t cause)
{
    emit qobject_->textChangeCause(cause);
}

void InputMethodV2::zwp_input_method_v2_content_type(uint32_t hint, uint32_t purpose)
{
    emit qobject_->contentType(hint, purpose);
}

void InputMethodV2::zwp_input_method_v2_done()
{
    emit qobject_->done();
}

void InputMethodV2::zwp_input_method_v2_unavailable()
{
    emit qobject_->unavailable();
}
