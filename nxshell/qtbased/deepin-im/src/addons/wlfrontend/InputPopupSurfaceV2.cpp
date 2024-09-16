// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "InputPopupSurfaceV2.h"

#include <QDebug>

using namespace org::deepin::dim;

InputPopupSurfaceV2QObj::InputPopupSurfaceV2QObj() = default;
InputPopupSurfaceV2QObj::~InputPopupSurfaceV2QObj() = default;

InputPopupSurfaceV2::InputPopupSurfaceV2(zwp_input_popup_surface_v2 *val)
    : wl::client::ZwpInputPopupSurfaceV2(val)
    , qObject_(std::make_unique<InputPopupSurfaceV2QObj>())
{
}

InputPopupSurfaceV2::~InputPopupSurfaceV2() = default;

void InputPopupSurfaceV2::zwp_input_popup_surface_v2_text_input_rectangle(int32_t x,
                                                                          int32_t y,
                                                                          int32_t width,
                                                                          int32_t height)
{
    qWarning() << "zwp_input_popup_surface_v2_text_input_rectangle";
    emit qObject_->textInputRectangle(x, y, width, height);
}
