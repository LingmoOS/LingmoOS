/* This file is part of the KDE libraries
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KABSTRACTIDLETIMEPOLLER_P_H
#define KABSTRACTIDLETIMEPOLLER_P_H

#include <kidletime_export.h>

#include <QObject>

#define KAbstractIdleTimePoller_iid "org.kde.kidletime.KAbstractIdleTimePoller"

class KIDLETIME_EXPORT KAbstractIdleTimePoller : public QObject
{
    Q_OBJECT

public:
    KAbstractIdleTimePoller(QObject *parent = nullptr);
    ~KAbstractIdleTimePoller() override;

    virtual bool isAvailable() = 0;
    virtual bool setUpPoller() = 0;
    virtual void unloadPoller() = 0;

public Q_SLOTS:
    virtual void addTimeout(int nextTimeout) = 0;
    virtual void removeTimeout(int nextTimeout) = 0;

    // TODO KF6: Make it a public method
    virtual QList<int> timeouts() const = 0; // clazy:exclude=const-signal-or-slot

    virtual int forcePollRequest() = 0;
    virtual void catchIdleEvent() = 0;
    virtual void stopCatchingIdleEvents() = 0;
    virtual void simulateUserActivity() = 0;

Q_SIGNALS:
    void resumingFromIdle();
    void timeoutReached(int msec);
};

Q_DECLARE_INTERFACE(KAbstractIdleTimePoller, KAbstractIdleTimePoller_iid)

#endif /* KABSTRACTIDLETIMEPOLLER_P_H */
