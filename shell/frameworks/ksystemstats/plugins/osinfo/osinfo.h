/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <systemstats/SensorPlugin.h>

class OSInfoPrivate;

class OSInfoPlugin : public KSysGuard::SensorPlugin
{
    Q_OBJECT
public:
    OSInfoPlugin(QObject *parent, const QVariantList &args);
    ~OSInfoPlugin() override;

    QString providerName() const override
    {
        return QStringLiteral("osinfo");
    }

    void update() override;

private:
    std::unique_ptr<OSInfoPrivate> d;
};
