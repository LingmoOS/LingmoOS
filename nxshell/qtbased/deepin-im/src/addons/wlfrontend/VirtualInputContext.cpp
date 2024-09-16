// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "VirtualInputContext.h"

#include "VirtualInputContextGlue.h"

using namespace org::deepin::dim;

VirtualInputContext::VirtualInputContext(VirtualInputContextGlue *parentIC, Dim *dim)
    : InputContext(dim)
    , parentIC_(parentIC)
{
}

VirtualInputContext::~VirtualInputContext() = default;

void VirtualInputContext::setWindowPos(const QPoint &leftTop)
{
    leftTop_ = leftTop;
}

void VirtualInputContext::updatePreeditImpl(const QString &text,
                                            int32_t cursorBegin,
                                            int32_t cursorEnd)
{
    parentIC_->updatePreeditDelegate(this, text, cursorBegin, cursorEnd);
}

void VirtualInputContext::commitStringImpl(const QString &text)
{
    parentIC_->commitStringDelegate(this, text);
}

void VirtualInputContext::commitImpl()
{
    parentIC_->commitDelegate();
}

void VirtualInputContext::forwardKeyImpl(uint32_t keycode, bool pressed)
{
    parentIC_->forwardKeyDelegate(this, keycode, pressed);
}
