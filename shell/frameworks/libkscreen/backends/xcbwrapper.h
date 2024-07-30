/*
    K Win - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2012, 2013 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <functional>
#include <type_traits>

#include <QScopedPointer>
#include <QtGui/private/qtx11extras_p.h>

#include <xcb/randr.h>
#include <xcb/xcb.h>

namespace XCB
{
template<typename T> using ScopedPointer = QScopedPointer<T, QScopedPointerPodDeleter>;

xcb_connection_t *connection();
void closeConnection();
xcb_screen_t *screenOfDisplay(xcb_connection_t *c, int screen);

struct GrabServer {
    GrabServer();
    ~GrabServer();
};

template<typename Reply, typename Cookie, typename ReplyFunc, ReplyFunc replyFunc, typename RequestFunc, RequestFunc requestFunc, typename... RequestFuncArgs>
class Wrapper
{
public:
    Wrapper()
        : m_retrieved(false)
        , m_window(XCB_WINDOW_NONE)
        , m_reply(nullptr)
    {
        m_cookie.sequence = 0;
    }

    explicit Wrapper(const RequestFuncArgs &... args)
        : m_retrieved(false)
        , m_cookie(requestFunc(connection(), args...))
        , m_window(requestWindow<RequestFuncArgs...>(args...))
        , m_reply(nullptr)
    {
    }

    explicit Wrapper(const Wrapper &other)
        : m_retrieved(other.m_retrieved)
        , m_cookie(other.m_cookie)
        , m_window(other.m_window)
        , m_reply(nullptr)
    {
        takeFromOther(const_cast<Wrapper &>(other));
    }

    virtual ~Wrapper()
    {
        cleanup();
    }

    inline Wrapper &operator=(const Wrapper &other)
    {
        if (this != &other) {
            // if we had managed a reply, free it
            cleanup();
            // copy members
            m_retrieved = other.m_retrieved;
            m_cookie = other.m_cookie;
            m_window = other.m_window;
            m_reply = other.m_reply;
            // take over the responsibility for the reply pointer
            takeFromOther(const_cast<Wrapper &>(other));
        }
        return *this;
    }

    inline operator const Reply *() const
    {
        getReply();
        return m_reply;
    }

    inline const Reply *operator->() const
    {
        getReply();
        return m_reply;
    }

    inline bool isNull() const
    {
        getReply();
        return m_reply == nullptr;
    }

    inline operator bool() const
    {
        return !isNull();
    }

    inline const Reply *data() const
    {
        getReply();
        return m_reply;
    }

    inline xcb_window_t window() const
    {
        return m_window;
    }

    inline bool isRetrieved() const
    {
        return m_retrieved;
    }
    /**
     * Returns the value of the reply pointer referenced by this object. The reply pointer of
     * this object will be reset to null. Calling any method which requires the reply to be valid
     * will crash.
     *
     * Callers of this function take ownership of the pointer.
     **/
    inline Reply *take()
    {
        getReply();
        Reply *ret = m_reply;
        m_reply = nullptr;
        m_window = XCB_WINDOW_NONE;
        return ret;
    }

protected:
    void getReply() const
    {
        if (m_retrieved || !m_cookie.sequence) {
            return;
        }
        m_reply = replyFunc(connection(), m_cookie, nullptr);
        m_retrieved = true;
    }

private:
    inline void cleanup()
    {
        if (!m_retrieved && m_cookie.sequence) {
            xcb_discard_reply(connection(), m_cookie.sequence);
        } else if (m_reply) {
            free(m_reply);
        }
    }

    inline void takeFromOther(Wrapper &other)
    {
        if (m_retrieved) {
            m_reply = other.take();
        } else {
            // ensure that other object doesn't try to get the reply or discards it in the dtor
            other.m_retrieved = true;
            other.m_window = XCB_WINDOW_NONE;
        }
    }

    template<typename... Args> constexpr xcb_window_t requestWindow(const Args &... args) const
    {
        return std::is_same<typename std::tuple_element<0, std::tuple<Args...>>::type, xcb_window_t>::value ? std::get<0>(std::tuple<Args...>(args...))
                                                                                                            : static_cast<xcb_window_t>(XCB_WINDOW_NONE);
    }

    mutable bool m_retrieved;
    Cookie m_cookie;
    xcb_window_t m_window;
    mutable Reply *m_reply;
};

#define XCB_DECLARE_TYPE(name, xcb_request, ...)                                                                                                               \
    typedef Wrapper<xcb_request##_reply_t,                                                                                                                     \
                    xcb_request##_cookie_t,                                                                                                                    \
                    decltype(&xcb_request##_reply),                                                                                                            \
                    xcb_request##_reply,                                                                                                                       \
                    decltype(&xcb_request),                                                                                                                    \
                    xcb_request,                                                                                                                               \
                    ##__VA_ARGS__>                                                                                                                             \
        name

XCB_DECLARE_TYPE(ScreenInfo, xcb_randr_get_screen_info, xcb_window_t);

XCB_DECLARE_TYPE(ScreenSize, xcb_randr_get_screen_size_range, xcb_window_t);

XCB_DECLARE_TYPE(PrimaryOutput, xcb_randr_get_output_primary, xcb_window_t);

XCB_DECLARE_TYPE(InternAtom, xcb_intern_atom, uint8_t, uint16_t, const char *);

XCB_DECLARE_TYPE(OutputInfo, xcb_randr_get_output_info, xcb_randr_output_t, xcb_timestamp_t);

XCB_DECLARE_TYPE(CRTCInfo, xcb_randr_get_crtc_info, xcb_randr_crtc_t, xcb_timestamp_t);

XCB_DECLARE_TYPE(AtomName, xcb_get_atom_name, xcb_atom_t);

}
