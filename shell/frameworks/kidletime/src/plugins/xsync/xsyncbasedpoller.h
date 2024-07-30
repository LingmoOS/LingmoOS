/* This file is part of the KDE libraries
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef XSYNCBASEDPOLLER_H
#define XSYNCBASEDPOLLER_H

#include "kabstractidletimepoller_p.h"
#include <QHash>

#include <config-kidletime.h>

#include <X11/Xlib.h>
#include <X11/extensions/sync.h>
#include <xcb/xcb.h>

#include "fixx11h.h"

class XSyncBasedPoller : public KAbstractIdleTimePoller
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID KAbstractIdleTimePoller_iid FILE "xcb.json")
    Q_INTERFACES(KAbstractIdleTimePoller)

public:
    static XSyncBasedPoller *instance();

    explicit XSyncBasedPoller(QObject *parent = nullptr);
    ~XSyncBasedPoller() override;

    bool isAvailable() override;
    bool setUpPoller() override;
    void unloadPoller() override;

    bool xcbEvent(xcb_generic_event_t *event);

    QList<int> timeouts() const override;

public Q_SLOTS:
    void addTimeout(int nextTimeout) override;
    void removeTimeout(int nextTimeout) override;

    int forcePollRequest() override;
    void catchIdleEvent() override;
    void stopCatchingIdleEvents() override;
    void simulateUserActivity() override;

private Q_SLOTS:
    int poll();
    void reloadAlarms();

private:
    void setAlarm(Display *dpy, XSyncAlarm *alarm, XSyncCounter counter, XSyncTestType test, XSyncValue value);

private:
    Display *m_display;
    xcb_connection_t *m_xcb_connection;

    int m_sync_event;
    XSyncCounter m_idleCounter;
    QHash<int, XSyncAlarm> m_timeoutAlarm;
    XSyncAlarm m_resetAlarm;
    bool m_available;
};

#endif /* XSYNCBASEDPOLLER_H */
