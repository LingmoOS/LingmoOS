// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "VirtualInputContextGlue.h"

#include "VirtualInputContextManager.h"

using namespace org::deepin::dim;

VirtualInputContextGlue::~VirtualInputContextGlue() = default;

VirtualInputContext *VirtualInputContextGlue::delegatedInputContext()
{
    assert(virtualICManager_ != nullptr);

    VirtualInputContext *vic = virtualICManager_->focusedVirtualIC();
    if (vic != nullptr) {
        return vic;
    }

    return this;
}

void VirtualInputContextGlue::focusInWrapper()
{
    if (virtualICManager_) {
        virtualICManager_->setRealFocus(true);
    } else {
        focusIn();
    }
}

void VirtualInputContextGlue::focusOutWrapper()
{
    if (virtualICManager_) {
        virtualICManager_->setRealFocus(false);
    } else {
        focusOut();
    }
}

void VirtualInputContextGlue::updateContentTypeWrapper()
{
    updateContentType();
    if (auto *ic = delegatedInputContext(); ic != this) {
        ic->contentType() = contentType();
        ic->updateContentType();
    }
}

void VirtualInputContextGlue::updateSurroundingTextWrapper()
{
    updateSurroundingText();
    if (auto *ic = delegatedInputContext(); ic != this) {
        ic->surroundingText() = surroundingText();
        ic->updateSurroundingText();
    }
}

void VirtualInputContextGlue::updatePreeditImpl(const QString &text,
                                                int32_t cursorBegin,
                                                int32_t cursorEnd)
{
    updatePreeditDelegate(this, text, cursorBegin, cursorEnd);
}

void VirtualInputContextGlue::commitStringImpl(const QString &text)
{
    commitStringDelegate(this, text);
}

void VirtualInputContextGlue::forwardKeyImpl(uint32_t keycode, bool pressed)
{
    forwardKeyDelegate(this, keycode, pressed);
}
