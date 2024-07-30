/*
    SPDX-FileCopyrightText: 2024 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <string.h>

/**
 * KXcbEvent allocates an xcb event in a 32 bytes large, zero-initialized buffer to avoid
 * out-of-bounds reads and uninitialized memory reads in xcb_send_event(). According to
 * XCB documentation, the wire size of all XCB events is 32 bytes, and that many bytes will
 * be read by xcb_send_event().
 */
template<class EventType, bool needsPadding = (sizeof(EventType) < 32)>
struct KXcbEvent;

template<class EventType>
class KXcbEvent<EventType, false> : public EventType
{
public:
    inline KXcbEvent()
    {
        static_assert(sizeof(*this) == s_evtSize);
        memset(this, 0, s_evtSize);
    }

    inline const char *buffer() const
    {
        return reinterpret_cast<const char *>(this);
    }

private:
    static constexpr size_t s_evtSize = 32;
};

template<class EventType>
class KXcbEvent<EventType, true> : public EventType
{
public:
    inline KXcbEvent()
    {
        static_assert(sizeof(*this) == s_evtSize);
        memset(this, 0, s_evtSize);
    }

    inline const char *buffer() const
    {
        return reinterpret_cast<const char *>(this);
    }

private:
    static constexpr size_t s_evtSize = 32;
    char m_filler[s_evtSize - sizeof(EventType)];
};
