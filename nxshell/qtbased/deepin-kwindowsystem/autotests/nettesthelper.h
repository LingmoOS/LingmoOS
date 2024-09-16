/*
 *   Copyright 2013 Martin Gräßlin <mgraesslin@kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef NETTESTHELPER_H
#define NETTESTHELPER_H

#include <QByteArray>
#include <QScopedPointer>
#include <xcb/xcb.h>

namespace KXUtils
{

template <typename T>
class ScopedCPointer : public QScopedPointer<T, QScopedPointerPodDeleter>
{
public:
    ScopedCPointer(T *p = nullptr) : QScopedPointer<T, QScopedPointerPodDeleter>(p) {}
};

/**
 * @brief Small helper class to fetch an intern atom through XCB.
 *
 * This class allows to request an intern atom and delay the retrieval of the reply
 * till it is needed. In case the reply is never retrieved the reply gets discarded
 * in the dtor. So there is no need to keep track manually about which atoms have been
 * retrieved.
 *
 * This class can be used as a drop-in replacement for everywhere where a xcb_atom_t is
 * needed as it implements the cast operator. The first time this operator is invoked it
 * will retrieve the reply. If the xcb request failed the value is @c XCB_ATOM_NONE, which
 * can be used to check whether the returned value is valid.
 *
 * This class has two modes of operations: a direct one which performs the request directly
 * during construction, and an indirect one which needs an explicit call to {@link fetch}.
 *
 * @code
 * Atom direct(QX11Info::connection(), QByteArrayLiteral("myAtomName"));
 * Atom indirect(QByteArrayLiteral("myAtomName"));
 * indirect.setConnection(QX11Info::connection());
 * indirect.fetch();
 *
 * if (direct == XCB_ATOM_NONE) {
 *     qWarning() << "Request failed";
 * }
 * if (indirect == XCB_ATOM_NONE) {
 *     qWarning() << "Request failed";
 * }
 * @endcode
 */
class Atom
{
public:
    explicit Atom(const QByteArray &name)
        : m_connection(nullptr)
        , m_retrieved(false)
        , m_atom(XCB_ATOM_NONE)
        , m_name(name)
    {
        m_cookie.sequence = 0;
    }
    explicit Atom(xcb_connection_t *c, const QByteArray &name, bool onlyIfExists = false)
        : m_connection(c)
        , m_retrieved(false)
        , m_cookie(xcb_intern_atom_unchecked(m_connection, onlyIfExists, name.length(), name.constData()))
        , m_atom(XCB_ATOM_NONE)
        , m_name(name)
    {
    }
    Atom() Q_DECL_EQ_DELETE;
    Atom(const Atom &) Q_DECL_EQ_DELETE;

    ~Atom()
    {
        if (!m_retrieved && m_cookie.sequence) {
            xcb_discard_reply(m_connection, m_cookie.sequence);
        }
    }

    void setConnection(xcb_connection_t *c)
    {
        m_connection = c;
    }

    void fetch(bool onlyIfExists = false)
    {
        if (!m_connection) {
            // set connection first!
            return;
        }
        if (m_retrieved || m_cookie.sequence) {
            // already fetched, don't fetch again
            return;
        }
        m_cookie = xcb_intern_atom_unchecked(m_connection, onlyIfExists, m_name.length(), m_name.constData());
    }

    operator xcb_atom_t() const
    {
        (const_cast<Atom *>(this))->getReply();
        return m_atom;
    }

    const QByteArray &name() const
    {
        return m_name;
    }

private:
    void getReply()
    {
        if (m_retrieved || !m_cookie.sequence) {
            return;
        }
        ScopedCPointer<xcb_intern_atom_reply_t> reply(xcb_intern_atom_reply(m_connection, m_cookie, nullptr));
        if (!reply.isNull()) {
            m_atom = reply->atom;
        }
        m_retrieved = true;
    }
    xcb_connection_t *m_connection;
    bool m_retrieved;
    xcb_intern_atom_cookie_t m_cookie;
    xcb_atom_t m_atom;
    QByteArray m_name;
};

inline xcb_window_t rootWindow(xcb_connection_t *c, int screen)
{
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(c));
    for (xcb_screen_iterator_t it = xcb_setup_roots_iterator(xcb_get_setup(c));
            it.rem;
            --screen, xcb_screen_next(&it)) {
        if (screen == 0) {
            return iter.data->root;
        }
    }
    return XCB_WINDOW_NONE;
}

}

#endif
