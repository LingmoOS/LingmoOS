// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Events.h"

using namespace org::deepin::dim;

Event::Event(EventType type)
    : type_(type)
{
}

InputContextEvent::InputContextEvent(EventType type, InputContext *ic)
    : Event(type)
    , ic_(ic)
{
}

InputContextKeyEvent::InputContextKeyEvent(InputContext *ic,
                                           uint32_t keyVal,
                                           uint32_t keycode,
                                           uint32_t state,
                                           bool isRelease,
                                           uint32_t time)
    : InputContextEvent(EventType::InputContextKeyEvent, ic)
    , keySym_(keyVal)
    , keycode_(keycode)
    , state_(state)
    , isRelease_(isRelease)
    , time_(time)
{
}

InputContextCursorRectChangeEvent::InputContextCursorRectChangeEvent(
    InputContext *ic, int32_t x, int32_t y, int32_t w, int32_t h)
    : InputContextEvent(EventType::InputContextCursorRectChanged, ic)
    , x(x)
    , y(y)
    , w(w)
    , h(h)
{
}

ProxyEvent::ProxyEvent(EventType type, ProxyAddon *proxyAddon)
    : Event(type)
    , proxyAddon_(proxyAddon)
{
}

ProxyEvent::~ProxyEvent() = default;
