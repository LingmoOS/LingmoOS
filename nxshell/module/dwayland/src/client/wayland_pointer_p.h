/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_POINTER_P_H
#define WAYLAND_POINTER_P_H

struct wl_proxy;

namespace KWayland
{
namespace Client
{
template<typename Pointer, void (*deleter)(Pointer *)>
class WaylandPointer
{
public:
    WaylandPointer() = default;
    WaylandPointer(Pointer *p)
        : m_pointer(p)
    {
    }
    WaylandPointer(const WaylandPointer &other) = delete;
    virtual ~WaylandPointer()
    {
        release();
    }

    void setup(Pointer *pointer, bool foreign = false)
    {
        Q_ASSERT(pointer);
        Q_ASSERT(!m_pointer);
        m_pointer = pointer;
        m_foreign = foreign;
    }

    void release()
    {
        if (!m_pointer) {
            return;
        }
        if (!m_foreign) {
            deleter(m_pointer);
        }
        m_pointer = nullptr;
    }

    void destroy()
    {
        if (!m_pointer) {
            return;
        }
        if (!m_foreign) {
            free(m_pointer);
        }
        m_pointer = nullptr;
    }

    bool isValid() const
    {
        return m_pointer != nullptr;
    }

    operator Pointer *()
    {
        return m_pointer;
    }

    operator Pointer *() const
    {
        return m_pointer;
    }

    operator wl_proxy *()
    {
        return reinterpret_cast<wl_proxy *>(m_pointer);
    }

    Pointer *operator->()
    {
        return m_pointer;
    }

    operator bool()
    {
        return isValid();
    }

    operator bool() const
    {
        return isValid();
    }

private:
    Pointer *m_pointer = nullptr;
    bool m_foreign = false;
};

}
}

#endif
