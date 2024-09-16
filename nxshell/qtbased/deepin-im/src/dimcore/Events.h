// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTS_H
#define EVENTS_H

#include <QString>

#include <stdint.h>

namespace org {
namespace deepin {
namespace dim {

class InputContext;
class ProxyAddon;

enum class EventType {
    InputContextCreated,
    InputContextDestroyed,
    InputContextFocused,
    InputContextUnfocused,
    InputContextKeyEvent,
    InputContextCursorRectChanged,
    InputContextUpdateContentType,
    InputContextUpdateSurroundingText,
    InputContextDone,

    ProxyActiveInputMethodsChanged,
};

class Event
{
public:
    explicit Event(EventType type_);
    ~Event() = default;

    inline EventType type() const { return type_; }

private:
    EventType type_;
};

class InputContextEvent : public Event
{
public:
    explicit InputContextEvent(EventType type_, InputContext *ic_);
    ~InputContextEvent() = default;

    inline InputContext *ic() const { return ic_; }

private:
    InputContext *ic_;
};

class InputContextKeyEvent : public InputContextEvent
{
public:
    InputContextKeyEvent(InputContext *ic,
                         uint32_t keyVal_,
                         uint32_t keycode,
                         uint32_t state,
                         bool isRelease,
                         uint32_t time);
    ~InputContextKeyEvent() = default;

public:
    inline uint32_t keySym() const { return keySym_; }

    inline uint32_t keycode() const { return keycode_; }

    inline uint32_t state() const { return state_; }

    inline bool isRelease() const { return isRelease_; }

    inline uint32_t time() const { return time_; }

private:
    uint32_t keySym_;
    uint32_t keycode_;
    uint32_t state_;
    bool isRelease_;
    uint32_t time_;
};

class InputContextCursorRectChangeEvent : public InputContextEvent
{
public:
    InputContextCursorRectChangeEvent(InputContext *ic, int32_t x, int32_t y, int32_t w, int32_t h);

    const int32_t x;
    const int32_t y;
    const int32_t w;
    const int32_t h;
};

class ProxyEvent : public Event
{
public:
    ProxyEvent(EventType type, ProxyAddon *proxyAddon);
    ~ProxyEvent();

    ProxyAddon *proxyAddon() const { return proxyAddon_; }

private:
    ProxyAddon *proxyAddon_;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !EVENTS_H
