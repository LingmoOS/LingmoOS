/****************************************************************************

 Copyright (C) 2001-2003 Lubos Lunak        <l.lunak@kde.org>
 Copyright 2012 David Faure <faure@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#include "kxmessages.h"
#include "kxutils_p.h"

#if KWINDOWSYSTEM_HAVE_X11

#include <qcoreapplication.h>
#include <QDebug>
#include <QWidget> // WId
#include <QAbstractNativeEventFilter>

#include <xcb/xcb.h>
#include <qx11info_x11.h>
#include <X11/Xlib.h>


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

    ~XcbAtom() {
        if (!m_retrieved && m_cookie.sequence && m_connection) {
            xcb_discard_reply(m_connection, m_cookie.sequence);
        }
    }

    operator xcb_atom_t() {
        getReply();
        return m_atom;
    }

    inline const QByteArray &name() const {
        return m_name;
    }

    inline void setConnection(xcb_connection_t *c) {
        m_connection = c;
    }

    inline void fetch() {
        if (!m_connection || m_name.isEmpty()) {
            return;
        }
        m_cookie = xcb_intern_atom_unchecked(m_connection, m_onlyIfExists, m_name.length(), m_name.constData());
    }

private:
    void getReply() {
        if (m_retrieved || !m_cookie.sequence || !m_connection) {
            return;
        }
        KXUtils::ScopedCPointer<xcb_intern_atom_reply_t> reply(xcb_intern_atom_reply(m_connection, m_cookie, nullptr));
        if (!reply.isNull()) {
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

class KXMessagesPrivate
    : public QAbstractNativeEventFilter
{
public:
    KXMessagesPrivate(KXMessages *parent, const char *acceptBroadcast, xcb_connection_t *c, xcb_window_t root)
        : accept_atom1(acceptBroadcast ? QByteArray(acceptBroadcast) + QByteArrayLiteral("_BEGIN") : QByteArray())
        , accept_atom2(acceptBroadcast ? QByteArray(acceptBroadcast) : QByteArray())
        , handle(new QWidget)
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
    QMap< WId, QByteArray > incoming_messages;
    QScopedPointer<QWidget> handle;
    KXMessages *q;
    bool valid;
    xcb_connection_t *connection;
    xcb_window_t rootWindow;

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE
    {
        Q_UNUSED(result);
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
        char buf[ 21 ]; // can't be longer
        // Copy the data in order to null-terminate it
        qstrncpy(buf, reinterpret_cast<char *>(cm_event->data.data8), 21);
        //qDebug() << cm_event->window << "buf=\"" << buf << "\" atom=" << (cm_event->type == accept_atom1 ? "atom1" : "atom2");
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
            emit q->gotMessage(QString::fromUtf8(incoming_messages[cm_event->window].constData()));
            incoming_messages.remove(cm_event->window);
        }
        return false; // lets other KXMessages instances get the event too
    }
};

#ifndef KWINDOWSYSTEM_NO_DEPRECATED
static void send_message_internal(WId w_P, const QString &msg_P, long mask_P,
                                  Display *disp, Atom atom1_P, Atom atom2_P, Window handle_P);
// for broadcasting
static const long BROADCAST_MASK = PropertyChangeMask;
// CHECKME
#endif
static void send_message_internal(xcb_window_t w, const QString &msg, xcb_connection_t *c,
                                  xcb_atom_t leadingMessage, xcb_atom_t followingMessage, xcb_window_t handle);

KXMessages::KXMessages(const char *accept_broadcast_P, QObject *parent_P)
    : QObject(parent_P)
    , d(new KXMessagesPrivate(this, accept_broadcast_P, QX11Info::isPlatformX11() ? QX11Info::connection() : nullptr, QX11Info::isPlatformX11() ? QX11Info::appRootWindow() : 0))
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

static
xcb_screen_t *defaultScreen(xcb_connection_t *c, int screen)
{
    for (xcb_screen_iterator_t it = xcb_setup_roots_iterator(xcb_get_setup(c));
            it.rem;
            --screen, xcb_screen_next(&it)) {
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
    send_message_internal(root, message_P, d->connection,
                          a1, a2, d->handle->winId());
}

#ifndef KWINDOWSYSTEM_NO_DEPRECATED
bool KXMessages::broadcastMessageX(Display *disp, const char *msg_type_P,
                                   const QString &message_P, int screen_P)
{
    if (disp == nullptr) {
        return false;
    }
    Atom a2 = XInternAtom(disp, msg_type_P, false);
    Atom a1 = XInternAtom(disp, QByteArray(QByteArray(msg_type_P) + "_BEGIN").constData(), false);
    Window root = screen_P == -1 ? DefaultRootWindow(disp) : RootWindow(disp, screen_P);
    Window win = XCreateSimpleWindow(disp, root, 0, 0, 1, 1,
                                     0, BlackPixel(disp, screen_P == -1 ? DefaultScreen(disp) : screen_P),
                                     BlackPixel(disp, screen_P == -1 ? DefaultScreen(disp) : screen_P));
    send_message_internal(root, message_P, BROADCAST_MASK, disp,
                          a1, a2, win);
    XDestroyWindow(disp, win);
    return true;
}
#endif

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
    xcb_create_window(c, XCB_COPY_FROM_PARENT, win, root, 0, 0, 1, 1,
                      0, XCB_COPY_FROM_PARENT, XCB_COPY_FROM_PARENT, 0, nullptr);
    send_message_internal(root, message, c, a1, a2, win);
    xcb_destroy_window(c, win);
    return true;
}

#if 0 // currently unused
void KXMessages::sendMessage(WId w_P, const char *msg_type_P, const QString &message_P)
{
    Atom a2 = XInternAtom(QX11Info::display(), msg_type_P, false);
    Atom a1 = XInternAtom(QX11Info::display(), QByteArray(QByteArray(msg_type_P) + "_BEGIN").constData(), false);
    send_message_internal(w_P, message_P, 0, QX11Info::display(), a1, a2, d->handle->winId());
}

bool KXMessages::sendMessageX(Display *disp, WId w_P, const char *msg_type_P,
                              const QString &message_P)
{
    if (disp == NULL) {
        return false;
    }
    Atom a2 = XInternAtom(disp, msg_type_P, false);
    Atom a1 = XInternAtom(disp, QByteArray(QByteArray(msg_type_P) + "_BEGIN").constData(), false);
    Window win = XCreateSimpleWindow(disp, DefaultRootWindow(disp), 0, 0, 1, 1,
                                     0, BlackPixelOfScreen(DefaultScreenOfDisplay(disp)),
                                     BlackPixelOfScreen(DefaultScreenOfDisplay(disp)));
    send_message_internal(w_P, message_P, 0, disp, a1, a2, win);
    XDestroyWindow(disp, win);
    return true;
}
#endif

#ifndef KWINDOWSYSTEM_NO_DEPRECATED
static void send_message_internal(WId w_P, const QString &msg_P, long mask_P,
                                  Display *disp, Atom atom1_P, Atom atom2_P, Window handle_P)
{
    //qDebug() << "send_message_internal" << w_P << msg_P << mask_P << atom1_P << atom2_P << handle_P;
    unsigned int pos = 0;
    QByteArray msg = msg_P.toUtf8();
    unsigned int len = strlen(msg.constData());
    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = atom1_P; // leading message
    e.xclient.display = disp;
    e.xclient.window = handle_P;
    e.xclient.format = 8;
    do {
        unsigned int i;
        for (i = 0;
                i < 20 && i + pos <= len;
                ++i) {
            e.xclient.data.b[ i ] = msg[ i + pos ];
        }
        XSendEvent(disp, w_P, false, mask_P, &e);
        e.xclient.message_type = atom2_P; // following messages
        pos += i;
    } while (pos <= len);
    XFlush(disp);
}
#endif

static void send_message_internal(xcb_window_t w, const QString &msg_P, xcb_connection_t *c,
                                  xcb_atom_t leadingMessage, xcb_atom_t followingMessage, xcb_window_t handle)
{
    unsigned int pos = 0;
    QByteArray msg = msg_P.toUtf8();
    const size_t len = strlen(msg.constData());

    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 8;
    event.sequence = 0;
    event.window = handle;
    event.type = leadingMessage;

    do {
        unsigned int i;
        for (i = 0;
                i < 20 && i + pos <= len;
                ++i) {
            event.data.data8[i] = msg[ i + pos ];
        }
        for (unsigned int j = i; j < 20; ++j) {
            event.data.data8[j] = 0;
        }
        xcb_send_event(c, false, w, XCB_EVENT_MASK_PROPERTY_CHANGE, (const char *) &event);
        event.type = followingMessage;
        pos += i;
    } while (pos <= len);

    xcb_flush(c);
}

#endif
