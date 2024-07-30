/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef POWER_H
#define POWER_H

#include <systemstats/SensorPlugin.h>

namespace Solid {
class Device;
}
namespace KSysGuard
{
    class SensorContainer;
}
class Battery;

class PowerPlugin : public KSysGuard::SensorPlugin {
    Q_OBJECT
public:
    PowerPlugin(QObject *parent, const QVariantList &args);
    QString providerName() const override
    {
        return QStringLiteral("power");
    };
private:
    KSysGuard::SensorContainer *m_container;
    QHash<QString, Battery*> m_batteriesByUdi;
};

#endif
