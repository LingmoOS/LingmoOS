/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>

   SPDX-License-Identifier: LGPL-2.1-or-later
*/

// Exceptionnally, include QCoreApplication before our own header, because that one includes X11 headers (#define None...)
#include <QCoreApplication>

#include "xsync_logging.h"

#include "xsyncbasedpoller.h"

#include <QAbstractNativeEventFilter>
#include <QGuiApplication>

#include <X11/Xlib-xcb.h> // XGetXCBConnection
#include <xcb/sync.h>

class XSyncBasedPollerHelper : public QAbstractNativeEventFilter
{
public:
    XSyncBasedPollerHelper()
        : q(nullptr)
        , isActive(false)
    {
    }
    ~XSyncBasedPollerHelper() override
    {
        delete q;
    }
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        Q_UNUSED(result);
        if (isActive && eventType == "xcb_generic_event_t") {
            q->xcbEvent(reinterpret_cast<xcb_generic_event_t *>(message));
        }
        return false;
    }
    XSyncBasedPoller *q;
    bool isActive;
};

Q_GLOBAL_STATIC(XSyncBasedPollerHelper, s_globalXSyncBasedPoller)

XSyncBasedPoller *XSyncBasedPoller::instance()
{
    if (!s_globalXSyncBasedPoller()->q) {
        new XSyncBasedPoller;
    }

    return s_globalXSyncBasedPoller()->q;
}

XSyncBasedPoller::XSyncBasedPoller(QObject *parent)
    : KAbstractIdleTimePoller(parent)
    , m_display(qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->display())
    , m_xcb_connection(nullptr)
    , m_sync_event(0)
    , m_idleCounter(None)
    , m_resetAlarm(None)
    , m_available(true)
{
    Q_ASSERT(!s_globalXSyncBasedPoller()->q);
    s_globalXSyncBasedPoller()->q = this;

    if (Q_UNLIKELY(!m_display)) {
        m_available = false;
        qCWarning(KIDLETIME_XSYNC_PLUGIN) << "xcb sync could not find display";
        return;
    }
    m_xcb_connection = XGetXCBConnection(m_display);

    QCoreApplication::instance()->installNativeEventFilter(s_globalXSyncBasedPoller());

    const xcb_query_extension_reply_t *sync_reply = xcb_get_extension_data(m_xcb_connection, &xcb_sync_id);
    if (!sync_reply || !sync_reply->present) {
        qCWarning(KIDLETIME_XSYNC_PLUGIN) << "xcb sync extension not found";
        m_available = false;
        return;
    }
    m_sync_event = sync_reply->first_event;

#if 0

    // Workaround for https://bugs.freedesktop.org/show_bug.cgi?id=23403
#define xcb_sync_systemcounter_name(sc) (((char *)&(sc)->name_len) + 2)

    xcb_sync_list_system_counters_cookie_t cookie = xcb_sync_list_system_counters(m_xcb_connection);
    xcb_sync_list_system_counters_reply_t *reply = xcb_sync_list_system_counters_reply(m_xcb_connection, cookie, NULL);

    xcb_sync_systemcounter_iterator_t iter;
    for (iter = xcb_sync_list_system_counters_counters_iterator(reply);
            iter.rem; xcb_sync_systemcounter_next(&iter)) {
        printf("%d: %.*s\n", iter.data->counter,
               iter.data->name_len, xcb_sync_systemcounter_name(iter.data));
        /* Extra info for debugging: */
        printf("  Actual name: %.*s\n", iter.data->name_len,
               ((char *) &iter.data->name_len) + 2);
    }

    int xcbcounters = xcb_sync_list_system_counters_counters_length(reply);
    xcb_sync_systemcounter_iterator_t it = xcb_sync_list_system_counters_counters_iterator(reply);
    for (int i = 0; i < xcbcounters; ++i) {
        qCDebug(KIDLETIME_XSYNC_PLUGIN) << it.data->counter << it.rem << it.index;
        qCDebug(KIDLETIME_XSYNC_PLUGIN) << "name length" << xcb_sync_systemcounter_name_length(it.data);
        QByteArray name(xcb_sync_systemcounter_name(it.data), xcb_sync_systemcounter_name_length(it.data));
        qCDebug(KIDLETIME_XSYNC_PLUGIN) << name;
        xcb_sync_systemcounter_next(&it);
    }
    delete reply;
#endif

    int sync_major;
    int sync_minor;
    int old_sync_event;
    int old_sync_error;
    if (!XSyncQueryExtension(m_display, &old_sync_event, &old_sync_error)) {
        m_available = false;
        return;
    }

    if (!XSyncInitialize(m_display, &sync_major, &sync_minor)) {
        m_available = false;
        return;
    }

    int ncounters;
    XSyncSystemCounter *counters = XSyncListSystemCounters(m_display, &ncounters);

    bool idleFound = false;

    qCDebug(KIDLETIME_XSYNC_PLUGIN) << ncounters << "counters";
    for (int i = 0; i < ncounters; ++i) {
        qCDebug(KIDLETIME_XSYNC_PLUGIN) << counters[i].name << counters[i].counter;
        if (!strcmp(counters[i].name, "IDLETIME")) {
            m_idleCounter = counters[i].counter;
            idleFound = true;
            break;
        }
    }

    XSyncFreeSystemCounterList(counters);

    if (!idleFound) {
        m_available = false;
    }

    if (m_available) {
        qCDebug(KIDLETIME_XSYNC_PLUGIN) << "XSync seems available and ready";
    } else {
        qCDebug(KIDLETIME_XSYNC_PLUGIN) << "XSync seems not available";
    }
}

XSyncBasedPoller::~XSyncBasedPoller()
{
}

bool XSyncBasedPoller::isAvailable()
{
    return m_available;
}

