/* This file is part of the KDE libraries
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KWINDOWBASEDIDLETIMEPOLLER_P_H
#define KWINDOWBASEDIDLETIMEPOLLER_P_H

#include <kidletime_export.h>

#include "kabstractidletimepoller_p.h"

class QTimer;
class QEvent;
class QWindow;

class KIDLETIME_EXPORT KWindowBasedIdleTimePoller : public KAbstractIdleTimePoller
{
    Q_OBJECT

public:
    explicit KWindowBasedIdleTimePoller(QObject *parent = nullptr);
    ~KWindowBasedIdleTimePoller() override;

    bool isAvailable() override;
    bool setUpPoller() override;
    void unloadPoller() override;

    QList<int> timeouts() const override;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

public Q_SLOTS:
    void addTimeout(int nextTimeout) override;
    void removeTimeout(int nextTimeout) override;

    int forcePollRequest() override;
    void catchIdleEvent() override;
    void stopCatchingIdleEvents() override;

private Q_SLOTS:
    int poll();
    virtual int getIdleTime() = 0;
    void detectedActivity();
    void waitForActivity();
    void releaseInputLock();

private:
    virtual bool additionalSetUp() = 0;

private:
    QTimer *m_pollTimer;
    QWindow *m_grabber;
    QList<int> m_timeouts;
};

#endif /* KWINDOWBASEDIDLETIMEPOLLER_P_H */
