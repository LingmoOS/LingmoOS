/*
    SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "../src/dbusutils_p.h"
#include <QObject>
#include <QVariantMap>

class TestRemoteRunner : public QObject
{
    Q_OBJECT
public:
    explicit TestRemoteRunner(const QString &serviceName, bool showLifecycleMethodCalls);

public Q_SLOTS:
    KRunner::Actions Actions();
    RemoteMatches Match(const QString &searchTerm);
    void Run(const QString &id, const QString &actionId);
    void Teardown();
    QVariantMap Config();

private:
    bool m_showLifecycleMethodCalls = false;
};
