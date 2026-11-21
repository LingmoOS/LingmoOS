// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <QList>
#include <QString>
#include <QStringList>

#include "base/signal.h"
#include "base/timer.h"
#include "common/environment.h"

// Owns every application start of the session: nothing else in the desktop
// spawns applications, so the runtime is the single place that knows which
// application a process belongs to and can therefore quit it again.
//
// startProcess() is the only place that actually creates a process.
class AppRuntime
{
public:
    AppRuntime();

    // What every launched application gets, apart from its activation token.
    static Lingmo::Core::Environment applicationEnvironment();

    uint32_t launchApplication(const QString &appId, const QStringList &arguments,
                               const QString &activationToken = {});
    uint32_t launchCommand(const QString &appId, const QStringList &command,
                           const QString &workingDirectory, const QString &activationToken = {});
    bool quitApplication(const QString &appId);
    bool quitAll();
    bool quitByPid(uint32_t pid);
    bool forceQuitByPid(uint32_t pid);
    bool isRunning(const QString &appId) const;
    QStringList runningApplications() const;
    QList<uint32_t> pidsForApplication(const QString &appId) const;

    Lingmo::Base::Signal<const QString &, uint32_t> applicationLaunched;
    Lingmo::Base::Signal<const QString &, uint32_t> applicationQuit;

private:
    struct Instance {
        QString appId;
        uint64_t startTime = 0;
    };

    uint32_t startProcess(const QString &appId, const QStringList &command,
                          const QString &workingDirectory, const QString &activationToken);
    bool terminate(uint32_t pid);
    void reap();

    static bool isAlive(uint32_t pid);
    static bool isSafeTarget(uint32_t pid);
    static bool isOwnedByUser(uint32_t pid);
    static uint32_t parentPid(uint32_t pid);
    static uint64_t startTime(uint32_t pid);

    std::map<uint32_t, Instance> m_instances;
    Lingmo::Base::Timer m_reaper;
};
