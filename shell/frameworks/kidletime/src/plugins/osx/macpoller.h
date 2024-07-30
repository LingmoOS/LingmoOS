/* This file is part of the KDE libraries
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MACPOLLER_H
#define MACPOLLER_H

#include "kabstractidletimepoller_p.h"

#include <Carbon/Carbon.h>

class MacPoller : public KAbstractIdleTimePoller
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID KAbstractIdleTimePoller_iid FILE "osx.json")
    Q_INTERFACES(KAbstractIdleTimePoller)

public:
    MacPoller(QObject *parent = 0);
    virtual ~MacPoller();

    bool isAvailable();
    bool setUpPoller();
    void unloadPoller();

    static pascal void IdleTimerAction(EventLoopTimerRef, EventLoopIdleTimerMessage inState, void *inUserData);

public Q_SLOTS:
    void addTimeout(int nextTimeout);
    void removeTimeout(int nextTimeout);
    QList<int> timeouts() const;
    int forcePollRequest();
    void catchIdleEvent();
    void stopCatchingIdleEvents();
    void simulateUserActivity();
    void triggerResume();

private Q_SLOTS:
    int poll();

private:
    QList<int> m_timeouts;
    EventLoopTimerRef m_timerRef;
    int m_secondsIdle;
    bool m_catch;
};

#endif /* MACPOLLER_H */
