/*
    SPDX-FileCopyrightText: 2021 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

#include "kabstractidletimepoller_p.h"

#include <QHash>
#include <memory>

class IdleManagerKwin;
class IdleManagerExt;

class IdleTimeout;

class Poller : public KAbstractIdleTimePoller
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID KAbstractIdleTimePoller_iid FILE "wayland.json")
    Q_INTERFACES(KAbstractIdleTimePoller)

public:
    explicit Poller(QObject *parent = nullptr);
    ~Poller() override;

    bool isAvailable() override;
    bool setUpPoller() override {
        return true;
    }
    void unloadPoller() override {}

public Q_SLOTS:
    void addTimeout(int nextTimeout) override;
    void removeTimeout(int nextTimeout) override;
    QList<int> timeouts() const override;
    int forcePollRequest() override;
    void catchIdleEvent() override;
    void stopCatchingIdleEvents() override;
    void simulateUserActivity() override;

private:
    bool initWayland();
    IdleTimeout* createTimeout(int timeout);

    QScopedPointer<IdleManagerKwin> m_idleManagerKwin;
    QScopedPointer<IdleManagerExt> m_idleManagerExt;
    QHash<int, QSharedPointer<IdleTimeout>> m_timeouts;
    QScopedPointer<IdleTimeout> m_catchResumeTimeout;

};
