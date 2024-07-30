// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Lucas Biaggi <lbjanuario@gmail.com>
/*
 * Firewalld backend for plasma firewall
 */
#ifndef SYSTEMDJOB_H
#define SYSTEMDJOB_H

// #include "kcm_firewall_core_export.h"
#include <KJob>
#include <QDBusMessage>
#include <QLoggingCategory>
#include <types.h>

Q_DECLARE_LOGGING_CATEGORY(SystemDJobDebug)

namespace SYSTEMD
{
// KCM_FIREWALL_CORE_EXPORT Q_NAMESPACE
enum actions { ERROR = -1, STOP, START };
}

class KCM_FIREWALL_CORE_EXPORT SystemdJob : public KJob
{
    Q_OBJECT

public:
    SystemdJob(const SYSTEMD::actions &action, const QString &service, bool serviceOnly = false);
    ~SystemdJob();
    void start() override;

private:
    void systemdAction(const SYSTEMD::actions value);
    void systemdUnit(const QVariantList values, SYSTEMD::actions action);
    void reloadSystemd();
    const SYSTEMD::actions m_action;
    const QString m_service;
    const bool m_serviceOnly;
};

#endif