bool XSyncBasedPoller::setUpPoller()
{
    if (!isAvailable()) {
        return false;
    }

    qCDebug(KIDLETIME_XSYNC_PLUGIN) << "XSync Inited";

    s_globalXSyncBasedPoller()->isActive = true;

    qCDebug(KIDLETIME_XSYNC_PLUGIN) << "Supported, init completed";

    return true;
}

void XSyncBasedPoller::unloadPoller()
{
    s_globalXSyncBasedPoller()->isActive = false;
}

void XSyncBasedPoller::addTimeout(int nextTimeout)
{
    /* We need to set the counter to the idle time + the value
     * requested for next timeout
     */

    // If there's already an alarm for the requested timeout, skip
    if (m_timeoutAlarm.contains(nextTimeout)) {
        return;
    }

    XSyncValue timeout;
    XSyncAlarm newalarm = None;

    XSyncIntToValue(&timeout, nextTimeout);

    setAlarm(m_display, &newalarm, m_idleCounter, XSyncPositiveComparison, timeout);

    m_timeoutAlarm.insert(nextTimeout, newalarm);
}

int XSyncBasedPoller::forcePollRequest()
{
    return poll();
}

int XSyncBasedPoller::poll()
{
    XSyncValue idleTime;
    XSyncQueryCounter(m_display, m_idleCounter, &idleTime);

    return XSyncValueLow32(idleTime);
}

void XSyncBasedPoller::removeTimeout(int timeout)
{
    if (m_timeoutAlarm.contains(timeout)) {
        XSyncAlarm a = m_timeoutAlarm[timeout];
        XSyncDestroyAlarm(m_display, a);
        m_timeoutAlarm.remove(timeout);
    }
}

QList<int> XSyncBasedPoller::timeouts() const
{
    return m_timeoutAlarm.keys();
}

void XSyncBasedPoller::stopCatchingIdleEvents()
{
    if (m_resetAlarm != None) {
        XSyncDestroyAlarm(m_display, m_resetAlarm);
        m_resetAlarm = None;
    }
}

void XSyncBasedPoller::catchIdleEvent()
{
    XSyncValue idleTime;

    XSyncQueryCounter(m_display, m_idleCounter, &idleTime);

    /* Set the reset alarm to fire the next time idleCounter < the
     * current counter value. XSyncNegativeComparison means <= so
     * we have to subtract 1 from the counter value
     */

    // NOTE: this must be a int, else compilation might fail
    int overflow;
    XSyncValue add;
    XSyncValue plusone;
    XSyncIntToValue(&add, -1);
    XSyncValueAdd(&plusone, idleTime, add, &overflow);
    setAlarm(m_display, &m_resetAlarm, m_idleCounter, XSyncNegativeComparison, plusone);
}

void XSyncBasedPoller::reloadAlarms()
{
    XSyncValue timeout;

    for (QHash<int, XSyncAlarm>::iterator i = m_timeoutAlarm.begin(); i != m_timeoutAlarm.end(); ++i) {
        XSyncIntToValue(&timeout, i.key());

        setAlarm(m_display, &(i.value()), m_idleCounter, XSyncPositiveComparison, timeout);
    }
}

bool XSyncBasedPoller::xcbEvent(xcb_generic_event_t *event)
{
    // qCDebug(KIDLETIME_XSYNC_PLUGIN) << event->response_type << "waiting for" << m_sync_event+XCB_SYNC_ALARM_NOTIFY;
    if (event->response_type != m_sync_event + XCB_SYNC_ALARM_NOTIFY) {
        return false;
    }

    xcb_sync_alarm_notify_event_t *alarmEvent = reinterpret_cast<xcb_sync_alarm_notify_event_t *>(event);

    if (alarmEvent->state == XCB_SYNC_ALARMSTATE_DESTROYED) {
        return false;
    }

    for (QHash<int, XSyncAlarm>::const_iterator i = m_timeoutAlarm.constBegin(); i != m_timeoutAlarm.constEnd(); ++i) {
        if (alarmEvent->alarm == i.value()) {
            /* Bling! Caught! */
            Q_EMIT timeoutReached(i.key());
            // Update the alarm to fire back if the system gets inactive for the same time
            catchIdleEvent();
            return false;
        }
    }

    if (alarmEvent->alarm == m_resetAlarm) {
        /* Resuming from idle here! */
        stopCatchingIdleEvents();
        reloadAlarms();
        Q_EMIT resumingFromIdle();
    }

    return false;
}

void XSyncBasedPoller::setAlarm(Display *dpy, XSyncAlarm *alarm, XSyncCounter counter, XSyncTestType test, XSyncValue value)
{
    XSyncAlarmAttributes attr;
    XSyncValue delta;
    unsigned int flags;

    XSyncIntToValue(&delta, 0);

    attr.trigger.counter = counter;
    attr.trigger.value_type = XSyncAbsolute;
    attr.trigger.test_type = test;
    attr.trigger.wait_value = value;
    attr.delta = delta;

    flags = XSyncCACounter | XSyncCAValueType | XSyncCATestType | XSyncCAValue | XSyncCADelta;

    if (*alarm) {
        // xcb_sync_change_alarm_checked(m_xcb_connection, alarmId,  ...
        XSyncChangeAlarm(dpy, *alarm, flags, &attr);
    } else {
        *alarm = XSyncCreateAlarm(dpy, flags, &attr);
        qCDebug(KIDLETIME_XSYNC_PLUGIN) << "Created alarm" << *alarm;
    }

    XFlush(m_display);
}

void XSyncBasedPoller::simulateUserActivity()
{
    XResetScreenSaver(m_display);
    XFlush(m_display);
}

#include "moc_xsyncbasedpoller.cpp"
