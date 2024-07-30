/*
    SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KRunner/AbstractRunner>

#include "dbusutils_p.h"
#include <QHash>
#include <QImage>
#include <QList>
#include <QSet>

namespace KRunner
{
class DBusRunner : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    explicit DBusRunner(QObject *parent, const KPluginMetaData &data);

    // matchInternal is overwritten. Meaning we do not need the original match
    void match(KRunner::RunnerContext &) override
    {
    }
    void reloadConfiguration() override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;

    Q_INVOKABLE void matchInternal(KRunner::RunnerContext context);

private:
    void teardown();

    // Returns RemoteActions with service name as key
    void requestActions();
    void requestActionsForService(const QString &service, const std::function<void()> &finishedCallback);
    QList<QueryMatch> convertMatches(const QString &service, const RemoteMatches &remoteMatches);
    void requestConfig();
    static QImage decodeImage(const RemoteImage &remoteImage);
    QSet<QString> m_matchingServices;
    QHash<QString, QList<KRunner::Action>> m_actions;
    const QString m_path;
    const bool m_hasUniqueResults;
    const bool m_requestActionsOnce;
    bool m_actionsForSessionRequested = false;
    bool m_matchWasCalled = false;
    bool m_callLifecycleMethods = false;
    const QString m_ifaceName;
    QSet<QString> m_requestedActionServices;
};
}
