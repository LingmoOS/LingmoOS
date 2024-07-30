/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_FD_POWER_NOTIFIER_H
#define SOLID_FD_POWER_NOTIFIER_H

#include "backends/powernotifier.h"
#include <QStringList>

namespace Solid
{
class FDPowerNotifier : public PowerNotifier
{
    Q_OBJECT
public:
    explicit FDPowerNotifier(QObject *parent = nullptr);

private Q_SLOTS:
    void upowerPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidated);
    void login1Resuming(bool active);
};
}

#endif // SOLID_FD_POWER_NOTIFIER_H
