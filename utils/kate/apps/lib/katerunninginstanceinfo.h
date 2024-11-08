/*
    SPDX-FileCopyrightText: 2009 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kateprivate_export.h"

#include <QDBusInterface>

#include <memory>
#include <vector>

class KateRunningInstanceInfo
{
public:
    explicit KateRunningInstanceInfo(const QString &serviceName_);
    const QString serviceName;
    std::unique_ptr<QDBusInterface> dbus_if;
    qint64 lastActivationChange = 0;
    QString sessionName;
};

KATE_PRIVATE_EXPORT std::vector<KateRunningInstanceInfo> fillinRunningKateAppInstances();
