/*
    SPDX-FileCopyrightText: 2001-2003 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2012 David Faure <faure@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "kxmessages.h"
#include "cptr_p.h"
#include "kxutils_p.h"
#include "kxcbevent_p.h"

#if KWINDOWSYSTEM_HAVE_X11

#include <QAbstractNativeEventFilter>
#include <QCoreApplication>
#include <QDebug>
#include <QWindow> // WId

#include <X11/Xlib.h>

#include <private/qtx11extras_p.h>

class XcbAtom
{
public:
    explicit XcbAtom(const QByteArray &name, bool onlyIfExists = false)
        : m_name(name)
        , m_atom(XCB_ATOM_NONE)
        , m_connection(nullptr)
        , m_retrieved(false)
        , m_onlyIfExists(onlyIfExists)
    {
        m_cookie.sequence = 0;
    }
    explicit XcbAtom(xcb_connection_t *c, const QByteArray &name, bool onlyIfExists = false)
        : m_name(name)
        , m_atom(XCB_ATOM_NONE)
        , m_cookie(xcb_intern_atom_unchecked(c, onlyIfExists, name.length(), name.constData()))
        , m_connection(c)
        , m_retrieved(false)
        , m_onlyIfExists(onlyIfExists)
    {
    }

    ~XcbAtom()
    {
        if (!m_retrieved && m_cookie.sequence && m_connection) {
            xcb_discard_reply(m_connection, m_cookie.sequence);
        }
    }

    operator xcb_atom_t()
    {
        getReply();
        return m_atom;
    }

    inline const QByteArray &name() const
    {
        return m_name;
    }

    inline void setConnection(xcb_connection_t *c)
    {
        m_connection = c;
    }

    inline void fetch()
    {
        if (!m_connection || m_name.isEmpty()) {
            return;
        }
        m_cookie = xcb_intern_atom_unchecked(m_connection, m_onlyIfExists, m_name.length(), m_name.constData());
    }

private:
    void getReply()
    {
        if (m_retrieved || !m_cookie.sequence || !m_connection) {
            return;
        }
        UniqueCPointer<xcb_intern_atom_reply_t> reply(xcb_intern_atom_reply(m_connection, m_cookie, nullptr));
        if (reply) {
            m_atom = reply->atom;
        }
        m_retrieved = true;
    }
    QByteArray m_name;
    xcb_atom_t m_atom;
    xcb_intern_atom_cookie_t m_cookie;
    xcb_connection_t *m_connection;
    bool m_retrieved;
    bool m_onlyIfExists;
};

class KXMessagesPrivate : public QAbstractNativeEventFilter
{
public:
    KXMessagesPrivate(KXMessages *parent, const char *acceptBroadcast, xcb_connection_t *c, xcb_window_t root)
        : accept_atom1(acceptBroadcast ? QByteArray(acceptBroadcast) + QByteArrayLiteral("_BEGIN") : QByteArray())
        , accept_atom2(acceptBroadcast ? QByteArray(acceptBroadcast) : QByteArray())
        , handle(new QWindow)
        , q(parent)
        , valid(c)
        , connection(c)
        , rootWindow(root)
    {
        if (acceptBroadcast) {
            accept_atom1.setConnection(c);
            accept_atom1.fetch();
            accept_atom2.setConnection(c);
            accept_atom2.fetch();
            QCoreApplication::instance()->installNativeEventFilter(this);
        }
    }
    XcbAtom accept_atom1;
    XcbAtom accept_atom2;
    QMap<WId, QByteArray> incoming_messages;
    std::unique_ptr<QWindow> handle;
    KXMessages *q;
    bool valid;
    xcb_connection_t *connection;
    xcb_window_t rootWindow;

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override
    {
        // A faster comparison than eventType != "xcb_generic_event_t"
        if (eventType[0] != 'x') {
            return false;
        }
        xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t *>(message);
        uint response_type = event->response_type & ~0x80;
        if (response_type != XCB_CLIENT_MESSAGE) {
            return false;
        }
        xcb_client_message_event_t *cm_event = reinterpret_cast<xcb_client_message_event_t *>(event);
        if (cm_event->format != 8) {
            return false;
        }
        if (cm_event->type != accept_atom1 && cm_event->type != accept_atom2) {
            return false;
        }
        char buf[21]; // can't be longer
        // Copy the data in order to null-terminate it
        qstrncpy(buf, reinterpret_cast<char *>(cm_event->data.data8), 21);
        // qDebug() << cm_event->window << "buf=\"" << buf << "\" atom=" << (cm_event->type == accept_atom1 ? "atom1" : "atom2");
        if (incoming_messages.contains(cm_event->window)) {
            if (cm_event->type == accept_atom1)
            // two different messages on the same window at the same time shouldn't happen anyway
            {
                incoming_messages[cm_event->window] = QByteArray();
            }
            incoming_messages[cm_event->window] += buf;
        } else {
            if (cm_event->type == accept_atom2) {
                return false; // middle of message, but we don't have the beginning
            }
            incoming_messages[cm_event->window] = buf;
        }
        if (strlen(buf) < 20) { // last message fragment
            Q_EMIT q->gotMessage(QString::fromUtf8(incoming_messages[cm_event->window].constData()));
            incoming_messages.remove(cm_event->window);
        }
        return false; // lets other KXMessages instances get the event too
    }
};

static void
send_message_internal(xcb_window_t w, const QString &msg, xcb_connection_t *c, xcb_atom_t leadingMessage, xcb_atom_t followingMessage, xcb_window_t handle);

KXMessages::KXMessages(const char *accept_broadcast_P, QObject *parent_P)
    : QObject(parent_P)
    , d(new KXMessagesPrivate(this,
                              accept_broadcast_P,
                              QX11Info::isPlatformX11() ? QX11Info::connection() : nullptr,
                              QX11Info::isPlatformX11() ? QX11Info::appRootWindow() : 0))
{
}

KXMessages::KXMessages(xcb_connection_t *connection, xcb_window_t rootWindow, const char *accept_broadcast, QObject *parent)
    : QObject(parent)
    , d(new KXMessagesPrivate(this, accept_broadcast, connection, rootWindow))
{
}

KXMessages::~KXMessages()
{
    delete d;
}

static xcb_screen_t *defaultScreen(xcb_connection_t *c, int screen)
{
    for (xcb_screen_iterator_t it = xcb_setup_roots_iterator(xcb_get_setup(c)); it.rem; --screen, xcb_screen_next(&it)) {
        if (screen == 0) {
            return it.data;
        }
    }
    return nullptr;
}

void KXMessages::broadcastMessage(const char *msg_type_P, const QString &message_P, int screen_P)
{
    if (!d->valid) {
        qWarning() << "KXMessages used on non-X11 platform! This is an application bug.";
        return;
    }
    const QByteArray msg(msg_type_P);
    XcbAtom a2(d->connection, msg);
    XcbAtom a1(d->connection, msg + QByteArrayLiteral("_BEGIN"));
    xcb_window_t root = screen_P == -1 ? d->rootWindow : defaultScreen(d->connection, screen_P)->root;
    send_message_internal(root, message_P, d->connection, a1, a2, d->handle->winId());
}

bool KXMessages::broadcastMessageX(xcb_connection_t *c, const char *msg_type_P, const QString &message, int screenNumber)
{
    if (!c) {
        return false;
    }
    const QByteArray msg(msg_type_P);
    XcbAtom a2(c, msg);
    XcbAtom a1(c, msg + QByteArrayLiteral("_BEGIN"));
    const xcb_screen_t *screen = defaultScreen(c, screenNumber);
    if (!screen) {
        return false;
    }
    const xcb_window_t root = screen->root;
    const xcb_window_t win = xcb_generate_id(c);
    xcb_create_window(c, XCB_COPY_FROM_PARENT, win, root, 0, 0, 1, 1, 0, XCB_COPY_FROM_PARENT, XCB_COPY_FROM_PARENT, 0, nullptr);
    send_message_internal(root, message, c, a1, a2, win);
    xcb_destroy_window(c, win);
    return true;
}

static void
send_message_internal(xcb_window_t w, const QString &msg_P, xcb_connection_t *c, xcb_atom_t leadingMessage, xcb_atom_t followingMessage, xcb_window_t handle)
{
    unsigned int pos = 0;
    QByteArray msg = msg_P.toUtf8();
    const size_t len = msg.size();

    KXcbEvent<xcb_client_message_event_t> event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 8;
    event.sequence = 0;
    event.window = handle;
    event.type = leadingMessage;

    do {
        unsigned int i;
        for (i = 0; i < 20 && i + pos < len; ++i) {
            event.data.data8[i] = msg[i + pos];
        }
        for (; i < 20; ++i) {
            event.data.data8[i] = 0;
        }
        xcb_send_event(c, false, w, XCB_EVENT_MASK_PROPERTY_CHANGE, event.buffer());
        event.type = followingMessage;
        pos += i;
    } while (pos <= len);

    xcb_flush(c);
}

#endif

#include "moc_kxmessages.cpp"
