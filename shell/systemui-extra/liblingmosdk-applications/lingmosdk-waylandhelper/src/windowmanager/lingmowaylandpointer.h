/*
 * liblingmosdk-waylandhelper's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */
#ifndef LINGMOWAYLANDPOINTER_H
#define LINGMOWAYLANDPOINTER_H

#include <QtGlobal>
struct wl_proxy;
template<typename Pointer, void (*deleter)(Pointer *)>
class LingmoUIWaylandPointer
{
public:
    LingmoUIWaylandPointer() = default;
    LingmoUIWaylandPointer(Pointer *p)
        : m_pointer(p)
    {
    }
    LingmoUIWaylandPointer(const LingmoUIWaylandPointer &other) = delete;
    virtual ~LingmoUIWaylandPointer()
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

#endif // LINGMOWAYLANDPOINTER_H
