/****************************************************************************

 Copyright (C) 2003 Lubos Lunak        <l.lunak@kde.org>

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

#include "kselectionowner.h"

#include <config-kwindowsystem.h>
#include "kwindowsystem.h"

#include <QtCore/QBasicTimer>
#include <QDebug>
#include <QGuiApplication>
#include <QTimerEvent>
#include <QAbstractNativeEventFilter>

#include <qx11info_x11.h>
#include <qwidget.h>

static xcb_window_t get_selection_owner(xcb_connection_t *c, xcb_atom_t selection)
{
    xcb_window_t owner = XCB_NONE;
    xcb_get_selection_owner_reply_t *reply = xcb_get_selection_owner_reply(c, xcb_get_selection_owner(c, selection), nullptr);

    if (reply) {
        owner = reply->owner;
        free(reply);
    }

    return owner;
}

static xcb_atom_t intern_atom(xcb_connection_t *c, const char *name)
{
    xcb_atom_t atom = XCB_NONE;
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(c, xcb_intern_atom(c, false, strlen(name), name), nullptr);

    if (reply) {
        atom = reply->atom;
        free(reply);
    }

    return atom;
}

class Q_DECL_HIDDEN KSelectionOwner::Private
    : public QAbstractNativeEventFilter
{
public:
    enum State { Idle, WaitingForTimestamp, WaitingForPreviousOwner };

    Private(KSelectionOwner *owner_P, xcb_atom_t selection_P, xcb_connection_t *c, xcb_window_t root)
        : state(Idle),
          selection(selection_P),
          connection(c),
          root(root),
          window(XCB_NONE),
          prev_owner(XCB_NONE),
          timestamp(XCB_CURRENT_TIME),
          extra1(0),
          extra2(0),
          force_kill(false),
          owner(owner_P)
    {
        QCoreApplication::instance()->installNativeEventFilter(this);
    }

    void claimSucceeded();
    void gotTimestamp();
    void timeout();

    State state;
    const xcb_atom_t selection;
    xcb_connection_t *connection;
    xcb_window_t root;
    xcb_window_t window;
    xcb_window_t prev_owner;
    xcb_timestamp_t timestamp;
    uint32_t extra1, extra2;
    QBasicTimer timer;
    bool force_kill;
    static xcb_atom_t manager_atom;
    static xcb_atom_t xa_multiple;
    static xcb_atom_t xa_targets;
    static xcb_atom_t xa_timestamp;

    static Private *create(KSelectionOwner *owner, xcb_atom_t selection_P, int screen_P);
    static Private *create(KSelectionOwner *owner, const char *selection_P, int screen_P);
    static Private *create(KSelectionOwner *owner, xcb_atom_t selection_P, xcb_connection_t *c, xcb_window_t root);
    static Private *create(KSelectionOwner *owner, const char *selection_P, xcb_connection_t *c, xcb_window_t root);

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE {
        Q_UNUSED(result);
        if (eventType != "xcb_generic_event_t")
        {
            return false;
        }
        return owner->filterEvent(message);
    }

private:
    KSelectionOwner *owner;
};

KSelectionOwner::Private* KSelectionOwner::Private::create(KSelectionOwner *owner, xcb_atom_t selection_P, int screen_P)
{
    if (KWindowSystem::isPlatformX11()) {
        return create(owner, selection_P, QX11Info::connection(), QX11Info::appRootWindow(screen_P));
    }
    qWarning() << "Trying to use KSelectionOwner on a non-X11 platform! This is an application bug.";
    return nullptr;
}

KSelectionOwner::Private *KSelectionOwner::Private::create(KSelectionOwner *owner, xcb_atom_t selection_P, xcb_connection_t *c, xcb_window_t root)
{
    return new Private(owner, selection_P, c, root);
}

KSelectionOwner::Private *KSelectionOwner::Private::create(KSelectionOwner *owner, const char *selection_P, int screen_P)
{
    if (KWindowSystem::isPlatformX11()) {
        return create(owner, selection_P, QX11Info::connection(), QX11Info::appRootWindow(screen_P));
    }
    qWarning() << "Trying to use KSelectionOwner on a non-X11 platform! This is an application bug.";
    return nullptr;
}

KSelectionOwner::Private *KSelectionOwner::Private::create(KSelectionOwner *owner, const char *selection_P, xcb_connection_t *c, xcb_window_t root)
{
    return new Private(owner, intern_atom(c, selection_P), c, root);
}

KSelectionOwner::KSelectionOwner(xcb_atom_t selection_P, int screen_P, QObject *parent_P)
    :   QObject(parent_P),
        d(Private::create(this, selection_P, screen_P))
{
}

KSelectionOwner::KSelectionOwner(const char *selection_P, int screen_P, QObject *parent_P)
    :   QObject(parent_P),
        d(Private::create(this, selection_P, screen_P))
{
}

KSelectionOwner::KSelectionOwner(xcb_atom_t selection, xcb_connection_t *c, xcb_window_t root, QObject *parent)
    : QObject(parent)
    , d(Private::create(this, selection, c, root))
{
}

KSelectionOwner::KSelectionOwner(const char *selection, xcb_connection_t *c, xcb_window_t root, QObject *parent)
    : QObject(parent)
    , d(Private::create(this, selection, c, root))
{
}

KSelectionOwner::~KSelectionOwner()
{
    if (d) {
        release();
        if (d->window != XCB_WINDOW_NONE) {
            xcb_destroy_window(d->connection, d->window); // also makes the selection not owned
        }
        delete d;
    }
}

void KSelectionOwner::Private::claimSucceeded()
{
    state = Idle;

    xcb_client_message_event_t ev;
    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.format = 32;
    ev.window = root;
    ev.type = Private::manager_atom;
    ev.data.data32[0] = timestamp;
    ev.data.data32[1] = selection;
    ev.data.data32[2] = window;
    ev.data.data32[3] = extra1;
    ev.data.data32[4] = extra2;

    xcb_send_event(connection, false, root, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *) &ev);

    // qDebug() << "Claimed selection";

    emit owner->claimedOwnership();
}

void KSelectionOwner::Private::gotTimestamp()
{
    Q_ASSERT(state == WaitingForTimestamp);

    state = Idle;

    xcb_connection_t *c = connection;

    // Set the selection owner and immediately verify that the claim was successful
    xcb_set_selection_owner(c, window, selection, timestamp);
    xcb_window_t new_owner = get_selection_owner(c, selection);

    if (new_owner != window) {
        // qDebug() << "Failed to claim selection : " << new_owner;
        xcb_destroy_window(c, window);
        timestamp = XCB_CURRENT_TIME;
        window = XCB_NONE;

        emit owner->failedToClaimOwnership();
        return;
    }

    if (prev_owner != XCB_NONE && force_kill) {
        // qDebug() << "Waiting for previous owner to disown";
        timer.start(1000, owner);
        state = WaitingForPreviousOwner;

        // Note: We've already selected for structure notify events
        //       on the previous owner window
    } else {
        // If there was no previous owner, we're done
        claimSucceeded();
    }
}

void KSelectionOwner::Private::timeout()
{
    Q_ASSERT(state == WaitingForPreviousOwner);

    state = Idle;

    if (force_kill) {
        // qDebug() << "Killing previous owner";
        xcb_connection_t *c = connection;

        // Ignore any errors from the kill request
        xcb_generic_error_t *err = xcb_request_check(c, xcb_kill_client_checked(c, prev_owner));
        free(err);

        claimSucceeded();
    } else {
        emit owner->failedToClaimOwnership();
    }
}

void KSelectionOwner::claim(bool force_P, bool force_kill_P)
{
    if (!d) {
        return;
    }
    Q_ASSERT(d->state == Private::Idle);

    if (Private::manager_atom == XCB_NONE) {
        getAtoms();
    }

    if (d->timestamp != XCB_CURRENT_TIME) {
        release();
    }

    xcb_connection_t *c = d->connection;
    d->prev_owner = get_selection_owner(c, d->selection);

    if (d->prev_owner != XCB_NONE) {
        if (!force_P) {
            // qDebug() << "Selection already owned, failing";
            emit failedToClaimOwnership();
            return;
        }

        // Select structure notify events so get an event when the previous owner
        // destroys the window
        uint32_t mask = XCB_EVENT_MASK_STRUCTURE_NOTIFY;
        xcb_change_window_attributes(c, d->prev_owner, XCB_CW_EVENT_MASK, &mask);
    }

    uint32_t values[] = { true, XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY };

    d->window = xcb_generate_id(c);
    xcb_create_window(c, XCB_COPY_FROM_PARENT, d->window, d->root, 0, 0, 1, 1, 0,
                      XCB_WINDOW_CLASS_INPUT_ONLY, XCB_COPY_FROM_PARENT,
                      XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK, values);

    // Trigger a property change event so we get a timestamp
    xcb_atom_t tmp = XCB_ATOM_ATOM;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, d->window, XCB_ATOM_ATOM, XCB_ATOM_ATOM, 32, 1, (const void *) &tmp);

    // Now we have to return to the event loop and wait for the property change event
    d->force_kill = force_kill_P;
    d->state = Private::WaitingForTimestamp;
}

// destroy resource first
void KSelectionOwner::release()
{
    if (!d) {
        return;
    }
    if (d->timestamp == XCB_CURRENT_TIME) {
        return;
    }

    xcb_destroy_window(d->connection, d->window); // also makes the selection not owned
    d->window = XCB_NONE;

    // qDebug() << "Releasing selection";

    d->timestamp = XCB_CURRENT_TIME;
}

xcb_window_t KSelectionOwner::ownerWindow() const
{
    if (!d) {
        return XCB_WINDOW_NONE;
    }
    if (d->timestamp == XCB_CURRENT_TIME) {
        return XCB_NONE;
    }

    return d->window;
}

void KSelectionOwner::setData(uint32_t extra1_P, uint32_t extra2_P)
{
    if (!d) {
        return;
    }
    d->extra1 = extra1_P;
    d->extra2 = extra2_P;
}

bool KSelectionOwner::filterEvent(void *ev_P)
{
    if (!d) {
        return false;
    }
    xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t *>(ev_P);
    const uint response_type = event->response_type & ~0x80;

#if 0
    // There's no generic way to get the window for an event in xcb, it depends on the type of event
    // This handleMessage virtual doesn't seem used anyway.
    if (d->timestamp != CurrentTime && ev_P->xany.window == d->window) {
        if (handleMessage(ev_P)) {
            return true;
        }
    }
#endif
    switch (response_type) {
    case XCB_SELECTION_CLEAR: {
        xcb_selection_clear_event_t *ev = reinterpret_cast<xcb_selection_clear_event_t *>(event);
        if (d->timestamp == XCB_CURRENT_TIME || ev->selection != d->selection) {
            return false;
        }

        d->timestamp = XCB_CURRENT_TIME;
//      qDebug() << "Lost selection";

        xcb_window_t window = d->window;
        emit lostOwnership();

        // Unset the event mask before we destroy the window so we don't get a destroy event
        uint32_t event_mask = XCB_NONE;
        xcb_change_window_attributes(d->connection, window, XCB_CW_EVENT_MASK, &event_mask);
        xcb_destroy_window(d->connection, window);
        return true;
    }
    case XCB_DESTROY_NOTIFY: {
        xcb_destroy_notify_event_t *ev = reinterpret_cast<xcb_destroy_notify_event_t *>(event);
        if (ev->window == d->prev_owner) {
            if (d->state == Private::WaitingForPreviousOwner) {
                d->timer.stop();
                d->claimSucceeded();
                return true;
            }
            // It is possible for the previous owner to be destroyed
            // while we're waiting for the timestamp
            d->prev_owner = XCB_NONE;
        }

        if (d->timestamp == XCB_CURRENT_TIME || ev->window != d->window) {
            return false;
        }

        d->timestamp = XCB_CURRENT_TIME;
//      qDebug() << "Lost selection (destroyed)";
        emit lostOwnership();
        return true;
    }
    case XCB_SELECTION_NOTIFY: {
        xcb_selection_notify_event_t *ev = reinterpret_cast<xcb_selection_notify_event_t *>(event);
        if (d->timestamp == XCB_CURRENT_TIME || ev->selection != d->selection) {
            return false;
        }

        // ignore?
        return false;
    }
    case XCB_SELECTION_REQUEST:
        filter_selection_request(event);
        return false;
    case XCB_PROPERTY_NOTIFY: {
        xcb_property_notify_event_t *ev = reinterpret_cast<xcb_property_notify_event_t *>(event);
        if (ev->window == d->window && d->state == Private::WaitingForTimestamp) {
            d->timestamp = ev->time;
            d->gotTimestamp();
            return true;
        }
        return false;
    }
    default:
        return false;
    }
}

void KSelectionOwner::timerEvent(QTimerEvent *event)
{
    if (!d) {
        QObject::timerEvent(event);
        return;
    }
    if (event->timerId() == d->timer.timerId()) {
        d->timer.stop();
        d->timeout();
        return;
    }

    QObject::timerEvent(event);
}

#if 0
bool KSelectionOwner::handleMessage(XEvent *)
{
    return false;
}
#endif

void KSelectionOwner::filter_selection_request(void *event)
{
    if (!d) {
        return;
    }
    xcb_selection_request_event_t *ev = reinterpret_cast<xcb_selection_request_event_t *>(event);

    if (d->timestamp == XCB_CURRENT_TIME || ev->selection != d->selection) {
        return;
    }

    if (ev->time != XCB_CURRENT_TIME && ev->time - d->timestamp > 1U << 31) {
        return;    // too old or too new request
    }

    // qDebug() << "Got selection request";

    xcb_connection_t *c = d->connection;
    bool handled = false;

    if (ev->target == Private::xa_multiple) {
        if (ev->property != XCB_NONE) {
            const int MAX_ATOMS = 100;

            xcb_get_property_cookie_t cookie = xcb_get_property(c, false, ev->requestor, ev->property,
                                               XCB_GET_PROPERTY_TYPE_ANY, 0, MAX_ATOMS);
            xcb_get_property_reply_t *reply = xcb_get_property_reply(c, cookie, nullptr);

            if (reply && reply->format == 32 && reply->value_len % 2 == 0) {
                xcb_atom_t *atoms = reinterpret_cast<xcb_atom_t *>(xcb_get_property_value(reply));
                bool handled_array[MAX_ATOMS];

                for (uint i = 0; i < reply->value_len / 2; i++) {
                    handled_array[i] = handle_selection(atoms[i * 2], atoms[i * 2 + 1], ev->requestor);
                }

                bool all_handled = true;
                for (uint i = 0; i < reply->value_len / 2; i++) {
                    if (!handled_array[i]) {
                        all_handled = false;
                        atoms[i * 2 + 1] = XCB_NONE;
                    }
                }

                if (!all_handled) {
                    xcb_change_property(c, ev->requestor, ev->property, XCB_ATOM_ATOM, 32, XCB_PROP_MODE_REPLACE,
                                        reply->value_len, reinterpret_cast<const void *>(atoms));
                }

                handled = true;
            }

            if (reply) {
                free(reply);
            }
        }
    } else {
        if (ev->property == XCB_NONE) { // obsolete client
            ev->property = ev->target;
        }

        handled = handle_selection(ev->target, ev->property, ev->requestor);
    }

    xcb_selection_notify_event_t xev;
    xev.response_type = XCB_SELECTION_NOTIFY;
    xev.selection = ev->selection;
    xev.requestor = ev->requestor;
    xev.target = ev->target;
    xev.property = handled ? ev->property : XCB_NONE;

    xcb_send_event(c, false, ev->requestor, 0, (const char *) &xev);
}

bool KSelectionOwner::handle_selection(xcb_atom_t target_P, xcb_atom_t property_P, xcb_window_t requestor_P)
{
    if (!d) {
        return false;
    }
    if (target_P == Private::xa_timestamp) {
        // qDebug() << "Handling timestamp request";
        xcb_change_property(d->connection, requestor_P, property_P, XCB_ATOM_INTEGER, 32,
                            XCB_PROP_MODE_REPLACE, 1, reinterpret_cast<const void *>(&d->timestamp));
    } else if (target_P == Private::xa_targets) {
        replyTargets(property_P, requestor_P);
    } else if (genericReply(target_P, property_P, requestor_P)) {
        // handled
    } else {
        return false; // unknown
    }

    return true;
}

void KSelectionOwner::replyTargets(xcb_atom_t property_P, xcb_window_t requestor_P)
{
    if (!d) {
        return;
    }
    xcb_atom_t atoms[3] = { Private::xa_multiple, Private::xa_timestamp, Private::xa_targets };

    xcb_change_property(d->connection, requestor_P, property_P, XCB_ATOM_ATOM, 32, XCB_PROP_MODE_REPLACE,
                        sizeof(atoms) / sizeof(atoms[0]), reinterpret_cast<const void *>(atoms));

    // qDebug() << "Handling targets request";
}

bool KSelectionOwner::genericReply(xcb_atom_t, xcb_atom_t, xcb_window_t)
{
    return false;
}

void KSelectionOwner::getAtoms()
{
    if (!d) {
        return;
    }
    if (Private::manager_atom != XCB_NONE) {
        return;
    }

    xcb_connection_t *c = d->connection;

    struct {
        const char *name;
        xcb_atom_t *atom;
    } atoms[] = {
        { "MANAGER",   &Private::manager_atom },
        { "MULTIPLE",  &Private::xa_multiple  },
        { "TARGETS",   &Private::xa_targets   },
        { "TIMESTAMP", &Private::xa_timestamp }
    };

    const int count = sizeof(atoms) / sizeof(atoms[0]);
    xcb_intern_atom_cookie_t cookies[count];

    for (int i = 0; i < count; i++) {
        cookies[i] = xcb_intern_atom(c, false, strlen(atoms[i].name), atoms[i].name);
    }

    for (int i = 0; i < count; i++) {
        if (xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(c, cookies[i], nullptr)) {
            *atoms[i].atom = reply->atom;
            free(reply);
        }
    }
}

xcb_atom_t KSelectionOwner::Private::manager_atom = XCB_NONE;
xcb_atom_t KSelectionOwner::Private::xa_multiple  = XCB_NONE;
xcb_atom_t KSelectionOwner::Private::xa_targets   = XCB_NONE;
xcb_atom_t KSelectionOwner::Private::xa_timestamp = XCB_NONE;

