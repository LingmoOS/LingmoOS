/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MEMORY_H
#define MEMORY_H

#include <systemstats/SensorPlugin.h>

#include <QObject>

#include <memory>

class MemoryBackend;

class MemoryPlugin : public KSysGuard::SensorPlugin
{
    Q_OBJECT
public:
    MemoryPlugin(QObject *parent, const QVariantList &args);
    ~MemoryPlugin();

    QString providerName() const override
    {
        return QStringLiteral("memory");
    }
    void update() override;
private:
    std::unique_ptr<MemoryBackend> m_backend;
};

#endif
